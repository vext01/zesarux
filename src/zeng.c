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
#include <string.h>
#include <fcntl.h>



#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "network.h"
#include "compileoptions.h"
#include "zeng.h"
#include "remote.h"



#ifdef USE_PTHREADS

#include <pthread.h>
#include <sys/types.h>


pthread_t thread_zeng;

#endif

//Si el thread se ha inicializado correctamente
z80_bit thread_zeng_inicializado={0};



//-ZENG: ZEsarUX Network Gaming



//La fifo

zeng_key_presses zeng_key_presses_array[ZENG_FIFO_SIZE];

int zeng_remote_socket;


//Tamanyo de la fifo
int zeng_fifo_current_size=0;

//Posicion de agregar de la fifo
int zeng_fifo_write_position=0;

//Posicion de leer de la fifo
int zeng_fifo_read_position=0;

//Si esta habilitado zeng
z80_bit zeng_enabled={0};

//Hostname remoto
char zeng_remote_hostname[256]="127.0.0.1";

//Puerto remoto
int zeng_remote_port=10000;

int segundos_cada_snapshot=2;

int zeng_i_am_master=0;


int zeng_next_position(int pos)
{
	pos++; 
	if (pos==ZENG_FIFO_SIZE) pos=0;
	return pos;
}

//Agregar elemento a la fifo
//Retorna 1 si esta llena
int zeng_fifo_add_element(zeng_key_presses *elemento)
{
	//Si esta llena, no hacer nada
	//TODO: esperar a flush
	//TODO: semaforo

	if (zeng_fifo_current_size==ZENG_FIFO_SIZE) return 1;

	//Escribir en la posicion actual
	zeng_key_presses_array[zeng_fifo_write_position].tecla=elemento->tecla;
	zeng_key_presses_array[zeng_fifo_write_position].pressrelease=elemento->pressrelease;

	//Y poner siguiente posicion
	zeng_fifo_write_position=zeng_next_position(zeng_fifo_write_position);

	//Y sumar total elementos
	zeng_fifo_current_size++;

	return 0;

}

//Leer elemento de la fifo
//Retorna 1 si esta vacia
int zeng_fifo_read_element(zeng_key_presses *elemento)
{
	//TODO: semaforo

	if (zeng_fifo_current_size==0) return 1;

	//Leer de la posicion actual
	elemento->tecla=zeng_key_presses_array[zeng_fifo_read_position].tecla;
	elemento->pressrelease=zeng_key_presses_array[zeng_fifo_read_position].pressrelease;

	//Y poner siguiente posicion
	zeng_fifo_read_position=zeng_next_position(zeng_fifo_read_position);

	//Y restar total elementos
	zeng_fifo_current_size--;

	return 0;

}


void zeng_send_key_event(enum util_teclas tecla,int pressrelease)
{
	if (zeng_enabled.v==0) return;

	//Si esta menu abierto, tampoco enviar
	if (menu_abierto) return;

	//teclas F no enviar
	switch (tecla) {
		case UTIL_KEY_F1:
		case UTIL_KEY_F2:
		case UTIL_KEY_F3:
		case UTIL_KEY_F4:
		case UTIL_KEY_F5:
		case UTIL_KEY_F6:
		case UTIL_KEY_F7:
		case UTIL_KEY_F8:
		case UTIL_KEY_F9:
		case UTIL_KEY_F10:
		case UTIL_KEY_F11:
		case UTIL_KEY_F12:
		case UTIL_KEY_F13:
		case UTIL_KEY_F14:
		case UTIL_KEY_F15:
			return;
		break;

		default:
		break;
	}

	zeng_key_presses elemento;

	elemento.tecla=tecla;
	elemento.pressrelease=pressrelease;

	//printf ("Adding zeng key event to fifo\n");

	if (zeng_fifo_add_element(&elemento)) {
		debug_printf (VERBOSE_DEBUG,"Error adding zeng key event. FIFO full");
		return;
	}

}

//Devuelve 0 si no conectado
int zeng_connect_remote(void)
{

		int indice_socket=z_sock_open_connection(zeng_remote_hostname,zeng_remote_port);

		if (indice_socket<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't create TCP socket");
			return 0;
		}

		 
		
		//Leer algo
		char buffer[200];

		//int leidos=z_sock_read(indice_socket,buffer,199);
		int leidos=zsock_read_all_until_command(indice_socket,buffer,199);
		if (leidos>0) {
			buffer[leidos]=0; //fin de texto
			//printf("Received text (length: %d):\n[\n%s\n]\n",leidos,buffer);
		}

		//zsock_wait_until_command_prompt(indice_socket);

		printf("Sending get-version\n");

		//Enviar un get-version
		z_sock_write_string(indice_socket,"get-version\n");


		//reintentar
		leidos=zsock_read_all_until_command(indice_socket,buffer,199);
		if (leidos>0) {
			buffer[leidos]=0; //fin de texto
			printf("Received text for get-version (length %d): \n[\n%s\n]\n",leidos,buffer);
		}		

		//escribir_socket(misocket,"Waiting until command prompt final");
		//printf("Waiting until command prompt final\n");

		//zsock_wait_until_command_prompt(indice_socket);

	zeng_remote_socket=indice_socket;

	return 1;
}

