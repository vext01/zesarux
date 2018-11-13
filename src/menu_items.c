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
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>


#include "menu.h"
#include "menu_items.h"
#include "screen.h"
#include "cpu.h"
#include "debug.h"
#include "zx8081.h"
#include "ay38912.h"
#include "tape.h"
#include "audio.h"
#include "timer.h"
#include "snap.h"
#include "operaciones.h"
#include "disassemble.h"
#include "utils.h"
#include "contend.h"
#include "joystick.h"
#include "ula.h"
#include "printers.h"
#include "realjoystick.h"
#include "scrstdout.h"
#include "z88.h"
#include "ulaplus.h"
#include "autoselectoptions.h"
#include "zxuno.h"
#include "charset.h"
#include "chardetect.h"
#include "textspeech.h"
#include "mmc.h"
#include "ide.h"
#include "divmmc.h"
#include "divide.h"
#include "diviface.h"
#include "zxpand.h"
#include "spectra.h"
#include "spritechip.h"
#include "jupiterace.h"
#include "timex.h"
#include "chloe.h"
#include "prism.h"
#include "cpc.h"
#include "sam.h"
#include "atomlite.h"
#include "if1.h"
#include "pd765.h"
#include "tbblue.h"
#include "dandanator.h"
#include "superupgrade.h"
#include "m68k.h"
#include "remote.h"
#include "snap_rzx.h"
#include "multiface.h"
#include "scmp.h"
#include "esxdos_handler.h"
#include "tsconf.h"
#include "kartusho.h"
#include "spritefinder.h"
#include "snap_spg.h"
#include "betadisk.h"
#include "tape_tzx.h" 
#include "snap_zsf.h"
#include "compileoptions.h"
#include "settings.h"

 
#if defined(__APPLE__)
	#include <sys/syslimits.h>

	#include <sys/resource.h>

#endif


#ifdef COMPILE_CURSES
	#include "scrcurses.h"
#endif

#ifdef COMPILE_AA
	#include "scraa.h"
#endif

#ifdef COMPILE_STDOUT
	#include "scrstdout.h"
//macro llama a funcion real
	#define scrstdout_menu_print_speech_macro scrstdout_menu_print_speech
//funcion llama
#else
//funcion no llama a nada
	#define scrstdout_menu_print_speech_macro(x)
#endif


#ifdef COMPILE_XWINDOWS
	#include "scrxwindows.h"
#endif


//
// Archivo para entradas de menu, excluyendo funciones auxiliares de soporte de menu
// Las funciones auxiliares de menu estan en menu.c
// Aunque aun falta mucho por mover, la mayoria de entradas de menu siguen en menu.c y habria que moverlas aqui
//


//Opciones seleccionadas para cada menu
int debug_pok_file_opcion_seleccionada=0;
int poke_opcion_seleccionada=0; 
int settings_debug_opcion_seleccionada=0;
int change_audio_driver_opcion_seleccionada=0;
int settings_audio_opcion_seleccionada=0;
int mem_breakpoints_opcion_seleccionada=0;

//Fin opciones seleccionadas para cada menu


//Ultima direccion pokeada
int last_debug_poke_dir=16384; 

//aofile. aofilename apuntara aqui
char aofilename_file[PATH_MAX];


void menu_mem_breakpoints_edit(MENU_ITEM_PARAMETERS)
{


        int brkp_type,dir;

        char string_type[4];
        char string_dir[10];

        strcpy (string_dir,"0");

        menu_ventana_scanf("Address",string_dir,10);

        dir=parse_string_to_number(string_dir);

        if (dir<0 || dir>65535) {
                debug_printf (VERBOSE_ERR,"Invalid address %d",dir);
                return;
        }				

        strcpy (string_type,"0");

        menu_ventana_scanf("Type (1:RD,2:WR,3:RW)",string_type,4);

        brkp_type=parse_string_to_number(string_type);

        if (brkp_type<0 || brkp_type>255) {
                debug_printf (VERBOSE_ERR,"Invalid value %d",brkp_type);
                return;
        }

	debug_set_mem_breakpoint(dir,brkp_type);
	//mem_breakpoint_array[dir]=brkp_type;
	

}

void menu_mem_breakpoints_list(MENU_ITEM_PARAMETERS)
{

        //int index_find;
		int index_buffer;

        char results_buffer[MAX_TEXTO_GENERIC_MESSAGE];

        //margen suficiente para que quepa una linea
        //direccion+salto linea+codigo 0
        char buf_linea[33];

        index_buffer=0;

        int encontrados=0;

        int salir=0;

		int i;

        for (i=0;i<65536 && salir==0;i++) {
			z80_byte tipo=mem_breakpoint_array[i];
			if (tipo) {
				if (tipo<MAX_MEM_BREAKPOINT_TYPES) {
					sprintf (buf_linea,"%04XH : %s\n",i,mem_breakpoint_types_strings[tipo]);
				}
				else {
					sprintf (buf_linea,"%04XH : Unknown (%d)\n",i,tipo);
				}

				sprintf (&results_buffer[index_buffer],"%s\n",buf_linea);
                index_buffer +=strlen(buf_linea);
                encontrados++;
                

                //controlar maximo
                //33 bytes de margen
                if (index_buffer>MAX_TEXTO_GENERIC_MESSAGE-33) {
                        debug_printf (VERBOSE_ERR,"Too many results to show. Showing only the first %d",encontrados);
                        //forzar salir
                        salir=1;
                }
			}

        }

        results_buffer[index_buffer]=0;

        menu_generic_message("List Memory Breakpoints",results_buffer);
}

void menu_mem_breakpoints_clear(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno("Clear breakpoints")) {
		clear_mem_breakpoints();
		menu_generic_message("Clear breakpoints","OK. All breakpoints cleared");
	}
}


