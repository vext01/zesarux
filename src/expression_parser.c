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
#include "operaciones.h"
#include "screen.h"

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
	{TPI_V_MRV,"MRV"},

	{TPI_V_MWV,"MWV"},
	{TPI_V_MWA,"MWA"},

	//Puertos
	{TPI_V_PRV,"PRV"},
	{TPI_V_PRA,"PRA"},

	{TPI_V_PWV,"PWV"},
	{TPI_V_PWA,"PWA"},

	{TPI_V_TSTATES,"TSTATES"},
	{TPI_V_TSTATESL,"TSTATESL"},
	{TPI_V_TSTATESP,"TSTATESP"},

	{TPI_V_SCANLINE,"SCANLINE"},


	{TPI_V_IFF1,"IFF1"},
	{TPI_V_IFF2,"IFF2"},

	{TPI_V_OUTFIRED,"OUTFIRED"},
	{TPI_V_INFIRED,"INFIRED"},
	{TPI_V_INTFIRED,"INTFIRED"},

    {TPI_FIN,""}
};

//Usado en la conversion de texto a tokens, registros:
token_parser_textos_indices tpti_registros[]={

	{TPI_R_PC,"PC"},
    {TPI_R_SP,"SP"},
    {TPI_R_IX,"IX"},
    {TPI_R_IY,"IY"},	

	{TPI_R_A,"A"},
	{TPI_R_B,"B"},
	{TPI_R_C,"C"},
	{TPI_R_D,"D"},
	{TPI_R_E,"E"},
	{TPI_R_F,"F"},
	{TPI_R_H,"H"},
	{TPI_R_L,"L"},
	{TPI_R_I,"I"},
	{TPI_R_R,"R"},

        {TPI_R_AF,"AF"},
        {TPI_R_BC,"BC"},
        {TPI_R_DE,"DE"},
        {TPI_R_HL,"HL"},    

	{TPI_R_A_SHADOW,"A'"},
	{TPI_R_B_SHADOW,"B'"},
	{TPI_R_C_SHADOW,"C'"},
	{TPI_R_D_SHADOW,"D'"},
	{TPI_R_E_SHADOW,"E'"},
	{TPI_R_F_SHADOW,"F'"},
	{TPI_R_H_SHADOW,"H'"},
	{TPI_R_L_SHADOW,"L'"},


        {TPI_R_AF_SHADOW,"AF'"},
        {TPI_R_BC_SHADOW,"BC'"},
        {TPI_R_DE_SHADOW,"DE'"},
        {TPI_R_HL_SHADOW,"HL'"},   

        {TPI_R_FS,"FS"},
        {TPI_R_FZ,"FZ"},
        {TPI_R_FP,"FP"},
        {TPI_R_FV,"FV"},    
        {TPI_R_FH,"FH"},
        {TPI_R_FN,"FN"},
        {TPI_R_FC,"FC"},  

	    {TPI_R_P_BC,"(BC)"},
        {TPI_R_P_DE,"(DE)"},
        {TPI_R_P_HL,"(HL)"},
        {TPI_R_P_SP,"(SP)"},
        {TPI_R_P_PC,"(PC)"},
        {TPI_R_P_IX,"(IX)"},
        {TPI_R_P_IY,"(IY)"},             


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
    //printf ("llamando a exp_par_is_token_parser_textos_indices con texto [%s]\n",texto);

    int i=0;

    while (textos_indices->indice!=TPI_FIN) {
        //printf ("i %d\n",i);
        if (!strcasecmp(texto,textos_indices->texto)) {
            //printf ("es texto %s indice %d\n",textos_indices->texto,textos_indices->indice);
            return textos_indices->indice;
        }
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

    
    primer_caracter=*texto;
    buffer_texto[0]=primer_caracter;
    buffer_texto[1]=0;

    //considerar condicional <>
    if (texto[0]=='<' && texto[1]=='>') {
        buffer_texto[0]='<';
        buffer_texto[1]='>';
        buffer_texto[2]=0;        
    }

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
    while (exp_par_is_letter(texto[i])) {
        i++;
    }

    *final=i;
    return 1;

}

//Considerar caracteres auxiliares para registros: ' , (), pero no ciua
int exp_par_char_is_reg_aux(char c)
{
    if (c=='\'' || c=='(' || c==')') return 1;
    else return 0;
}

//Considerar caracteres auxiliares para variables: 12,  como iff1 y iff2, pero no al principio
int exp_par_char_is_reg_aux_more(char c,int i)
{
    if (i==0) return 0;

    if (c=='1' || c=='2') return 1;
    else return 0;
}

//Dice si una expresion es variable/registro, y dice donde acaba (caracter siguiente)
//Retorna 1 si lo es. 0 si no. -1 si hay error parseando
int exp_par_is_var_reg(char *texto,int *final)
{
   

    //Buscar hasta final letras
    char buffer_texto[MAX_PARSER_TEXTOS_INDICE_LENGTH];

    int i=0;
    while (*texto && (exp_par_is_letter(*texto) || exp_par_char_is_reg_aux(*texto) || exp_par_char_is_reg_aux_more(*texto,i) ) && i<MAX_PARSER_TEXTOS_INDICE_LENGTH)  {
        //consideramos que pueden acabar los registros con ' ()

        buffer_texto[i]=*texto;
        i++;
        texto++;
    }

    if (i==MAX_PARSER_TEXTOS_INDICE_LENGTH) {
        //Final de buffer. error
        return -1;
    }

    buffer_texto[i]=0;

    //printf ("probando tpti_variables\n");
    if (exp_par_is_token_parser_textos_indices(buffer_texto,tpti_variables)>=0) {
        //printf ("es variable\n");
        *final=strlen(buffer_texto);
        return 1;
    }

    if (exp_par_is_token_parser_textos_indices(buffer_texto,tpti_registros)>=0) {
        //printf ("es registro\n");
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


//Parsear texto como operador
//Devuelve 0 si no existe
int exp_par_parse_operador(char *texto,enum token_parser_tipo *tipo,enum token_parser_indice *indice_final)
{
    int indice;

 
    indice=exp_par_is_token_parser_textos_indices(texto,tpti_operador_condicional);
    if (indice>=0) {
        *tipo=TPT_OPERADOR_CONDICIONAL;
        *indice_final=indice;
        return 1;
    }

    indice=exp_par_is_token_parser_textos_indices(texto,tpti_operador_calculo);
    if (indice>=0) {
        *tipo=TPT_OPERADOR_CALCULO;
        *indice_final=indice;
        return 1;
    }


    indice=exp_par_is_token_parser_textos_indices(texto,tpti_operador_logico);
    if (indice>=0) {
        *tipo=TPT_OPERADOR_LOGICO;
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

            //Suponer primero que son variables/registros

            //Consideramos variable/registro
            //printf ("parsing variable/register from %s\n",expression);
            resultado=exp_par_is_var_reg(expression,&final);
            //printf ("resultado exp_par_is_var_reg %d\n",resultado);
            if (resultado>0) {
                //printf ("final index: %d\n",final);

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

            else {

                //Consideramos numero

                //printf ("parsing number from %s\n",expression);
                resultado=exp_par_is_number(expression,&final);
            
                if (resultado<=0) {
                    //printf ("return number with error\n");
                    return -1; //error
                }

            
                //printf ("final index: %d\n",final);
                //Es un numero
                //printf ("end number: %c\n",expression[final]);


                //Metemos en buffer temporal
                char buffer_temp[MAX_PARSER_TEXTOS_INDICE_LENGTH];
                exp_par_copy_string(expression,buffer_temp,final);                

                //Parseamos numero
                int valor=parse_string_to_number(buffer_temp);

                //Meter valor en token
                tokens[indice_token].tipo=TPT_NUMERO;
                //TODO: formato, signo

                //Meter valor
                tokens[indice_token].valor=valor;

            }

            

            //Siguiente expresion
            indice_token++;
            expression=&expression[final];

            //saltar espacios
            while ( (*expression)==' ') expression++;

            //Si no final, 
            if ( (*expression)!=0) {
                //Calcular operador
                //printf ("parsing operador from %s\n",expression);
                resultado=exp_par_is_operador(expression,&final);
                if (resultado==-1) {
                    printf ("return operador with error\n");
                    return -1; //error
                }

                //printf ("final index: %d\n",final);

                //printf ("end number: %c\n",expression[final]);

                //Parsear expresion. TODO
                //tokens[indice_token].tipo=TPT_OPERADOR_LOGICO; //temporal    



                //Metemos en buffer temporal
                char buffer_temp[MAX_PARSER_TEXTOS_INDICE_LENGTH];
                exp_par_copy_string(expression,buffer_temp,final);

                enum token_parser_tipo tipo;
                enum token_parser_indice indice;

                if (!exp_par_parse_operador(buffer_temp,&tipo,&indice)) {
                    //printf ("return error exp_par_parse_operador [%s]\n",buffer_temp);
                    return -1;
                }

                tokens[indice_token].tipo=tipo;
                tokens[indice_token].indice=indice;                
       
            
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





/*
Funciones de paso de tokens a string
*/

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
            int indice=tokens[i].indice; //indice a buscar

            //buscar texto que corresponda con ese indice
            int j;
            for (j=0;indice_a_tabla[j].indice!=TPT_FIN;j++) {
                if (indice_a_tabla[j].indice==indice) break;
            }

            if (!espacio) sprintf(&expression[dest_string],"%s",indice_a_tabla[j].texto);
            else sprintf(&expression[dest_string]," %s ",indice_a_tabla[j].texto);

            //printf ("***MRA=%d \n",TPI_V_MRA);
        }

        int longitud=strlen(&expression[dest_string]);
        dest_string +=longitud;    

            
		i++;
	}
}

//calcula valor de token, si es numero, variable o registro
int exp_par_calculate_numvarreg(token_parser *token)
{

    enum token_parser_tipo tipo=token->tipo;
    enum token_parser_indice indice=token->indice;

        int resultado=0; //asumimos cero

        switch (tipo) {
            case TPT_NUMERO:
                resultado=token->valor;
            break;

	        case TPT_VARIABLE: //mra,mrw, etc
                switch (indice) {
                    

//Variables de la MMU
	//Memoria
    case TPI_V_MRA: return debug_mmu_mra; break;
	case TPI_V_MRV: return debug_mmu_mrv; break;

	case TPI_V_MWV: return debug_mmu_mwv; break;
	case TPI_V_MWA: return debug_mmu_mwa; break;

	//Puertos
	case TPI_V_PRV: return debug_mmu_prv; break;
	case TPI_V_PRA: return debug_mmu_pra; break;

	case TPI_V_PWV: return debug_mmu_pwv; break;
	case TPI_V_PWA: return debug_mmu_pwa; break;

	//T-estados
	case TPI_V_TSTATES: return t_estados; break;
	case TPI_V_TSTATESL: return t_estados % screen_testados_linea; break;
	case TPI_V_TSTATESP: return debug_t_estados_parcial; break;

	case TPI_V_SCANLINE: return t_scanline_draw; break;



	//interrupciones
	case TPI_V_IFF1: return iff1.v; break;
	case TPI_V_IFF2: return iff2.v; break;

	//se acaba de lanzar un out
	case TPI_V_OUTFIRED: return debug_fired_out; break;
	//se acaba de lanzar un in
	case TPI_V_INFIRED: return debug_fired_in; break;
	//se acaba de generar una interrupcion
	case TPI_V_INTFIRED: return debug_fired_interrupt; break;	


                    default:
                        //Para que no se queje el compilador por demas valores enum no tratados
                    break;

/*
//si (NN)
	if (registro[0]=='(') {
		int s=strlen(registro);
		if (s>2) {
			if (registro[s-1]==')') {
				char buf_direccion[MAX_BREAKPOINT_CONDITION_LENGTH];
				//copiar saltando parentesis inicial
				sprintf (buf_direccion,"%s",&registro[1]);
				//quitar parentesis final
				//(16384) -> s=7 -> buf_direccion=16384). () -> s=2 ->buf_direccion=) .
				buf_direccion[s-2]=0;
				//printf ("buf_direccion: %s\n",buf_direccion);
				z80_int direccion=parse_string_to_number(buf_direccion);
				return peek_byte_no_time(direccion);
			}
		}
	}

	//ram mapeada en 49152-65535 de Spectrum
	if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
		if (!strcasecmp(registro,"ram")) return debug_paginas_memoria_mapeadas[3];

		//rom mapeada en Spectrum
		if (!strcasecmp(registro,"rom")) return (debug_paginas_memoria_mapeadas[0] & 127);

		//TODO. condiciones especiales para mapeo de paginas del +2A tipo ram en rom
	}

	//ram mapeada en 49152-65535 de Prism
        if (MACHINE_IS_PRISM) {
                if (!strcasecmp(registro,"ram")) return prism_retorna_ram_entra()*2;
	}

	//bancos memoria Z88
	if (MACHINE_IS_Z88) {
		if (!strcasecmp(registro,"seg0")) return blink_mapped_memory_banks[0];
		if (!strcasecmp(registro,"seg1")) return blink_mapped_memory_banks[1];
		if (!strcasecmp(registro,"seg2")) return blink_mapped_memory_banks[2];
		if (!strcasecmp(registro,"seg3")) return blink_mapped_memory_banks[3];
	}

	

	

	//enterrom, exitrom

//Avisa cuando se ha entrado o salido de rom. Solo salta una vez el breakpoint
//0: no esta en rom
//1: esta en rom y aun no ha saltado breakpoint
//2: esta en rom y ya ha saltado breakpoint
//int debug_enterrom=0;

//0: no ha salido de rom
//1: ha salido de rom y aun no ha saltado breakpoint
//2: ha salido de rom y ya ha saltado breakpoint
//int debug_exitrom=0;


	if (!strcasecmp(registro,"enterrom")) {
		if (debug_enterrom==1) {
			debug_enterrom++;
			return 1;
		}
		return 0;
	}

	if (!strcasecmp(registro,"exitrom")) {
		if (debug_exitrom==1) {
			debug_exitrom++;
			return 1;
		}
		return 0;
	}
 */

                }
                //temporal
                resultado=66;
            break;

	        case TPT_REGISTRO: //a, bc, de, etc
                /*/if (indice==TPI_R_A) return reg_a;
                if (indice==TPI_R_BC) return reg_bc;
                if (indice==TPI_R_DE) return reg_de;*/
                switch (indice) {
    case TPI_R_PC: return reg_pc; break;
    case TPI_R_SP: return reg_sp; break;
    case TPI_R_IX: return reg_ix; break;
    case TPI_R_IY: return reg_iy; break;	

	case TPI_R_A: return reg_a; break;
	case TPI_R_B: return reg_b; break;
	case TPI_R_C: return reg_c; break;
	case TPI_R_D: return reg_d; break;
	case TPI_R_E: return reg_e; break;
	case TPI_R_F: return Z80_FLAGS; break;
	case TPI_R_H: return reg_h; break;
	case TPI_R_L: return reg_l; break;
	case TPI_R_I: return reg_i; break;
	case TPI_R_R: return (reg_r&127)|(reg_r_bit7&128); break;

        case TPI_R_AF: return REG_AF; break;
        case TPI_R_BC: return reg_bc; break;
        case TPI_R_DE: return reg_de; break;
        case TPI_R_HL: return reg_hl; break;



	case TPI_R_A_SHADOW: return reg_a_shadow; break;
	case TPI_R_B_SHADOW: return reg_b_shadow; break;
	case TPI_R_C_SHADOW: return reg_c_shadow; break;
	case TPI_R_D_SHADOW: return reg_d_shadow; break;
	case TPI_R_E_SHADOW: return reg_e_shadow; break;
	case TPI_R_F_SHADOW: return Z80_FLAGS_SHADOW; break;
	case TPI_R_H_SHADOW: return reg_h_shadow; break;
	case TPI_R_L_SHADOW: return reg_l_shadow; break;

        case TPI_R_AF_SHADOW: return REG_AF_SHADOW; break;
        case TPI_R_BC_SHADOW: return REG_BC_SHADOW; break;
        case TPI_R_DE_SHADOW: return REG_DE_SHADOW; break;
        case TPI_R_HL_SHADOW: return REG_HL_SHADOW; break;

        case TPI_R_FS: return ( Z80_FLAGS & FLAG_S ? 1 : 0); break;
        case TPI_R_FZ: return ( Z80_FLAGS & FLAG_Z ? 1 : 0); break;

        case TPI_R_FP: 
        case TPI_R_FV: 
            return ( Z80_FLAGS & FLAG_PV ? 1 : 0);
        break;

        case TPI_R_FH: return ( Z80_FLAGS & FLAG_H ? 1 : 0); break;
        case TPI_R_FN: return ( Z80_FLAGS & FLAG_N ? 1 : 0); break;
        case TPI_R_FC: return ( Z80_FLAGS & FLAG_C ? 1 : 0); break;


	    case TPI_R_P_BC: return peek_byte_no_time(reg_bc); break;
        case TPI_R_P_DE: return peek_byte_no_time(reg_de); break;
        case TPI_R_P_HL: return peek_byte_no_time(reg_hl); break;
        case TPI_R_P_SP: return peek_byte_no_time(reg_sp); break;
        case TPI_R_P_PC: return peek_byte_no_time(reg_pc); break;
        case TPI_R_P_IX: return peek_byte_no_time(reg_ix); break;
        case TPI_R_P_IY: return peek_byte_no_time(reg_iy); break;        


                    default:
                        //Para que no se queje el compilador por demas valores enum no tratados
                    break;
                }




            break;
            


	        case TPT_OPERADOR_LOGICO:  //and, or, xor
            case TPT_OPERADOR_CONDICIONAL:  //=, <,>, <>,
            case TPT_OPERADOR_CALCULO: //+,-,*,/. & (and), | (or), ^ (xor)
            case TPT_FIN:
                //esto no se llega nunca aqui. Lo pongo para que no se queje el compilador
            break;



    }

    return resultado;
}

//calcula valor resultante de aplicar operador, puede ser 
int exp_par_calculate_operador(int valor_izquierda,int valor_derecha,enum token_parser_tipo tipo,enum token_parser_indice indice)
{

    int resultado=0; //asumimos cero
    
    /*
    	TPT_OPERADOR_LOGICO, //and, or, xor
	TPT_OPERADOR_CONDICIONAL, //=, <,>, <>,
	TPT_OPERADOR_CALCULO //+,-,*,/. & (and), | (or), ^ (xor)
     */

    //printf ("exp_par_calculate_operador tipo %d indice %d\n",tipo,indice);

    switch (tipo) {
            case TPT_NUMERO:
	        case TPT_VARIABLE: //mra,mrw, etc
	        case TPT_REGISTRO: //a, bc, de, etc
            case TPT_FIN:
                //esto no se llega nunca aqui. Lo pongo para que no se queje el compilador
            break;


	        case TPT_OPERADOR_LOGICO:  //and, or, xor

                if (indice==TPI_OL_AND) {   
                    if (valor_izquierda && valor_derecha) return 1;
                }

                if (indice==TPI_OL_OR) {   
                    if (valor_izquierda || valor_derecha) return 1;
                }                

                if (indice==TPI_OL_XOR) {
                    if (valor_izquierda && valor_derecha) return 0;  
                    else if (!valor_izquierda && valor_derecha) return 1;  
                    else if (valor_izquierda && !valor_derecha) return 1;  
                    else return 0; //ambos a 0
                }                   

            break;

            case TPT_OPERADOR_CONDICIONAL:  //=, <,>, <>,
            //printf ("operaodr condicional\n"); 
                if (indice==TPI_OC_MAYOR) {
                    //printf ("operaodr mayor\n");    
                    if (valor_izquierda>valor_derecha) return 1;
                }
                if (indice==TPI_OC_MENOR) {   
                    if (valor_izquierda<valor_derecha) return 1;
                }

                if (indice==TPI_OC_IGUAL) {   
                    if (valor_izquierda==valor_derecha) return 1;
                }           

                if (indice==TPI_OC_DIFERENTE) {  
                    if (valor_izquierda!=valor_derecha) return 1;
                }                          

            break;

            case TPT_OPERADOR_CALCULO: //+,-,*,/. & (and), | (or), ^ (xor)

                if (indice==TPI_OC_SUMA) {
                    //printf ("sumando %d y %d\n",valor_izquierda,valor_derecha);
                    return valor_izquierda + valor_derecha;
                }         

                if (indice==TPI_OC_RESTA) {
                    return valor_izquierda - valor_derecha;
                }          

                if (indice==TPI_OC_MULTIPLICACION) {
                    //printf ("multiplicando %d y %d\n",valor_izquierda,valor_derecha);
                    return valor_izquierda * valor_derecha;
                }         

                if (indice==TPI_OC_DIVISION) {

                    //controlar division por cero
                    if (valor_derecha==0) {
                        //retornamos valor 16 bits maximo
                        return 0xffff;
                    }
                    else return valor_izquierda / valor_derecha;
                }        

                if (indice==TPI_OC_AND) {
                    return valor_izquierda & valor_derecha;
                }     

                if (indice==TPI_OC_OR) {
                    return valor_izquierda | valor_derecha;
                }          

                if (indice==TPI_OC_XOR) {
                    return valor_izquierda ^ valor_derecha;
                }                                                                          

            break;



    }

    return resultado;
}


//Calcula la expresion identificada por tokens. Funcion recursiva
//final identifica al siguiente token despues del final. Poner valor alto para que no tenga final y detecte token de fin
int exp_par_evaluate_token(token_parser *tokens,int final,int *error_code)
{
/*
Evaluar expresión:

Buscar si hay operador logico and, or, xor. Si lo hay, separar sub condiciones

Evaluar condiciones: buscar si hay comparadores =, <,>, <>. Si lo hay, evaluar cada uno de los valores
Cada condición genera 0 o 1

Evaluar valores: por orden, evaluar valores, variables  y posibles operadores de cálculo: +,-,*,/. & (and), | (or), ^ (xor)
 */
    //printf ("evaluando tokens hasta longitud %d\n",final);

    *error_code=0; //asumimos ok

    if (final==0) {
        //expresion vacia. no deberia suceder. retornar 0
        *error_code=1;
        return 0;
    }

    //Ver si hay operadores logicos
    //int i;

    int i=0;

 

    for (i=0;i<final && tokens[i].tipo!=TPT_FIN;i++) {

        if (tokens[i].tipo==TPT_OPERADOR_CONDICIONAL ) {
            //Evaluar parte izquierda y derecha y aplicar operador
            int valor_izquierda;
            int valor_derecha;

            int errorcode1,errorcode2;

            //printf ("dividiendo condicionales\n");
            valor_izquierda=exp_par_evaluate_token(tokens,i,&errorcode1);
            valor_derecha=exp_par_evaluate_token(&tokens[i+1],MAX_PARSER_TOKENS_NUM,&errorcode2);

            return exp_par_calculate_operador(valor_izquierda,valor_derecha,tokens[i].tipo,tokens[i].indice);
        }

        if (tokens[i].tipo==TPT_OPERADOR_LOGICO ) {
            //Evaluar parte izquierda y derecha y aplicar operador
            int valor_izquierda;
            int valor_derecha;

            int errorcode1,errorcode2;

            valor_izquierda=exp_par_evaluate_token(tokens,i,&errorcode1);
            valor_derecha=exp_par_evaluate_token(&tokens[i+1],MAX_PARSER_TOKENS_NUM,&errorcode2);

            return exp_par_calculate_operador(valor_izquierda,valor_derecha,tokens[i].tipo,tokens[i].indice);
        }



        if (tokens[i].tipo==TPT_OPERADOR_CALCULO) {
            //Evaluar parte izquierda y derecha y aplicar operador
            int valor_izquierda;
            int valor_derecha;

            int errorcode1,errorcode2;

            //printf ("calculando desde ")
            valor_izquierda=exp_par_evaluate_token(tokens,i,&errorcode1);
            valor_derecha=exp_par_evaluate_token(&tokens[i+1],MAX_PARSER_TOKENS_NUM,&errorcode2);

            return exp_par_calculate_operador(valor_izquierda,valor_derecha,tokens[i].tipo,tokens[i].indice);
        }   

    }

    i=0;

    if ( (tokens[i].tipo==TPT_NUMERO || tokens[i].tipo==TPT_VARIABLE || tokens[i].tipo==TPT_REGISTRO)
             )
    {
            //printf ("es variable\n");
            //tiene que ser numero
            int resultado=exp_par_calculate_numvarreg(&tokens[i]);
            printf("resultado variable: %d\n",resultado);
            return resultado;
    }


    //Aqui no deberia llegar nunca
    *error_code=1;
    return 0;

}