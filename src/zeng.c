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
int zeng_remote_port=10010;


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


void zeng_key_event(enum util_teclas tecla,int pressrelease)
{
	if (zeng_enabled.v==0) return;

	//Si esta menu abierto, tampoco enviar
	if (menu_abierto) return;

	zeng_key_presses elemento;

	elemento.tecla=tecla;
	elemento.pressrelease=pressrelease;

	printf ("Adding zeng key event to fifo\n");

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
			printf("Received text (length: %d):\n[\n%s\n]\n",leidos,buffer);
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

	return 1;
}

void *thread_zeng_function(void *nada)
{
	while (1) {
		usleep(10000); //dormir 10 ms

		zeng_key_presses elemento;
		while (!zeng_fifo_read_element(&elemento)) {
			printf ("leido evento de la zeng fifo tecla %d pressrelease %d\n",elemento.tecla,elemento.pressrelease);
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



