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


//-ZENG: ZEsarUX Network Gaming



//La fifo

zeng_key_presses zeng_key_presses_array[ZENG_FIFO_SIZE];


//Tamanyo de la fifo
int zeng_fifo_current_size=0;

//Posicion de agregar de la fifo
int zeng_fifo_write_position=0;

//Posicion de leer de la fifo
int zeng_fifo_read_position=0;


int zeng_next_position(int pos)
{
	pos++; 
	if (pos==ZENG_FIFO_SIZE) pos=0;
	return pos;
}

//Agregar elemento a la fifo
void zeng_fifo_add_element(zeng_key_presses *elemento){
	//Si esta llena, no hacer nada
	//TODO: esperar a flush
	//TODO: semaforo

	if (zeng_fifo_current_size==ZENG_FIFO_SIZE) return;

	//Escribir en la posicion actual
	zeng_key_presses_array[zeng_fifo_write_position].tecla=elemento->tecla;
	zeng_key_presses_array[zeng_fifo_write_position].pressrelease=elemento->pressrelease;

	//Y poner siguiente posicion
	zeng_fifo_write_position=zeng_next_position(zeng_fifo_write_position);

	//Y sumar total elementos
	zeng_fifo_current_size++;

}

//Leer elemento de la fifo
//Retorna 0 si esta vacia
int zeng_fifo_read_element(zeng_key_presses *elemento){
	//TODO: semaforo

	if (zeng_fifo_current_size==0) return 0;

	//Leer de la posicion actual
	elemento->tecla=zeng_key_presses_array[zeng_fifo_read_position].tecla;
	elemento->pressrelease=zeng_key_presses_array[zeng_fifo_read_position].pressrelease;

	//Y poner siguiente posicion
	zeng_fifo_read_position=zeng_next_position(zeng_fifo_read_position);

	//Y restar total elementos
	zeng_fifo_current_size--;

	return 1;

}