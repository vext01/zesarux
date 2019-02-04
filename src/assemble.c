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



#include "cpu.h"
#include "assemble.h"
#include "debug.h"
#include "utils.h"



//Assembler. Ver http://www.z80.info/decoding.htm

//Retorna opcode y primer y segundo operador
/*
Formato entrada: 


OPCODE 
OPCODE OP1
OPCODE OP1,OP2  

Retorna puntero a primer parametro, util para comandos como DEFB 0,0,0,.....
*/
char *asm_return_op_ops(char *origen,char *opcode,char *primer_op,char *segundo_op)
{
	//Primero asumimos todos nulos
	*opcode=0;
	*primer_op=0;
	*segundo_op=0;

        char *puntero_primer_parametro;

	//Opcode empieza es hasta el espacio
	int i;
	for (i=0;origen[i] && origen[i]!=' ';i++) {
		*opcode=origen[i];
		opcode++;
	}

	*opcode=0;

	//Buscamos hasta algo diferente de espacio
	for (;origen[i]==' ';i++) {
        }


        puntero_primer_parametro=&origen[i];

	//Primer operador es hasta la ,
	for (;origen[i] && origen[i]!=',';i++) {
                *primer_op=origen[i];
                primer_op++;
        }

        *primer_op=0;
	
	if (origen[i]==',') i++;

	//Y ya hasta final de cadena
        for (;origen[i];i++) {
                *segundo_op=origen[i];
                segundo_op++;
        }

        *segundo_op=0;

        return puntero_primer_parametro;

}


/*
Tablas de opcodes para ensamblado:

-Nombre opcode: LD, INC, HALT, etc
-De cada tipo opcode, mascara opcode, tipo parametros que admite, cuantos parametros, y mascara parametro:
Ejemplos:   LD r,n .  LD RR,NN.    o HALT (sin parametros).

LD r,n. base opcode=6 (00000110). tipo parametros: r. mascara parametro 1: 00XXX000
Por ejemplo, si LD A,33 -> A en tabla r vale 7. 
Valor final:
00000110  OR 00111000 = 00111110 = 62

La mascara de operador no tiene mucho sentido el numero de bits en mascara, solo el primer bit de la derecha que esta a 1,
dado que meteremos el valor del operador final ahí con un OR, rotando tantos bits a la izquierda como corresponda

--Tipos parametros:
n
nn
dis
r
rp (bc,de,hl,sp)
rp2 (bc,de,hl,af)
cc (nz,z,nc,c,po,pe,p,m)
string tal cual (como "AF'" en "ex af,af'"), o como ("1" en "IM 1"), o como ("HL" en "JP HL")


-Casos especiales: EX AF,AF' -> un solo opcode sin parametros. Quiza en estos casos decir: opcode=EX. parametro 1=string=AF, parametro 2=string=AF'

-cada opcode en strings apartes:
char *asm_opcode_ld="LD";
char *asm_opcode_inc="INC",

en tabla opcodes:
{ asm_opcode_ld,r,n },
{ asm_opcode_ld,rr,nn } ,
{ asm_opcode_inc,r } ,
{ asm_opcode_inc,rr }

Para no tener que repetir strings (guardamos solo el char *)

*/

enum asm_tipo_parametro
{
        ASM_PARM_NONE,
	ASM_PARM_CONST, //caso de EX AF,AF'
	ASM_PARM_R,
	ASM_PARM_RP,
	ASM_PARM_RP2,
	ASM_PARM_N,
	ASM_PARM_NN,
	ASM_PARM_DIS
};

enum asm_tipo_parametro_entrada
{
	ASM_PARM_IN_NONE,
        ASM_PARM_IN_R,
        ASM_PARM_IN_RP,
        ASM_PARM_IN_RP2,
	ASM_PARM_IN_NUMERO
};

struct s_tabla_ensamblado {
        char *texto_opcode;
        int mascara_opcode;
        int prefijo; //203,221,237,251

        int tipo_parametro_1;
        int desplazamiento_mascara_p1; //Cuantos bits a desplazar a la izquierda
	char *const_parm1; //Para EX AF,AF'

        int tipo_parametro_2;
        int desplazamiento_mascara_p2;
	char *const_parm2; //Para EX AF,AF'
};

