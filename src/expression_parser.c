/*
    ZEsarUX  ZX Second-Emulator And Released for UniX
    Copyright (C) 2013 Cesar Hernandez Bano

    This file is part of ZEsarUX.

    ZEsarUX is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>



#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif


#ifdef MINGW
//Para usar GetLogicalDrives
#include <winbase.h>
#endif


#include "cpu.h"
#include "expression_parser.h"
#include "utils.h"
#include "debug.h"

/*


-Parser breakpoints
*función compilar ascii a tokens
*función inversa
*función evaluar desde tokens
*espacio en bytes de cada token fijo



Parser con tokens: token que identifique un valor, tendrá en su posición del array (estructura realmente) el valor entero almacenado directamente como int

Así cada elemento token puede ser una estructura tipo:

z80_byte tipo (variable, registro, operador, número, fin, etc)
z80_byte indice (si es registro , podría ser: 0 para registro A, 1 para registro B, etc)
z80_byte formato (para números, decir si se muestra como hexadecimal, binario, ascii etc)
z80_byte signo (signo del valor almacenado)
int  valor (guarda el valor absoluto en variable int)

Esto serán 11 bytes por cada token (en máquina de 64 bits)



Evaluar expresión:

Buscar si hay operador logico and, or, xor. Si lo hay, separar sub condiciones

Evaluar condiciones: buscar si hay comparadores =, <,>, <>. Si lo hay, evaluar cada uno de los valores
Cada condición genera 0 o 1

Evaluar valores: por orden, evaluar valores, variables  y posibles operadores de cálculo: +,-,*,/. & (and), | (or), ^ (xor)


NO existe el uso de paréntesis. 

Aquí lo difícil es el parser que convierte el ascii en tokens

Como interpretar un -/+ inicial? Tiene que ser un valor como tal y no una suma o resta. Pero no es solo inicial, sino también después de operadores, por ejemplo:
A>-3
-3 es un valor

Pero:
A>10-3
-3 es operador de resta y valor 3

Para saber si es valor con signo, hay que ver lo que hay antes:
-comparadores <>= etc
-inicio de string
-operador lógico and,or,xor
-operador de cálculo &|^* etc

En resto de casos, se trata de suma o resta y valor absoluto (o sea, cuando lo de la izquierda es un número o variable)


Al final esto dará un valor 0 o diferente de 0. A efectos de disparar breakpoint, este lo hará cuando el valor sea diferente de 0

 */


//Usado en la conversion de texto a tokens, variables:
token_parser_textos_indices tpti_variables[]={
	{TPI_V_MRA,"MRA"},
	{TPI_V_MRW,"MRW"},

    {TPI_FIN,""}
};

//Usado en la conversion de texto a tokens, registros:
token_parser_textos_indices tpti_registros[]={
    {TPI_R_A,"A"},
    {TPI_R_BC,"BC"},
    {TPI_R_DE,"DE"},

    {TPI_FIN,""}
};

//Usado en la conversion de texto a tokens, operador logico:
token_parser_textos_indices tpti_operador_logico[]={

	//de tipo operador logico
	{TPI_OL_AND,"AND"},
	{TPI_OL_OR,"OR"},
	{TPI_OL_XOR,"XOR"},

    {TPI_FIN,""}
};


//Usado en la conversion de texto a tokens, operador condicional:
token_parser_textos_indices tpti_operador_condicional[]={

	{TPI_OC_IGUAL,"="},
	{TPI_OC_MENOR,"<"},
	{TPI_OC_MAYOR,">"},
	{TPI_OC_DIFERENTE,"<>"},

    {TPI_FIN,""}
};


//Usado en la conversion de texto a tokens, operador calculo:
token_parser_textos_indices tpti_operador_calculo[]={

	{TPI_OC_SUMA,"+"},
	{TPI_OC_RESTA,"-"},
	{TPI_OC_MULTIPLICACION,"*"},
	{TPI_OC_DIVISION,"/"},
	{TPI_OC_AND,"&"},	
	{TPI_OC_OR,"|"},
	{TPI_OC_XOR,"^"},

    {TPI_FIN,""}
};    

//Dice si caracter es digito 0...9
int exp_par_is_digit(char c)
{
    if (c>='0' && c<='9') return 1;
    else return 0;
}

//Dice si caracter es digito hexadecimal 0...9...F
int exp_par_is_hexadigit(char c)
{
    if (exp_par_is_digit(c)) return 1;
    if (letra_minuscula(c)>='a' && letra_minuscula(c)<='f') return 1;
    else return 0;
}



//Dice si caracter es letra
int exp_par_is_letter(char c)
{
    if (letra_minuscula(c)>='a' && letra_minuscula(c)<='z') return 1;
    else return 0;
}

