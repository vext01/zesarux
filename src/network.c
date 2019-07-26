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

//Para uso de funciones va_
#include <stdarg.h>


#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "network.h"
#include "compileoptions.h"





#include <pthread.h>
#include <sys/types.h>

#ifdef MINGW
	#include <winsock2.h>
#else
	#include <sys/socket.h>
	#include <netdb.h>
	#include <unistd.h>
#endif



#ifdef USE_PTHREADS


//Si se envia CR despues de cada sentencia de escritura
int enviar_cr=0;


//Crea un socket TCP per la connexio en xarxa
int crear_socket_TCP(void)
{
	#ifdef MINGW
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(1,1), &wsadata) == SOCKET_ERROR) {
		debug_printf(VERBOSE_ERR,"Error creating socket.");
		return -1;
	}
	#endif


    return socket(PF_INET,SOCK_STREAM,0);
}


int omplir_adr_internet
(struct sockaddr_in *adr,char *host,unsigned short n_port)
{
        struct hostent *h;

        adr->sin_family=AF_INET;
        if (host!=NULL) {
                if ((h=gethostbyname(host))==NULL) return -1;
                adr->sin_addr=*(struct in_addr *)h->h_addr;

                //printf ("\nncdd_util: %s : %d = %lX\n",host,(int)n_port,(unsigned long)adr->sin_addr.s_addr);

        }
        else {
                adr->sin_addr.s_addr=htonl(INADDR_ANY);
  }
        adr->sin_port=htons(n_port);
        return 0;
}

//Assignar l'adre<E7>a al socket
//Host ha de valer NULL si es tracta del servidor
int assignar_adr_internet
(int sock,char *host,unsigned short n_port)
{
        struct sockaddr_in adr;

        if (omplir_adr_internet(&adr,host,n_port)<0) return -1;
        return bind(sock,(struct sockaddr *)&adr,sizeof(adr));
}






int escribir_socket(int socket, char *buffer)
{

	char cr=13;

	int efectivo_enviar_cr=0;

	if (enviar_cr) {
		//Si ultimo caracter es 10, agregar un 13
		int longitud=strlen(buffer);
		if (longitud) {
			if (buffer[longitud-1]==10) efectivo_enviar_cr=1;
		}
	}

#ifdef MINGW

	int smsg=send(socket,buffer,strlen(buffer),0);
	 if(smsg==SOCKET_ERROR){
			 debug_printf(VERBOSE_ERR,"Error writing to socket");
			 return -1;
	 }
	 if (efectivo_enviar_cr) send(socket,&cr,1,0);
	 return smsg;

#else

	int escrito=write(socket,buffer,strlen(buffer));

	if (efectivo_enviar_cr) write(socket,&cr,1);

	return escrito;

#endif

}

//int leidos = read(sock_conectat, buffer_lectura_socket, 1023);
int leer_socket(int s, char *buffer, int longitud)
{
#ifdef MINGW

int leidos=recv(s,buffer,longitud,0);
 if(leidos==SOCKET_ERROR){
	 	debug_printf(VERBOSE_ERR,"Error reading from socket");
		return -1;
 }
 return leidos;

#else
	//int leidos=read(s, buffer, longitud);
	int leidos=recv(s,buffer,longitud,0);
	//printf ("leidos en leer_socket: %d\n",leidos);
	return leidos;

#endif
}





//Fin de funciones CON pthreads
#else



//No hacer nada, no hay pthreads disponibles
//funciones nulas de socket. Nadie deberia usarlas si no hay sockets pero por si acaso...



//Crea un socket TCP per la connexio en xarxa
int crear_socket_TCP(void)
{
	debug_printf (VERBOSE_ERR,"Pthreads unavailable but trying to use TCP/IP sockets");

    return -1;
}


int omplir_adr_internet(struct sockaddr_in *adr GCC_UNUSED,char *host GCC_UNUSED,unsigned short n_port GCC_UNUSED)
{
	debug_printf (VERBOSE_ERR,"Pthreads unavailable but trying to use TCP/IP sockets");

    return -1;

}

//Assignar l'adre<E7>a al socket
//Host ha de valer NULL si es tracta del servidor
int assignar_adr_internet(int sock GCC_UNUSED,char *host GCC_UNUSED,unsigned short n_port GCC_UNUSED)
{
	debug_printf (VERBOSE_ERR,"Pthreads unavailable but trying to use TCP/IP sockets");

    return -1;
}



int escribir_socket(int socket GCC_UNUSED, char *buffer GCC_UNUSED)
{

	debug_printf (VERBOSE_ERR,"Pthreads unavailable but trying to use TCP/IP sockets");

    return -1;

}

//int leidos = read(sock_conectat, buffer_lectura_socket, 1023);
int leer_socket(int s GCC_UNUSED, char *buffer GCC_UNUSED, int longitud GCC_UNUSED)
{
	debug_printf (VERBOSE_ERR,"Pthreads unavailable but trying to use TCP/IP sockets");

    return -1;
}







#endif
//Fin de funciones SIN pthreads



//Funciones (pocas) que no necesitan pthreads

void escribir_socket_format (int misocket, const char * format , ...)
{

    char buffer_final[4096];

    va_list args;
    va_start (args, format);
    vsprintf (buffer_final,format, args);
    va_end (args);

    escribir_socket(misocket,buffer_final);
}