typedef struct s_tabla_ensamblado tabla_ensamblado;

//char *ensamblado_opcode_nop="NOP";
//char *ensabmlado_opcode_ld="LD";

tabla_ensamblado array_tabla_ensamblado[]={
        {"NOP",0,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},
	{"LD",1,0,    ASM_PARM_RP,4,NULL, ASM_PARM_NN, 0,NULL},   //LD rp,NN        
        {"LD",2,0,  ASM_PARM_CONST,0,"(BC)", ASM_PARM_CONST, 0,"A"},   //LD (BC),A
        {"INC",3,0,    ASM_PARM_RP,4,NULL, ASM_PARM_NONE, 0,NULL},   //INC rp        
        {"INC",4,0,  ASM_PARM_R,4,NULL,  ASM_PARM_NONE,  0,NULL},   //INC r
        {"DEC",5,0,  ASM_PARM_R,4,NULL,  ASM_PARM_NONE,  0,NULL},   //DEC r
	{"LD",6,0,  ASM_PARM_R,3,NULL,  ASM_PARM_N,  0,NULL},   //LD r,n        
        {"RLCA",7,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},
        {"EX",8,0,  ASM_PARM_CONST,0,"AF", ASM_PARM_CONST, 0,"AF'"},   //EX AF,AF'
        {"ADD",9,0, ASM_PARM_CONST,0,"HL", ASM_PARM_RP,4,NULL}, //ADD HL,rp
        {"LD",10,0,  ASM_PARM_CONST,0,"A", ASM_PARM_CONST, 0,"BC"},   //LD A,(BC)

        {"DEC",12,0,    ASM_PARM_RP,4,NULL, ASM_PARM_NONE, 0,NULL},   //DEC rp

	{"LD",64,0, ASM_PARM_R,3,NULL,  ASM_PARM_R,  0,NULL},   //LD r,r        

	{"PUSH",197,0,  ASM_PARM_RP2,4,NULL, ASM_PARM_NONE, 0,NULL},   //PUSH rp2        

        {"EXX",217,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL},        

	{"LD",33,221,  ASM_PARM_CONST,4,"IX", ASM_PARM_NN, 0,NULL},   //LD IX,NN
	{"LD",33,253,  ASM_PARM_CONST,4,"IY", ASM_PARM_NN, 0,NULL},   //LD IY,NN





        {NULL,0,0, ASM_PARM_NONE,0,NULL, ASM_PARM_NONE,0,NULL}
};

char *asm_parametros_tipo_r[]={
	"B","C","D","E","H","L","(HL)","A",
	NULL
};

char *asm_parametros_tipo_rp[]={
	"BC","DE","HL","SP",
	NULL
};

char *asm_parametros_tipo_rp2[]={
	"BC","DE","HL","AF",
	NULL
};

//Buscar en el array de char* de parametros si coincide y su valor
char *assemble_find_array_params(char *texto_buscar,char *tabla[],int *valor)
{
	int i;

	for (i=0;tabla[i]!=NULL;i++) {
		if (!strcasecmp(texto_buscar,tabla[i])) {
			*valor=i;
			return tabla[i];
		}
	}

	return NULL;
}




//Devuelve tipo de parametro en entrada, de enum asm_tipo_parametro_entrada
//Si no es de ninguna tabla, retornara numero (N o NN)
int assemble_find_param_type(char *buf_op,int *valor)
{
	char *tabla;

        //Buscar primero en tabla R
        tabla=assemble_find_array_params(buf_op,asm_parametros_tipo_r,valor);
	if (tabla!=NULL) return ASM_PARM_IN_R;

        tabla=assemble_find_array_params(buf_op,asm_parametros_tipo_rp,valor);
	if (tabla!=NULL) return ASM_PARM_IN_RP;

        tabla=assemble_find_array_params(buf_op,asm_parametros_tipo_rp2,valor);
	if (tabla!=NULL) return ASM_PARM_IN_RP2;


	//final
	return ASM_PARM_IN_NUMERO;
}


