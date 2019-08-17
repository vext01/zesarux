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
#include "chardevice.h"


/*
Estos ya vienen de network.h
#ifdef USE_PTHREADS
	#include <pthread.h>
#endif


#include <sys/types.h>

#ifdef MINGW
	#include <winsock2.h>
#else
	#include <sys/socket.h>
	#include <netdb.h>
	#include <unistd.h>
#endif
*/


//Estructura para guardar sockets
#define MAX_Z_SOCKETS 30

struct s_z_sockets_struct {
	int used;
	struct sockaddr_in adr;
	int socket_number;
};


typedef struct s_z_sockets_struct z_sockets_struct;

//array de sockets
z_sockets_struct sockets_list[MAX_Z_SOCKETS];


 
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


int omplir_adr_internet(struct sockaddr_in *adr,char *host,unsigned short n_port)
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


int connectar_socket(int s,struct sockaddr_in *adr) 
{
	//TODO: como funciona esto en Windows?
	int retorno=connect(s,(struct sockaddr *)adr,sizeof(struct sockaddr_in));

    if (retorno<0) {
        debug_printf (VERBOSE_ERR,"Error stablishing connection with host");
    }

	return retorno;

}

int cerrar_socket(int s)
{


#ifdef MINGW
	return closesocket(s);
	//desactivo esto ya que esto implica que no se va a usar mas los windows sockets, cosa no cierta (se pueden usar en zeng por ejemplo)
	//ademas no estamos llamando a WSAStartup al inicio
	//Se deberia hacer el WSACleanup al finalizar el emulador
	//WSACleanup();
#else
	return close(s);
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



int connectar_socket(int s GCC_UNUSED,struct sockaddr_in *adr GCC_UNUSED) 
{

	debug_printf (VERBOSE_ERR,"Pthreads unavailable but trying to use TCP/IP sockets");

    return -1;

}


int cerrar_socket(int s GCC_UNUSED)
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



void init_network_tables(void)
{
	int i;
	for (i=0;i<MAX_Z_SOCKETS;i++) {
		sockets_list[i].used=0;
	}
}

int find_free_socket(void)
{
	int i;
	for (i=0;i<MAX_Z_SOCKETS;i++) {
		if (sockets_list[i].used==0) {
			printf ("Found free socket at index %d\n",i);
			return i;
		}
	}

	return -1;
}

//Retorna indice a la tabla de sockets. <0 si error
int z_sock_open_connection(char *host,int port)
{

	int indice_tabla=find_free_socket();
	if (indice_tabla<0) {
		debug_printf(VERBOSE_ERR,"Too many open sockets (%d)",MAX_Z_SOCKETS);
		return -1;		
	}
		
	int test_socket;
		

	if ((test_socket=crear_socket_TCP())<0) {
		debug_printf(VERBOSE_ERR,"Can't create TCP socket");
		return -1;
    }

		//struct sockaddr_in adr;

        if (omplir_adr_internet(&sockets_list[indice_tabla].adr,host,port)<0) {
                debug_printf(VERBOSE_ERR,"Error parsing host");
                return -1;
        }

		if (connectar_socket(test_socket,&sockets_list[indice_tabla].adr)<0) {
                debug_printf(VERBOSE_ERR,"Error stablishing connection with %s:%d",host,port);
				return -1;
        }

	sockets_list[indice_tabla].socket_number=test_socket;
	sockets_list[indice_tabla].used=1;

	return 0;

} 

int get_socket_number(int indice_tabla)
{

	if (indice_tabla<0 || indice_tabla>=MAX_Z_SOCKETS) return -1;

	if (!sockets_list[indice_tabla].used) {
				return -1;
	}	

	else return sockets_list[indice_tabla].socket_number;
}
	

int z_sock_close_connection(int indice_tabla) 
{

	int sock=get_socket_number(indice_tabla);

	if (sock<0) {
                //debug_printf(VERBOSE_ERR,"Socket is not open");
				return -1;
	}


	sockets_list[indice_tabla].used=0;

	return cerrar_socket(sock);
}


//Liberar el socket de la lista pero sin desconectar
int z_sock_free_connection(int indice_tabla) 
{

	int sock=get_socket_number(indice_tabla);

	if (sock<0) {
                //debug_printf(VERBOSE_ERR,"Socket is not open");
				return -1;
	}


	sockets_list[indice_tabla].used=0;

	return 0;
}

int z_sock_read(int indice_tabla, z80_byte *buffer, int longitud)
{

	int sock=get_socket_number(indice_tabla);

	if (sock<0) {
                debug_printf(VERBOSE_ERR,"Socket is not open");
				return -1;
	}

	return leer_socket(sock,buffer,longitud);
}


int z_sock_write_string(int indice_tabla, char *buffer)
{ 

	int sock=get_socket_number(indice_tabla);

	if (sock<0) {
                debug_printf(VERBOSE_ERR,"Socket is not open");
				return -1;
	}

	return escribir_socket(sock,buffer);
}

int zsock_wait_until_command_prompt(int indice_tabla)
{
	//Leemos hasta que no haya mas datos para leer. Idealmente estara el "command> "
	int leidos;

	int sock=get_socket_number(indice_tabla);

	if (sock<0) {
                debug_printf(VERBOSE_ERR,"Socket is not open");
				return -1;
	}	



		char buffer[100];

		do {
			//TODO: en windows siempre retorna datos disponibles. lo cual seria un problema por que si no hay datos,
			//la conexion se queda en read colgada
			if (chardevice_status(sock) & CHDEV_ST_RD_AVAIL_DATA) {
				leidos=z_sock_read(indice_tabla,buffer,100);
				//printf ("leidos en zsock_wait_until_command_prompt: %d\n",leidos);
				if (leidos<0) return -1;
			}
			else {
				leidos=0;
			}
		} while (leidos>0);

		return 0;

	}


//Esta funcion no la usa nadie de momento
//Si se activa alguna vez, comparar con zsock_read_all_until_command pues son casi iguales (esta ultima comprueba el prompt final, y zsock_read_all no)
/*
int zsock_read_all(int indice_tabla,z80_byte *buffer,int max_buffer)
{

	//Leemos hasta que no haya mas datos para leer. Idealmente estara el "command> "
	int leidos;

	int sock=get_socket_number(indice_tabla);

	if (sock<0) {
        debug_printf(VERBOSE_ERR,"Socket is not open");
		return -1;
	}	


	int pos_destino=0;
	int total_leidos=0;

		do {
			//TODO: en windows siempre retorna datos disponibles. lo cual seria un problema por que si no hay datos,
			//la conexion se queda en read colgada
			if (chardevice_status(sock) & CHDEV_ST_RD_AVAIL_DATA) {
				leidos=z_sock_read(indice_tabla,&buffer[pos_destino],max_buffer);
				//printf ("leidos en zsock_wait_until_command_prompt: %d\n",leidos);
				if (leidos<0) return -1;
				else {
					max_buffer -=leidos;
					total_leidos +=leidos;
					pos_destino +=leidos;
				}
			}
			else {
				leidos=0;
			}
		} while (leidos>0 && max_buffer>0);

	return total_leidos;

}
*/

void zsock_debug_dump_ascii(char *buffer,int total_leidos)
{
	for (;total_leidos>0;total_leidos--,buffer++) {
		printf ("%c",util_printable_char(*buffer));
	}
}

//devuelve en posicion_command donde empieza el "command>", si es >=0
int zsock_read_all_until_command(int indice_tabla,z80_byte *buffer,int max_buffer,int *posicion_command)
{

	//printf ("inicio zsock_read_all_until_command\n");

	//Leemos hasta que no haya mas datos para leer. Idealmente estara el "command> "
	int leidos;

	//Inicialmente no sabemos la posicion del command>
	*posicion_command=-1;

	int sock=get_socket_number(indice_tabla);

	if (sock<0) {
		//printf ("Socket is not open\n");
        debug_printf(VERBOSE_ERR,"Socket is not open");
		return -1;
	}	


	int pos_destino=0;
	int total_leidos=0;
	int leido_command_prompt=0;
	int reintentos=0;
	//printf ("entrando bucle zsock_read_all_until_command\n");
	do {

		do {
			//TODO: en windows siempre retorna datos disponibles. lo cual seria un problema por que si no hay datos,
			//la conexion se queda en read colgada
			if (chardevice_status(sock) & CHDEV_ST_RD_AVAIL_DATA) {
				leidos=z_sock_read(indice_tabla,&buffer[pos_destino],max_buffer);
				//printf ("leidos en zsock_wait_until_command_prompt: %d\n",leidos);
				if (leidos<0) return -1;
				else {
					max_buffer -=leidos;
					total_leidos +=leidos;
					pos_destino +=leidos;
				}
			}
			else {
				leidos=0;
			}
		} while (leidos>0 && max_buffer>0);

		//Ver si hay "command> al final"
		if (total_leidos>=9) { //"command> " ocupa 9
			//printf ("Leido al final: %02XH %02XH [%c%c]\n",buffer[total_leidos-2],buffer[total_leidos-1],
			//util_printable_char(buffer[total_leidos-2]),util_printable_char(buffer[total_leidos-1]));

			//printf ("Leido todo: ");
			//zsock_debug_dump_ascii(buffer,total_leidos);
			//printf ("\n");

			if (buffer[total_leidos-1]==' ' && buffer[total_leidos-2]=='>') {
				leido_command_prompt=1;
				*posicion_command=total_leidos-9;
				//printf ("Recibido command prompt\n");
			}
		}

		else {
			//printf ("total leidos: %d\n",total_leidos);
		}


		if (!leido_command_prompt) {
			//printf ("NO recibido command prompt. Reintentar\n");
			usleep(10000); //10 ms
		}

		reintentos++;

		//TODO controlar maximo reintentos
	} while (!leido_command_prompt && reintentos<500);
	//5 segundos de reintentos

	//TODO: si se llega aqui sin haber recibido command prompt. Se gestionara en retorno mediante posicion_command
	return total_leidos;

}

int zsock_http(char *host, char *url)
{
	int indice_socket=z_sock_open_connection(host,80);

		if (indice_socket<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't create TCP socket");
			return 0;
		}
		
		char request[1024];
		
		sprintf(request,"GET %s HTTP/1.0\r\n"
						"Host: %s\r\n"
						"User-Agent: ZEsarUX\r\n" 
						"\r\n",url,host);

		
		int escritos=z_sock_write_string(indice_socket,request);

		if (escritos<0) {
			debug_printf(VERBOSE_ERR,"ERROR. Can't send request");
			return 0;	
		}
		
		//todo buffer asignar
		char response[65535];
		
		int leidos=z_sock_read(indice_socket,response,65535);
		
		if (leidos>0) {
			response[leidos]=0;
			printf ("leidos: %d\n",leidos);
			printf ("respuesta:\n%s\n",response);
			z_sock_close_connection(indice_socket);
			return 0;
		}
		
		else return -1;
		
		
}