void menu_mem_breakpoints(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();

        menu_item *array_menu_mem_breakpoints;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


		menu_add_item_menu_inicial_format(&array_menu_mem_breakpoints,MENU_OPCION_NORMAL,menu_mem_breakpoints_edit,NULL,"~~Edit Breakpoint");
		menu_add_item_menu_shortcut(array_menu_mem_breakpoints,'e');
		menu_add_item_menu_tooltip(array_menu_mem_breakpoints,"Edit Breakpoints");
		menu_add_item_menu_ayuda(array_menu_mem_breakpoints,"Edit Breakpoints");

		menu_add_item_menu_format(array_menu_mem_breakpoints,MENU_OPCION_NORMAL,menu_mem_breakpoints_list,NULL,"~~List breakpoints");
		menu_add_item_menu_shortcut(array_menu_mem_breakpoints,'l');
		menu_add_item_menu_tooltip(array_menu_mem_breakpoints,"List breakpoints");
		menu_add_item_menu_ayuda(array_menu_mem_breakpoints,"List enabled memory breakpoints");


		menu_add_item_menu_format(array_menu_mem_breakpoints,MENU_OPCION_NORMAL,menu_mem_breakpoints_clear,NULL,"~~Clear breakpoints");
		menu_add_item_menu_shortcut(array_menu_mem_breakpoints,'c');
		menu_add_item_menu_tooltip(array_menu_mem_breakpoints,"Clear all memory breakpoints");
		menu_add_item_menu_ayuda(array_menu_mem_breakpoints,"Clear all memory breakpoints");


                menu_add_item_menu(array_menu_mem_breakpoints,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                menu_add_ESC_item(array_menu_mem_breakpoints);
                retorno_menu=menu_dibuja_menu(&mem_breakpoints_opcion_seleccionada,&item_seleccionado,array_menu_mem_breakpoints,"Memory Breakpoints" );

                cls_menu_overlay();

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                cls_menu_overlay();
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}


 
void menu_debug_poke(MENU_ITEM_PARAMETERS)
{

        int valor_poke,dir,veces;

        char string_poke[4];
        char string_dir[10];
	char string_veces[6];

        sprintf (string_dir,"%XH",last_debug_poke_dir);

        menu_ventana_scanf("Address",string_dir,10);

        dir=parse_string_to_number(string_dir);

        /*if ( (dir<0 || dir>65535) && MACHINE_IS_SPECTRUM) {
                debug_printf (VERBOSE_ERR,"Invalid address %d",dir);
                return;
        }*/

				last_debug_poke_dir=dir;

        sprintf (string_poke,"0");

        menu_ventana_scanf("Poke Value",string_poke,4);

        valor_poke=parse_string_to_number(string_poke);

        if (valor_poke<0 || valor_poke>255) {
                debug_printf (VERBOSE_ERR,"Invalid value %d",valor_poke);
                return;
        }


	sprintf (string_veces,"1");

	menu_ventana_scanf("How many bytes?",string_veces,6);

	veces=parse_string_to_number(string_veces);

	if (veces<1 || veces>65536) {
                debug_printf (VERBOSE_ERR,"Invalid quantity %d",veces);
		return;
	}


	for (;veces;veces--,dir++) {

	        //poke_byte_no_time(dir,valor_poke);
		//poke_byte_z80_moto(dir,valor_poke);
		menu_debug_write_mapped_byte(dir,valor_poke);

	}

}



void menu_debug_poke_pok_file(MENU_ITEM_PARAMETERS)
{

        char *filtros[2];

        filtros[0]="pok";
        filtros[1]=0;

	char pokfile[PATH_MAX];

        int ret;

        ret=menu_filesel("Select POK File",filtros,pokfile);

	//contenido
	//MAX_LINEAS_POK_FILE es maximo de lineas de pok file
	//normalmente la tabla de pokes sera menor que el numero de lineas en el archivo .pok
	struct s_pokfile tabla_pokes[MAX_LINEAS_POK_FILE];

	//punteros
	struct s_pokfile *punteros_pokes[MAX_LINEAS_POK_FILE];

	int i;
	for (i=0;i<MAX_LINEAS_POK_FILE;i++) punteros_pokes[i]=&tabla_pokes[i];


        if (ret==1) {

                cls_menu_overlay();
		int total=util_parse_pok_file(pokfile,punteros_pokes);

		if (total<1) {
			debug_printf (VERBOSE_ERR,"Error parsing POK file");
			return;
		}

		int j;
		for (j=0;j<total;j++) {
			debug_printf (VERBOSE_DEBUG,"menu poke index %d text %s bank %d address %d value %d value_orig %d",
				punteros_pokes[j]->indice_accion,
				punteros_pokes[j]->texto,
				punteros_pokes[j]->banco,
				punteros_pokes[j]->direccion,
				punteros_pokes[j]->valor,
				punteros_pokes[j]->valor_orig);
		}


		//Meter cada poke en un menu




        menu_item *array_menu_debug_pok_file;
        menu_item item_seleccionado;
        int retorno_menu;
	//Resetear siempre ultima linea = 0
	debug_pok_file_opcion_seleccionada=0;

	//temporal para mostrar todos los caracteres 0-255
	//int temp_conta=1;

        do {



		//Meter primer item de menu
		//truncar texto a 28 caracteres si excede de eso
		if (strlen(punteros_pokes[0]->texto)>28) punteros_pokes[0]->texto[28]=0;
                menu_add_item_menu_inicial_format(&array_menu_debug_pok_file,MENU_OPCION_NORMAL,NULL,NULL,"%s", punteros_pokes[0]->texto);


		//Luego recorrer array de pokes y cuando el numero de poke se incrementa, agregar
		int poke_anterior=0;

		int total_elementos=1;

		for (j=1;j<total;j++) {
			if (punteros_pokes[j]->indice_accion!=poke_anterior) {

				//temp para mostrar todos los caracteres 0-255
				//int kk;
				//for (kk=0;kk<strlen(punteros_pokes[j]->texto);kk++) {
				//	punteros_pokes[j]->texto[kk]=temp_conta++;
				//	if (temp_conta==256) temp_conta=1;
				//}

				poke_anterior=punteros_pokes[j]->indice_accion;
				//truncar texto a 28 caracteres si excede de eso
				if (strlen(punteros_pokes[j]->texto)>28) punteros_pokes[j]->texto[28]=0;
				menu_add_item_menu_format(array_menu_debug_pok_file,MENU_OPCION_NORMAL,NULL,NULL,"%s", punteros_pokes[j]->texto);

				total_elementos++;
				if (total_elementos==20) {
					debug_printf (VERBOSE_DEBUG,"Too many pokes to show on Window. Showing only first 20");
					menu_warn_message("Too many pokes to show on Window. Showing only first 20");
					break;
				}


			}
		}



                menu_add_item_menu(array_menu_debug_pok_file,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                //menu_add_item_menu(array_menu_debug_pok_file,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_debug_pok_file);

                retorno_menu=menu_dibuja_menu(&debug_pok_file_opcion_seleccionada,&item_seleccionado,array_menu_debug_pok_file,"Select Poke" );

                cls_menu_overlay();

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion

			//Hacer poke sabiendo la linea seleccionada. Desde ahi, ejecutar todos los pokes de dicha accion
			debug_printf (VERBOSE_DEBUG,"Doing poke/s from line %d",debug_pok_file_opcion_seleccionada);

			z80_byte banco;
			z80_int direccion;
			z80_byte valor;

			//buscar indice_accion
			int result_poke=0;
			for (j=0;j<total && result_poke==0;j++) {

				debug_printf (VERBOSE_DEBUG,"index %d looking %d current %d",j,debug_pok_file_opcion_seleccionada,punteros_pokes[j]->indice_accion);

				if (punteros_pokes[j]->indice_accion==debug_pok_file_opcion_seleccionada) {
					banco=punteros_pokes[j]->banco;
					direccion=punteros_pokes[j]->direccion;
					valor=punteros_pokes[j]->valor;
					debug_printf (VERBOSE_DEBUG,"Doing poke bank %d address %d value %d",banco,direccion,valor);
					result_poke=util_poke(banco,direccion,valor);
				}


                        //        cls_menu_overlay();

			}
			if (result_poke==0) menu_generic_message("Poke","OK. Poke applied");
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




        }

}





void menu_poke(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_poke;
        menu_item item_seleccionado;
        int retorno_menu;

        do {


                menu_add_item_menu_inicial_format(&array_menu_poke,MENU_OPCION_NORMAL,menu_debug_poke,NULL,"~~Poke");
                menu_add_item_menu_shortcut(array_menu_poke,'p');
                menu_add_item_menu_tooltip(array_menu_poke,"Poke address");
                menu_add_item_menu_ayuda(array_menu_poke,"Poke address for infinite lives, etc... This item follows active memory zone. "
					"You can also poke on read-only memory, depending on the current memory zone");

		//No tiene sentido pues se puede usar las memory zones para esto
		/*if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3 || MACHINE_IS_ZXUNO_BOOTM_DISABLED) {
			menu_add_item_menu(array_menu_poke,"Poke 128~~k mode",MENU_OPCION_NORMAL,menu_debug_poke_128k,NULL);
			menu_add_item_menu_shortcut(array_menu_poke,'k');
			menu_add_item_menu_tooltip(array_menu_poke,"Poke bank & address");
			menu_add_item_menu_ayuda(array_menu_poke,"Poke bank & address");
		}*/

		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu(array_menu_poke,"Poke from .POK ~~file",MENU_OPCION_NORMAL,menu_debug_poke_pok_file,NULL);
			menu_add_item_menu_shortcut(array_menu_poke,'f');
			menu_add_item_menu_tooltip(array_menu_poke,"Poke reading .POK file");
			menu_add_item_menu_ayuda(array_menu_poke,"Poke reading .POK file");
		}


                menu_add_item_menu(array_menu_poke,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                //menu_add_item_menu(array_menu_poke,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_poke);

                retorno_menu=menu_dibuja_menu(&poke_opcion_seleccionada,&item_seleccionado,array_menu_poke,"Poke" );

                cls_menu_overlay();

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                cls_menu_overlay();
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}


void menu_debug_registers_console(MENU_ITEM_PARAMETERS) {
	debug_registers ^=1;
}

void menu_debug_configuration_stepover(MENU_ITEM_PARAMETERS)
{
	//debug_core_evitamos_inter.v ^=1;
	remote_debug_settings ^=32;
}


void menu_breakpoints_condition_behaviour(MENU_ITEM_PARAMETERS)
{
	debug_breakpoints_cond_behaviour.v ^=1;
}

void menu_debug_configuration_remoteproto_port(MENU_ITEM_PARAMETERS)
{
	char string_port[6];
	int port;

	sprintf (string_port,"%d",remote_protocol_port);

	menu_ventana_scanf("Port",string_port,6);

	if (string_port[0]==0) return;

	else {
			port=parse_string_to_number(string_port);

			if (port<1 || port>65535) {
								debug_printf (VERBOSE_ERR,"Invalid port %d",port);
								return;
			}


			end_remote_protocol();
			remote_protocol_port=port;
			init_remote_protocol();
	}

}

void menu_debug_shows_invalid_opcode(MENU_ITEM_PARAMETERS)
{
	debug_shows_invalid_opcode.v ^=1;
}

void menu_debug_settings_show_fired_breakpoint(MENU_ITEM_PARAMETERS)
{
	debug_show_fired_breakpoints_type++;
	if (debug_show_fired_breakpoints_type==3) debug_show_fired_breakpoints_type=0;
}

void menu_debug_settings_show_screen(MENU_ITEM_PARAMETERS)
{
	debug_settings_show_screen.v ^=1;
}
void menu_debug_settings_show_scanline(MENU_ITEM_PARAMETERS)
{
	menu_debug_registers_if_showscan.v ^=1;
}

void menu_debug_configuration_remoteproto(MENU_ITEM_PARAMETERS)
{
	if (remote_protocol_enabled.v) {
		end_remote_protocol();
		remote_protocol_enabled.v=0;
	}

	else {
		remote_protocol_enabled.v=1;
		init_remote_protocol();
	}
}


void menu_debug_verbose(MENU_ITEM_PARAMETERS)
{
	verbose_level++;
	if (verbose_level>4) verbose_level=0;
}

void menu_zesarux_zxi_hardware_debug_file(MENU_ITEM_PARAMETERS)
{

	char *filtros[2];

    filtros[0]="";
    filtros[1]=0;


    if (menu_filesel("Select Debug File",filtros,zesarux_zxi_hardware_debug_file)==1) {
    	//Ver si archivo existe y preguntar
		if (si_existe_archivo(zesarux_zxi_hardware_debug_file)) {
            if (menu_confirm_yesno_texto("File exists","Append?")==0) {
				zesarux_zxi_hardware_debug_file[0]=0;
				return;
			}
        }

    }

	else zesarux_zxi_hardware_debug_file[0]=0;

}

void menu_hardware_debug_port(MENU_ITEM_PARAMETERS)
{
	hardware_debug_port.v ^=1;
}

//menu debug settings
void menu_settings_debug(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_settings_debug;
        menu_item item_seleccionado;
	int retorno_menu;
        do {


      char string_zesarux_zxi_hardware_debug_file_shown[18];
      


		menu_add_item_menu_inicial_format(&array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_registers_console,NULL,"Show r~~egisters in console: %s",(debug_registers==1 ? "On" : "Off"));
		menu_add_item_menu_shortcut(array_menu_settings_debug,'e');

		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_shows_invalid_opcode,NULL,"Show ~~invalid opcode: %s",
			(debug_shows_invalid_opcode.v ? "Yes" : "No") );
		menu_add_item_menu_shortcut(array_menu_settings_debug,'i');
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Show which opcodes are invalid (considering ED, DD, FD prefixes)");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Show which opcodes are invalid (considering ED, DD, FD prefixes). "
								"A message will be shown on console, when verbose level is 2 or higher");


		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_breakpoints_condition_behaviour,NULL,"~~Breakp. behaviour: %s",(debug_breakpoints_cond_behaviour.v ? "On Change" : "Always") );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Indicates whether breakpoints are fired always or only on change from false to true");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Indicates whether breakpoints are fired always or only on change from false to true");
		menu_add_item_menu_shortcut(array_menu_settings_debug,'b');


        menu_add_item_menu(array_menu_settings_debug,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_configuration_stepover,NULL,"Step ~~over interrupt: %s",(remote_debug_settings&32 ? "Yes" : "No") );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Avoid step to step or continuous execution of nmi or maskable interrupt routines on debug cpu menu");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Avoid step to step or continuous execution of nmi or maskable interrupt routines on debug cpu menu");
		menu_add_item_menu_shortcut(array_menu_settings_debug,'o');






		char show_fired_breakpoint_type[30];
		if (debug_show_fired_breakpoints_type==0) strcpy(show_fired_breakpoint_type,"Always");
		else if (debug_show_fired_breakpoints_type==1) strcpy(show_fired_breakpoint_type,"NoPC");
		else strcpy(show_fired_breakpoint_type,"Never");																	//						   OnlyNonPC
																															//  01234567890123456789012345678901
		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_show_fired_breakpoint,NULL,"Show fired breakpoint: %s",show_fired_breakpoint_type);
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Tells to show the breakpoint condition when it is fired");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Tells to show the breakpoint condition when it is fired. "
								"Possible values:\n"
								"Always: always shows the condition\n"
								"NoPC: only shows conditions that are not like PC=XXXX\n"
								"Never: never shows conditions\n" );

		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_show_screen,NULL,"Show display on debug: %s",
			( debug_settings_show_screen.v ? "Yes" : "No") );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"If shows emulated screen on every key action on debug registers menu");	
		menu_add_item_menu_ayuda(array_menu_settings_debug,"If shows emulated screen on every key action on debug registers menu");	


		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_show_scanline,NULL,"Shows electron on debug: %s",
			( menu_debug_registers_if_showscan.v ? "Yes" : "No") );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Shows TV electron position when debugging, using a coloured line. Requires real video");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Shows TV electron position when debugging, using a coloured line. Requires real video");

        menu_add_item_menu(array_menu_settings_debug,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_verbose,NULL,"Verbose ~~level: %d",verbose_level);
		menu_add_item_menu_shortcut(array_menu_settings_debug,'l');


#ifdef USE_PTHREADS
		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_configuration_remoteproto,NULL,"~~Remote protocol: %s",(remote_protocol_enabled.v ? "Enabled" : "Disabled") );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Enables or disables ZEsarUX remote command protocol (ZRCP)");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Enables or disables ZEsarUX remote command protocol (ZRCP)");
		menu_add_item_menu_shortcut(array_menu_settings_debug,'r');

		if (remote_protocol_enabled.v) {
			menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_configuration_remoteproto_port,NULL,"Remote protocol ~~port: %d",remote_protocol_port );
			menu_add_item_menu_tooltip(array_menu_settings_debug,"Changes remote command protocol port");
			menu_add_item_menu_ayuda(array_menu_settings_debug,"Changes remote command protocol port");
			menu_add_item_menu_shortcut(array_menu_settings_debug,'p');
		}

#endif


		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_hardware_debug_port,NULL,"Hardware ~~debug ports: %s",(hardware_debug_port.v ? "Yes" : "No") );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"If hardware debug ports are enabled");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"It shows a ASCII character or a number on console sending some OUT sequence to ports. "
														"Read file docs/zesarux_zxi_registers.txt for more information");
		menu_add_item_menu_shortcut(array_menu_settings_debug,'d');


		if (hardware_debug_port.v) {
			menu_tape_settings_trunc_name(zesarux_zxi_hardware_debug_file,string_zesarux_zxi_hardware_debug_file_shown,18);
        	menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_zesarux_zxi_hardware_debug_file,NULL,"Byte ~~file: %s",string_zesarux_zxi_hardware_debug_file_shown);
			menu_add_item_menu_tooltip(array_menu_settings_debug,"File used on using register 6 (HARDWARE_DEBUG_BYTE_FILE)");		
			menu_add_item_menu_ayuda(array_menu_settings_debug,"File used on using register 6 (HARDWARE_DEBUG_BYTE_FILE)");	
			menu_add_item_menu_shortcut(array_menu_settings_debug,'f');							
		}


                menu_add_item_menu(array_menu_settings_debug,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_settings_debug,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_settings_debug);

                retorno_menu=menu_dibuja_menu(&settings_debug_opcion_seleccionada,&item_seleccionado,array_menu_settings_debug,"Debug Settings" );

                cls_menu_overlay();

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				cls_menu_overlay();
                        }
                }

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}



int num_menu_audio_driver;
int num_previo_menu_audio_driver;


//Determina cual es el audio driver actual
void menu_change_audio_driver_get(void)
{
        int i;
        for (i=0;i<num_audio_driver_array;i++) {
		//printf ("actual: %s buscado: %s indice: %d\n",audio_driver_name,audio_driver_array[i].driver_name,i);
                if (!strcmp(audio_driver_name,audio_driver_array[i].driver_name)) {
                        num_menu_audio_driver=i;
                        num_previo_menu_audio_driver=i;
			return;
                }

        }

}


void menu_change_audio_driver_change(MENU_ITEM_PARAMETERS)
{
        num_menu_audio_driver++;
        if (num_menu_audio_driver==num_audio_driver_array) num_menu_audio_driver=0;
}