//Si coincide el tipo de parametro de entrada con el de la tabla
int asm_check_parameter_in_table(enum asm_tipo_parametro_entrada tipo_parametro_entrada,enum asm_tipo_parametro tipo_en_tabla)
{

	//Para cada caso
	switch (tipo_parametro_entrada) {
		case ASM_PARM_IN_R:
			if (tipo_en_tabla==ASM_PARM_R) return 1;
			else return 0;
		break;

		case ASM_PARM_IN_RP:
		case ASM_PARM_IN_RP2:
			//TODO: controlar AF y SP. En caso de RP, no permite AF. En caso de RP2, no permite SP
			if (tipo_en_tabla==ASM_PARM_RP || tipo_en_tabla==ASM_PARM_RP2) return 1;
			else return 0;
		break;

		case ASM_PARM_IN_NUMERO:
			if (tipo_en_tabla==ASM_PARM_N || tipo_en_tabla==ASM_PARM_NN || tipo_en_tabla==ASM_PARM_DIS) return 1;
			else return 0;
		break;

	}

	return 0;

}



//Ensamblado de instruccion.
//Retorna longitud de opcode. 0 si error
//Lo ensambla en puntero indicado destino
//Maximo 255 bytes 
int assemble_opcode(char *texto,z80_byte *destino)
{

        printf ("\n\nAssemble %s\n",texto);

        int longitud_instruccion=0;
        //Parsear opcode y parametros

	char buf_opcode[256];
	char buf_primer_op[256];
	char buf_segundo_op[256];

        asm_return_op_ops(texto,buf_opcode,buf_primer_op,buf_segundo_op);

        int parametros_entrada=0;
        if (buf_segundo_op[0]!=0) parametros_entrada++;
        if (buf_primer_op[0]!=0) parametros_entrada++;       

        //Aqui tenemos ya el numero de parametros

        //TODO: tipo de parametros de la instruccion. Tener en cuenta que algunos pueden ser n y nn a la vez, o rp y rp2 a la vez, etc
	//Si coincide con algun tipo de parametro conocido, y si no, se trata como numero

	if (parametros_entrada) {
		//TODO
	}

        //Recorrer array de ensamblado
        int i;

	int salir=0;
	int encontrado_indice=-1;


	int valor_parametro_1=0; //Valor del parametro 1 cuando no es N ni NN
	int valor_parametro_2=0; //Valor del parametro 2 cuando no es N ni NN

        for (i=0;array_tabla_ensamblado[i].texto_opcode!=NULL && !salir;i++) {
                printf ("%s\n",array_tabla_ensamblado[i].texto_opcode);
                if (!strcasecmp(buf_opcode,array_tabla_ensamblado[i].texto_opcode)) {
                        printf ("Match opcode\n");
                        //TODO: ver si hace match numero parametros y tipo
	
			//Contar numero de parametros en array
			int parametros_array=0;

			if (array_tabla_ensamblado[i].tipo_parametro_1!=ASM_PARM_NONE) parametros_array++;
			if (array_tabla_ensamblado[i].tipo_parametro_2!=ASM_PARM_NONE) parametros_array++;

			if (parametros_array==parametros_entrada) {
				//comprobar los tipos de parametros que coincidan
				if (array_tabla_ensamblado[i].tipo_parametro_1!=ASM_PARM_NONE) {

					//Parametros de tipo constante
					if (array_tabla_ensamblado[i].tipo_parametro_1==ASM_PARM_CONST) {
						if (strcmp(buf_primer_op,array_tabla_ensamblado[i].const_parm1)) {
							printf ("No coincide tipo const\n");
							continue;
						}
					}

					else {
		
						enum asm_tipo_parametro_entrada tipo_parametro_entrada_1;

						tipo_parametro_entrada_1=assemble_find_param_type(buf_primer_op,&valor_parametro_1);
				
						//Que coincidan los tipos
						if (!asm_check_parameter_in_table(tipo_parametro_entrada_1,array_tabla_ensamblado[i].tipo_parametro_1)) {
							printf ("No coinciden los tipos en parm1\n");
							continue;
						}
					}
				}


				if (array_tabla_ensamblado[i].tipo_parametro_2!=ASM_PARM_NONE) {
					//Parametros de tipo constante
                                        if (array_tabla_ensamblado[i].tipo_parametro_2==ASM_PARM_CONST) {
						if (strcmp(buf_segundo_op,array_tabla_ensamblado[i].const_parm2)) {
                                                        printf ("No coincide tipo const\n");
                                                        continue;
                                                }
                                        }

					else {

						enum asm_tipo_parametro_entrada tipo_parametro_entrada_2;

        	                                tipo_parametro_entrada_2=assemble_find_param_type(buf_segundo_op,&valor_parametro_2);

                	                        //Que coincidan los tipos
                        	                if (!asm_check_parameter_in_table(tipo_parametro_entrada_2,array_tabla_ensamblado[i].tipo_parametro_2)) {
                                	                printf ("No coinciden los tipos en parm2\n");
                                                	continue;
                                        	}
					}
				}

                        	printf ("Match opcode and parameters type\n");

				longitud_instruccion=1;


				//Salir del bucle 
                        	salir=1;
				encontrado_indice=i;
			}
                }
        }

        printf ("Indice: %d\n\n",encontrado_indice);

        if (encontrado_indice==-1) {
                printf ("No match\n");
        }

	else {


		longitud_instruccion=1;

		//Ensamblarlo
		if (array_tabla_ensamblado[encontrado_indice].prefijo) {
			*destino=array_tabla_ensamblado[encontrado_indice].prefijo;
			destino++;
			longitud_instruccion++;		
		}

		//TODO meter base opcode y mascara parametros
		z80_byte opcode_final=array_tabla_ensamblado[encontrado_indice].mascara_opcode;

		if (array_tabla_ensamblado[encontrado_indice].tipo_parametro_1!=ASM_PARM_NONE) {
			if (array_tabla_ensamblado[encontrado_indice].tipo_parametro_1==ASM_PARM_N || array_tabla_ensamblado[encontrado_indice].tipo_parametro_1==ASM_PARM_NN) {
				//Parseamos valor
				unsigned int valor_parametro_1=parse_string_to_number(buf_primer_op);

				//Si es N
				if (array_tabla_ensamblado[encontrado_indice].tipo_parametro_1==ASM_PARM_N) {
					//En destino+1
					destino[1]=valor_parametro_1 & 0xFF;
					longitud_instruccion++;
				}

				//Si es NN
				if (array_tabla_ensamblado[encontrado_indice].tipo_parametro_1==ASM_PARM_NN) {
					//En destino+1
					destino[1]=valor_parametro_1 & 0xFF;
					destino[2]=(valor_parametro_1>>8) & 0xFF;
					longitud_instruccion+=2;
				}
			}

			else {
				//Aqui se entra tambien cuando tipo es CONST. Pero como valor y desplazamiento valen 0, es un OR de 0
				valor_parametro_1 <<= array_tabla_ensamblado[encontrado_indice].desplazamiento_mascara_p1;
				opcode_final |= valor_parametro_1;

				
			}
		}

                if (array_tabla_ensamblado[encontrado_indice].tipo_parametro_2!=ASM_PARM_NONE) {
                        if (array_tabla_ensamblado[encontrado_indice].tipo_parametro_2==ASM_PARM_N || array_tabla_ensamblado[encontrado_indice].tipo_parametro_2==ASM_PARM_NN) {
				//Parseamos valor
				unsigned int valor_parametro_2=parse_string_to_number(buf_segundo_op);

				//Si es N
                                if (array_tabla_ensamblado[encontrado_indice].tipo_parametro_2==ASM_PARM_N) {
                                        //En destino+1
                                        destino[1]=valor_parametro_2 & 0xFF;
                                        longitud_instruccion++;
                                }

                                //Si es NN
                                if (array_tabla_ensamblado[encontrado_indice].tipo_parametro_2==ASM_PARM_NN) {
                                        //En destino+1
                                        destino[1]=valor_parametro_2 & 0xFF;
                                        destino[2]=(valor_parametro_2>>8) & 0xFF;
                                        longitud_instruccion+=2;
                                }

                        }

                        else {
				//Aqui se entra tambien cuando tipo es CONST. Pero como valor y desplazamiento valen 0, es un OR de 0
				valor_parametro_2 <<= array_tabla_ensamblado[encontrado_indice].desplazamiento_mascara_p2;
                                opcode_final |= valor_parametro_2;
                        }
                }


		*destino=opcode_final;


		//TODO Sumar longitud segun parametros
	}
        return longitud_instruccion;

}