//Dice si una expresion es numérica, y dice donde acaba (caracter siguiente)
//Retorna 1 si lo es. 0 si no. -1 si hay error parseando
int exp_par_is_number(char *texto,int *final)
{
    /*
    Expresiones admitidas:
    numero decimal: 0123456789
    numero hexadecimal: 0123456789ABCDEFH
    numero binario: 01%
    numero ascii "A" o 'a'
     */

    //Primero probar ascii
    if (texto[0]=='"' || texto[0]=='\'') {
        //si final texto, error
        if (texto[1]==0) return -1;
        if (texto[2]=='"' || texto[2]=='\'') {
            *final=3;
            return 1;
        }
        else return -1; //no hay comillas de cierre
    }

    //Tendra que empezar con numero o hexa, si no, error
    if (!exp_par_is_hexadigit(texto[0])) return -1;

    //Parseamos hasta encontrar sufijo, si lo hay
    int i;
    for (i=0;texto[i];i++) {
        if (letra_minuscula(texto[i])=='h' || texto[i]=='%') {
            *final=i+1;
            return 1;
        }
        if (!exp_par_is_hexadigit(texto[i])) break;

    }

    *final=i;
    return 1;

}

//Dice si el texto es uno de los contenidos en array de token_parser_textos_indices
//Retorna el indice si esta, -1 si no
int exp_par_is_token_parser_textos_indices(char *texto,token_parser_textos_indices *textos_indices)
{

    int i=0;

    while (textos_indices->indice!=TPI_FIN) {
        if (!strcasecmp(texto,textos_indices->texto)) return i;
        i++;
        textos_indices++;
    }

    return -1;
}


//Dice si una expresion es operador, y dice donde acaba (caracter siguiente)
//Retorna 1 si lo es. 0 si no. -1 si hay error parseando
int exp_par_is_operador(char *texto,int *final)
{
    //Considerar letras y tpti_operador_condicional y tpti_operador_calculo
    char primer_caracter;

    char buffer_texto[3];

    //TODO: considerar condicional <>
    primer_caracter=*texto;
    buffer_texto[0]=primer_caracter;
    buffer_texto[1]=0;

    //tpti_operador_condicional
    if (exp_par_is_token_parser_textos_indices(buffer_texto,tpti_operador_condicional)>=0) {
        *final=strlen(buffer_texto);
        return 1;
    }

    //tpti_operador_calculo
    if (exp_par_is_token_parser_textos_indices(buffer_texto,tpti_operador_calculo)>=0) {
        *final=strlen(buffer_texto);
        return 1;
    }

    //letras hasta que no sean letras
    int i=0;
    while (!exp_par_is_letter(texto[i])) {
        i++;
    }

    *final=i;
    return 1;

}


//Dice si una expresion es variable/registro, y dice donde acaba (caracter siguiente)
//Retorna 1 si lo es. 0 si no. -1 si hay error parseando
int exp_par_is_var_reg(char *texto,int *final)
{
   

    //Buscar hasta final letras
    char buffer_texto[MAX_PARSER_TEXTOS_INDICE_LENGTH];

    int i=0;
    while (*texto && exp_par_is_letter(*texto) && i<MAX_PARSER_TEXTOS_INDICE_LENGTH)  {
        buffer_texto[i]=*texto;

        i++;
        texto++;
    }

    if (i==MAX_PARSER_TEXTOS_INDICE_LENGTH) {
        //Final de buffer. error
        return -1;
    }

    buffer_texto[i]=0;

    //tpti_operador_condicional
    if (exp_par_is_token_parser_textos_indices(buffer_texto,tpti_variables)>=0) {
        *final=strlen(buffer_texto);
        return 1;
    }

    //tpti_operador_calculo
    if (exp_par_is_token_parser_textos_indices(buffer_texto,tpti_registros)>=0) {
        *final=strlen(buffer_texto);
        return 1;
    }

    return 0;

}


//Parsear texto como variable o registro
//Devuelve 0 si no existe
int exp_par_parse_var_reg(char *texto,enum token_parser_tipo *tipo,enum token_parser_indice *indice_final)
{
   
    int indice;

    indice=exp_par_is_token_parser_textos_indices(texto,tpti_variables);
    if (indice>=0) {
        *tipo=TPT_VARIABLE;
        *indice_final=indice;
        return 1;
    }

    indice=exp_par_is_token_parser_textos_indices(texto,tpti_registros);
    if (indice>=0) {
        *tipo=TPT_REGISTRO;
        *indice_final=indice;
        return 1;
    }

    return 0;

}

//Copia la cadena origen en destino, con longitud indicada. Agrega 0 al final
void exp_par_copy_string(char *origen,char *destino, int longitud)
{
    int i;

    for (i=0;i<longitud;i++) {
        destino[i]=origen[i];
    }

    destino[i]=0;
}