void menu_change_audio_driver_apply(MENU_ITEM_PARAMETERS)
{

	audio_end();

        int (*funcion_init) ();
        int (*funcion_set) ();

        funcion_init=audio_driver_array[num_menu_audio_driver].funcion_init;
        funcion_set=audio_driver_array[num_menu_audio_driver].funcion_set;
                if ( (funcion_init()) ==0) {
                        funcion_set();
			menu_generic_message("Apply Driver","OK. Driver applied");
			salir_todos_menus=1;
                }

                else {
                        debug_printf(VERBOSE_ERR,"Can not set audio driver. Restoring to previous driver %s",audio_driver_name);
			menu_change_audio_driver_get();

                        //Restaurar audio driver
                        funcion_init=audio_driver_array[num_previo_menu_audio_driver].funcion_init;
                        funcion_set=audio_driver_array[num_previo_menu_audio_driver].funcion_set;

                        funcion_init();
                        funcion_set();
                }



}


void menu_change_audio_driver(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_change_audio_driver;
        menu_item item_seleccionado;
        int retorno_menu;

       	menu_change_audio_driver_get();

        do {

                menu_add_item_menu_inicial_format(&array_menu_change_audio_driver,MENU_OPCION_NORMAL,menu_change_audio_driver_change,NULL,"Audio Driver: %s",audio_driver_array[num_menu_audio_driver].driver_name );

                menu_add_item_menu_format(array_menu_change_audio_driver,MENU_OPCION_NORMAL,menu_change_audio_driver_apply,NULL,"Apply Driver" );

                menu_add_item_menu(array_menu_change_audio_driver,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_change_audio_driver,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_change_audio_driver);

                retorno_menu=menu_dibuja_menu(&change_audio_driver_opcion_seleccionada,&item_seleccionado,array_menu_change_audio_driver,"Change Audio Driver" );

                cls_menu_overlay();

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                cls_menu_overlay();
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}






int menu_cond_ay_chip(void)
{
	return ay_chip_present.v;
}


void menu_audio_beep_filter_on_rom_save(MENU_ITEM_PARAMETERS)
{
	output_beep_filter_on_rom_save.v ^=1;
}


void menu_audio_beep_alter_volume(MENU_ITEM_PARAMETERS)
{
	output_beep_filter_alter_volume.v ^=1;
}


void menu_audio_beep_volume(MENU_ITEM_PARAMETERS)
{

        char string_vol[4];

        sprintf (string_vol,"%d",output_beep_filter_volume);


        menu_ventana_scanf("Volume (0-127)",string_vol,4);

        int v=parse_string_to_number(string_vol);

        if (v>127 || v<0) {
                debug_printf (VERBOSE_ERR,"Invalid volume value");
                return;
        }

        output_beep_filter_volume=v;
}

void menu_audio_beeper_real (MENU_ITEM_PARAMETERS)
{
	beeper_real_enabled ^=1;
}

void menu_audio_volume(MENU_ITEM_PARAMETERS)
{
        char string_perc[4];

        sprintf (string_perc,"%d",audiovolume);


        menu_ventana_scanf("Volume in %",string_perc,4);

        int v=parse_string_to_number(string_perc);

	if (v>100 || v<0) {
		debug_printf (VERBOSE_ERR,"Invalid volume value");
		return;
	}

	audiovolume=v;
}

void menu_audio_ay_chip(MENU_ITEM_PARAMETERS)
{
	ay_chip_present.v^=1;
}

void menu_audio_ay_chip_autoenable(MENU_ITEM_PARAMETERS)
{
	autoenable_ay_chip.v^=1;
}

void menu_audio_envelopes(MENU_ITEM_PARAMETERS)
{
	ay_envelopes_enabled.v^=1;
}

void menu_audio_speech(MENU_ITEM_PARAMETERS)
{
        ay_speech_enabled.v^=1;
}

void menu_audio_sound_zx8081(MENU_ITEM_PARAMETERS)
{
	zx8081_vsync_sound.v^=1;
}

void menu_audio_zx8081_detect_vsync_sound(MENU_ITEM_PARAMETERS)
{
	zx8081_detect_vsync_sound.v ^=1;
}



void menu_setting_ay_piano_grafico(MENU_ITEM_PARAMETERS)
{
	setting_mostrar_ay_piano_grafico.v ^=1;
}


void menu_aofile_insert(MENU_ITEM_PARAMETERS)
{

	if (aofile_inserted.v==0) {
		init_aofile();

		//Si todo ha ido bien
		if (aofile_inserted.v) {
			menu_generic_message_format("File information","%s\n%s\n\n%s",
			last_message_helper_aofile_vofile_file_format,last_message_helper_aofile_vofile_bytes_minute_audio,last_message_helper_aofile_vofile_util);
		}

	}

        else if (aofile_inserted.v==1) {
                close_aofile();
        }

}

int menu_aofile_cond(void)
{
	if (aofilename!=NULL) return 1;
	else return 0;
}

void menu_aofile(MENU_ITEM_PARAMETERS)
{

	aofile_inserted.v=0;


        char *filtros[3];

#ifdef USE_SNDFILE
        filtros[0]="rwa";
        filtros[1]="wav";
        filtros[2]=0;
#else
        filtros[0]="rwa";
        filtros[1]=0;
#endif


        if (menu_filesel("Select Audio File",filtros,aofilename_file)==1) {

       	        if (si_existe_archivo(aofilename_file)) {

               	        if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) {
				aofilename=NULL;
				return;
			}

       	        }

                aofilename=aofilename_file;


        }

	else {
		aofilename=NULL;
	}


}




/*void menu_audio_audiodac(MENU_ITEM_PARAMETERS)
{
	audiodac_enabled.v ^=1;
}*/

void menu_audio_audiodac_type(MENU_ITEM_PARAMETERS)
{
	if (audiodac_enabled.v==0) {
		audiodac_enabled.v=1;
		audiodac_selected_type=0;
	}

	else {
		audiodac_selected_type++;
		if (audiodac_selected_type==MAX_AUDIODAC_TYPES) {
			audiodac_selected_type=0;
			audiodac_enabled.v=0;
		}
	}
}

void menu_audio_audiodac_set_port(MENU_ITEM_PARAMETERS)
{
	char string_port[4];

	sprintf (string_port,"%02XH",audiodac_types[MAX_AUDIODAC_TYPES-1].port);

	menu_ventana_scanf("Port Value",string_port,4);

	int valor_port=parse_string_to_number(string_port);

	if (valor_port<0 || valor_port>255) {
					debug_printf (VERBOSE_ERR,"Invalid value %d",valor_port);
					return;
	}

	audiodac_set_custom_port(valor_port);
	//audiodac_types[MAX_AUDIODAC_TYPES-1].port=valor_port;
	//audiodac_selected_type=MAX_AUDIODAC_TYPES-1;

}

void menu_audio_beeper(MENU_ITEM_PARAMETERS)
{
	beeper_enabled.v ^=1;
}

void menu_audio_change_ay_chips(MENU_ITEM_PARAMETERS)
{
	if (total_ay_chips==MAX_AY_CHIPS) total_ay_chips=1;
	else total_ay_chips++;

	ay_chip_selected=0;
}

void menu_audio_ay_stereo(MENU_ITEM_PARAMETERS)
{
	ay3_stereo_mode++;

	if (ay3_stereo_mode==5) ay3_stereo_mode=0;
}


void menu_audio_ay_stereo_custom(MENU_ITEM_PARAMETERS)
{
	ay3_custom_stereo_A++;
	if (ay3_custom_stereo_A==3) {
		ay3_custom_stereo_A=2;

		ay3_custom_stereo_B++;
		if (ay3_custom_stereo_B==3) {
			ay3_custom_stereo_B=2;

			ay3_custom_stereo_C++;
			if (ay3_custom_stereo_C==3) {
				ay3_custom_stereo_A=0;
				ay3_custom_stereo_B=0;
				ay3_custom_stereo_C=0;
			}
		}
	}	
}

void menu_audio_ay_stereo_custom_A(MENU_ITEM_PARAMETERS)
{
	ay3_custom_stereo_A++;
	if (ay3_custom_stereo_A==3) ay3_custom_stereo_A=0;
}

void menu_audio_ay_stereo_custom_B(MENU_ITEM_PARAMETERS)
{
	ay3_custom_stereo_B++;
	if (ay3_custom_stereo_B==3) ay3_custom_stereo_B=0;
}

void menu_audio_ay_stereo_custom_C(MENU_ITEM_PARAMETERS)
{
	ay3_custom_stereo_C++;
	if (ay3_custom_stereo_C==3) ay3_custom_stereo_C=0;
}

char *menu_stereo_positions[]={
	"Left",
	"    Center",
	"          Right"
};

void menu_settings_audio(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_settings_audio;
	menu_item item_seleccionado;
	int retorno_menu;

        do {

		menu_add_item_menu_inicial_format(&array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_volume,NULL,"Audio Output ~~Volume: %d %%", audiovolume);
		menu_add_item_menu_shortcut(array_menu_settings_audio,'v');

		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_ay_chip_autoenable,NULL,"Autoenable AY Chip: %s",(autoenable_ay_chip.v==1 ? "On" : "Off"));
		menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable AY Chip automatically when it is needed");
		menu_add_item_menu_ayuda(array_menu_settings_audio,"This option is usefor for example on Spectrum 48k games that uses AY Chip "
					"and for some ZX80/81 games that also uses it (Bi-Pak ZON-X81, but not Quicksilva QS Sound board)");		

		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_ay_chip,NULL,"~~AY Chip: %s", (ay_chip_present.v==1 ? "On" : "Off"));
		menu_add_item_menu_shortcut(array_menu_settings_audio,'a');
		menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable AY Chip on this machine");
		menu_add_item_menu_ayuda(array_menu_settings_audio,"It enables the AY Chip for the machine, by activating the following hardware:\n"
					"-Normal AY Chip for Spectrum\n"
					"-Fuller audio box for Spectrum\n"
					"-Quicksilva QS Sound board on ZX80/81\n"
					"-Bi-Pak ZON-X81 Sound on ZX80/81\n"
			);



			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_change_ay_chips,menu_cond_ay_chip,"Total AY Chips: %d%s",total_ay_chips,
				(total_ay_chips>1 ? ". Turbosound" : "") );

		if (si_complete_video_driver() ) {
			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_setting_ay_piano_grafico,NULL,"Show ~~Piano: %s",
					(setting_mostrar_ay_piano_grafico.v ? "Graphic" : "Text") );
			menu_add_item_menu_shortcut(array_menu_settings_audio,'p');
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Shows AY/Beeper Piano menu with graphic or with text");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Shows AY/Beeper Piano menu with graphic or with text");

		}


		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_envelopes,menu_cond_ay_chip,"AY ~~Envelopes: %s", (ay_envelopes_enabled.v==1 ? "On" : "Off"));
		menu_add_item_menu_shortcut(array_menu_settings_audio,'e');
		menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable or disable volume envelopes for the AY Chip");
		menu_add_item_menu_ayuda(array_menu_settings_audio,"Enable or disable volume envelopes for the AY Chip");

		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_speech,menu_cond_ay_chip,"AY ~~Speech: %s", (ay_speech_enabled.v==1 ? "On" : "Off"));
		menu_add_item_menu_shortcut(array_menu_settings_audio,'s');
		menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable or disable AY Speech effects");
		menu_add_item_menu_ayuda(array_menu_settings_audio,"These effects are used, for example, in Chase H.Q.");


		if (MACHINE_IS_SPECTRUM) {
	//		int ay3_stereo_mode=0;
	/*
    	      0=Mono
        	  1=ACB Stereo (Canal A=Izq,Canal C=Centro,Canal B=Der)
          	2=ABC Stereo (Canal A=Izq,Canal B=Centro,Canal C=Der)
		  	3=BAC Stereo (Canal A=Centro,Canal B=Izquierdo,Canal C=Der)
	*/

			/*if (ay3_stereo_mode==4) {
				//Metemos separador, que queda mas bonito
				menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);

			}*/

			char ay3_stereo_string[16];
			if (ay3_stereo_mode==1) strcpy(ay3_stereo_string,"ACB");
			else if (ay3_stereo_mode==2) strcpy(ay3_stereo_string,"ABC");
			else if (ay3_stereo_mode==3) strcpy(ay3_stereo_string,"BAC");
			else if (ay3_stereo_mode==4) strcpy(ay3_stereo_string,"Custom");
			else strcpy(ay3_stereo_string,"Mono");

			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_ay_stereo,menu_cond_ay_chip,"AY Stereo mode: %s",
				ay3_stereo_string);

			if (ay3_stereo_mode==4) {	

				menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_ay_stereo_custom_A,menu_cond_ay_chip,
					"Channel A: %s",menu_stereo_positions[ay3_custom_stereo_A]);

				menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_ay_stereo_custom_B,menu_cond_ay_chip,
					"Channel B: %s",menu_stereo_positions[ay3_custom_stereo_B]);

				menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_ay_stereo_custom_C,menu_cond_ay_chip,
					"Channel C: %s",menu_stereo_positions[ay3_custom_stereo_C]);								

				//menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);

			}

		}



		if (MACHINE_IS_SPECTRUM) {

			menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);

			char string_audiodac[32];

				if (audiodac_enabled.v) {
					sprintf (string_audiodac,". %s",audiodac_types[audiodac_selected_type].name);
				}
				else {
					strcpy(string_audiodac,"");
				}

				menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_audiodac_type,NULL,"DAC: %s%s",(audiodac_enabled.v ? "On" : "Off" ),
						string_audiodac);
				if (audiodac_enabled.v) {
					menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_audiodac_set_port,NULL,"Port: %02XH",audiodac_types[audiodac_selected_type].port);
				}



		}


    menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		if (!MACHINE_IS_ZX8081) {

			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beeper,NULL,"Beeper: %s",(beeper_enabled.v==1 ? "On" : "Off"));
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable or disable beeper output");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Enable or disable beeper output");

		}



		if (MACHINE_IS_ZX8081) {
			//sound on zx80/81

			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_zx8081_detect_vsync_sound,menu_cond_zx8081,"Detect VSYNC Sound: %s",(zx8081_detect_vsync_sound.v ? "Yes" : "No"));
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Tries to detect when vsync sound is played. This feature is experimental");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Tries to detect when vsync sound is played. This feature is experimental");


			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_sound_zx8081,menu_cond_zx8081,"VSYNC Sound on zx80/81: %s", (zx8081_vsync_sound.v==1 ? "On" : "Off"));
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Enables or disables VSYNC sound on ZX80 and ZX81");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"This method uses the VSYNC signal on the TV to make sound");


		}




		int mostrar_real_beeper=0;

		if (MACHINE_IS_ZX8081) {
			if (zx8081_vsync_sound.v) mostrar_real_beeper=1;
		}

		else {
			if (beeper_enabled.v) mostrar_real_beeper=1;
		}

		if (mostrar_real_beeper) {

			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beeper_real,NULL,"Real ~~Beeper: %s",(beeper_real_enabled==1 ? "On" : "Off"));
			menu_add_item_menu_shortcut(array_menu_settings_audio,'b');
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable or disable Real Beeper enhanced sound. ");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Real beeper produces beeper sound more realistic but uses a bit more cpu. Needs beeper enabled (or vsync sound on zx80/81)");
		}


		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beep_filter_on_rom_save,NULL,"Audio filter on ROM SAVE: %s",(output_beep_filter_on_rom_save.v ? "Yes" : "No"));
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Apply filter on ROM save routines");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"It detects when on ROM save routines and alter audio output to use only "
					"the MIC bit of the FEH port");

