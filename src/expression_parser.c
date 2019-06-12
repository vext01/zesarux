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
            resultado=exp_par_is_number(expression,&final);
            if (resultado==-1) return -1; //error

            //Parseamos numero
            int valor=parse_string_to_number(expression);

            //Meter valor en token
            tokens[indice_token].tipo=TPT_NUMERO;
            //TODO: formato, signo

            //Meter valor
            tokens[indice_token].valor=valor;

            //Siguiente expresion
            indice_token++;
            expression=&expression[final];

            //saltar espacios
            while ( (*expression)==' ') expression++;

            //Si no final, 
            if ( (*expression)!=0) {
                //Calcular operador
            }

        }
    };

    //Poner final de token
    tokens[indice_token].tipo=TPT_FIN;

    return 0;

}