int contador_envio_snapshot=0;
z80_byte *zeng_send_snapshot_mem=NULL;
int zeng_send_snapshot_longitud=0;

void zeng_send_snapshot(int socket)
{
	//Enviar snapshot cada 20*250=5000 ms->5 segundos
		printf ("Enviando snapshot\n");

				//z80_byte *buffer_temp;
				//buffer_temp=zeng_send_snapshot_mem;
				//if (buffer_temp==NULL) cpu_panic("Can not allocate memory for get-snapshot");

				//z80_byte *puntero=buffer_temp; 
				int longitud=zeng_send_snapshot_longitud;

  				//save_zsf_snapshot_file_mem(NULL,puntero,&longitud);

				//printf ("longitud: %d\n",longitud);

				printf ("Sending put-snapshot length: %d\n",longitud);
				z_sock_write_string(socket,"put-snapshot ");

				int i;
				z80_byte *buffer_put_snapshot_temp;
				buffer_put_snapshot_temp=malloc(ZRCP_GET_PUT_SNAPSHOT_MEM*2); //16 MB es mas que suficiente

				int char_destino=0;

		
				for (i=0;i<longitud;i++,char_destino +=2) {
					sprintf (&buffer_put_snapshot_temp[char_destino],"%02X",zeng_send_snapshot_mem[i]);
				}

				//metemos salto de linea al final
				strcpy (&buffer_put_snapshot_temp[char_destino],"\n");

				//TODO esto es ineficiente y que tiene que calcular la longitud. hacer otra z_sock_write sin tener que calcular
				z_sock_write_string(socket,buffer_put_snapshot_temp);

				free(buffer_put_snapshot_temp);

				//z_sock_write_string(indice_socket,"\n");

	 			free(zeng_send_snapshot_mem);
				zeng_send_snapshot_mem=NULL;

				char buffer[200];
				//Leer hasta prompt
				int leidos=zsock_read_all_until_command(socket,buffer,199);

		
}



void *thread_zeng_function(void *nada)
{
	/*
Hilo de sincronización de juego:

-si flag de envío de snapshot, se envía. Ese flag lo activa el core al final de frame, cada X segundos, y cuando somos el máster

-si hay que enviar mensaje al otro jugador, enviarlo

-ver la fifo usada en envío de eventos:
*tecla
*press/release

Dicha fifo hay que controlarla mediante semáforos
Se mete elementos en fifo cuando se llama a util send press/release
Se leen y envían eventos de la fifo desde este thread 

-dormir durante 10ms - mitad de frame 

Para las rutinas zsock también haría falta semáforos pero como no voy a llamarla desde dos sitios distintos a la vez pues..

Poder enviar mensajes a otros jugadores 	
	 */

	int contador_veces=0;

	while (1) {
		usleep(10000); //dormir 10 ms

		zeng_key_presses elemento;
		while (!zeng_fifo_read_element(&elemento)) {
			//printf ("leido evento de la zeng fifo tecla %d pressrelease %d\n",elemento.tecla,elemento.pressrelease);

			//command> help send-keys-event
			//Syntax: send-keys-event key event


				//printf ("longitud: %d\n",longitud);
				char buffer_comando[256];
				sprintf(buffer_comando,"send-keys-event %d %d\n",elemento.tecla,elemento.pressrelease);

				z_sock_write_string(zeng_remote_socket,buffer_comando);

				char buffer[200];

				//Leer hasta prompt
				int leidos=zsock_read_all_until_command(zeng_remote_socket,buffer,199);


		}

		contador_veces++;


		if (zeng_i_am_master) {
			if (zeng_send_snapshot_mem!=NULL && zeng_send_snapshot_longitud!=0) {
				zeng_send_snapshot(zeng_remote_socket);
			}
		}
	}
}




void zeng_send_snapshot_if_needed(void)
{

	if (zeng_i_am_master) {
		contador_envio_snapshot++;
		if ( (contador_envio_snapshot % (50*segundos_cada_snapshot) )==0) { //cada 5 segundos
				z80_byte *buffer_temp;
				buffer_temp=malloc(ZRCP_GET_PUT_SNAPSHOT_MEM); //16 MB es mas que suficiente
				if (buffer_temp==NULL) cpu_panic("Can not allocate memory for get-snapshot");

				z80_byte *puntero=buffer_temp; 
				int longitud;

  				save_zsf_snapshot_file_mem(NULL,zeng_send_snapshot_mem,&longitud);	

				zeng_send_snapshot_mem=buffer_temp;
				zeng_send_snapshot_longitud=longitud;

				printf ("Poniendo en cola snapshot para enviar\n");
		}
	}
}

void zeng_enable(void)
{

	//ya  inicializado
	if (zeng_enabled.v) return;

	if (zeng_remote_hostname[0]==0) return;

#ifdef USE_PTHREADS

	//Conectar a remoto
	if (!zeng_connect_remote()) return;


	//Inicializar thread

	thread_zeng_inicializado.v=0;

	if (pthread_create( &thread_zeng, NULL, &thread_zeng_function, NULL) ) {
		debug_printf(VERBOSE_ERR,"Can not create zeng pthread");
		return;
	}


	thread_zeng_inicializado.v=1;


	zeng_enabled.v=1;
#else
	//sin threads
	zeng_enabled.v=0;
#endif



}