//extern z80_bit output_beep_filter_alter_volume;
//extern char output_beep_filter_volume;

			if (output_beep_filter_on_rom_save.v) {
				menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beep_alter_volume,NULL,"Alter beeper volume: %s",
				(output_beep_filter_alter_volume.v ? "Yes" : "No") );

				menu_add_item_menu_tooltip(array_menu_settings_audio,"Alter output beeper volume");
				menu_add_item_menu_ayuda(array_menu_settings_audio,"Alter output beeper volume. You can set to a maximum to "
							"send the audio to a real spectrum to load it");


				if (output_beep_filter_alter_volume.v) {
					menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beep_volume,NULL,"Volume: %d",output_beep_filter_volume);
				}
			}

		}

		//if (si_complete_video_driver() ) {
			//menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_espectro_sonido,NULL,"View ~~Waveform");
			//menu_add_item_menu_shortcut(array_menu_settings_audio,'w');
        //	        menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);
		//}



		menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		char string_aofile_shown[10];
		menu_tape_settings_trunc_name(aofilename,string_aofile_shown,10);
		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_aofile,NULL,"Audio ~~out to file: %s",string_aofile_shown);
		menu_add_item_menu_shortcut(array_menu_settings_audio,'o');
		menu_add_item_menu_tooltip(array_menu_settings_audio,"Saves the generated sound to a file");
		menu_add_item_menu_ayuda(array_menu_settings_audio,"You can save .raw format and if compiled with sndfile, to .wav format. "
					"You can see the file parameters on the console enabling verbose debug level to 2 minimum");



		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_aofile_insert,menu_aofile_cond,"Audio file ~~inserted: %s",(aofile_inserted.v ? "Yes" : "No" ));
		menu_add_item_menu_shortcut(array_menu_settings_audio,'i');


                menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_change_audio_driver,NULL,"Change Audio Driver");


                menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                //menu_add_item_menu(array_menu_settings_audio,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_settings_audio);

                retorno_menu=menu_dibuja_menu(&settings_audio_opcion_seleccionada,&item_seleccionado,array_menu_settings_audio,"Audio Settings" );

                cls_menu_overlay();

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
	                //llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
	                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				cls_menu_overlay();
        	        }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}


void menu_debug_cpu_resumen_stats(MENU_ITEM_PARAMETERS)
{

        char textostats[32];

        menu_espera_no_tecla();
        //menu_dibuja_ventana(0,1,32,18,"CPU Compact Statistics");

		zxvision_window ventana;

	zxvision_new_window(&ventana,0,1,32,18,
							31,16,"CPU Compact Statistics");
	zxvision_draw_window(&ventana);
		

        z80_byte acumulado;

        char dumpassembler[32];

        //Empezar con espacio
        dumpassembler[0]=' ';

				int valor_contador_segundo_anterior;

				valor_contador_segundo_anterior=contador_segundo;

		z80_byte tecla=0;

        do {


                //esto hara ejecutar esto 2 veces por segundo
                //if ( (contador_segundo%500) == 0 || menu_multitarea==0) {
									if ( ((contador_segundo%500) == 0 && valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {
											valor_contador_segundo_anterior=contador_segundo;
                        //contador_segundo_anterior=contador_segundo;
												//printf ("Refrescando. contador_segundo=%d\n",contador_segundo);

			int linea=0;
                        int opcode;

			unsigned int sumatotal;
                        sumatotal=util_stats_sum_all_counters();
                        sprintf (textostats,"Total opcodes run: %u",sumatotal);
						//menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(&ventana,1,linea++,textostats);
                        


						//menu_escribe_linea_opcion(linea++,-1,1,"Most used op. for each preffix");
						zxvision_print_string_defaults(&ventana,1,linea++,"Most used op. for each preffix");

                        opcode=util_stats_find_max_counter(stats_codsinpr);
                        sprintf (textostats,"Op nopref:    %02XH: %u",opcode,util_stats_get_counter(stats_codsinpr,opcode) );
						//menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(&ventana,1,linea++,textostats);
                        

                        //Opcode
						menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],0,0);
						//menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(&ventana,1,linea++,dumpassembler);
						



                        opcode=util_stats_find_max_counter(stats_codpred);
                        sprintf (textostats,"Op pref ED:   %02XH: %u",opcode,util_stats_get_counter(stats_codpred,opcode) );
						//menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(&ventana,1,linea++,textostats);
                        

                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],237,0);
						//menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(&ventana,1,linea++,dumpassembler);
                        

	
                        opcode=util_stats_find_max_counter(stats_codprcb);
                        sprintf (textostats,"Op pref CB:   %02XH: %u",opcode,util_stats_get_counter(stats_codprcb,opcode) );
						//menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(&ventana,1,linea++,textostats);


                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],203,0);
						//menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(&ventana,1,linea++,dumpassembler);




                        opcode=util_stats_find_max_counter(stats_codprdd);
                        sprintf (textostats,"Op pref DD:   %02XH: %u",opcode,util_stats_get_counter(stats_codprdd,opcode) );
                        //menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(&ventana,1,linea++,textostats);

                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],221,0);
                        //menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(&ventana,1,linea++,dumpassembler);


                        opcode=util_stats_find_max_counter(stats_codprfd);
                        sprintf (textostats,"Op pref FD:   %02XH: %u",opcode,util_stats_get_counter(stats_codprfd,opcode) );
                        //menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(&ventana,1,linea++,textostats);

                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],253,0);
                        //menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(&ventana,1,linea++,dumpassembler);


                        opcode=util_stats_find_max_counter(stats_codprddcb);
                        sprintf (textostats,"Op pref DDCB: %02XH: %u",opcode,util_stats_get_counter(stats_codprddcb,opcode) );
                        //menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(&ventana,1,linea++,textostats);

                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],221,203);
                        //menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(&ventana,1,linea++,dumpassembler);



                        opcode=util_stats_find_max_counter(stats_codprfdcb);
                        sprintf (textostats,"Op pref FDCB: %02XH: %u",opcode,util_stats_get_counter(stats_codprfdcb,opcode) );
                        //menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(&ventana,1,linea++,textostats);

                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],253,203);
                        //menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(&ventana,1,linea++,dumpassembler);


						zxvision_draw_window_contents(&ventana);

                        if (menu_multitarea==0) menu_refresca_pantalla();

                }

                menu_cpu_core_loop();
                //acumulado=menu_da_todas_teclas();

                //si no hay multitarea, esperar tecla y salir
                if (menu_multitarea==0) {
                        menu_espera_tecla();

                        //acumulado=0;
                }

				//tecla=menu_get_pressed_key();
				tecla=zxvision_read_keyboard();

				//con enter no salimos. TODO: esto se hace porque el mouse esta enviando enter al pulsar boton izquierdo, y lo hace tambien al hacer dragging
				//lo ideal seria que mouse no enviase enter al pulsar boton izquierdo y entonces podemos hacer que se salga tambien con enter
				if (tecla==13) tecla=0;

        //} while (  (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA && tecla==0)  ;

		} while (tecla==0);

        cls_menu_overlay();

		zxvision_destroy_window(&ventana);

}



void zxvision_test_sleep_quarter(void)
{
	int previo_contador_segundo=contador_segundo;

	while (1) {
		menu_cpu_core_loop();
		if (previo_contador_segundo!=contador_segundo && (contador_segundo%250)==0) return;

		if (menu_get_pressed_key()!=0) return;
	
	}
}