//Convierte expression de entrada en tokens. Devuelve <0 si error
int exp_par_exp_to_tokens(char *expression,token_parser *tokens)
{
    /*
    Expresion de entrada:

    NUMERO/VARIABLE/REGISTRO OPERADOR NUMERO/VARIABLE/REGISTRO OPERADOR .... NUMERO/VARIABLE/REGISTRO


    	TPT_NUMERO,
	TPT_VARIABLE, //mra, mrw, etc
	TPT_REGISTRO, //a, bc, de, etc
	TPT_OPERADOR_LOGICO, //and, or, xor
	TPT_OPERADOR_CONDICIONAL, //=, <,>, <>,
	TPT_OPERADOR_CALCULO //+,-,*,/. & (and), | (or), ^ (xor)

     */
    //Empezamos a leer texto y dividir secciones de NUMERO/VARIABLE/REGISTRO y OPERADOR

    int indice_token=0;

    while (*expression) {
        //Si hay espacio, saltar
        if ( (*expression)==' ') expression++;
        else {
            //Obtener numero
            int final;
            int resultado;
            printf ("parsing number from %s\n",expression);
            resultado=exp_par_is_number(expression,&final);
            
            /* if (resultado==-1) {
                printf ("return number with error\n");
                return -1; //error
            }*/

            if (resultado>=0) {
                printf ("final index: %d\n",final);
                //Es un numero
                printf ("end number: %c\n",expression[final]);

                //Parseamos numero
                int valor=parse_string_to_number(expression);

                //Meter valor en token
                tokens[indice_token].tipo=TPT_NUMERO;
                //TODO: formato, signo

                //Meter valor
                tokens[indice_token].valor=valor;

            }

            else {
                //Consideramos variable/registro
                printf ("parsing variable/register from %s\n",expression);
                resultado=exp_par_is_var_reg(expression,&final);
                if (resultado==-1) {
                     printf ("return var_reg with error\n");
                    return -1; //error
                }

                printf ("final index: %d\n",final);

                //Parsear expresion. TODO
                //tokens[indice_token].tipo=TPT_VARIABLE; //temporal

                //Metemos en buffer temporal
                char buffer_temp[MAX_PARSER_TEXTOS_INDICE_LENGTH];
                exp_par_copy_string(expression,buffer_temp,final);

                enum token_parser_tipo tipo;
                enum token_parser_indice indice;

                if (!exp_par_parse_var_reg(buffer_temp,&tipo,&indice)) {
                    printf ("return error exp_par_parse_var_reg\n");
                    return -1;
                }

                tokens[indice_token].tipo=tipo;
                tokens[indice_token].indice=indice;

           
            }

            //Siguiente expresion
            indice_token++;
            expression=&expression[final];

            //saltar espacios
            while ( (*expression)==' ') expression++;

            //Si no final, 
            if ( (*expression)!=0) {
                //Calcular operador
                printf ("parsing operador from %s\n",expression);
                resultado=exp_par_is_operador(expression,&final);
                if (resultado==-1) {
                    printf ("return operador with error\n");
                    return -1; //error
                }

                printf ("final index: %d\n",final);

                //printf ("end number: %c\n",expression[final]);

                //Parsear expresion. TODO
                tokens[indice_token].tipo=TPT_OPERADOR_LOGICO; //temporal            
            
                //Siguiente expresion
                indice_token++;
                expression=&expression[final];


            }

        }
    };

    //Poner final de token
    tokens[indice_token].tipo=TPT_FIN;

    return 0;

}

//Convierte tokens en string
void exp_par_tokens_to_exp(token_parser *tokens,char *expression)
{
	int i=0;
    int dest_string=0;

	while (tokens[i].tipo!=TPT_FIN) {
        int esnumero=0;
        int espacio=0;
        enum token_parser_tipo tipo=tokens[i].tipo;

        token_parser_textos_indices *indice_a_tabla;

        switch (tipo) {
            case TPT_NUMERO:
                esnumero=1;
            break;

	        case TPT_VARIABLE: //mra,mrw, etc
                indice_a_tabla=tpti_variables;
            break;

	        case TPT_REGISTRO: //a, bc, de, etc
                indice_a_tabla=tpti_registros;
            break;


	        case TPT_OPERADOR_LOGICO:  //and, or, xor
                indice_a_tabla=tpti_operador_logico;
                espacio=1;
            break;

            case TPT_OPERADOR_CONDICIONAL:  //=, <,>, <>,
                indice_a_tabla=tpti_operador_condicional;
            break;

            case TPT_OPERADOR_CALCULO: //+,-,*,/. & (and), | (or), ^ (xor)
                indice_a_tabla=tpti_operador_calculo;
            break;

            case TPT_FIN:
                //esto se gestiona desde el while y por tanto no se llega nunca aqui. Lo pongo para que no se queje el compilador
            break;

        }

        if (esnumero) {
            //TODO: signo, formato hexa,binario etc
           sprintf(&expression[dest_string],"%d",tokens[i].valor); 
        }

        else {
            int indice=tokens[i].indice;
            if (!espacio) sprintf(&expression[dest_string],"%s",indice_a_tabla[indice].texto);
            else sprintf(&expression[dest_string]," %s ",indice_a_tabla[indice].texto);
        }

        int longitud=strlen(&expression[dest_string]);
        dest_string +=longitud;    

            
		i++;
	}
}