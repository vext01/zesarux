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

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif


#include "codetests.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "ula.h"
#include "mem128.h"
#include "screen.h"
#include "tbblue.h"

#include "disassemble.h"


void codetests_repetitions(void)
{

	z80_byte repetitions0[]={1,2,3,4,5,6,7,8,9,10};
	z80_byte repetitions1[]={1,1,3,4,5,6,7,8,9,10};
	z80_byte repetitions2[]={1,1,1,4,5,6,7,8,9,10};
	z80_byte repetitions3[]={1,1,1,1,5,6,7,8,9,10};
	z80_byte repetitions4[]={1,1,1,1,1,6,7,8,9,10};

	//int util_get_byte_repetitions(z80_byte *memoria,int longitud,z80_byte *byte_repetido)

	int repeticiones[5];
	z80_byte byte_repetido[5];

	int i;
	z80_byte *puntero=NULL;

	for (i=0;i<5;i++) {
		if      (i==0) puntero=repetitions0;
		else if (i==1) puntero=repetitions1;
		else if (i==2) puntero=repetitions2;
		else if (i==3) puntero=repetitions3;
		else if (i==4) puntero=repetitions4;

		repeticiones[i]=util_get_byte_repetitions(puntero,10,&byte_repetido[i]);

		printf ("step %d repetitions: %d byte_repeated: %d\n",i,repeticiones[i],byte_repetido[i]);

		//Validar cantidad de valores repetidos y que byte repetido
		printf ("expected: repetitions: %d byte_repeated: 1\n",i+1);
		if (byte_repetido[i]!=1 || repeticiones[i]!=i+1) {
			printf ("error\n");
			exit(1);
		}
	}
}

void coretests_dumphex(z80_byte *ptr,int longitud)
{
	while (longitud) {
		printf ("%02X ",*ptr);
		ptr++;
		longitud--;
	}
}



//mostrar unos cuantos del inicio y del final
void coretests_dumphex_inicio_fin(z80_byte *ptr,int longitud,int max_mostrar)
{

	int mostrar;
	int cortado=0;
	if (longitud>max_mostrar*2) {
		mostrar=max_mostrar;
		cortado=1;
	}
	else mostrar=longitud;

	coretests_dumphex(ptr,mostrar);


	if (cortado) {
		printf (" ... ");
		coretests_dumphex(ptr+longitud-mostrar,mostrar);
	}

}

void coretests_compress_repetitions_write_arr(z80_byte *variable,int indice,z80_byte valor,int size_array)
{
	//El bucle que llama aqui se sale de array, tanto por arriba como por abajo, por eso lo controlo y no escribo en ese caso
	if (indice<0 || indice>=size_array) {
		//printf ("Out of array: %d\n",indice);
		return;
	}
	variable[indice]=valor;
}