void menu_zxvision_test(MENU_ITEM_PARAMETERS)
{

        //Desactivamos interlace - si esta. Con interlace la forma de onda se dibuja encima continuamente, sin borrar
        //z80_bit copia_video_interlaced_mode;
        //copia_video_interlaced_mode.v=video_interlaced_mode.v;

        //disable_interlace();


        menu_espera_no_tecla();


		//zxvision_generic_message_tooltip("pruebas", 30, 0, 0, generic_message_tooltip_return *retorno, const char * texto_format , ...)
		zxvision_generic_message_tooltip("Pruebas", 0, 0, 0, NULL, "%s", "Hola que tal como estas esto es una prueba de escribir texto. "
					"No se que mas poner pero me voy a empezar a repetir, " 
					"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore "
					"et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip"
					" ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore "
					" eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia "
					"deserunt mollit anim id est laborum. Adios");
        //z80_byte acumulado;



        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de audio waveform
	//set_menu_overlay_function(menu_audio_draw_sound_wave);

	zxvision_window ventana;

#define SOUND_ZXVISION_WAVE_X 1
#define SOUND_ZXVISION_WAVE_Y 3
#define SOUND_ZXVISION_WAVE_ANCHO 27
#define SOUND_ZXVISION_WAVE_ALTO 14

	int ancho_visible=SOUND_ZXVISION_WAVE_ANCHO;
	int alto_visible=SOUND_ZXVISION_WAVE_ALTO+4;

	int ancho_total=20;
	int alto_total=alto_visible+2;

	menu_item *array_menu_audio_new_waveform;
        menu_item item_seleccionado;
        int retorno_menu;
        


	  //Hay que redibujar la ventana desde este bucle
	//menu_dibuja_ventana(SOUND_WAVE_X,SOUND_WAVE_Y-2,SOUND_WAVE_ANCHO,SOUND_WAVE_ALTO+4,"Waveform");
	zxvision_new_window(&ventana,SOUND_ZXVISION_WAVE_X,SOUND_ZXVISION_WAVE_Y-2,ancho_visible,alto_visible,
							ancho_total,alto_total,"ZXVision Test");
	zxvision_draw_window(&ventana);

	printf ("Created window\n");

	menu_espera_tecla();
	menu_espera_no_tecla();

	zxvision_draw_window_contents(&ventana);

	printf ("Drawn window contents\n");

	menu_espera_tecla(); 
	menu_espera_no_tecla();


	overlay_screen caracter;

/*	struct s_overlay_screen {
	z80_byte tinta,papel,parpadeo;
	z80_byte caracter;
};*/

	caracter.tinta=ESTILO_GUI_TINTA_NORMAL;
	caracter.papel=ESTILO_GUI_PAPEL_NORMAL;
	caracter.parpadeo=0;

	//Relleno pantalla
	z80_byte caracter_print=32;



	int x,y;

	for (y=0;y<alto_total;y++) {
		for (x=0;x<ancho_total;x++) {
			caracter.caracter=caracter_print;
			zxvision_print_char(&ventana,x,y,&caracter);	

			caracter_print++;
			if (caracter_print>126) caracter_print=32;		
		}
	}

	caracter.caracter='A';

	zxvision_print_char(&ventana,0,0,&caracter);

	caracter.caracter='B';

	zxvision_print_char(&ventana,0,1,&caracter);


	zxvision_print_string(&ventana,2,4,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," This is a test ");

	zxvision_print_string(&ventana,2,5,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," Press a key ");
	zxvision_print_string(&ventana,2,6,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," to next step ");

	zxvision_print_string(&ventana,2,7,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0," --^^flash^^--");
	zxvision_print_string(&ventana,2,8,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0," --~~inverse--");
	zxvision_print_string(&ventana,2,9,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0," --$$2ink--");


                    /*    menu_add_item_menu_inicial_format(&array_menu_audio_new_waveform,MENU_OPCION_NORMAL,menu_audio_new_waveform_shape,NULL,"Change wave ~~Shape");
                        menu_add_item_menu_shortcut(array_menu_audio_new_waveform,'s');

                        //Evito tooltips en los menus tabulados que tienen overlay porque al salir el tooltip detiene el overlay
                        //menu_add_item_menu_tooltip(array_menu_audio_new_waveform,"Change wave Shape");
                        menu_add_item_menu_ayuda(array_menu_audio_new_waveform,"Change wave Shape: simple line or vertical fill");
						//0123456789
						// Change wave Shape
						
			menu_add_item_menu_tabulado(array_menu_audio_new_waveform,1,0);





		//Nombre de ventana solo aparece en el caso de stdout
                retorno_menu=menu_dibuja_menu(&audio_new_waveform_opcion_seleccionada,&item_seleccionado,array_menu_audio_new_waveform,"Waveform" );*/


	menu_espera_tecla();
	menu_espera_no_tecla();

	zxvision_draw_window_contents(&ventana);

	menu_espera_tecla(); 
	menu_espera_no_tecla();

	//Jugar con offset
	int i;
/*
	for (i=0;i<7;i++) {
		zxvision_set_offset_x(&ventana,i);

		zxvision_draw_window_contents(&ventana);

		printf ("Offset x %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}


	for (i=0;i<7;i++) {
		zxvision_set_offset_y(&ventana,i);

		zxvision_draw_window_contents(&ventana);

		printf ("Offset y %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}



	for (i=0;i<10;i++) {
		zxvision_set_x_position(&ventana,i);

		printf ("Move x %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}

	for (i=0;i<10;i++) {
		zxvision_set_y_position(&ventana,i);

		printf ("Move y %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}

	zxvision_set_x_position(&ventana,0);
	zxvision_set_y_position(&ventana,0);

	for (i=25;i<35;i++) {
		zxvision_set_visible_width(&ventana,i);

		printf ("width %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}	

	for (i=18;i<28;i++) {
		zxvision_set_visible_height(&ventana,i);

		printf ("height %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}	


	for (i=5;i>=0;i--) {
		zxvision_set_visible_width(&ventana,i);

		printf ("width %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}	

	zxvision_set_visible_width(&ventana,20);

	for (i=5;i>=0;i--) {
		zxvision_set_visible_height(&ventana,i);

		printf ("height %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}	


*/
	zxvision_print_string(&ventana,2,5,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," Use cursors ");
	zxvision_print_string(&ventana,2,6,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," to move offset ");	
	zxvision_print_string(&ventana,2,7,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," QAOP size");	
	zxvision_print_string(&ventana,2,8,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," ESC exit ");	


	//Rebotar
	int contador=0;

	int xpos=0;
	int ypos=0;

	int incx=+1;
	int incy=+1;

	int offsetx=0;
	int offsety=0;	

	int ancho=22;
	int alto=10;

	zxvision_set_visible_height(&ventana,alto);
	zxvision_set_visible_width(&ventana,ancho);	

	z80_byte tecla=0;

	//Salir con ESC
	while (tecla!=2) {

		zxvision_set_offset_x(&ventana,offsetx);
		zxvision_set_offset_y(&ventana,offsety);

		zxvision_set_x_position(&ventana,xpos);
		zxvision_set_y_position(&ventana,ypos);		

		zxvision_test_sleep_quarter();

		tecla=menu_get_pressed_key();
		//Cambio offset con cursores
		if (tecla==8) {
			offsetx--;
			printf ("Decrement offset x to %d\n",offsetx);
		}

		if (tecla==9) {
			offsetx++;
			printf ("Increment offset x to %d\n",offsetx);
		}

		if (tecla==10) {
			offsety++;
			printf ("Increment offset y to %d\n",offsety);
		}

		if (tecla==11) {
			offsety--;
			printf ("Decrement offset y to %d\n",offsety);
		}

		//Cambio tamanyo
		if (tecla=='a' && ypos+alto<24) {
			alto++;
			printf ("Increment height to %d\n",alto);
			zxvision_set_visible_height(&ventana,alto);
		}

		if (tecla=='q' && alto>1) {
			alto--;
			printf ("Decrement height to %d\n",alto);
			zxvision_set_visible_height(&ventana,alto);
		}

		if (tecla=='p' && xpos+ancho<32) {
			ancho++;
			printf ("Increment width to %d\n",ancho);
			zxvision_set_visible_width(&ventana,ancho);
		}

		if (tecla=='o' && ancho>7) {
			ancho--;
			printf ("Decrement width to %d\n",ancho);
			zxvision_set_visible_width(&ventana,ancho);
		}

		xpos +=incx;
		if (xpos+ancho>=32 || xpos<=0) {
			incx=-incx;
		}

		ypos +=incy;
		if (ypos+alto>=24 || ypos<=0) {
			incy=-incy;
		}		
		
		contador++;

		if (tecla!=0) menu_espera_no_tecla();

	}


	zxvision_print_string(&ventana,1,5,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," Use mouse    ");
	zxvision_print_string(&ventana,1,6,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," to move and   ");	
	zxvision_print_string(&ventana,1,7,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," resize window  ");	
	zxvision_print_string(&ventana,1,8,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," Right button exits ");	

	zxvision_draw_window_contents(&ventana);

	tecla=0;

	while (!mouse_right) {
		//menu_espera_tecla();
		tecla=menu_get_pressed_key();
		//Comprobar eventos raton
		menu_cpu_core_loop();
		//zxvision_handle_mouse_events(&ventana);
	} 

	zxvision_destroy_window(&ventana);
            


        cls_menu_overlay();

}


void menu_about_core_statistics(MENU_ITEM_PARAMETERS)
{

    //char textostats[32];

    menu_espera_no_tecla();
    //menu_dibuja_ventana(0,7,32,9,"Core Statistics");

	zxvision_window ventana;

	zxvision_new_window(&ventana,0,7,32,9,
							31,7,"Core Statistics");

	zxvision_draw_window(&ventana);

    z80_byte acumulado;

        char texto_buffer[33];


        //Empezar con espacio
    texto_buffer[0]=' ';

        int valor_contador_segundo_anterior;

        valor_contador_segundo_anterior=contador_segundo;


		z80_byte tecla=0;

        do {


                //esto hara ejecutar esto 2 veces por segundo
            //if ( (contador_segundo%500) == 0 || menu_multitarea==0) {
                        if ( ((contador_segundo%500) == 0 && valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {
                                                                                        valor_contador_segundo_anterior=contador_segundo;
                        //contador_segundo_anterior=contador_segundo;
                                                                                                //printf ("Refrescando. contador_segundo=%d\n",contador_segundo);

                                int linea=0;
                                //int opcode;
                                //int sumatotal;

/*

Nota: calcular el tiempo entre ejecuciones de cada opcode no penaliza mucho el uso de cpu real.
Ejemplo:
--vo null --machine 48k 

Sin calcular ese tiempo: 9% cpu
Calculando ese tiempo: 12% cpu

*/


//Ultimo intervalo de tiempo
//long core_cpu_timer_frame_difftime;
//Media de todos los intervalos
//long core_cpu_timer_frame_media=0;

				long valor_mostrar;
				valor_mostrar=core_cpu_timer_frame_difftime;
				//controlar maximos
				if (valor_mostrar>999999) valor_mostrar=999999;
			     //01234567890123456789012345678901
			     // Last core frame: 999999 us
				sprintf (texto_buffer,"Last core frame:     %6ld us",valor_mostrar);
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);	
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

                                valor_mostrar=core_cpu_timer_frame_media;
                                //controlar maximos
				if (valor_mostrar>999999) valor_mostrar=999999;
                                //01234567890123456789012345678901
                                 // Last core frame: 999999 us
                                sprintf (texto_buffer," Average: %6ld us",valor_mostrar);
                                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
								zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);


                                valor_mostrar=core_cpu_timer_refresca_pantalla_difftime;
                                //controlar maximos
                                if (valor_mostrar>999999) valor_mostrar=999999;
                             //01234567890123456789012345678901
                             // Last render display: 999999 us
                                sprintf (texto_buffer,"Last full render:    %6ld us",valor_mostrar);
                                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
								zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

                                valor_mostrar=core_cpu_timer_refresca_pantalla_media;
                                //controlar maximos
                                if (valor_mostrar>999999) valor_mostrar=999999;
                                //01234567890123456789012345678901
                                 // Last core refresca_pantalla: 999999 us
                                sprintf (texto_buffer," Average: %6ld us",valor_mostrar);
                                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
								zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);


                                valor_mostrar=core_cpu_timer_each_frame_difftime;
                                //controlar maximos
                                if (valor_mostrar>999999) valor_mostrar=999999;
                             //01234567890123456789012345678901
                             // Time between frames: 999999 us
                                sprintf (texto_buffer,"Time between frames: %6ld us",valor_mostrar);
                                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
								zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

                                valor_mostrar=core_cpu_timer_each_frame_media;
                                //controlar maximos
                                if (valor_mostrar>999999) valor_mostrar=999999;
                                //01234567890123456789012345678901
                                 // Last core each_frame: 999999 us
                                sprintf (texto_buffer," Average: %6ld us",valor_mostrar);
                                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
								zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

								 //menu_escribe_linea_opcion(linea++,-1,1," (ideal):  20000 us");
								 zxvision_print_string_defaults(&ventana,1,linea++," (ideal):  20000 us");


								zxvision_draw_window_contents(&ventana);

                        if (menu_multitarea==0) menu_refresca_pantalla();

                }

                menu_cpu_core_loop();
                //acumulado=menu_da_todas_teclas();

                //si no hay multitarea, esperar tecla y salir
                if (menu_multitarea==0) {
                        menu_espera_tecla();

                        //acumulado=0;
                }

				//tecla=menu_get_pressed_key();
				tecla=zxvision_read_keyboard();

				//con enter no salimos. TODO: esto se hace porque el mouse esta enviando enter al pulsar boton izquierdo, y lo hace tambien al hacer dragging
				//lo ideal seria que mouse no enviase enter al pulsar boton izquierdo y entonces podemos hacer que se salga tambien con enter
				if (tecla==13) tecla=0;

        //} while (  (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA && tecla==0)  ;

		} while (tecla==0);

        cls_menu_overlay();

		zxvision_destroy_window(&ventana);


}




int ayregisters_previo_valor_volume_A[MAX_AY_CHIPS];
int ayregisters_previo_valor_volume_B[MAX_AY_CHIPS];
int ayregisters_previo_valor_volume_C[MAX_AY_CHIPS];

	int menu_ayregisters_valor_contador_segundo_anterior;

zxvision_window *menu_ay_registers_overlay_window;

