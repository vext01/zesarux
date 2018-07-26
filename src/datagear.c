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

#include "datagear.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "ula.h"



//Si esta recibiendo parametros de comando.
//Si no es 0, indica cuantos parámetros le quedan por recibir
//int datagear_receiving_parameters=0;

//Mascara de los parametros a leer. Por ejemplo si WR0 enviara los 4 parametros, tendra valor 00001111b
z80_byte datagear_mask_commands=0;

//Indice al numero de parametro leido
int datagear_command_index;
//Ejemplo, en WR0 vale 0 cuando vamos a leer el primer parametro (Port A starting address Low byte), vale 1 cuando vamos a leer Port A adress high byte

//Indica ultimo comando leido, tal cual el primer byte
//z80_byte datagear_last_command_byte;


//Indica ultimo comando leido, 0=WR0, 1=WR1, etc
z80_byte datagear_last_command;

z80_byte datagear_port_a_start_addr_low;
z80_byte datagear_port_a_start_addr_high;

z80_byte datagear_port_b_start_addr_low;
z80_byte datagear_port_b_start_addr_high;

z80_byte datagear_block_length_low;
z80_byte datagear_block_length_high;

z80_byte datagear_port_a_variable_timing_byte;
z80_byte datagear_port_b_variable_timing_byte;

//Ultimo valor recibido para los registros
z80_byte datagear_wr0;
z80_byte datagear_wr1;
z80_byte datagear_wr2;
z80_byte datagear_wr3;
z80_byte datagear_wr4;
z80_byte datagear_wr5;
z80_byte datagear_wr6;

z80_bit datagear_dma_emulation={0};

void datagear_dma_disable(void)
{
    datagear_dma_emulation.v=0;
}

void datagear_dma_enable(void)
{
    datagear_dma_emulation.v=1;
}


void datagear_reset(void)
{
    datagear_mask_commands=0;

    datagear_wr0=datagear_wr1=datagear_wr2=datagear_wr3=datagear_wr4=datagear_wr5=datagear_wr6=0;

}