void coretests_compress_repetitions(void)
{


#define MAX_COMP_REP_ARRAY 2048

	z80_byte repetitions[MAX_COMP_REP_ARRAY]; 

	z80_byte compressed_data[MAX_COMP_REP_ARRAY*2];

	int max_array=MAX_COMP_REP_ARRAY; //siempre menor o igual que MAX_COMP_REP_ARRAY. tamanyo de los datos a analizar

    int i;

	int max_veces=MAX_COMP_REP_ARRAY; //Siempre menor o igual que MAX_COMP_REP_ARRAY. cuantos bytes repetimos

	z80_byte magic_byte=0xDD;

    for (i=0;i<=max_veces;i++) {

		int j;

		//Inicializar con valores consecutivos
		printf ("Initializing with consecutive values\n");
		for (j=0;j<max_array;j++) {
			//repetitions[j]=j&255;
			coretests_compress_repetitions_write_arr(repetitions,j,j&255,MAX_COMP_REP_ARRAY);
		}

		printf ("Initializing with 0 from the left\n");
		//Meter valores "0" al principio
		for (j=0;j<=i;j++) {
			//repetitions[j]=0;
			coretests_compress_repetitions_write_arr(repetitions,j,0,MAX_COMP_REP_ARRAY);
		}

		printf ("Initializing with 1 from the right\n");
		//Meter valores "1" al final 
		for (j=0;j<=i;j++) {
			coretests_compress_repetitions_write_arr(repetitions,max_array-1-j,1,MAX_COMP_REP_ARRAY);
			//repetitions[max_array-1-j]=1;
		}

                //repeticiones[i]=util_get_byte_repetitions(puntero,10,&byte_repetido[i]);
		printf ("step %d length: %d. 0's at beginning: %d. 1's at end: %d\n",i,max_array,i+1,i+1);

		coretests_dumphex_inicio_fin(repetitions,max_array,20);

		printf ("\n");

		int longitud_destino=util_compress_data_repetitions(repetitions,compressed_data,max_array,magic_byte);

		printf ("compressed length: %d\n",longitud_destino);

		//coretests_dumphex(compressed_data,longitud_destino);
		coretests_dumphex_inicio_fin(compressed_data,longitud_destino,20);
		printf ("\n");



		//Validar, pero solo para iteraciones < 256. mas alla de ahi, dificil de calcular
		int limite=256-4-magic_byte;
		//A partir de 33 con magic_byte=0xDD falla el calculo porque hay:
		//D8 D9 DA DB DC DD 1 1 1 1 1 1 1 1 ....... Ese DD aislado hay que escaparlo como 1 sola repeticion
		//que se traduce en:
		//D8 D9 DA DB DC DD DD DD 01 DD DD 01 22 

		if (i<limite) {
			//Validacion solo de longitud comprimida. El contenido, hacer una validacion manual
			int valor_esperado_comprimido=max_array;
			if (i>3) valor_esperado_comprimido=max_array-(i-3)*2;

			printf ("Expected length: %d\n",valor_esperado_comprimido);

			if (valor_esperado_comprimido!=longitud_destino) {
                	        printf ("error\n");
                        	exit(1);
	                }
		}

		printf ("\n");
    }


}

void coretests_read_file_memory(char *filename,z80_byte *memoria)
{
		long int tamanyo;
		tamanyo=get_file_size(filename);


                FILE *ptr_file;
                ptr_file=fopen(filename,"rb");

                if (!ptr_file) {
                        printf ("Unable to open file %s",filename);
                        exit(1);
                }




                fread(memoria,1,tamanyo,ptr_file);


                fclose(ptr_file);
}

void coretests_compress_uncompress_repetitions_aux(char *filename)
{
	z80_byte *memoria_file_orig;
	z80_byte *memoria_file_compressed;
	z80_byte *memoria_file_uncompressed;

	long int tamanyo=get_file_size(filename);

	//Memoria para lectura, comprimir y descomprimir
	//tamanyo, tamanyo*2, tamanyo*2

	memoria_file_orig=malloc(tamanyo);
	if (memoria_file_orig==NULL) {
		printf("Error allocating memory\n");
		exit(1);
	}

        memoria_file_compressed=malloc(tamanyo*2);
        if (memoria_file_compressed==NULL) {
                printf("Error allocating memory\n");
                exit(1);
        }

        memoria_file_uncompressed=malloc(tamanyo*2);
        if (memoria_file_uncompressed==NULL) {
                printf("Error allocating memory\n");
                exit(1);
        }

	coretests_read_file_memory(filename,memoria_file_orig);

/*
extern int util_compress_data_repetitions(z80_byte *origen,z80_byte *destino,int longitud,z80_byte magic_byte);

extern int util_uncompress_data_repetitions(z80_byte *origen,z80_byte *destino,int longitud,z80_byte magic_byte);
*/

	z80_byte magic_byte=0xDD;

	printf ("Original size: %ld\n",tamanyo);

	int longitud_comprimido=util_compress_data_repetitions(memoria_file_orig,memoria_file_compressed,tamanyo,magic_byte);
	int porcentaje;
	if (tamanyo==0) porcentaje=100;
	else porcentaje=(longitud_comprimido*100)/tamanyo;
	printf ("Compressed size: %d (%d%%)\n",longitud_comprimido,porcentaje);

	int longitud_descomprido=util_uncompress_data_repetitions(memoria_file_compressed,memoria_file_uncompressed,longitud_comprimido,magic_byte);
	printf ("Uncompressed size: %d\n",longitud_descomprido);

	int error=0;

	//Primera comprobacion de tamanyo
	if (tamanyo!=longitud_descomprido) {
		printf ("Original size and uncompressed size doesnt match\n");
		error=1;
	}

	//Y luego comparar byte a byte
	int i;
	for (i=0;i<tamanyo;i++) {
		z80_byte byte_orig,byte_uncompress;
		byte_orig=memoria_file_orig[i];
		byte_uncompress=memoria_file_uncompressed[i];
		if (byte_orig!=byte_uncompress) {
			printf("Difference in offset %XH. Original byte: %02XH Uncompressed byte: %02XH\n",i,byte_orig,byte_uncompress);
			error++;
		}

		if (error>=10) {
			printf ("And more errors.... showing only first 10\n");
			exit(1);
		}
	}
	

	if (error) {
		exit(1);
	}

	else {
		printf ("Compress/Uncompress ok\n");
	}

}