void menu_ay_registers_overlay(void)
{

	//NOTA: //Hemos de suponer que current window es esta de ay registers

    normal_overlay_texto_menu();

	char volumen[32],textotono[32];
	char textovolumen[35]; //32+3 de posible color rojo del maximo


	int total_chips=ay_retorna_numero_chips();

	//Como maximo mostrarmos 3 canales ay
	if (total_chips>3) total_chips=3;

	int chip;

	int linea=0;

	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

		int vol_A[MAX_AY_CHIPS],vol_B[MAX_AY_CHIPS],vol_C[MAX_AY_CHIPS];

	for (chip=0;chip<total_chips;chip++) {


        	vol_A[chip]=ay_3_8912_registros[chip][8] & 15;
        	vol_B[chip]=ay_3_8912_registros[chip][9] & 15;
        	vol_C[chip]=ay_3_8912_registros[chip][10] & 15;

			//Controlar limites, dado que las variables entran sin inicializar
			if (ayregisters_previo_valor_volume_A[chip]>16) ayregisters_previo_valor_volume_A[chip]=16;
			if (ayregisters_previo_valor_volume_B[chip]>16) ayregisters_previo_valor_volume_B[chip]=16;
			if (ayregisters_previo_valor_volume_C[chip]>16) ayregisters_previo_valor_volume_C[chip]=16;
			

			ayregisters_previo_valor_volume_A[chip]=menu_decae_ajusta_valor_volumen(ayregisters_previo_valor_volume_A[chip],vol_A[chip]);
			ayregisters_previo_valor_volume_B[chip]=menu_decae_ajusta_valor_volumen(ayregisters_previo_valor_volume_B[chip],vol_B[chip]);
			ayregisters_previo_valor_volume_C[chip]=menu_decae_ajusta_valor_volumen(ayregisters_previo_valor_volume_C[chip],vol_C[chip]);


        		//if (ayregisters_previo_valor_volume_A[chip]<vol_A[chip]) ayregisters_previo_valor_volume_A[chip]=vol_A[chip];
        		//if (ayregisters_previo_valor_volume_B[chip]<vol_B[chip]) ayregisters_previo_valor_volume_B[chip]=vol_B[chip];
        		//if (ayregisters_previo_valor_volume_C[chip]<vol_C[chip]) ayregisters_previo_valor_volume_C[chip]=vol_C[chip];


			menu_string_volumen(volumen,ay_3_8912_registros[chip][8],ayregisters_previo_valor_volume_A[chip]);
			sprintf (textovolumen,"Volume A: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textovolumen);

			menu_string_volumen(volumen,ay_3_8912_registros[chip][9],ayregisters_previo_valor_volume_B[chip]);
			sprintf (textovolumen,"Volume B: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textovolumen);

			menu_string_volumen(volumen,ay_3_8912_registros[chip][10],ayregisters_previo_valor_volume_C[chip]);
			sprintf (textovolumen,"Volume C: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textovolumen);



			int freq_a=ay_retorna_frecuencia(0,chip);
			int freq_b=ay_retorna_frecuencia(1,chip);
			int freq_c=ay_retorna_frecuencia(2,chip);
			sprintf (textotono,"Channel A:  %3s %7d Hz",get_note_name(freq_a),freq_a);
			//menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);

			sprintf (textotono,"Channel B:  %3s %7d Hz",get_note_name(freq_b),freq_b);
			//menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);

			sprintf (textotono,"Channel C:  %3s %7d Hz",get_note_name(freq_c),freq_c);
			//menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);


			//Si hay 3 canales, los 3 siguientes items no se ven
			if (total_chips<3) {

			                        //Frecuencia ruido
                        int freq_temp=ay_3_8912_registros[chip][6] & 31;
                        //printf ("Valor registros ruido : %d Hz\n",freq_temp);
                        freq_temp=freq_temp*16;

                        //controlamos divisiones por cero
                        if (!freq_temp) freq_temp++;

                        int freq_ruido=FRECUENCIA_NOISE/freq_temp;

                        sprintf (textotono,"Frequency Noise: %6d Hz",freq_ruido);
                        //menu_escribe_linea_opcion(linea++,-1,1,textotono);
						zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);


			//Envelope

                        freq_temp=ay_3_8912_registros[chip][11]+256*(ay_3_8912_registros[chip][12] & 0xFF);


                        //controlamos divisiones por cero
                        if (!freq_temp) freq_temp++;
                        int freq_envelope=FRECUENCIA_ENVELOPE/freq_temp;

                        //sprintf (textotono,"Freq Envelope(*10): %5d Hz",freq_envelope);

			int freq_env_10=freq_envelope/10;
			int freq_env_decimal=freq_envelope-(freq_env_10*10);

			sprintf (textotono,"Freq Envelope:   %4d.%1d Hz",freq_env_10,freq_env_decimal);
      		//menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);



			char envelope_name[32];
			z80_byte env_type=ay_3_8912_registros[chip][13] & 0x0F;
			return_envelope_name(env_type,envelope_name);
			sprintf (textotono,"Env.: %2d (%s)",env_type,envelope_name);
            //menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);


			}



			sprintf (textotono,"Tone Channels:  %c %c %c",
				( (ay_3_8912_registros[chip][7]&1)==0 ? 'A' : ' '),
				( (ay_3_8912_registros[chip][7]&2)==0 ? 'B' : ' '),
				( (ay_3_8912_registros[chip][7]&4)==0 ? 'C' : ' '));
			//menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);

			//Si hay 3 canales, los 3 siguientes items no se ven
			if (total_chips<3) {

                        sprintf (textotono,"Noise Channels: %c %c %c",
                                ( (ay_3_8912_registros[chip][7]&8)==0  ? 'A' : ' '),
                                ( (ay_3_8912_registros[chip][7]&16)==0 ? 'B' : ' '),
                                ( (ay_3_8912_registros[chip][7]&32)==0 ? 'C' : ' '));
                        //menu_escribe_linea_opcion(linea++,-1,1,textotono);
						zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);
			}

	}




	//Hacer decaer volumenes
                        //Decrementar volumenes que caen, pero hacerlo no siempre, sino 2 veces por segundo
                    //esto hara ejecutar esto 2 veces por segundo
                        if ( ((contador_segundo%500) == 0 && menu_ayregisters_valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {

                                 menu_ayregisters_valor_contador_segundo_anterior=contador_segundo;
                                //printf ("Refrescando. contador_segundo=%d. chip: %d\n",contador_segundo,chip);

				for (chip=0;chip<total_chips;chip++) {

					ayregisters_previo_valor_volume_A[chip]=menu_decae_dec_valor_volumen(ayregisters_previo_valor_volume_A[chip],vol_A[chip]);
					ayregisters_previo_valor_volume_B[chip]=menu_decae_dec_valor_volumen(ayregisters_previo_valor_volume_B[chip],vol_B[chip]);
					ayregisters_previo_valor_volume_C[chip]=menu_decae_dec_valor_volumen(ayregisters_previo_valor_volume_C[chip],vol_C[chip]);

                                //if (ayregisters_previo_valor_volume_A[chip]>vol_A[chip]) ayregisters_previo_valor_volume_A[chip]--;
                                //if (ayregisters_previo_valor_volume_B[chip]>vol_B[chip]) ayregisters_previo_valor_volume_B[chip]--;
                                //if (ayregisters_previo_valor_volume_C[chip]>vol_C[chip]) ayregisters_previo_valor_volume_C[chip]--;

				}


        }


	zxvision_draw_window_contents(menu_ay_registers_overlay_window); 


}



void menu_ay_registers(MENU_ITEM_PARAMETERS)
{
        menu_espera_no_tecla();

		if (!menu_multitarea) {
			menu_warn_message("This menu item needs multitask enabled");
			return;
		}

		int total_chips=ay_retorna_numero_chips();
		if (total_chips>3) total_chips=3;

		int yventana;
		int alto_ventana;

        if (total_chips==1) {
			//menu_dibuja_ventana(1,5,30,14,"AY Registers");
			yventana=5;
			alto_ventana=14;
		}
		else {
			//menu_dibuja_ventana(1,0,30,24,"AY Registers");
			yventana=0;
			alto_ventana=24;
		}

		zxvision_window ventana;

		//menu_dibuja_ventana(1,yventana,30,alto_ventana,"AY Registers");
		zxvision_new_window(&ventana,1,yventana,30,alto_ventana,
							30-1,alto_ventana-2,"AY Registers");

		zxvision_draw_window(&ventana);		

        z80_byte acumulado;


        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de onda + texto
        set_menu_overlay_function(menu_ay_registers_overlay);

		menu_ay_registers_overlay_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui

				int valor_contador_segundo_anterior;

				valor_contador_segundo_anterior=contador_segundo;

	z80_byte tecla=0;
   do {

                //esto hara ejecutar esto 2 veces por segundo
                //if ( (contador_segundo%500) == 0 || menu_multitarea==0) {
									if ( ((contador_segundo%500) == 0 && valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {
		valor_contador_segundo_anterior=contador_segundo;

										//printf ("Refrescando. contador_segundo=%d\n",contador_segundo);
                       if (menu_multitarea==0) menu_refresca_pantalla();


                }


				menu_cpu_core_loop();
                //acumulado=menu_da_todas_teclas();

                //si no hay multitarea, esperar tecla y salir
                if (menu_multitarea==0) {
                        menu_espera_tecla();

                        //acumulado=0;
                }

				//tecla=menu_get_pressed_key();
				tecla=zxvision_read_keyboard();

				//con enter no salimos. TODO: esto se hace porque el mouse esta enviando enter al pulsar boton izquierdo, y lo hace tambien al hacer dragging
				//lo ideal seria que mouse no enviase enter al pulsar boton izquierdo y entonces podemos hacer que se salga tambien con enter
				if (tecla==13) tecla=0;

        //} while (  (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA && tecla==0)  ;

		} while (tecla!=2);				

 
		menu_espera_no_tecla(); //Si no, se va al menu anterior.
		//En AY Piano por ejemplo esto no pasa aunque el estilo del menu es el mismo...

       //restauramos modo normal de texto de menu
       set_menu_overlay_function(normal_overlay_texto_menu);


    cls_menu_overlay();
		
	zxvision_destroy_window(&ventana);		
}




void menu_debug_tsconf_tbblue_videoregisters(MENU_ITEM_PARAMETERS)
{

    //char textostats[32];

    menu_espera_no_tecla();
	int xventana=0;
	int ancho_ventana=32;

	int yventana;
	int alto_ventana;
    

	if (MACHINE_IS_TBBLUE) {
		yventana=2;
		alto_ventana=19;
	}

	else {
		yventana=7;
		alto_ventana=8;
	}

	zxvision_window ventana;

		zxvision_new_window(&ventana,xventana,yventana,ancho_ventana,alto_ventana,
							ancho_ventana-1,alto_ventana-2,"Video Info");							

	zxvision_draw_window(&ventana);


    z80_byte acumulado;

	char texto_buffer[64];

	char texto_buffer2[64];

	//Empezar con espacio
    texto_buffer[0]=' ';

	int valor_contador_segundo_anterior;

	valor_contador_segundo_anterior=contador_segundo;

	z80_byte tecla=0;


    	do {


        	//esto hara ejecutar esto 2 veces por segundo
            //if ( (contador_segundo%500) == 0 || menu_multitarea==0) {
			if ( ((contador_segundo%500) == 0 && valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {
											valor_contador_segundo_anterior=contador_segundo;
                        //contador_segundo_anterior=contador_segundo;
												//printf ("Refrescando. contador_segundo=%d\n",contador_segundo);

				int linea=0;
				//int opcode;
				//int sumatotal;


				if (MACHINE_IS_TSCONF) {

				int vpage_addr=tsconf_get_vram_page()*16384;

				tsconf_get_current_video_mode(texto_buffer2);
				sprintf (texto_buffer,"Video mode: %s",texto_buffer2);
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);
				
                sprintf (texto_buffer,"Video addr: %06XH",vpage_addr);
                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

				sprintf (texto_buffer,"Tile Map Page: %06XH",tsconf_return_tilemappage() );
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

				sprintf (texto_buffer,"Tile 0 Graphics addr: %06XH",tsconf_return_tilegraphicspage(0) );
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

				sprintf (texto_buffer,"Tile 1 Graphics addr: %06XH",tsconf_return_tilegraphicspage(1) );
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);


				sprintf (texto_buffer,"Sprite Graphics addr: %06XH",tsconf_return_spritesgraphicspage() );
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

				}

				if (MACHINE_IS_TBBLUE) {

					//menu_escribe_linea_opcion(linea++,-1,1,"ULA Video mode:");		
					zxvision_print_string_defaults(&ventana,1,linea++,"ULA Video mode:");

					//menu_escribe_linea_opcion(linea++,-1,1,get_spectrum_ula_string_video_mode() );
					zxvision_print_string_defaults(&ventana,1,linea++,get_spectrum_ula_string_video_mode() );

					linea++;

					//menu_escribe_linea_opcion(linea++,-1,1,"Palette format:");
					zxvision_print_string_defaults(&ventana,1,linea++,"Palette format:");

					tbblue_get_string_palette_format(texto_buffer);
					//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					linea++;

					/*
					(R/W) 0x12 (18) => Layer 2 RAM page
 bits 7-6 = Reserved, must be 0
 bits 5-0 = SRAM page (point to page 8 after a Reset)

(R/W) 0x13 (19) => Layer 2 RAM shadow page
 bits 7-6 = Reserved, must be 0
 bits 5-0 = SRAM page (point to page 11 after a Reset)
					*/

				//tbblue_get_offset_start_layer2_reg
					//menu_escribe_linea_opcion(linea++,-1,1,"Layer 2 RAM page");
					sprintf (texto_buffer,"Layer 2 addr:        %06XH",tbblue_get_offset_start_layer2_reg(tbblue_registers[18]) );
					//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					//menu_escribe_linea_opcion(linea++,-1,1,"Layer 2 RAM shadow page");
					sprintf (texto_buffer,"Layer 2 shadow addr: %06XH",tbblue_get_offset_start_layer2_reg(tbblue_registers[19]) );					
					//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);


					/*
					z80_byte clip_window_layer2[4];
z80_byte clip_window_layer2_index;

z80_byte clip_window_sprites[4];
z80_byte clip_window_sprites_index;

z80_byte clip_window_ula[4];
					*/

					linea++;
					sprintf (texto_buffer,"Clip Windows:");
					//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					sprintf (texto_buffer,"Layer2:  X=%3d-%3d Y=%3d-%3d",
					clip_window_layer2[0],clip_window_layer2[1],clip_window_layer2[2],clip_window_layer2[3]);
					//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					sprintf (texto_buffer,"Sprites: X=%3d-%3d Y=%3d-%3d",
					clip_window_sprites[0],clip_window_sprites[1],clip_window_sprites[2],clip_window_sprites[3]);
					//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					sprintf (texto_buffer,"ULA:     X=%3d-%3d Y=%3d-%3d",
					clip_window_ula[0],clip_window_ula[1],clip_window_ula[2],clip_window_ula[3]);
					//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					linea++;
					sprintf (texto_buffer,"Offset Windows:");
					//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);	
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					sprintf (texto_buffer,"Layer2: X=%3d Y=%3d",tbblue_registers[22],tbblue_registers[23]);
					//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);


					sprintf (texto_buffer,"LoRes:  X=%3d Y=%3d",tbblue_registers[50],tbblue_registers[51]);
					//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);


				}



				zxvision_draw_window_contents(&ventana);

                        if (menu_multitarea==0) menu_refresca_pantalla();

                }

                menu_cpu_core_loop();
                //acumulado=menu_da_todas_teclas();

                //si no hay multitarea, esperar tecla y salir
                if (menu_multitarea==0) {
                        menu_espera_tecla();

                        //acumulado=0;
                }

				//tecla=menu_get_pressed_key();
				tecla=zxvision_read_keyboard();

				//con enter no salimos. TODO: esto se hace porque el mouse esta enviando enter al pulsar boton izquierdo, y lo hace tambien al hacer dragging
				//lo ideal seria que mouse no enviase enter al pulsar boton izquierdo y entonces podemos hacer que se salga tambien con enter
				if (tecla==13) tecla=0;

        //} while (  (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA && tecla==0)  ;

		} while (tecla==0);

        cls_menu_overlay();

		zxvision_destroy_window(&ventana);


}








//int tsconf_spritenav_window_y=2;
//int tsconf_spritenav_window_alto=20;

#define TSCONF_SPRITENAV_WINDOW_X 0
#define TSCONF_SPRITENAV_WINDOW_Y 2
#define TSCONF_SPRITENAV_WINDOW_ANCHO 32
#define TSCONF_SPRITENAV_WINDOW_ALTO 20
//#define TSCONF_SPRITENAV_SPRITES_PER_WINDOW 8
//#define TSCONF_SPRITENAV_SPRITES_PER_WINDOW ((tsconf_spritenav_window_alto-4)/2)





//int menu_debug_tsconf_tbblue_spritenav_current_palette=0;
int menu_debug_tsconf_tbblue_spritenav_current_sprite=0;


zxvision_window *menu_debug_tsconf_tbblue_spritenav_draw_sprites_window;

int menu_debug_tsconf_tbblue_spritenav_get_total_sprites(void)
{
	int limite;

	limite=TSCONF_MAX_SPRITES; //85 sprites max

	if (MACHINE_IS_TBBLUE) limite=TBBLUE_MAX_SPRITES;

	return limite;
}

//Muestra lista de sprites
void menu_debug_tsconf_tbblue_spritenav_lista_sprites(void)
{

	char dumpmemoria[33];

	int linea_color;
	int limite;

	int linea=0;
	/*limite=TSCONF_MAX_SPRITES; //85 sprites max

	if (MACHINE_IS_TBBLUE) limite=TBBLUE_MAX_SPRITES;*/

	limite=menu_debug_tsconf_tbblue_spritenav_get_total_sprites();

	//z80_byte tbsprite_sprites[TBBLUE_MAX_SPRITES][4];
	/*
	1st: X position (bits 7-0).
2nd: Y position (0-255).
3rd: bits 7-4 is palette offset, bit 3 is X mirror, bit 2 is Y mirror, bit 1 is the rotate flag and bit 0 is X MSB.
4th: bit 7 is the visible flag, bit 6 is reserved, bits 5-0 is Name (pattern index, 0-63).
*/

	int current_sprite;


		/*for (linea_color=0;linea_color<TSCONF_SPRITENAV_SPRITES_PER_WINDOW &&
				menu_debug_tsconf_tbblue_spritenav_current_sprite+linea_color<limite;
				linea_color++) {*/
		for (linea_color=0;linea_color<limite;linea_color++) {					

			current_sprite=menu_debug_tsconf_tbblue_spritenav_current_sprite+linea_color;

			if (MACHINE_IS_TSCONF) {

			int offset=current_sprite*6;
			z80_byte sprite_r0h=tsconf_fmaps[0x200+offset+1];

			z80_byte sprite_leap=sprite_r0h&64;

			int sprite_act=sprite_r0h&32;
        	int y=tsconf_fmaps[0x200+offset]+256*(sprite_r0h&1);
	      	z80_byte ysize=8*(1+((sprite_r0h>>1)&7));
	               

        	z80_byte sprite_r1h=tsconf_fmaps[0x200+offset+3];
		    int x=tsconf_fmaps[0x200+offset+2]+256*(sprite_r1h&1);
			z80_byte xsize=8*(1+((sprite_r1h>>1)&7));

			z80_byte sprite_r2h=tsconf_fmaps[0x200+offset+5];
			z80_int tnum=(tsconf_fmaps[0x200+offset+4])+256*(sprite_r2h&15);
			    	//Tile Number for upper left corner. Bits 0-5 are X Position in Graphics Bitmap, bits 6-11 - Y Position.
			z80_int tnum_x=tnum & 63;
    		z80_int tnum_y=(tnum>>6)&63;

		    z80_byte spal=(sprite_r2h>>4)&15;

			z80_byte sprite_xf=sprite_r1h&128;
			z80_byte sprite_yf=sprite_r0h&128;

			sprintf (dumpmemoria,"%02d X: %3d Y: %3d (%2dX%2d)",current_sprite,x,y,xsize,ysize);
			//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
			zxvision_print_string_defaults(menu_debug_tsconf_tbblue_spritenav_draw_sprites_window,1,linea++,dumpmemoria);

			sprintf (dumpmemoria,"Tile:%2d,%2d %s %s %s %s P:%2d",tnum_x,tnum_y,
				(sprite_act ? "ACT" : "   "),(sprite_leap ? "LEAP": "    "),
				(sprite_xf ? "XF" : "  "),(sprite_yf ? "YF": "  "),
				spal );

			//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
			zxvision_print_string_defaults(menu_debug_tsconf_tbblue_spritenav_draw_sprites_window,1,linea++,dumpmemoria);
			}

			if (MACHINE_IS_TBBLUE) {
					//z80_byte tbsprite_sprites[TBBLUE_MAX_SPRITES][4];
	/*
	1st: X position (bits 7-0).
2nd: Y position (0-255).
3rd: bits 7-4 is palette offset, bit 3 is X mirror, bit 2 is Y mirror, bit 1 is the rotate flag and bit 0 is X MSB.
4th: bit 7 is the visible flag, bit 6 is reserved, bits 5-0 is Name (pattern index, 0-63).
*/

				z80_int x=tbsprite_sprites[current_sprite][0]; //
				z80_byte y=tbsprite_sprites[current_sprite][1];  //

				z80_byte byte_3=tbsprite_sprites[current_sprite][2];
				z80_byte paloff=byte_3 & 0xF0; //
				z80_byte mirror_x=byte_3 & 8; //
				z80_byte mirror_y=byte_3 & 4; //
				z80_byte rotate=byte_3 & 2; //
				z80_byte msb_x=byte_3 &1; //

				x +=msb_x*256;

				z80_byte byte_4=tbsprite_sprites[current_sprite][3];
				z80_byte visible=byte_4 & 128; //
				z80_byte pattern=byte_4 & 63; //

			sprintf (dumpmemoria,"%02d X: %3d Y: %3d %s %s %s",current_sprite,x,y,
					(mirror_x ? "MIRX" : "    "),(mirror_y ? "MIRY" : "    "),(rotate ? "ROT" : "   ")
			);
			//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
			zxvision_print_string_defaults(menu_debug_tsconf_tbblue_spritenav_draw_sprites_window,1,linea++,dumpmemoria);

			sprintf (dumpmemoria," Pattn: %2d Palof: %3d Vis: %s"
				,pattern,paloff, (visible ? "Yes" : "No ") );


				//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
				zxvision_print_string_defaults(menu_debug_tsconf_tbblue_spritenav_draw_sprites_window,1,linea++,dumpmemoria);

			}			

			


					
		}

	zxvision_draw_window_contents(menu_debug_tsconf_tbblue_spritenav_draw_sprites_window); 


}

void menu_debug_tsconf_tbblue_spritenav_draw_sprites(void)
{



				/*menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech


				//Mostrar lista sprites
				menu_debug_tsconf_tbblue_spritenav_lista_sprites();

				//Esto tiene que estar despues de escribir la lista de sprites, para que se refresque y se vea
				//Si estuviese antes, al mover el cursor hacia abajo dejándolo pulsado, el texto no se vería hasta que no se soltase la tecla
				normal_overlay_texto_menu();*/


				menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech
				normal_overlay_texto_menu();
				menu_debug_tsconf_tbblue_spritenav_lista_sprites();



}



void menu_debug_tsconf_tbblue_spritenav(MENU_ITEM_PARAMETERS)
{
	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();

	
	zxvision_window ventana;

	zxvision_new_window(&ventana,TSCONF_SPRITENAV_WINDOW_X,TSCONF_SPRITENAV_WINDOW_Y,TSCONF_SPRITENAV_WINDOW_ANCHO,TSCONF_SPRITENAV_WINDOW_ALTO,
							TSCONF_SPRITENAV_WINDOW_ANCHO-1,menu_debug_tsconf_tbblue_spritenav_get_total_sprites()*2,"Sprite navigator");

	zxvision_draw_window(&ventana);		

    set_menu_overlay_function(menu_debug_tsconf_tbblue_spritenav_draw_sprites);

	menu_debug_tsconf_tbblue_spritenav_draw_sprites_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui


	z80_byte tecla;

	
    do {
    	menu_speech_tecla_pulsada=0; //Que envie a speech

   		tecla=zxvision_common_getkey_refresh();
	
		zxvision_handle_cursors_pgupdn(&ventana,tecla);


	} while (tecla!=2); 

	//restauramos modo normal de texto de menu
    set_menu_overlay_function(normal_overlay_texto_menu);		

    cls_menu_overlay();

	zxvision_destroy_window(&ventana);
}








#define TSCONF_TILENAV_WINDOW_X 0
#define TSCONF_TILENAV_WINDOW_Y 0
#define TSCONF_TILENAV_WINDOW_ANCHO 32
#define TSCONF_TILENAV_WINDOW_ALTO 24
#define TSCONF_TILENAV_TILES_VERT_PER_WINDOW 64
#define TSCONF_TILENAV_TILES_HORIZ_PER_WINDOW 64



//int menu_debug_tsconf_tbblue_tilenav_current_palette=0;
//int menu_debug_tsconf_tbblue_tilenav_current_tile=0;

int menu_debug_tsconf_tbblue_tilenav_current_tilelayer=0;

z80_bit menu_debug_tsconf_tbblue_tilenav_showmap={0};

zxvision_window *menu_debug_tsconf_tbblue_tilenav_lista_tiles_window;


#define DEBUG_TSCONF_TILENAV_MAX_TILES (64*64)


char menu_debug_tsconf_tbblue_tiles_retorna_visualchar(int tnum)
{
	//Hacer un conjunto de 64 caracteres. Mismo set de caracteres que para Base64. Por que? Por que si :)
			   //0123456789012345678901234567890123456789012345678901234567890123
	char *caracter_list="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	int index=tnum % 64;
	return caracter_list[index];
}

int menu_debug_tsconf_tbblue_tilenav_total_vert(void)
{
	int limite_vertical=DEBUG_TSCONF_TILENAV_MAX_TILES;
	if (menu_debug_tsconf_tbblue_tilenav_showmap.v) limite_vertical=TSCONF_TILENAV_TILES_VERT_PER_WINDOW;	

	return limite_vertical;
}

//Muestra lista de tiles
void menu_debug_tsconf_tbblue_tilenav_lista_tiles(void)
{

	//Suficientemente grande para almacenar regla superior en modo visual
	char dumpmemoria[68]; //64 + 3 espacios izquierda + 0 final

	
	int limite;

	int linea=0;
	limite=DEBUG_TSCONF_TILENAV_MAX_TILES;

	int current_tile;

	z80_byte *puntero_tilemap;
	z80_byte *puntero_tilemap_orig;
	puntero_tilemap=tsconf_ram_mem_table[0]+tsconf_return_tilemappage();
	puntero_tilemap_orig=puntero_tilemap;

	//int limite_vertical=DEBUG_TSCONF_TILENAV_MAX_TILES;
	//if (menu_debug_tsconf_tbblue_tilenav_showmap.v) limite_vertical=TSCONF_TILENAV_TILES_VERT_PER_WINDOW*2;

	int limite_vertical=menu_debug_tsconf_tbblue_tilenav_total_vert();

	//int current_tile_x=0;

	//int linea_color=0;
	int offset_vertical=0;

	if (menu_debug_tsconf_tbblue_tilenav_showmap.v) {
				  //0123456789012345678901234567890123456789012345678901234567890123
		strcpy(dumpmemoria,"   0    5    10   15   20   25   30   35   40   45   50   55   60  ");

		//Indicar codigo 0 de final
		//dumpmemoria[current_tile_x+TSCONF_TILENAV_TILES_HORIZ_PER_WINDOW+3]=0;  //3 espacios al inicio

		//menu_escribe_linea_opcion(linea++,-1,1,&dumpmemoria[current_tile_x]); //Mostrar regla superior
		zxvision_print_string_defaults(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window,1,linea++,dumpmemoria);
	}
	else {
			//Aumentarlo en cuanto al offset que estamos (si modo lista)
	//TODO: limite final +24 de alto como mucho, inicio donde escribimos, inicio de tile

		int offset_y=menu_debug_tsconf_tbblue_tilenav_lista_tiles_window->offset_y;
		

		offset_vertical=offset_y/2;
		linea=offset_vertical*2;

		limite_vertical=offset_vertical+24; //24 a voleo

	}

		printf ("Init drawing tiles from vertical offset %d to %d\n",offset_vertical,limite_vertical);
		/*for (linea_color=0;linea_color<limite_vertical &&
				menu_debug_tsconf_tbblue_tilenav_current_tile+linea_color<limite;
				linea_color++) {*/

		for (;offset_vertical<limite_vertical;offset_vertical++) {

			int repetir_ancho=1;
			int mapa_tile_x=3;
			if (menu_debug_tsconf_tbblue_tilenav_showmap.v==0) {
				//Modo lista tiles
				current_tile=offset_vertical;
			}

			else {
				//Modo mapa tiles
				current_tile=offset_vertical*64;
				repetir_ancho=TSCONF_TILENAV_TILES_HORIZ_PER_WINDOW;

				//poner regla vertical
				int linea_tile=current_tile/64;
				if ( (linea_tile%5)==0) sprintf (dumpmemoria,"%2d ",linea_tile);
				else sprintf (dumpmemoria,"   ");
			}

			//printf ("linea: %3d current tile: %10d puntero: %10d\n",linea_color,current_tile,puntero_tilemap-tsconf_ram_mem_table[0]-tsconf_return_tilemappage()	);

			do {
				int y=current_tile/64;
				int x=current_tile%64; 

				//printf ("x: %d y: %d\n",x,y);

				int offset=256*y+x*2;

				offset+=menu_debug_tsconf_tbblue_tilenav_current_tilelayer*128;

				int tnum=puntero_tilemap[offset]+256*(puntero_tilemap[offset+1]&0xF);

				z80_byte tnum_x=tnum&63;
				z80_byte tnum_y=(tnum>>6)&63;

		    	z80_byte tpal=(puntero_tilemap[offset+1]>>4)&3;

				z80_byte tile_xf=puntero_tilemap[offset+1]&64;
				z80_byte tile_yf=puntero_tilemap[offset+1]&128;

				if (menu_debug_tsconf_tbblue_tilenav_showmap.v==0) {
					//Modo lista tiles
					sprintf (dumpmemoria,"X: %3d Y: %3d                   ",x,y);
					//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
					zxvision_print_string_defaults(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window,1,linea++,dumpmemoria);

					sprintf (dumpmemoria," Tile: %2d,%2d %s %s P:%2d",tnum_x,tnum_y,
						(tile_xf ? "XF" : "  "),(tile_yf ? "YF": "  "),
						tpal );
					//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
					zxvision_print_string_defaults(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window,1,linea++,dumpmemoria);
				}
				else {
					//Modo mapa tiles
					z80_byte caracter_final;

					if (tnum==0) {
						caracter_final=' '; 
					}
					else {
						//int caracteres_totales=50; //127-33;
						//caracter_final=33+(tnum%caracteres_totales);
						caracter_final=menu_debug_tsconf_tbblue_tiles_retorna_visualchar(tnum);
					}

					dumpmemoria[mapa_tile_x++]=caracter_final;
				}

				puntero_tilemap+=2;
				repetir_ancho--;
			} while (repetir_ancho);

			if (menu_debug_tsconf_tbblue_tilenav_showmap.v) {
				//dumpmemoria[mapa_tile_x++]=0;
				//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
				zxvision_print_string_defaults(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window,1,linea++,dumpmemoria);

				//Siguiente linea de tiles (saltar 64 posiciones, 2 layers, 2 bytes por tile)
				//puntero_tilemap_orig +=64*2*2;
				puntero_tilemap=puntero_tilemap_orig;
			}
					
		}



	//return linea;

	zxvision_draw_window_contents(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window); 
}

void menu_debug_tsconf_tbblue_tilenav_draw_tiles(void)
{
/*
				menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech


				//Mostrar lista tiles
				menu_debug_tsconf_tbblue_tilenav_lista_tiles();

				//Esto tiene que estar despues de escribir la lista de tiles, para que se refresque y se vea
				//Si estuviese antes, al mover el cursor hacia abajo dejándolo pulsado, el texto no se vería hasta que no se soltase la tecla
				normal_overlay_texto_menu();
				*/



				menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech
				normal_overlay_texto_menu();
				menu_debug_tsconf_tbblue_tilenav_lista_tiles();				

}



void menu_debug_tsconf_tbblue_tilenav_new_window(zxvision_window *ventana)
{

		char titulo[33];
		/*
				if (menu_debug_tsconf_tbblue_tilenav_showmap.v) {
			sprintf (buffer_linea,"Move: Cursors, PgUp/Dn. ~~Layer %d",menu_debug_tsconf_tbblue_tilenav_current_tilelayer);
			menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
			menu_escribe_linea_opcion(linea++,-1,1,"~~Mode: Visual");
		}

		else {
			sprintf (buffer_linea,"Move: Cursors, PgUp/Dn. ~~Layer %d",menu_debug_tsconf_tbblue_tilenav_current_tilelayer);
			menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
			menu_escribe_linea_opcion(linea++,-1,1,"~~Mode: List");
		}
		*/

		int total_height=menu_debug_tsconf_tbblue_tilenav_total_vert();
		if (menu_debug_tsconf_tbblue_tilenav_showmap.v) {
			sprintf (titulo,"Tiles M:Visual L:Lyr %d",menu_debug_tsconf_tbblue_tilenav_current_tilelayer);
			
		}

		else {
			sprintf (titulo,"Tiles M:List L:Lyr %d",menu_debug_tsconf_tbblue_tilenav_current_tilelayer);
			total_height*=2;
		}

		zxvision_new_window(ventana,TSCONF_TILENAV_WINDOW_X,TSCONF_TILENAV_WINDOW_Y,TSCONF_TILENAV_WINDOW_ANCHO,TSCONF_TILENAV_WINDOW_ALTO,
							TSCONF_TILENAV_TILES_HORIZ_PER_WINDOW+4,total_height+1,titulo);

		zxvision_draw_window(ventana);										
}

void menu_debug_tsconf_tbblue_tilenav(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();

	
		zxvision_window ventana;


		menu_debug_tsconf_tbblue_tilenav_new_window(&ventana);



        z80_byte acumulado;


        set_menu_overlay_function(menu_debug_tsconf_tbblue_tilenav_draw_tiles);


		menu_debug_tsconf_tbblue_tilenav_lista_tiles_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui


		z80_byte tecla;

	do {
    	menu_speech_tecla_pulsada=0; //Que envie a speech
			

		tecla=zxvision_common_getkey_refresh();				

        
				switch (tecla) {

					case 'l':
						zxvision_destroy_window(&ventana);	
						menu_debug_tsconf_tbblue_tilenav_current_tilelayer ^=1;
						menu_debug_tsconf_tbblue_tilenav_new_window(&ventana);
					break;

					case 'm':

						zxvision_destroy_window(&ventana);		
						menu_debug_tsconf_tbblue_tilenav_showmap.v ^=1;
						menu_debug_tsconf_tbblue_tilenav_new_window(&ventana);

						//menu_debug_tsconf_tbblue_tilenav_current_tile=0;
					break;


					default:
						zxvision_handle_cursors_pgupdn(&ventana,tecla);
					break;
				}		

		


	} while (tecla!=2); 

	//restauramos modo normal de texto de menu
    set_menu_overlay_function(normal_overlay_texto_menu);		

    cls_menu_overlay();

	zxvision_destroy_window(&ventana);		


	
    /*do {

    	menu_speech_tecla_pulsada=0; //Que envie a speech

		int linea=TSCONF_TILENAV_WINDOW_Y+TSCONF_TILENAV_TILES_VERT_PER_WINDOW*2+1;

			
		char buffer_linea[40];

		//Forzar a mostrar atajos
		z80_bit antes_menu_writing_inverse_color;
		antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;

		menu_writing_inverse_color.v=1;

		if (menu_debug_tsconf_tbblue_tilenav_showmap.v) {
			sprintf (buffer_linea,"Move: Cursors, PgUp/Dn. ~~Layer %d",menu_debug_tsconf_tbblue_tilenav_current_tilelayer);
			menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
			menu_escribe_linea_opcion(linea++,-1,1,"~~Mode: Visual");
		}

		else {
			sprintf (buffer_linea,"Move: Cursors, PgUp/Dn. ~~Layer %d",menu_debug_tsconf_tbblue_tilenav_current_tilelayer);
			menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
			menu_escribe_linea_opcion(linea++,-1,1,"~~Mode: List");
		}

		menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;


		if (menu_multitarea==0) menu_refresca_pantalla();

		menu_espera_tecla();

		tecla=menu_get_pressed_key();

		menu_espera_no_tecla_con_repeticion();

		int aux_pgdnup;
		//int limite;

				switch (tecla) {

					case 'l':
						menu_debug_tsconf_tbblue_tilenav_current_tilelayer ^=1;
					break;

					case 'm':
						menu_debug_tsconf_tbblue_tilenav_showmap.v ^=1;
						menu_debug_tsconf_tbblue_tilenav_current_tile=0;
					break;

					//Salir con ESC
					case 2:
						salir=1;
					break;
				}


        } while (salir==0);

		//restauramos modo normal de texto de menu
        set_menu_overlay_function(normal_overlay_texto_menu);


	cls_menu_overlay();
	//menu_escribe_linea_startx=1;
	*/

}

