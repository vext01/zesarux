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
#include <unistd.h>
#include <fcntl.h>

#ifndef MINGW
//select no existe en windows
#include <sys/select.h>
#endif


#include "chardevice.h"
#include "cpu.h"
#include "debug.h"


//Devuelve file handle. <0 si error
int chardevice_open(char *path,enum chardevice_openmode mode)
{

    int openflag;

    switch (mode) {
        case CHDEV_RDONLY:
            openflag=O_RDONLY;
        break;

	    case CHDEV_WRONLY:
            openflag=O_WRONLY;
        break;

        case CHDEV_RDWR:
            openflag=O_RDWR;
        break;

        default:
            return -1;
        break;
    }

    //Agregar no bloqueo
    //openflag |=O_NONBLOCK;


    int handler=open(path,openflag);


    if (handler>=0) {
        //Agregar no bloqueo. manera estandard
        //Haciendolo con el open tambien seria valido, pero mejor asi
        int flags = fcntl(handler, F_GETFL, 0);
        fcntl(handler, F_SETFL, flags | O_NONBLOCK);
    }

    return handler;
}

//Lee 1 byte
//Devuelve numero bytes leidos. <0 si error. 0=no bytes leidos, tambien error
int chardevice_read(int handler,z80_byte *buffer)
{
    int leidos = read(handler, buffer, 1);

    return leidos;
}

//Escribe 1 byte
//Devuelve numero bytes escritos. <0 si error. 0=no bytes leidos, tambien error
int chardevice_write(int handler,z80_byte valor_escribir)
{
    int escritos=write(handler,&valor_escribir,1);

    return escritos;
}

//0 si ok. -1 si error
int chardevice_close(int handler)
{
    int retorno=close(handler);

    return retorno;
}

//Dice si hay datos disponibles para leer en el file handler
int chardevice_dataread_avail(int handler)
{

#ifdef MINGW
    //Select no esta disponible en windows
    //Decimos que hay siempre datos disponibles
    return 1;
#else
    fd_set readset;

    FD_ZERO(&readset);
    FD_SET(handler, &readset);


    struct timeval timeout;
    //timeout de 64 microsegundos, lo que dura un scanline aprox de spectrum
    //Por que 64 microsegundos y no otro valor? Porque creo que es un valor considerable para esperar si hay datos o no
    timeout.tv_sec  = 0;
    timeout.tv_usec = 64;

    //Si pusieramos timeout a null, se haria bloqueo hasta que hubieran datos.


    int resultado=select(handler + 1, &readset, NULL, NULL, &timeout);


    if (resultado<=0) return 0;
    else return 1;
 #endif

}

//Retorna el estado del dispositivo
int chardevice_status(int handler)
{

    int status=0;

    if (chardevice_dataread_avail(handler)) status |= CHDEV_ST_RD_AVAIL_DATA;

    return status;

}