void coretests_compress_uncompress_repetitions(char *archivo)
{
	printf ("Testing compression routine with file %s\n",archivo);
	coretests_compress_uncompress_repetitions_aux(archivo);
}

void codetests_tbblue_get_horizontal_raster(void)
{


	screen_testados_linea=224;

	int i;
	for (i=0;i<69888;i++) {
		t_estados=i;
		int estados_en_linea=t_estados % screen_testados_linea;
		int linea=t_estados/screen_testados_linea;
		int horiz=tbblue_get_current_raster_horiz_position();

		printf ("t-total %5d line %3d t_states %3d. horiz: %3d\n",i,linea,estados_en_linea,horiz );
		if (horiz!=estados_en_linea/4) {
			printf ("Error\n");
			exit(1);
		}
	}
}

/*void codetests_cut_line(void)
{

	//	extern void menu_util_cut_line_at_spaces(int posicion_corte, char *texto,char *linea1, char *linea2);

	char linea1[200];
	char linea2[200];

	char *entrada="Hola como estas yo bien y tu";

	int corte;

	for (corte=0;corte<30;corte++) {
		menu_util_cut_line_at_spaces(corte,entrada,linea1,linea2);
		printf ("\nEntrada: [%s]\nCorte en %d\nLinea 1: [%s]\nLinea 2: [%s]\n",entrada,corte,linea1,linea2);
	}

	exit(0);


}*/



void codetests_tbblue_layers(void)
{
//tbblue_get_string_layer_prio
//+extern char *tbblue_get_string_layer_prio(int layer,z80_byte prio);


	int layer, prio;

	for (prio=0;prio<8;prio++) {
		printf ("Priority %d\n",prio);
		for (layer=0;layer<3;layer++) {
			printf ("Layer %d : %s\n",layer,tbblue_get_string_layer_prio(layer,prio));
		}
	}
}

void codetests_assembler_print(char *s1,char *s2,char *s3, char *s4)
{
	printf ("%s\nOpcode: [%s]\nFirst op: [%s]\nSecond op: [%s]\n\n",s1,s2,s3,s4);
}

void codetests_assemble_opcode(char *instruccion,z80_byte *destino)
{
	int longitud=assemble_opcode(instruccion,destino);
        printf ("Longitud opcode: %d\n",longitud);
	if (longitud) {
		printf ("Codigo generado: ");
	}

	while (longitud) {
		printf ("%02XH ",*destino);
		destino++;
		longitud--;
	};

	if (longitud) {
                printf ("\n");
	}
}