void datagear_write_value(z80_byte value)
{
	//gestionar si estamos esperando parametros de comando
	if (datagear_mask_commands) {
		switch (datagear_last_command) {

			//WR0
			case 0:
				//si parametro de indice actual se salta porque mascara vale 0 en bit bajo
				while ( (datagear_mask_commands&1)==0) {
					datagear_mask_commands=datagear_mask_commands >> 1;
					datagear_command_index++;
				}

				//Aqui tenemos, en datagear_command_index, el parametro que vamos a leer ahora:
				//0=Port A starting address Low byte
				//1=Port A starting address High byte
				//2=Block length low byte
				//3=Block length high byte
				switch (datagear_command_index) {
					case 0:
						datagear_port_a_start_addr_low=value;
						printf ("Setting port a start address low to %02XH\n",value);
					break;

					case 1:
						datagear_port_a_start_addr_high=value;
						printf ("Setting port a start address high to %02XH\n",value);
					break;					

					case 2:
						datagear_block_length_low=value;
						printf ("Setting block length low to %02XH\n",value);
					break;

					case 3:
						datagear_block_length_high=value;
						printf ("Setting block length high to %02XH\n",value);
					break;

				}

				datagear_mask_commands=datagear_mask_commands >> 1;
				datagear_command_index++;

			break;

			case 1:
				//si parametro de indice actual se salta porque mascara vale 0 en bit bajo
				while ( (datagear_mask_commands&1)==0) {
					datagear_mask_commands=datagear_mask_commands >> 1;
					datagear_command_index++;
				}

				//Aqui tenemos, en datagear_command_index, el parametro que vamos a leer ahora:
				//0=Port A variable timing byte

				switch (datagear_command_index) {
					case 0:
						datagear_port_a_variable_timing_byte=value;
						printf ("Setting port a variable timing byte to %02XH\n",value);
					break;

				}

				datagear_mask_commands=datagear_mask_commands >> 1;
				datagear_command_index++;			
			break;

			case 2:
				//si parametro de indice actual se salta porque mascara vale 0 en bit bajo
				while ( (datagear_mask_commands&1)==0) {
					datagear_mask_commands=datagear_mask_commands >> 1;
					datagear_command_index++;
				}

				//Aqui tenemos, en datagear_command_index, el parametro que vamos a leer ahora:
				//0=Port B variable timing byte

				switch (datagear_command_index) {
					case 0:
						datagear_port_b_variable_timing_byte=value;
						printf ("Setting port b variable timing byte to %02XH\n",value);
					break;

				}

				datagear_mask_commands=datagear_mask_commands >> 1;
				datagear_command_index++;	            
			break;

			case 3:
			break;

			case 4:
				//si parametro de indice actual se salta porque mascara vale 0 en bit bajo
				while ( (datagear_mask_commands&1)==0) {
					datagear_mask_commands=datagear_mask_commands >> 1;
					datagear_command_index++;
				}

				//Aqui tenemos, en datagear_command_index, el parametro que vamos a leer ahora:
				//0=Port B starting address Low byte
				//1=Port B starting address High byte
				switch (datagear_command_index) {
					case 0:
						datagear_port_b_start_addr_low=value;
						printf ("Setting port b start address low to %02XH\n",value);
					break;

					case 1:
						datagear_port_b_start_addr_high=value;
						printf ("Setting port b start address high to %02XH\n",value);
					break;					

				}

				datagear_mask_commands=datagear_mask_commands >> 1;
				datagear_command_index++;				
			break;

			case 5:
			break;

			case 6:
			break;

		}

	}

	else {

		//datagear_last_command_byte=value;
		datagear_command_index=0;
	//Obtener tipo de comando
	//SI WR0

	z80_byte value_mask_wr0_wr3=value&(128+2+1);
	if (value_mask_wr0_wr3==1 || value_mask_wr0_wr3==2 ||value_mask_wr0_wr3==3 ) {
		printf ("WR0\n");
		datagear_last_command=0;
		datagear_wr0=value;

		//Ver bits 4,5,6,7 y longitud comando
/*
#  D7  D6  D5  D4  D3  D2  D1  D0  PORT A STARTING ADDRESS (LOW BYTE)
#       |   |   V
#  D7  D6  D5  D4  D3  D2  D1  D0  PORT A STARTING ADDRESS (HIGH BYTE)
#       |   V
#  D7  D6  D5  D4  D3  D2  D1  D0  BLOCK LENGTH (LOW BYTE)
#       V
#  D7  D6  D5  D4  D3  D2  D1  D0  BLOCK LENGTH (HIGH BYTE)
*/		

		datagear_mask_commands=(value>>3)&15;

		z80_byte transfer_type=value&3;
		if (transfer_type==1) printf ("Type: transfer\n");
		else if (transfer_type==2) printf ("Type: search\n");
		else if (transfer_type==3) printf ("Type: search/transfer\n");

		if (value&4) printf ("Port A -> Port B\n");
		else printf ("Port B -> Port A\n");


	}

	if (value_mask_wr0_wr3==128) {
		printf ("WR3\n");
		datagear_last_command=3;
		datagear_wr3=value;
	}

	if (value_mask_wr0_wr3==129) {
		printf ("WR4\n");
		datagear_last_command=4;
		datagear_wr4=value;

		//TODO. Bit D4 diferente de 0. En next, D4 siempre es 0
		datagear_mask_commands=(value>>2)&3;



	}	

	if (value_mask_wr0_wr3==128+2+1) {
		printf ("WR6\n");
		datagear_last_command=6;
		datagear_wr6=value;

		//Tratar todos los diferentes comandos
		switch (value) {
			case 0xCF:
				printf ("Load starting address for both ports, clear byte counter\n");
			break;

			case 0xAB:
				printf ("Enable interrupts\n");
			break;

			case 0x87:
				printf ("Enable DMA\n");
				//Prueba rapida de transferencia DMA
				z80_int transfer_length=value_8_to_16(datagear_block_length_high,datagear_block_length_low);
				z80_int transfer_port_a,transfer_port_b;

				//if (datagear_wr0 & 4) 	{
					//printf ("Port A -> Port B\n");
					transfer_port_a=value_8_to_16(datagear_port_a_start_addr_high,datagear_port_a_start_addr_low);
					transfer_port_b=value_8_to_16(datagear_port_b_start_addr_high,datagear_port_b_start_addr_low);
				//}
				/*else {
					printf ("Port B -> Port A\n");			
					transfer_source=value_8_to_16(datagear_port_b_start_addr_high,datagear_port_b_start_addr_low);
					transfer_destination=value_8_to_16(datagear_port_a_start_addr_high,datagear_port_a_start_addr_low);
				}*/					

				if (datagear_wr0 & 4) printf ("Copying %d bytes from %04XH to %04XH\n",transfer_length,transfer_port_a,transfer_port_b);
                else printf ("Copying %d bytes from %04XH to %04XH\n",transfer_length,transfer_port_b,transfer_port_a);

                if (datagear_wr1 & 8) printf ("Port A I/O. not implemented yet\n");
                if (datagear_wr2 & 8) printf ("Port B I/O. not implemented yet\n");

				while (transfer_length) {
                    z80_byte byte_leido;
                    if (datagear_wr0 & 4) {
                        byte_leido=peek_byte_no_time(transfer_port_a);
					    poke_byte_no_time(transfer_port_b,byte_leido);
                    }

                    else {
                        byte_leido=peek_byte_no_time(transfer_port_b);
					    poke_byte_no_time(transfer_port_a,byte_leido);                        
                    }

                    if ( (datagear_wr1 & 32) == 0 ) {
                        if (datagear_wr1 & 16) transfer_port_a++;
                        else transfer_port_a--;
                    }

                    if ( (datagear_wr2 & 32) == 0 ) {
                        if (datagear_wr2 & 16) transfer_port_b++;
                        else transfer_port_b--;
                    }                    

					transfer_length--;
				}


			break;		
			
			case 0xB3:
				printf ("Force an internal ready condition independent 'on the rdy' input\n");
			break;				

			case 0xB7:
				printf ("Enable after RETI so dma requests bus only after receiving a reti\n");
			break;


		}		


	}	

	z80_byte value_mask_wr1_wr2=value&(128+4+2+1);
	if (value_mask_wr1_wr2==4) {
		printf ("WR1\n");
		datagear_last_command=1;
		datagear_wr1=value;

		//Ver bits D6
        //D6 Port A variable timing byte
	
		datagear_mask_commands=(value>>6)&1;

	}

	if (value_mask_wr1_wr2==0) {
		printf ("WR2\n");
		datagear_last_command=2;
		datagear_wr2=value;

		//Ver bits D6
        //D6 Port B variable timing byte
	
		datagear_mask_commands=(value>>6)&1;        
	}

	z80_byte value_mask_wr5=value&(128+64+4+2+1);
	if (value_mask_wr5==128+2) {
		printf ("WR5\n");
		datagear_last_command=5;
		datagear_wr5=value;
	}

	}
}