void codetests_assembler(void)
{
	//void util_asm_return_op_ops(char *origen,char *opcode,char *primer_op,char *segundo_op)

	char buf_opcode[100];
	char buf_primer_op[100];
	char buf_segundo_op[100];

	util_asm_return_op_ops("NOP",buf_opcode,buf_primer_op,buf_segundo_op);
	codetests_assembler_print("NOP",buf_opcode,buf_primer_op,buf_segundo_op);

	util_asm_return_op_ops("PUSH AF",buf_opcode,buf_primer_op,buf_segundo_op);
	codetests_assembler_print("PUSH AF",buf_opcode,buf_primer_op,buf_segundo_op);

	util_asm_return_op_ops("EX DE,HL",buf_opcode,buf_primer_op,buf_segundo_op);
	codetests_assembler_print("EX DE,HL",buf_opcode,buf_primer_op,buf_segundo_op);

	util_asm_return_op_ops("PUSH   AF",buf_opcode,buf_primer_op,buf_segundo_op);
	codetests_assembler_print("PUSH   AF",buf_opcode,buf_primer_op,buf_segundo_op);

	util_asm_return_op_ops("EX     DE,HL   ",buf_opcode,buf_primer_op,buf_segundo_op);
	codetests_assembler_print("EX     DE,HL   ",buf_opcode,buf_primer_op,buf_segundo_op);


	printf ("Assembling\n");

	//int assemble_opcode(char *texto,z80_byte *destino)
	z80_byte destino_ensamblado[256];

	codetests_assemble_opcode("NOP",destino_ensamblado);

	codetests_assemble_opcode("NOP 33",destino_ensamblado);

	codetests_assemble_opcode("LD A,2",destino_ensamblado);
	codetests_assemble_opcode("LD B,B",destino_ensamblado);
	codetests_assemble_opcode("LD D,A",destino_ensamblado);
	codetests_assemble_opcode("LD (HL),(HL)",destino_ensamblado); //TODO Incorrecto. debe salir error

	codetests_assemble_opcode("LD BC,2",destino_ensamblado);
	codetests_assemble_opcode("LD HL,260",destino_ensamblado);
	codetests_assemble_opcode("LD IX,260",destino_ensamblado);
	codetests_assemble_opcode("LD IY,260",destino_ensamblado);

	codetests_assemble_opcode("EXX",destino_ensamblado);
	codetests_assemble_opcode("EX AF,AF'",destino_ensamblado);
	codetests_assemble_opcode("EX AF,BC",destino_ensamblado);
	codetests_assemble_opcode("PUSH DE",destino_ensamblado);
	codetests_assemble_opcode("PUSH DE,AF",destino_ensamblado);


	//Prueba ensamblando todas instrucciones

	//Primero sin opcode
	int i;

	z80_byte origen_ensamblado[256];


	char texto_desensamblado[256];

	for (i=0;i<256;i++) {
		//Primero metemos 4 bytes y desensamblamos
		//Evitar opcode
		if (i==203 || i==221 || i==237 || i==253) continue;

		//Metemos el primer byte con ese valor y 3 mas de relleno
		disassemble_array[0]=i;

		disassemble_array[1]=0; //0x3e;
		disassemble_array[2]=0; //0x6e;
		disassemble_array[3]=0; //0xab;

		//Desensamblamos
		size_t longitud_opcode_desensamblado;
		debugger_disassemble_array(texto_desensamblado,255,&longitud_opcode_desensamblado,0);

		printf ("Ensamblando Opcode %d : %s\n",i,texto_desensamblado);

		//Ensamblar
		int longitud_destino=assemble_opcode(texto_desensamblado,destino_ensamblado);

		if (longitud_destino==0) {
			printf ("Error longitud=0\n");
			return;
		}

		else {
			printf ("OK. Dump original and destination:\n");
			int j;
			for (j=0;j<longitud_opcode_desensamblado;j++) {
				z80_byte byte_origen=disassemble_array[j];
				z80_byte byte_destino=destino_ensamblado[j];
				printf ("orig: %02XH dest: %02XH .  ",byte_origen,byte_destino);
				if (byte_origen!=byte_destino) {
					printf ("\nDo not match bytes\n");
					return;
				}
			}

			printf ("\n");
		}
		

		sleep(1);
	}

		

}

void codetests_main(int main_argc,char *main_argv[])
{

	if (main_argc>2) {
		printf ("\nRunning compress/uncompress repetitions code\n");
		coretests_compress_uncompress_repetitions(main_argv[2]);
		exit(0);
	}

	printf ("\nRunning assembler tests\n");
	codetests_assembler();


	//printf ("\nRunning tbblue layers strings\n");
	//codetests_tbblue_layers();

	printf ("\nRunning repetitions code\n");
	//codetests_repetitions();

	printf ("\nRunning compress repetitions code\n");
	//coretests_compress_repetitions();

	printf ("\nRunning get raster tbblue horizontal\n");
	//codetests_tbblue_get_horizontal_raster();

	exit(0);
}



