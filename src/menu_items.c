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
int audio_new_waveform_opcion_seleccionada=0;
int debug_new_visualmem_opcion_seleccionada=0;
int audio_new_ayplayer_opcion_seleccionada=0;
int osd_adventure_keyboard_opcion_seleccionada=0;

//Fin opciones seleccionadas para cada menu


//Ultima direccion pokeada
int last_debug_poke_dir=16384; 

//aofile. aofilename apuntara aqui
char aofilename_file[PATH_MAX];

char last_ay_file[PATH_MAX]="";


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



#ifdef EMULATE_CPU_STATS
void menu_debug_cpu_resumen_stats(MENU_ITEM_PARAMETERS)
{

        char textostats[32];

	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

		zxvision_window ventana;

	zxvision_new_window(&ventana,0,1,32,18,
							31,16,"CPU Compact Statistics");
	zxvision_draw_window(&ventana);
		

        //z80_byte acumulado;

        char dumpassembler[32];

        //Empezar con espacio
        dumpassembler[0]=' ';

				int valor_contador_segundo_anterior;

				valor_contador_segundo_anterior=contador_segundo;

		z80_byte tecla;

        do {

			//esto hara ejecutar esto 2 veces por segundo
			if ( ((contador_segundo%500) == 0 && valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {
											valor_contador_segundo_anterior=contador_segundo;
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


                }

				//Nota: No usamos zxvision_common_getkey_refresh porque necesitamos que el bucle se ejecute continuamente para poder 
				//refrescar contenido de ventana, dado que aqui no llamamos a menu_espera_tecla
				//(a no ser que este multitarea off)
				tecla=zxvision_common_getkey_refresh_noesperatecla();
               

				zxvision_handle_cursors_pgupdn(&ventana,tecla);


		} while (tecla!=2);

        cls_menu_overlay();

		zxvision_destroy_window(&ventana);

}

#endif

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
		zxvision_generic_message_tooltip("Pruebas", 0, 0, 0, NULL, 0, "Hola que tal como estas esto es una prueba de escribir texto. "
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

	//menu_item *array_menu_audio_new_waveform;
      //  menu_item item_seleccionado;
        //int retorno_menu;
        


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
/*	int i;

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

    menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();


	zxvision_window ventana;

	zxvision_new_window(&ventana,0,7,32,9,
							31,7,"Core Statistics");

	zxvision_draw_window(&ventana);


        char texto_buffer[33];


        //Empezar con espacio
    texto_buffer[0]=' ';

        int valor_contador_segundo_anterior;

        valor_contador_segundo_anterior=contador_segundo;


		z80_byte tecla;

        do {


                //esto hara ejecutar esto 2 veces por segundo
                if ( ((contador_segundo%500) == 0 && valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {
                     valor_contador_segundo_anterior=contador_segundo;
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


                }

				//Nota: No usamos zxvision_common_getkey_refresh porque necesitamos que el bucle se ejecute continuamente para poder 
				//refrescar contenido de ventana, dado que aqui no llamamos a menu_espera_tecla
				//(a no ser que este multitarea off)
				tecla=zxvision_common_getkey_refresh_noesperatecla();				


				zxvision_handle_cursors_pgupdn(&ventana,tecla);


		} while (tecla!=2);

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


				}


        }


	zxvision_draw_window_contents(menu_ay_registers_overlay_window); 


}



void menu_ay_registers(MENU_ITEM_PARAMETERS)
{
    menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();

		if (!menu_multitarea) {
			menu_warn_message("This menu item needs multitask enabled");
			return;
		}

		int total_chips=ay_retorna_numero_chips();
		if (total_chips>3) total_chips=3;

		int yventana;
		int alto_ventana;

        if (total_chips==1) {
			yventana=5;
			alto_ventana=14;
		}
		else {
			yventana=0;
			alto_ventana=24;
		}

		zxvision_window ventana;

		zxvision_new_window(&ventana,1,yventana,30,alto_ventana,
							30-1,alto_ventana-2,"AY Registers");

		zxvision_draw_window(&ventana);		




        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de onda + texto
        set_menu_overlay_function(menu_ay_registers_overlay);

		menu_ay_registers_overlay_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui

				int valor_contador_segundo_anterior;

				valor_contador_segundo_anterior=contador_segundo;

	z80_byte tecla;

	do {
		tecla=zxvision_common_getkey_refresh();		
		zxvision_handle_cursors_pgupdn(&ventana,tecla);
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

	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();

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




	char texto_buffer[64];

	char texto_buffer2[64];

	//Empezar con espacio
    texto_buffer[0]=' ';

	int valor_contador_segundo_anterior;

	valor_contador_segundo_anterior=contador_segundo;

	z80_byte tecla;


    	do {


        	//esto hara ejecutar esto 2 veces por segundo
			if ( ((contador_segundo%500) == 0 && valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {
											valor_contador_segundo_anterior=contador_segundo;
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


                }

				//Nota: No usamos zxvision_common_getkey_refresh porque necesitamos que el bucle se ejecute continuamente para poder 
				//refrescar contenido de ventana, dado que aqui no llamamos a menu_espera_tecla
				//(a no ser que este multitarea off)
				tecla=zxvision_common_getkey_refresh_noesperatecla();


				zxvision_handle_cursors_pgupdn(&ventana,tecla);


		} while (tecla!=2);

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

		//Si no esta multitarea, hacer un refresco inicial para que aparezca el contenido de la ventana sin tener que pulsar una tecla
		//dado que luego funciona como overlay, el overlay se aplica despues de hacer el render
		//esto solo es necesario para ventanas que usan overlay
	    if (!menu_multitarea) {
			printf ("refresca pantalla inicial\n");
			menu_refresca_pantalla();
		}			

	
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

		int offset_y=menu_debug_tsconf_tbblue_tilenav_lista_tiles_window->offset_y;
		

		offset_vertical=offset_y/2;
		linea=offset_vertical*2;

		limite_vertical=offset_vertical+((24-2)/2)+1; //El maximo que cabe en pantalla, +1 para cuando se baja 1 posicion con cursor

	}

		//printf ("Init drawing tiles from vertical offset %d to %d. line print starts at %d\n",offset_vertical,limite_vertical,linea);
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
				

				int offset=(256*y)+(x*2);

				offset+=menu_debug_tsconf_tbblue_tilenav_current_tilelayer*128;

				int tnum=puntero_tilemap[offset]+256*(puntero_tilemap[offset+1]&0xF);

				//printf ("Current tile: %d  x: %d y: %d  tnum: %d\n",current_tile,x,y,tnum);

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

				//puntero_tilemap+=2;
				current_tile++;

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
		

		int total_height=menu_debug_tsconf_tbblue_tilenav_total_vert();
		int total_width=31;
		if (menu_debug_tsconf_tbblue_tilenav_showmap.v) {
			sprintf (titulo,"Tiles M:Visual L:Lyr %d",menu_debug_tsconf_tbblue_tilenav_current_tilelayer);
			total_width=TSCONF_TILENAV_TILES_HORIZ_PER_WINDOW+4;
			total_height++; //uno mas pues hay la primera linea con la regla de columnas
		}

		else {
			sprintf (titulo,"Tiles M:List L:Lyr %d",menu_debug_tsconf_tbblue_tilenav_current_tilelayer);
			total_height*=2;
		}

		zxvision_new_window(ventana,TSCONF_TILENAV_WINDOW_X,TSCONF_TILENAV_WINDOW_Y,TSCONF_TILENAV_WINDOW_ANCHO,TSCONF_TILENAV_WINDOW_ALTO,
							total_width,total_height,titulo);

		zxvision_draw_window(ventana);										
}

void menu_debug_tsconf_tbblue_tilenav(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();

	
		zxvision_window ventana;


		menu_debug_tsconf_tbblue_tilenav_new_window(&ventana);



        set_menu_overlay_function(menu_debug_tsconf_tbblue_tilenav_draw_tiles);


		menu_debug_tsconf_tbblue_tilenav_lista_tiles_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui


		z80_byte tecla;

		//Si no esta multitarea, hacer un refresco inicial para que aparezca el contenido de la ventana sin tener que pulsar una tecla
		//dado que luego funciona como overlay, el overlay se aplica despues de hacer el render
		//esto solo es necesario para ventanas que usan overlay
	    if (!menu_multitarea) {
			printf ("refresca pantalla inicial\n");
			menu_refresca_pantalla();
		}				


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


	
    

}




#define SOUND_WAVE_X 1
#define SOUND_WAVE_Y 3
#define SOUND_WAVE_ANCHO 30
#define SOUND_WAVE_ALTO 15

int menu_sound_wave_llena=1;
int menu_audio_draw_sound_wave_ycentro;

char menu_audio_draw_sound_wave_valor_medio,menu_audio_draw_sound_wave_valor_max,menu_audio_draw_sound_wave_valor_min;
int menu_audio_draw_sound_wave_frecuencia_aproximada;

int menu_audio_draw_sound_wave_volumen=0;
int menu_audio_draw_sound_wave_volumen_escalado=0;


//Usado dentro del overlay de waveform, para mostrar dos veces por segundo el texto que average, etc
int menu_waveform_valor_contador_segundo_anterior;

int menu_waveform_previous_volume=0;



zxvision_window *menu_audio_draw_sound_wave_window;

void menu_audio_draw_sound_wave(void)
{

	normal_overlay_texto_menu();

	//workaround_pentagon_clear_putpixel_cache();

				char buffer_texto_medio[40]; //32+3+margen de posible color rojo del maximo

	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

	//esto hara ejecutar esto 2 veces por segundo
	if ( ((contador_segundo%500) == 0 && menu_waveform_valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {

		menu_waveform_valor_contador_segundo_anterior=contador_segundo;
		//printf ("Refrescando. contador_segundo=%d\n",contador_segundo);

		//menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

			//Average, min, max    

			sprintf (buffer_texto_medio,"Av.: %d Min: %d Max: %d",
			menu_audio_draw_sound_wave_valor_medio,menu_audio_draw_sound_wave_valor_min,menu_audio_draw_sound_wave_valor_max);


			//menu_escribe_linea_opcion(1,-1,1,buffer_texto_medio);
			zxvision_print_string_defaults_fillspc(menu_audio_draw_sound_wave_window,1,1,buffer_texto_medio);


	

			//Hacer decaer el volumen
			//if (menu_waveform_previous_volume>menu_audio_draw_sound_wave_volumen_escalado) menu_waveform_previous_volume--;
			menu_waveform_previous_volume=menu_decae_dec_valor_volumen(menu_waveform_previous_volume,menu_audio_draw_sound_wave_volumen_escalado);


			//Frecuency
			sprintf (buffer_texto_medio,"Average freq: %d Hz (%s)",
				menu_audio_draw_sound_wave_frecuencia_aproximada,get_note_name(menu_audio_draw_sound_wave_frecuencia_aproximada));
			//menu_escribe_linea_opcion(3,-1,1,buffer_texto_medio);
			zxvision_print_string_defaults_fillspc(menu_audio_draw_sound_wave_window,1,3,buffer_texto_medio);

			//printf ("menu_speech_tecla_pulsada: %d\n",menu_speech_tecla_pulsada);
	}







	int ancho=(SOUND_WAVE_ANCHO-2);
	int alto=(SOUND_WAVE_ALTO-4);
	//int xorigen=(SOUND_WAVE_X+1);
	//int yorigen=(SOUND_WAVE_Y+4);

	int xorigen=1;
	int yorigen=4;


	if (si_complete_video_driver() ) {
        	ancho *=menu_char_width;
	        alto *=8;
        	xorigen *=menu_char_width;
	        yorigen *=8;
	}


	//int ycentro=yorigen+alto/2;
	menu_audio_draw_sound_wave_ycentro=yorigen+alto/2;

	int x,y,lasty;


	//Para drivers de texto, borrar zona

	if (!si_complete_video_driver() ) {
	        for (x=xorigen;x<xorigen+ancho;x++) {
        	        for (y=yorigen;y<yorigen+alto;y++) {
				//putchar_menu_overlay(x,y,' ',0,7);
				//putchar_menu_overlay(x,y,' ',ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL);
				//putchar_menu_overlay(x,y,' ',ESTILO_GUI_COLOR_WAVEFORM,ESTILO_GUI_PAPEL_NORMAL);

				zxvision_print_char_simple(menu_audio_draw_sound_wave_window,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,' ');
	                }
        	}
	}


	

	audiobuffer_stats audiostats;
	audio_get_audiobuffer_stats(&audiostats);


	menu_audio_draw_sound_wave_valor_max=audiostats.maximo;
	menu_audio_draw_sound_wave_valor_min=audiostats.minimo;
	menu_audio_draw_sound_wave_frecuencia_aproximada=audiostats.frecuencia;
	menu_audio_draw_sound_wave_volumen=audiostats.volumen;
	menu_audio_draw_sound_wave_volumen_escalado=audiostats.volumen_escalado;

	int audiomedio=audiostats.medio;
	menu_audio_draw_sound_wave_valor_medio=audiomedio;
        audiomedio=audiomedio*alto/256;

        //Lo situamos en el centro. Negativo hacia abajo (Y positiva)
        audiomedio=menu_audio_draw_sound_wave_ycentro-audiomedio;

	int puntero_audio=0;
	char valor_audio;
	for (x=xorigen;x<xorigen+ancho;x++) {

		//Obtenemos valor medio de audio
		int valor_medio=0;

		//Calcular cuantos valores representa un pixel, teniendo en cuenta maximo buffer
		const int max_valores=AUDIO_BUFFER_SIZE/ancho;

		int valores=max_valores;
		for (;valores>0;valores--,puntero_audio++) {
			if (puntero_audio>=AUDIO_BUFFER_SIZE) {
				//por si el calculo no es correcto, salir.
				//esto no deberia suceder ya que el calculo de max_valores se hace en base al maximo
				cpu_panic("menu_audio_draw_sound_wave: pointer beyond AUDIO_BUFFER_SIZE");
			}

			//stereo 
			//if (audio_driver_accepts_stereo.v) {
				int suma_canales=audio_buffer[puntero_audio*2]+audio_buffer[(puntero_audio*2)+1];
				suma_canales /=2;
				valor_medio=valor_medio+suma_canales;
			//}

			//else valor_medio=valor_medio+audio_buffer[puntero_audio];


		}

		valor_medio=valor_medio/max_valores;

		valor_audio=valor_medio;

		//Lo escalamos a maximo alto

		y=valor_audio;
		y=valor_audio*alto/256;

		//Lo situamos en el centro. Negativo hacia abajo (Y positiva)
		y=menu_audio_draw_sound_wave_ycentro-y;


		//unimos valor anterior con actual con una linea vertical
		if (x!=xorigen) {
			if (si_complete_video_driver() ) {

				//Onda no llena
				if (!menu_sound_wave_llena) menu_linea_zxvision(menu_audio_draw_sound_wave_window,x,lasty,y,ESTILO_GUI_COLOR_WAVEFORM);

        			//dibujar la onda "llena", es decir, siempre contar desde centro
			        //el centro de la y de la onda es variable... se saca valor medio de todos los valores mostrados en pantalla

				//Onda llena
				else menu_linea_zxvision(menu_audio_draw_sound_wave_window,x,audiomedio,y,ESTILO_GUI_COLOR_WAVEFORM);



			}
		}

		lasty=y;

		//dibujamos valor actual
		if (si_complete_video_driver() ) {
			//menu_scr_putpixel(x,y,ESTILO_GUI_COLOR_WAVEFORM);
			zxvision_putpixel(menu_audio_draw_sound_wave_window,x,y,ESTILO_GUI_COLOR_WAVEFORM);
		}

		else {
			//putchar_menu_overlay(SOUND_WAVE_X+x,SOUND_WAVE_Y+y,'#',ESTILO_GUI_COLOR_WAVEFORM,ESTILO_GUI_PAPEL_NORMAL);
			zxvision_print_char_simple(menu_audio_draw_sound_wave_window,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,'#');
		}


	}

	//printf ("%d ",puntero_audio);


	//Volume. Mostrarlo siempre, no solo dos veces por segundo, para que se actualice mas frecuentemente
	//if (menu_waveform_previous_volume<menu_audio_draw_sound_wave_volumen_escalado) menu_waveform_previous_volume=menu_audio_draw_sound_wave_volumen_escalado;
	menu_waveform_previous_volume=menu_decae_ajusta_valor_volumen(menu_waveform_previous_volume,menu_audio_draw_sound_wave_volumen_escalado);

	char texto_volumen[32]; 
    menu_string_volumen(texto_volumen,menu_audio_draw_sound_wave_volumen_escalado,menu_waveform_previous_volume);
                                                                //"Volume C: %s"

	sprintf (buffer_texto_medio,"Volume: %3d %s",menu_audio_draw_sound_wave_volumen,texto_volumen);
	//menu_escribe_linea_opcion(2,-1,1,buffer_texto_medio);
	zxvision_print_string_defaults_fillspc(menu_audio_draw_sound_wave_window,1,2,buffer_texto_medio);


	zxvision_draw_window_contents(menu_audio_draw_sound_wave_window); 

}




void menu_audio_new_waveform_shape(MENU_ITEM_PARAMETERS)
{
	menu_sound_wave_llena ^=1;
}



void menu_audio_new_waveform(MENU_ITEM_PARAMETERS)
{

 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	zxvision_new_window(&ventana,SOUND_WAVE_X,SOUND_WAVE_Y-2,SOUND_WAVE_ANCHO,SOUND_WAVE_ALTO+4,
							SOUND_WAVE_ANCHO-1,SOUND_WAVE_ALTO+4-2,"Waveform");
	zxvision_draw_window(&ventana);		

    
    //Cambiamos funcion overlay de texto de menu
    //Se establece a la de funcion de audio waveform
	set_menu_overlay_function(menu_audio_draw_sound_wave);

	menu_audio_draw_sound_wave_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui

	menu_item *array_menu_audio_new_waveform;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


		menu_add_item_menu_inicial_format(&array_menu_audio_new_waveform,MENU_OPCION_NORMAL,menu_audio_new_waveform_shape,NULL,"Change wave ~~Shape");
        menu_add_item_menu_shortcut(array_menu_audio_new_waveform,'s');

        //Evito tooltips en los menus tabulados que tienen overlay porque al salir el tooltip detiene el overlay
        //menu_add_item_menu_tooltip(array_menu_audio_new_waveform,"Change wave Shape");
        menu_add_item_menu_ayuda(array_menu_audio_new_waveform,"Change wave Shape: simple line or vertical fill");
						
		menu_add_item_menu_tabulado(array_menu_audio_new_waveform,1,0);


		//Nombre de ventana solo aparece en el caso de stdout
    	retorno_menu=menu_dibuja_menu(&audio_new_waveform_opcion_seleccionada,&item_seleccionado,array_menu_audio_new_waveform,"Waveform" );


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


	//restauramos modo normal de texto de menu
    set_menu_overlay_function(normal_overlay_texto_menu);


    cls_menu_overlay();

	//En caso de menus tabulados, es responsabilidad de este de liberar ventana
	zxvision_destroy_window(&ventana);

}


zxvision_window *menu_debug_draw_visualmem_window;



#ifdef EMULATE_VISUALMEM

#define VISUALMEM_MAX_ALTO 24
#define VISUALMEM_MAX_ANCHO 32

//int visualmem_ancho_variable=VISUALMEM_MAX_ANCHO-2;
//int visualmem_alto_variable=VISUALMEM_MAX_ALTO-2;

#define visualmem_ancho_variable (menu_debug_draw_visualmem_window->visible_width-1)
#define visualmem_alto_variable (menu_debug_draw_visualmem_window->visible_height-1)

#define VISUALMEM_MIN_X 0
#define VISUALMEM_MIN_Y 0

#define VISUALMEM_DEFAULT_X (VISUALMEM_MIN_X+1)
int visualmem_x_variable=VISUALMEM_DEFAULT_X;

#define VISUALMEM_DEFAULT_Y (VISUALMEM_MIN_Y+1)
int visualmem_y_variable=VISUALMEM_DEFAULT_Y;

#define VISUALMEM_ANCHO (visualmem_ancho_variable)
#define VISUALMEM_ALTO (visualmem_alto_variable)

#define VISUALMEM_DEFAULT_WINDOW_ANCHO (VISUALMEM_MAX_ANCHO-2)
#define VISUALMEM_DEFAULT_WINDOW_ALTO (VISUALMEM_MAX_ALTO-2)

//0=vemos visualmem write
//1=vemos visualmem read
//2=vemos visualmem opcode
int menu_visualmem_donde=0;


int visualmem_bright_multiplier=10;


void menu_debug_draw_visualmem(void)
{

        normal_overlay_texto_menu();

		//workaround_pentagon_clear_putpixel_cache();


        int ancho=(VISUALMEM_ANCHO-2);
        int alto=(VISUALMEM_ALTO-4);

		if (ancho<1 || alto<1) return;

        //int xorigen=(visualmem_x_variable+1);
        //int yorigen=(visualmem_y_variable+5);
        int xorigen=1;
        int yorigen=3;


        if (si_complete_video_driver() ) {
                ancho *=menu_char_width;
                alto *=8;
                xorigen *=menu_char_width;
                yorigen *=8;
        }


	int tamanyo_total=ancho*alto;

        int x,y;


        int inicio_puntero_membuffer,final_puntero_membuffer;

	//Por defecto
	inicio_puntero_membuffer=16384;
	final_puntero_membuffer=65536;

	//printf ("ancho: %d alto: %d\n",ancho,alto);

	//En spectrum 16kb
	if (MACHINE_IS_SPECTRUM_16) {
		//printf ("spec 16kb\n");
		final_puntero_membuffer=32768;
	}

	if (MACHINE_IS_Z88) {
		        inicio_puntero_membuffer=0;
	}

	//En Inves, mostramos modificaciones a la RAM baja
	if (MACHINE_IS_INVES) {
                        inicio_puntero_membuffer=0;
        }



	//En zx80, zx81 mostrar desde 8192 por si tenemos expansion packs
	if (MACHINE_IS_ZX8081) {
		//por defecto
		//printf ("ramtop_zx8081: %d\n",ramtop_zx8081);
		final_puntero_membuffer=ramtop_zx8081+1;

		if (ram_in_8192.v) {
			//printf ("memoria en 8192\n");
			inicio_puntero_membuffer=8192;
		}

        	if (ram_in_32768.v) {
			//printf ("memoria en 32768\n");
			final_puntero_membuffer=49152;
		}

	        if (ram_in_49152.v) {
			//printf ("memoria en 49152\n");
			final_puntero_membuffer=65536;
		}

	}

        //En Jupiter Ace, desde 8192
        if (MACHINE_IS_ACE) {
                //por defecto
                final_puntero_membuffer=ramtop_ace+1;
                inicio_puntero_membuffer=8192;

        }


	//En CPC, desde 0
	if (MACHINE_IS_CPC) {
		inicio_puntero_membuffer=0;
	}

	if (MACHINE_IS_SAM) {
                inicio_puntero_membuffer=0;
        }



	//En modos de RAM en ROM de +2a en puntero apuntara a direccion 0
	if (MACHINE_IS_SPECTRUM_P2A_P3) {
		if ( (puerto_32765 & 32) == 0 ) {
			//paginacion habilitada

			if ( (puerto_8189 & 1) ) {
				//paginacion de ram en rom
				//printf ("paginacion de ram en rom\n");
				inicio_puntero_membuffer=0;
			}
		}
	}

	if (MACHINE_IS_QL) {
		//inicio_puntero_membuffer=0x18000;
		//la ram propiamente empieza en 20000H
		inicio_puntero_membuffer=0x20000;
		final_puntero_membuffer=QL_MEM_LIMIT+1;
	}

	//Si es de opcode o read, puede ser desde cualquier sitio desde la rom
	if (menu_visualmem_donde>0) {
		inicio_puntero_membuffer=0;
	}

	//Valores entre 0 y 255: numero de veces byte modificado
	//Valor 65535 especial
        //int si_modificado;


             //Calcular cuantos bytes modificados representa un pixel, teniendo en cuenta maximo buffer
	int max_valores=(final_puntero_membuffer-inicio_puntero_membuffer)/tamanyo_total;

	//printf ("max_valores: %d\n",max_valores);
	//le damos uno mas para poder llenar la ventana
	//printf ("inicio: %06XH final: %06XH\n",inicio_puntero_membuffer,final_puntero_membuffer);
	max_valores++;

	for (y=yorigen;y<yorigen+alto;y++) {
        for (x=xorigen;x<xorigen+ancho;x++) {

                //Obtenemos conjunto de bytes modificados

                int valores=max_valores;

		int acumulado=0;
		//si_modificado=0;
                for (;valores>0;valores--,inicio_puntero_membuffer++) {
                        if (inicio_puntero_membuffer>=final_puntero_membuffer) {
				//printf ("llegado a final con x: %d y: %d ",x,y);
				//Fuera de memoria direccionable. Zona gris. Decrementamos valor
				//Como se lee a trozos de "max_valores" tamanyo, cuando este trozo empieza ya fuera de memoria
				//acumulado acabara siendo <0 y saldra gris. Si es a medias, si acaba restando mucho, saldra gris tambien
				//(eso solo pasara en el ultimo pixel de la zona direccionable)
				acumulado--;
                        }
			else {
				//Es en memoria direccionable. Sumar valor de visualmem y luego haremos valor medio
				if (menu_visualmem_donde==0) {
					acumulado +=visualmem_buffer[inicio_puntero_membuffer];
					clear_visualmembuffer(inicio_puntero_membuffer);
				}

				else if (menu_visualmem_donde==1) {
					acumulado +=visualmem_read_buffer[inicio_puntero_membuffer];
					clear_visualmemreadbuffer(inicio_puntero_membuffer);
				}

				else {
					acumulado +=visualmem_opcode_buffer[inicio_puntero_membuffer];
					clear_visualmemopcodebuffer(inicio_puntero_membuffer);
				}


			}
                }
		//if (acumulado>0) printf ("final pixel %d %d (divisor: %d)\n",inicio_puntero_membuffer,acumulado,max_valores);

                //dibujamos valor medio
                if (acumulado>0) {

			if (si_complete_video_driver() ) {

				//Sacar valor medio
				int color_final=acumulado/max_valores;

				//printf ("color final: %d\n",color_final);

				//Aumentar el brillo del color
				color_final=color_final*visualmem_bright_multiplier;
				if (color_final>255) color_final=255;



				//menu_scr_putpixel(x,y,ESTILO_GUI_TINTA_NORMAL);
				//menu_scr_putpixel(x,y,HEATMAP_INDEX_FIRST_COLOR+color_final);
				zxvision_putpixel(menu_debug_draw_visualmem_window,x,y,HEATMAP_INDEX_FIRST_COLOR+color_final);
			}

			else {
				//putchar_menu_overlay(x,y,'#',ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL);
				zxvision_print_char_simple(menu_debug_draw_visualmem_window,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,'#');
			}
		}

		//color ficticio para indicar fuera de memoria y por tanto final de ventana... para saber donde acaba
		else if (acumulado<0) {
			if (si_complete_video_driver() ) {
				//menu_scr_putpixel(x,y,ESTILO_GUI_COLOR_UNUSED_VISUALMEM);
				zxvision_putpixel(menu_debug_draw_visualmem_window,x,y,ESTILO_GUI_COLOR_UNUSED_VISUALMEM);
			}
			else {
				//putchar_menu_overlay(x,y,'-',ESTILO_GUI_COLOR_UNUSED_VISUALMEM,ESTILO_GUI_PAPEL_NORMAL);
				zxvision_print_char_simple(menu_debug_draw_visualmem_window,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,'-');
            }

		}

		//Valor 0
		else {
			if (si_complete_video_driver() ) {
				//menu_scr_putpixel(x,y,ESTILO_GUI_PAPEL_NORMAL);
				zxvision_putpixel(menu_debug_draw_visualmem_window,x,y,ESTILO_GUI_PAPEL_NORMAL);
			}
			else {
				//putchar_menu_overlay(x,y,' ',ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL);
				zxvision_print_char_simple(menu_debug_draw_visualmem_window,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,' ');
			}
		}

        }
	}

	zxvision_draw_window_contents(menu_debug_draw_visualmem_window); 

}









void menu_debug_new_visualmem_looking(MENU_ITEM_PARAMETERS)
{
	menu_visualmem_donde++;
	if (menu_visualmem_donde==3) menu_visualmem_donde=0;
}


void menu_debug_new_visualmem_bright(MENU_ITEM_PARAMETERS)
{
                        if (visualmem_bright_multiplier>=200) visualmem_bright_multiplier=1;
                        else if (visualmem_bright_multiplier==1) visualmem_bright_multiplier=10;
                        else visualmem_bright_multiplier +=10;
}

/*
void menu_debug_new_visualmem_key_o(MENU_ITEM_PARAMETERS)
{
    /if (visualmem_ancho_variable>23) {
		visualmem_ancho_variable--;

		if (visualmem_ancho_variable<VISUALMEM_MAX_ANCHO-1) visualmem_x_variable=VISUALMEM_DEFAULT_X;
	}
}


void menu_debug_new_visualmem_key_p(MENU_ITEM_PARAMETERS)
{
    if (visualmem_ancho_variable<VISUALMEM_MAX_ANCHO) {
		visualmem_ancho_variable++;

		//Mover a la izquierda si maximo
		if (visualmem_ancho_variable>=VISUALMEM_MAX_ANCHO-1) visualmem_x_variable=VISUALMEM_MIN_X;
	}
}

void menu_debug_new_visualmem_key_q(MENU_ITEM_PARAMETERS)
{
    if (visualmem_alto_variable>7) {
		visualmem_alto_variable--;

		if (visualmem_alto_variable<VISUALMEM_MAX_ALTO-1) visualmem_y_variable=VISUALMEM_DEFAULT_Y;
	}
}

void menu_debug_new_visualmem_key_a(MENU_ITEM_PARAMETERS)
{
    if (visualmem_alto_variable<VISUALMEM_MAX_ALTO) {
		visualmem_alto_variable++;

		//Mover a la arriba si maximo
		if (visualmem_alto_variable>=VISUALMEM_MAX_ALTO-1) visualmem_y_variable=VISUALMEM_MIN_Y;		
	}
}
*/
void menu_debug_new_visualmem(MENU_ITEM_PARAMETERS)
{


 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	zxvision_new_window(&ventana,visualmem_x_variable,visualmem_y_variable,VISUALMEM_DEFAULT_WINDOW_ANCHO,VISUALMEM_DEFAULT_WINDOW_ALTO,
							VISUALMEM_DEFAULT_WINDOW_ANCHO-1,VISUALMEM_DEFAULT_WINDOW_ALTO-2,"Visual memory");
	zxvision_draw_window(&ventana);				


        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de visualmem + texto
        set_menu_overlay_function(menu_debug_draw_visualmem);


	menu_debug_draw_visualmem_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui		


	menu_dibuja_menu_permite_repeticiones_hotk=1;


	menu_item *array_menu_debug_new_visualmem;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


	


	//Forzar a mostrar atajos
	z80_bit antes_menu_writing_inverse_color;
	antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
	menu_writing_inverse_color.v=1;


	char texto_linea[33];
	//sprintf (texto_linea,"Size: ~~O~~P~~Q~~A ~~Bright: %d",visualmem_bright_multiplier);
	sprintf (texto_linea,"~~Bright: %d",visualmem_bright_multiplier);
	//menu_escribe_linea_opcion(0,-1,1,texto_linea);
	zxvision_print_string_defaults_fillspc(&ventana,1,0,texto_linea);



	if (menu_visualmem_donde == 0) sprintf (texto_linea,"~~Looking: Written Mem");
	else if (menu_visualmem_donde == 1) sprintf (texto_linea,"~~Looking: Read Mem");
	else sprintf (texto_linea,"~~Looking: Opcode");


	//sprintf (texto_linea,"~~Looking: %s",(menu_visualmem_donde == 0 ? "Written Mem" : "Opcode") );
	//menu_escribe_linea_opcion(1,-1,1,texto_linea);
	zxvision_print_string_defaults_fillspc(&ventana,1,1,texto_linea);


	//Restaurar comportamiento atajos
	menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;



//        char texto_linea[33];
//        sprintf (texto_linea,"Size: ~~O~~P~~Q~~A ~~Bright: %d",visualmem_bright_multiplier);
//        menu_escribe_linea_opcion(VISUALMEM_Y,-1,1,texto_linea);

/*
                        menu_add_item_menu_inicial_format(&array_menu_debug_new_visualmem,MENU_OPCION_NORMAL,menu_debug_new_visualmem_key_o,NULL,"~~O");
                        menu_add_item_menu_shortcut(array_menu_debug_new_visualmem,'o');
                        menu_add_item_menu_tooltip(array_menu_debug_new_visualmem,"Decrease window width");
                        menu_add_item_menu_ayuda(array_menu_debug_new_visualmem,"Decrease window width");
						//0123456789
						// Size: OPQA
						// Size: OPQA Bright: %d
						// Looking
			menu_add_item_menu_tabulado(array_menu_debug_new_visualmem,7,0);

                        menu_add_item_menu_format(array_menu_debug_new_visualmem,MENU_OPCION_NORMAL,menu_debug_new_visualmem_key_p,NULL,"~~P");
                        menu_add_item_menu_shortcut(array_menu_debug_new_visualmem,'p');
                        //Evito tooltips en los menus tabulados que tienen overlay porque al salir el tooltip detiene el overlay
                        //menu_add_item_menu_tooltip(array_menu_debug_new_visualmem,"Increase window width");
                        menu_add_item_menu_ayuda(array_menu_debug_new_visualmem,"Increase window width");
			menu_add_item_menu_tabulado(array_menu_debug_new_visualmem,8,0);

                        menu_add_item_menu_format(array_menu_debug_new_visualmem,MENU_OPCION_NORMAL,menu_debug_new_visualmem_key_q,NULL,"~~Q");
                        menu_add_item_menu_shortcut(array_menu_debug_new_visualmem,'q');
                        //menu_add_item_menu_tooltip(array_menu_debug_new_visualmem,"Decrease window height");
                        menu_add_item_menu_ayuda(array_menu_debug_new_visualmem,"Decrease window height");
			menu_add_item_menu_tabulado(array_menu_debug_new_visualmem,9,0);

                        menu_add_item_menu_format(array_menu_debug_new_visualmem,MENU_OPCION_NORMAL,menu_debug_new_visualmem_key_a,NULL,"~~A");
                        menu_add_item_menu_shortcut(array_menu_debug_new_visualmem,'a');
                        //menu_add_item_menu_tooltip(array_menu_debug_new_visualmem,"Increase window height");
                        menu_add_item_menu_ayuda(array_menu_debug_new_visualmem,"Increase window height");
			menu_add_item_menu_tabulado(array_menu_debug_new_visualmem,10,0);
*/

						menu_add_item_menu_inicial_format(&array_menu_debug_new_visualmem,MENU_OPCION_NORMAL,menu_debug_new_visualmem_bright,NULL,"~~Bright: %d",visualmem_bright_multiplier);
                        //menu_add_item_menu_format(array_menu_debug_new_visualmem,MENU_OPCION_NORMAL,menu_debug_new_visualmem_bright,NULL,"~~Bright: %d",visualmem_bright_multiplier);
                        menu_add_item_menu_shortcut(array_menu_debug_new_visualmem,'b');
                        //menu_add_item_menu_tooltip(array_menu_debug_new_visualmem,"Change bright value");
                        menu_add_item_menu_ayuda(array_menu_debug_new_visualmem,"Change bright value");
			menu_add_item_menu_tabulado(array_menu_debug_new_visualmem,1,0);


			char texto_looking[32];
	        	if (menu_visualmem_donde == 0) sprintf (texto_looking,"Written Mem");
        		else if (menu_visualmem_donde == 1) sprintf (texto_looking,"Read Mem");
		        else sprintf (texto_looking,"Opcode");

                        menu_add_item_menu_format(array_menu_debug_new_visualmem,MENU_OPCION_NORMAL,menu_debug_new_visualmem_looking,NULL,"~~Looking: %s",texto_looking);
                        menu_add_item_menu_shortcut(array_menu_debug_new_visualmem,'l');
                        //menu_add_item_menu_tooltip(array_menu_debug_new_visualmem,"Which visualmem to look at");
                        menu_add_item_menu_ayuda(array_menu_debug_new_visualmem,"Which visualmem to look at");
                        menu_add_item_menu_tabulado(array_menu_debug_new_visualmem,1,1);



		//Nombre de ventana solo aparece en el caso de stdout
                retorno_menu=menu_dibuja_menu(&debug_new_visualmem_opcion_seleccionada,&item_seleccionado,array_menu_debug_new_visualmem,"Visual memory" );


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



	menu_dibuja_menu_permite_repeticiones_hotk=0;



       //restauramos modo normal de texto de menu
       set_menu_overlay_function(normal_overlay_texto_menu);


    cls_menu_overlay();
	//En caso de menus tabulados, es responsabilidad de este de liberar ventana
	zxvision_destroy_window(&ventana);		


}





#endif




void menu_ay_player_load(MENU_ITEM_PARAMETERS)
{
	char *filtros[2];

	filtros[0]="ay";

	filtros[1]=0;

	//guardamos directorio actual
	char directorio_actual[PATH_MAX];
	getcwd(directorio_actual,PATH_MAX);

	//Obtenemos directorio de ultimo archivo
	//si no hay directorio, vamos a rutas predefinidas
	if (last_ay_file[0]==0) menu_chdir_sharedfiles();

	else {
					char directorio[PATH_MAX];
					util_get_dir(last_ay_file,directorio);
					//printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

					//cambiamos a ese directorio, siempre que no sea nulo
					if (directorio[0]!=0) {
									debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
									menu_filesel_chdir(directorio);
					}
	}


	int ret;

	ret=menu_filesel("Select AY File",filtros,last_ay_file);
	//volvemos a directorio inicial
	menu_filesel_chdir(directorio_actual);


	if (ret==1) {


		ay_player_load_and_play(last_ay_file);

	}
}

/*
void menu_ay_player_exit_tracker(MENU_ITEM_PARAMETERS)
{
	ay_player_stop_player();
}
*/

//Retorna indice a string teniendo en cuenta maximo en pantalla e incrementando en 1
int menu_ay_player_get_continuous_string(int indice_actual,int max_length,char *string,int *retardo)
{

	if ( (*retardo)<10 ) {
		(*retardo)++;
		return 0;
	}

	int longitud=strlen(&string[indice_actual]);
	if (longitud<=max_length) {
		indice_actual=0;
		*retardo=0;
	}
	else {
		indice_actual++;
	}

	return indice_actual;
}





int menu_audio_new_ayplayer_si_mostrar(void)
{
	int mostrar_player;


	mostrar_player=1;
	if (audio_ay_player_mem==NULL) mostrar_player=0;
	if (ay_player_playing.v==0) mostrar_player=0;

	return mostrar_player;
}


//Usado dentro del overlay de ayplayer
int menu_ayplayer_valor_contador_segundo_anterior;

int ayplayer_new_contador_string_author=0;
int ayplayer_new_contador_string_track_name=0;
int ayplayer_new_contador_string_misc=0;
int ayplayer_new_retardo_song_name=0;
int ayplayer_new_retardo_author=0;
int ayplayer_new_retardo_misc=0;


//Para hacer las barras de volumen con "caracter" que decae
int ayplayer_previo_valor_escalado=0;

int ayplayer_previo_valor_volume_A=0;
int ayplayer_previo_valor_volume_B=0;
int ayplayer_previo_valor_volume_C=0;

zxvision_window *menu_audio_new_ayplayer_overlay_window;	

void menu_audio_new_ayplayer_overlay(void)
{

    normal_overlay_texto_menu();

                        int linea;


    linea=7;
	int valor_escalado; 

	int vol_A,vol_B,vol_C;


    	if (menu_audio_new_ayplayer_si_mostrar()) {
    	//Los volumenes mostrarlos siempre a cada refresco
	char volumen[32];
	char textovolumen[35]; //32+3 de posible color rojo del maximo

	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

	vol_A=ay_3_8912_registros[0][8] & 15;
	vol_B=ay_3_8912_registros[0][9] & 15;
	vol_C=ay_3_8912_registros[0][10] & 15;

	ayplayer_previo_valor_volume_A=menu_decae_ajusta_valor_volumen(ayplayer_previo_valor_volume_A,vol_A);
	ayplayer_previo_valor_volume_B=menu_decae_ajusta_valor_volumen(ayplayer_previo_valor_volume_B,vol_B);
	ayplayer_previo_valor_volume_C=menu_decae_ajusta_valor_volumen(ayplayer_previo_valor_volume_C,vol_C);





	menu_string_volumen(volumen,ay_3_8912_registros[0][8],ayplayer_previo_valor_volume_A);
			sprintf (textovolumen,"Volume A: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textovolumen);

			menu_string_volumen(volumen,ay_3_8912_registros[0][9],ayplayer_previo_valor_volume_B);
			sprintf (textovolumen,"Volume B: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textovolumen);

			menu_string_volumen(volumen,ay_3_8912_registros[0][10],ayplayer_previo_valor_volume_C);
			sprintf (textovolumen,"Volume C: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textovolumen);



	//Obtenemos antes valor medio total y tambien maximo y minimo
	//Esto solo es necesario para dibujar onda llena



        audiobuffer_stats audiostats;
        audio_get_audiobuffer_stats(&audiostats);

	//int volumen_buffer=audiostats.volumen;

	//Ahora tenemos valor entre 0 y 128. Pasar a entre 0 y 15 
	//int valor_escalado=(mayor*16)/128;

	valor_escalado=audiostats.volumen_escalado;

	/*

	int valor_escalado=(volumen_buffer*16)/128;

	//Vigilar que no pase de 15
	if (valor_escalado>15) valor_escalado=15;
	*/

	//printf ("audiomin: %d audiomax: %d maximo: %d valor_escalado: %d\n",audiomin,audiomax,mayor,valor_escalado);

	//Y mostramos indicador volumen
	/*
	Nota: realmente este calculo de volumen no es del todo cierto, estoy viendo el valor maximo de la onda, aunque se puede generar
	sonido muy bajo, por ejemplo, oscilando valores entre 100 y 120 (considerando signed 8 bits), es mas, hay juegos que, al usar beeper,
	"mueven" esa onda hacia arriba, y aunque el indicador de volumen diga que esta muy alto, realmente se oye a volumen normal
	Pero bueno, la mayoria de las veces si que coincide bien el valor de volumen
	*/
	

	ayplayer_previo_valor_escalado=menu_decae_ajusta_valor_volumen(ayplayer_previo_valor_escalado,valor_escalado);
	//if (ayplayer_previo_valor_escalado<valor_escalado) ayplayer_previo_valor_escalado=valor_escalado;

			menu_string_volumen(volumen,valor_escalado,ayplayer_previo_valor_escalado);




								//"Volume C: %s"
			sprintf (textovolumen,"Output:   %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textovolumen);

	}


    //esto hara ejecutar esto 2 veces por segundo
    if ( ((contador_segundo%500) == 0 && menu_ayplayer_valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {

        menu_ayplayer_valor_contador_segundo_anterior=contador_segundo;
        //printf ("Refrescando. contador_segundo=%d\n",contador_segundo);
       
    


char textoplayer[40];

       


int mostrar_player;



	mostrar_player=menu_audio_new_ayplayer_si_mostrar();


         


		if (mostrar_player) {

			linea=0;

	//Indicadores de volumen que decaen
	ayplayer_previo_valor_escalado=menu_decae_dec_valor_volumen(ayplayer_previo_valor_escalado,valor_escalado);


	ayplayer_previo_valor_volume_A=menu_decae_dec_valor_volumen(ayplayer_previo_valor_volume_A,vol_A);
	ayplayer_previo_valor_volume_B=menu_decae_dec_valor_volumen(ayplayer_previo_valor_volume_B,vol_B);
	ayplayer_previo_valor_volume_C=menu_decae_dec_valor_volumen(ayplayer_previo_valor_volume_C,vol_C);




			//printf ("Dibujando player\n");

			z80_byte minutos,segundos,minutos_total,segundos_total;
			minutos=ay_song_length_counter/60/50;
			segundos=(ay_song_length_counter/50)%60;
			minutos_total=ay_song_length/60/50;
			segundos_total=(ay_song_length/50)%60;

//printf ("segundo. contador segundo: %d\n",contador_segundo);

			sprintf (textoplayer,"Track: %03d/%03d  (%02d:%02d/%02d:%02d)",ay_player_pista_actual,ay_player_total_songs(),minutos,segundos,minutos_total,segundos_total);
			//menu_escribe_linea_opcion(linea++,-1,1,textoplayer);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textoplayer);


			strncpy(textoplayer,&ay_player_file_song_name[ayplayer_new_contador_string_track_name],28);
			textoplayer[28]=0;
			//menu_escribe_linea_opcion(linea++,-1,1,textoplayer);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textoplayer);
			ayplayer_new_contador_string_track_name=menu_ay_player_get_continuous_string(ayplayer_new_contador_string_track_name,28,ay_player_file_song_name,&ayplayer_new_retardo_song_name);

			//menu_escribe_linea_opcion(linea++,-1,1,"Author");
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,"Author");
			strncpy(textoplayer,&ay_player_file_author[ayplayer_new_contador_string_author],28);
			textoplayer[28]=0;
			//menu_escribe_linea_opcion(linea++,-1,1,textoplayer);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textoplayer);
			ayplayer_new_contador_string_author=menu_ay_player_get_continuous_string(ayplayer_new_contador_string_author,28,ay_player_file_author,&ayplayer_new_retardo_author);

			//menu_escribe_linea_opcion(linea++,-1,1,"Misc");
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,"Misc");
			strncpy(textoplayer,&ay_player_file_misc[ayplayer_new_contador_string_misc],28);
			textoplayer[28]=0;
			//menu_escribe_linea_opcion(linea++,-1,1,textoplayer);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textoplayer);
			ayplayer_new_contador_string_misc=menu_ay_player_get_continuous_string(ayplayer_new_contador_string_misc,28,ay_player_file_misc,&ayplayer_new_retardo_misc);




		}
	}

	zxvision_draw_window_contents(menu_audio_new_ayplayer_overlay_window);
}



void menu_audio_new_ayplayer_load(MENU_ITEM_PARAMETERS)
{

	//restauramos modo normal de texto de menu
        set_menu_overlay_function(normal_overlay_texto_menu);

	
	menu_ay_player_load(0);

	//Restauramos funcion de overlay
	set_menu_overlay_function(menu_audio_new_ayplayer_overlay);

}

void menu_audio_new_ayplayer_prev(MENU_ITEM_PARAMETERS)
{
	ay_player_previous_track();

}

void menu_audio_new_ayplayer_next(MENU_ITEM_PARAMETERS)
{
	ay_player_next_track();

}

void menu_audio_new_ayplayer_stop(MENU_ITEM_PARAMETERS)
{
	ay_player_stop_player();

}

void menu_audio_new_ayplayer_repeat(MENU_ITEM_PARAMETERS)
{
	ay_player_repeat_file.v ^=1;

}


void menu_audio_new_ayplayer_exitend(MENU_ITEM_PARAMETERS)
{
	ay_player_exit_emulator_when_finish.v ^=1;
}

void menu_audio_new_ayplayer_cpcmode(MENU_ITEM_PARAMETERS)
{
	ay_player_cpc_mode.v ^=1;
	audio_ay_player_play_song(ay_player_pista_actual);
}

												

void menu_audio_new_ayplayer_inftracks(MENU_ITEM_PARAMETERS)
{
	//restauramos modo normal de texto de menu
        set_menu_overlay_function(normal_overlay_texto_menu);

	
	char string_length[5];
	sprintf(string_length,"%d",ay_player_limit_infinite_tracks/50);

        menu_ventana_scanf("Length (0-1310)",string_length,5);
	int l=parse_string_to_number(string_length);

	if (l<0 || l>1310) {
		menu_error_message("Invalid length value");
	}

	else ay_player_limit_infinite_tracks=l*50;

	

	//Restauramos funcion de overlay
	set_menu_overlay_function(menu_audio_new_ayplayer_overlay);
}

void menu_audio_new_ayplayer_len_anytracks(MENU_ITEM_PARAMETERS)
{

	//restauramos modo normal de texto de menu
        set_menu_overlay_function(normal_overlay_texto_menu);

	char string_length[5];
	sprintf(string_length,"%d",ay_player_limit_any_track/50);

	menu_ventana_scanf("Length (0-1310)",string_length,5);
	int l=parse_string_to_number(string_length);

	if (l<0 || l>1310) {
		menu_error_message("Invalid length value");
	}

	else ay_player_limit_any_track=l*50;


	//Restauramos funcion de overlay
	set_menu_overlay_function(menu_audio_new_ayplayer_overlay);
}



void menu_audio_new_ayplayer(MENU_ITEM_PARAMETERS)
{


        //menu_espera_no_tecla();




				if (!menu_multitarea) {
					menu_warn_message("This menu item needs multitask enabled");
					return;
				}

 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	zxvision_new_window(&ventana,0,1,32,20,
							32-1,20-2,"AY Player");
	zxvision_draw_window(&ventana);			


        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de audio waveform
	set_menu_overlay_function(menu_audio_new_ayplayer_overlay);


	menu_audio_new_ayplayer_overlay_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui		



	menu_item *array_menu_audio_new_ayplayer;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

        			ayplayer_new_contador_string_author=0;
				ayplayer_new_contador_string_track_name=0;
				ayplayer_new_contador_string_misc=0;

 				ayplayer_new_retardo_song_name=0;
				ayplayer_new_retardo_author=0;
				ayplayer_new_retardo_misc=0;

        	char textoplayer[40];
            //Hay que redibujar la ventana desde este bucle
            //menu_audio_new_ayplayer_dibuja_ventana();



            menu_add_item_menu_inicial_format(&array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_load,NULL,"~~Load");
            menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'l');
            menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Load AY file");

						
			

			int lin=12;

			menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin+5);

			if (menu_audio_new_ayplayer_si_mostrar() ) {
				//Vamos a borrar con espacios para que no quede rastro de opciones anteriores, como Yes/No 
				//Si no, pasaria que mostraria "Nos" como parte de la s final de Yes
				int i;
				for (i=13;i<=16;i++) {
					zxvision_fill_width_spaces(&ventana,i);
				}

				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_prev,NULL,"~~Prev");
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'p');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Previous song");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin);

				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_stop,NULL,"~~Stop");
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'s');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Stop song");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,6,lin);	

				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_next,NULL,"~~Next");
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'n');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Next song");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,11,lin);

				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_repeat,NULL,"~~Repeat: %s",
					(ay_player_repeat_file.v ? "Yes" : "No"));

				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'r');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Repeat from the beginning when finished all songs");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin+1);	

				
				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_exitend,NULL,"~~Exit end: %s",
					(ay_player_exit_emulator_when_finish.v ? "Yes" : "No") );
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'e');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Exit emulator when finished all songs");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,13,lin+1);	


				if (ay_player_limit_infinite_tracks==0) sprintf(textoplayer,"Length ~~infinite tracks: inf");
				else sprintf(textoplayer,"Length ~~infinite tracks: %d s",ay_player_limit_infinite_tracks/50);
				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_inftracks,NULL,textoplayer);
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'i');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Time limit for songs which doesn't have time limit");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin+2);			


				if (ay_player_limit_any_track==0) sprintf(textoplayer,"Length ~~any track: No limit");
				else sprintf(textoplayer,"Length ~~any track: %d s",ay_player_limit_any_track/50);
				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_len_anytracks,NULL,textoplayer);
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'a');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Time limit for all songs");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin+3);

				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_cpcmode,NULL,"~~CPC mode: %s",
					(ay_player_cpc_mode.v ? "Yes" : "No"));
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'c');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Switch to AY CPC mode");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin+4);		


			}			
/*


			sprintf(textoplayer,"~~CPC mode: %s",(ay_player_cpc_mode.v ? "Yes" : "No"));
			menu_escribe_linea_opcion(linea++,-1,1,textoplayer);
*/



		//Nombre de ventana solo aparece en el caso de stdout
                retorno_menu=menu_dibuja_menu(&audio_new_ayplayer_opcion_seleccionada,&item_seleccionado,array_menu_audio_new_ayplayer,"AY Player" );


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



       //restauramos modo normal de texto de menu
       set_menu_overlay_function(normal_overlay_texto_menu);


        cls_menu_overlay();

	//En caso de menus tabulados, es responsabilidad de este de liberar ventana
	zxvision_destroy_window(&ventana);				

}








#define DEBUG_HEXDUMP_WINDOW_X 0
#define DEBUG_HEXDUMP_WINDOW_Y 1
#define DEBUG_HEXDUMP_WINDOW_ANCHO 32
#define DEBUG_HEXDUMP_WINDOW_ALTO 22



void menu_debug_hexdump_with_ascii(char *dumpmemoria,menu_z80_moto_int dir_leida,int bytes_por_linea,z80_byte valor_xor)
{
	//dir_leida=adjust_address_space_cpu(dir_leida);

	menu_debug_set_memory_zone_attr();


	int longitud_direccion=MAX_LENGTH_ADDRESS_MEMORY_ZONE;

	menu_debug_print_address_memory_zone(dumpmemoria,dir_leida);

	

	//cambiamos el 0 final por un espacio
	dumpmemoria[longitud_direccion]=' ';

	menu_debug_registers_dump_hex(&dumpmemoria[longitud_direccion+1],dir_leida,bytes_por_linea);

	//01234567890123456789012345678901
	//000FFF ABCDABCDABCDABCD 12345678

	//metemos espacio
	int offset=longitud_direccion+1+bytes_por_linea*2;

	dumpmemoria[offset]=' ';
	//dumpmemoria[offset]='X';

	//Tener en cuenta el valor xor

	menu_debug_registers_dump_ascii(&dumpmemoria[offset+1],dir_leida,bytes_por_linea,menu_debug_hexdump_with_ascii_modo_ascii,valor_xor);

	//printf ("%s\n",dumpmemoria);
}


menu_z80_moto_int menu_debug_hexdump_direccion=0;

int menu_hexdump_edit_position_x=0; //Posicion del cursor relativa al inicio del volcado hexa
int menu_hexdump_edit_position_y=0; //Posicion del cursor relativa al inicio del volcado hexa
const int menu_hexdump_lineas_total=13;

int menu_hexdump_edit_mode=0;
const int menu_hexdump_bytes_por_linea=8;

void menu_debug_hexdump_cursor_abajo(void);
void menu_debug_hexdump_cursor_arriba(void);



void menu_debug_hexdump_print_editcursor(zxvision_window *ventana,int x,int y,char caracter)
{
	//z80_byte papel=ESTILO_GUI_PAPEL_NORMAL;
    //z80_byte tinta=ESTILO_GUI_TINTA_NORMAL;

	//Inverso
	z80_byte papel=ESTILO_GUI_PAPEL_SELECCIONADO;
    z80_byte tinta=ESTILO_GUI_TINTA_SELECCIONADO;	

	//Si multitarea esta off, no se vera el parpadeo. Entonces cambiar el caracter por cursor '_'
	if (!menu_multitarea) caracter='_';

	//putchar_menu_overlay_parpadeo(x,y,caracter,tinta,papel,1);
	zxvision_print_char_simple(ventana,x,y,tinta,papel,1,caracter);

}

void menu_debug_hexdump_print_editcursor_nibble(zxvision_window *ventana,int x,int y,char caracter)
{
	//z80_byte papel=ESTILO_GUI_PAPEL_NORMAL;
    //z80_byte tinta=ESTILO_GUI_TINTA_NORMAL;

	//Inverso
	z80_byte papel=ESTILO_GUI_PAPEL_SELECCIONADO;
    z80_byte tinta=ESTILO_GUI_TINTA_SELECCIONADO;	

	//putchar_menu_overlay_parpadeo(x,y,caracter,tinta,papel,0);
	zxvision_print_char_simple(ventana,x,y,tinta,papel,0,caracter);

}

void menu_debug_hexdump_edit_cursor_izquierda(void)
{
	if (menu_hexdump_edit_position_x>0) {
		menu_hexdump_edit_position_x--;

		//Si en medio del espacio entre hexa y ascii
		if (menu_hexdump_edit_position_x==menu_hexdump_bytes_por_linea*2) menu_hexdump_edit_position_x--;
	}

	else {
		//Aparecer por la derecha
		menu_debug_hexdump_cursor_arriba();
		menu_hexdump_edit_position_x=menu_hexdump_bytes_por_linea*3;
	}

}

//escribiendo_memoria cursor indica que si estamos a la derecha de zona de edicion escribiendo,
//tiene que saltar a la zona izquierda de la zona ascii o hexa, al llegar a la derecha de dicha zona
void menu_debug_hexdump_edit_cursor_derecha(int escribiendo_memoria)
{

	//Hexdump. bytes_por_linea*2 espacio bytes_por_linea

	int ancho_linea=menu_hexdump_bytes_por_linea*3+1;

	if (menu_hexdump_edit_position_x<ancho_linea-1) {
		menu_hexdump_edit_position_x++;

		if (menu_hexdump_edit_position_x==menu_hexdump_bytes_por_linea*2) { //Fin zona derecha hexa
			if (escribiendo_memoria) {
				//Ponernos al inicio zona hexa de nuevo saltando siguiente linea
				menu_hexdump_edit_position_x=0;
				menu_debug_hexdump_cursor_abajo();
			}
			else {
				//Saltar a zona ascii
				menu_hexdump_edit_position_x++;
			}
		}
	}

	else {
		//Fin zona derecha ascii. 
		menu_debug_hexdump_cursor_abajo();

		if (escribiendo_memoria) {
			//Ponernos en el principio zona ascii
			menu_hexdump_edit_position_x=menu_hexdump_bytes_por_linea*2+1;
		}
		else {
			menu_hexdump_edit_position_x=0;
		}
	}

}

void menu_debug_hexdump_cursor_arriba(void)
{
	int alterar_ptr=0;
						//arriba
						if (menu_hexdump_edit_mode) {
							if (menu_hexdump_edit_position_y>0) menu_hexdump_edit_position_y--;
							else alterar_ptr=1;
						}

						else {
							alterar_ptr=1;
						}

						if (alterar_ptr) {
							menu_debug_hexdump_direccion -=menu_hexdump_bytes_por_linea;
							menu_debug_hexdump_direccion=menu_debug_hexdump_adjusta_en_negativo(menu_debug_hexdump_direccion,menu_hexdump_bytes_por_linea);
						}
}

void menu_debug_hexdump_cursor_abajo(void)
{
	int alterar_ptr=0;
						//abajo
						if (menu_hexdump_edit_mode) {
							if (menu_hexdump_edit_position_y<menu_hexdump_lineas_total-1) menu_hexdump_edit_position_y++;
							else alterar_ptr=1;
						}						
						else {
							alterar_ptr=1;
						}

						if (alterar_ptr) {
							menu_debug_hexdump_direccion +=menu_hexdump_bytes_por_linea;
						}
}

void menu_debug_hexdump_copy(void)
{


    char string_address[10];

    sprintf (string_address,"%XH",menu_debug_hexdump_direccion);
    menu_ventana_scanf("Source?",string_address,10);
	menu_z80_moto_int source=parse_string_to_number(string_address);

    sprintf (string_address,"%XH",source);
    menu_ventana_scanf("Destination?",string_address,10);
	menu_z80_moto_int destination=parse_string_to_number(string_address);

    strcpy (string_address,"1");
    menu_ventana_scanf("Length?",string_address,10);
	menu_z80_moto_int longitud=parse_string_to_number(string_address);	

	if (menu_confirm_yesno("Copy bytes")) {
		for (;longitud>0;source++,destination++,longitud--) poke_byte_z80_moto(destination,peek_byte_z80_moto(source) );
	}


}

void menu_debug_hexdump_aviso_edit_filezone(zxvision_window *w)
{
							menu_warn_message("Memory zone is File zone. Changes won't be saved to the file");
							//Volver a dibujar ventana, pues se ha borrado al aparecer el aviso
							//menu_debug_hexdump_ventana();	
	zxvision_draw_window(w);
}



void menu_debug_hexdump(MENU_ITEM_PARAMETERS)
{
	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	//asignamos mismo ancho visible que ancho total para poder usar la ultima columna de la derecha, donde se suele poner scroll vertical
	zxvision_new_window(&ventana,DEBUG_HEXDUMP_WINDOW_X,DEBUG_HEXDUMP_WINDOW_Y,DEBUG_HEXDUMP_WINDOW_ANCHO,DEBUG_HEXDUMP_WINDOW_ALTO,
							DEBUG_HEXDUMP_WINDOW_ANCHO,DEBUG_HEXDUMP_WINDOW_ALTO-2,"Hexadecimal Editor");


	ventana.can_use_all_width=1; //Para poder usar la ultima columna de la derecha donde normalmente aparece linea scroll

	zxvision_draw_window(&ventana);


    z80_byte tecla;

	int salir=0;

	z80_byte valor_xor=0;



	if (MACHINE_IS_ZX80) menu_debug_hexdump_with_ascii_modo_ascii=1;
	else if (MACHINE_IS_ZX81) menu_debug_hexdump_with_ascii_modo_ascii=2;

	else menu_debug_hexdump_with_ascii_modo_ascii=0;


	int asked_about_writing_rom=0;

                //Guardar estado atajos
                z80_bit antes_menu_writing_inverse_color;
                antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;	



        do {

			int cursor_en_zona_ascii=0;
			int editando_en_zona_ascii=0;


					//Si maquina no es QL, direccion siempre entre 0 y 65535
					//menu_debug_hexdump_direccion=adjust_address_space_cpu(menu_debug_hexdump_direccion);
					menu_debug_hexdump_direccion=adjust_address_memory_size(menu_debug_hexdump_direccion);


		int linea=0;

		int lineas_hex=0;



		int bytes_por_ventana=menu_hexdump_bytes_por_linea*menu_hexdump_lineas_total;

		char dumpmemoria[33];



		//Antes de escribir, normalizar zona memoria
		menu_debug_set_memory_zone_attr();

				char textoshow[33];

		sprintf (textoshow,"Showing %d bytes per page:",bytes_por_ventana);
        //menu_escribe_linea_opcion(linea++,-1,1,textoshow);
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,textoshow);

        //menu_escribe_linea_opcion(linea++,-1,1,"");
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");


		//Hacer que texto ventana empiece pegado a la izquierda
		menu_escribe_linea_startx=0;

		//No mostrar caracteres especiales
		menu_disable_special_chars.v=1;

		//Donde esta el otro caracter que acompanya al nibble, en caso de cursor en zona hexa
		int menu_hexdump_edit_position_x_nibble=menu_hexdump_edit_position_x^1;

		
		if (menu_hexdump_edit_position_x>menu_hexdump_bytes_por_linea*2) cursor_en_zona_ascii=1;


		if (menu_hexdump_edit_mode && cursor_en_zona_ascii) editando_en_zona_ascii=1;		

		char nibble_char='X';	
		char nibble_char_cursor='X';	

		for (;lineas_hex<menu_hexdump_lineas_total;lineas_hex++,linea++) {

			menu_z80_moto_int dir_leida=menu_debug_hexdump_direccion+lineas_hex*menu_hexdump_bytes_por_linea;
			menu_debug_hexdump_direccion=adjust_address_memory_size(menu_debug_hexdump_direccion);

			menu_debug_hexdump_with_ascii(dumpmemoria,dir_leida,menu_hexdump_bytes_por_linea,valor_xor);

			zxvision_print_string_defaults_fillspc(&ventana,0,linea,dumpmemoria);

			//Meter el nibble_char si corresponde
			if (lineas_hex==menu_hexdump_edit_position_y) {
				nibble_char_cursor=dumpmemoria[7+menu_hexdump_edit_position_x];
				if (!editando_en_zona_ascii) nibble_char=dumpmemoria[7+menu_hexdump_edit_position_x_nibble];
			}
		}


		menu_escribe_linea_startx=1;

		//Volver a mostrar caracteres especiales
		menu_disable_special_chars.v=0;		



		//Mostrar cursor si en modo edicion
		
		//Al mostrar en cursor: si esta en parte ascii, hacer parpadear el caracter en esa zona, metiendo color de opcion seleccionada
		//Si esta en parte hexa, parpadeamos la parte del nibble que editamos, el otro nibble no parpadea. Ambos tienen color de opcion seleccionada
		//Si multitarea esta a off, no existe el parpadeo, y por tanto, para que se viera en que nibble edita, se mostrara el caracter _, logicamente
		//tapando el caracter de debajo
		//Para ver los caracteres de debajo, los asignamos antes, en el bucle que hace el volcado hexa, y lo guardo en las variables
		//nibble_char_cursor (que dice el caracter de debajo del cursor) y nibble_char (que dice el otro caracter que acompanya al nibble)
	
		
		if (menu_hexdump_edit_mode) {
			//int xfinal=DEBUG_HEXDUMP_WINDOW_X+7+menu_hexdump_edit_position_x;
			//int yfinal=DEBUG_HEXDUMP_WINDOW_Y+3+menu_hexdump_edit_position_y;
			int xfinal=7+menu_hexdump_edit_position_x;
			int yfinal=2+menu_hexdump_edit_position_y;			

			menu_debug_hexdump_print_editcursor(&ventana,xfinal,yfinal,nibble_char_cursor);

			//Indicar nibble entero. En caso de edit hexa
			if (!editando_en_zona_ascii) {
				//xfinal=DEBUG_HEXDUMP_WINDOW_X+7+menu_hexdump_edit_position_x_nibble;
				xfinal=7+menu_hexdump_edit_position_x_nibble;
				menu_debug_hexdump_print_editcursor_nibble(&ventana,xfinal,yfinal,nibble_char);
			}
		}


				//Forzar a mostrar atajos
                menu_writing_inverse_color.v=1;


//printf ("zone size: %x dir: %x\n",menu_debug_memory_zone_size,menu_debug_hexdump_direccion);

        //menu_escribe_linea_opcion(linea++,-1,1,"");
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");

		char buffer_linea[64]; //Por si acaso, entre negritas y demas

		char buffer_char_type[20];

		char string_atajos[3]="~~"; 
		//Si esta en edit mode y en zona de ascii, no hay atajos



		if (editando_en_zona_ascii) string_atajos[0]=0;

				if (menu_debug_hexdump_with_ascii_modo_ascii==0) {
					sprintf (buffer_char_type,"ASCII");
				}

				else if (menu_debug_hexdump_with_ascii_modo_ascii==1) {
                	sprintf (buffer_char_type,"ZX80");
                }

				else sprintf (buffer_char_type,"ZX81");



				sprintf (buffer_linea,"%sMemptr C%sopy",string_atajos,string_atajos);


				//menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_linea);

				sprintf (buffer_linea,"%sInvert:%s Edi%st:%s C%shar:%s",
					string_atajos,
					(valor_xor==0 ? "No" : "Yes"), 
					string_atajos,
					(menu_hexdump_edit_mode==0 ? "No" : "Yes" ),
					string_atajos,
					buffer_char_type
					);
				//menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_linea);


				char memory_zone_text[64]; //espacio temporal mas grande por si acaso
				if (menu_debug_show_memory_zones==0) {
					sprintf (memory_zone_text,"Mem %szone (mapped memory)",string_atajos);
				}
				else {
					//printf ("Info zona %d\n",menu_debug_memory_zone);
					char buffer_name[MACHINE_MAX_MEMORY_ZONE_NAME_LENGHT+1];
					//int readwrite;
					machine_get_memory_zone_name(menu_debug_memory_zone,buffer_name);
					sprintf (memory_zone_text,"Mem %szone (%d %s)",string_atajos,menu_debug_memory_zone,buffer_name);
					//printf ("size: %X\n",menu_debug_memory_zone_size);
					//printf ("Despues zona %d\n",menu_debug_memory_zone);
				}

				//truncar texto a 32 por si acaso
				memory_zone_text[32]=0;
				//menu_escribe_linea_opcion(linea++,-1,1,memory_zone_text);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,memory_zone_text);

				sprintf (textoshow,"   Size: %d (%d KB)",menu_debug_memory_zone_size,menu_debug_memory_zone_size/1024);
				//menu_escribe_linea_opcion(linea++,-1,1,textoshow);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,textoshow);

		



//Restaurar comportamiento atajos
menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;

			zxvision_draw_window_contents(&ventana);
			//NOTA: este menu no acostumbra a refrescar rapido la ventana cuando la redimensionamos con el raton
			//es una razon facil: el volcado de hexa usa relativamente mucha cpu,
			//cada vez que redimensionamos ventana, se llama al bucle continuamente, usando mucha cpu y si esta el autoframeskip,
			//hace saltar frames
			

			tecla=zxvision_common_getkey_refresh();		


				//Aviso: hay que conseguir que las letras de accion no esten entre la a-f, porque asi,
				//podemos usar dichas letras para editar hexa

				switch (tecla) {

					case 11:
						menu_debug_hexdump_cursor_arriba();

					break;

					case 10:
						menu_debug_hexdump_cursor_abajo();

					break;

					case 8:
					case 12:
						//izquierda o delete
						if (menu_hexdump_edit_mode) {
							//if (menu_hexdump_edit_position_x>0) menu_hexdump_edit_position_x--;
							menu_debug_hexdump_edit_cursor_izquierda();
						}
					break;

					case 9:
						//derecha
						if (menu_hexdump_edit_mode) {
							menu_debug_hexdump_edit_cursor_derecha(0);
							//if (menu_hexdump_edit_position_x<(bytes_por_linea*2)-1) menu_hexdump_edit_position_x++;
						}
					break;					

					case 24:
						//PgUp
						menu_debug_hexdump_direccion -=bytes_por_ventana;
						menu_debug_hexdump_direccion=menu_debug_hexdump_adjusta_en_negativo(menu_debug_hexdump_direccion,bytes_por_ventana);
					break;

					case 25:
						//PgDn
						menu_debug_hexdump_direccion +=bytes_por_ventana;
					break;

					case 'm':
						if (!editando_en_zona_ascii)  {
							menu_debug_hexdump_direccion=menu_debug_hexdump_change_pointer(menu_debug_hexdump_direccion);
							//menu_debug_hexdump_ventana();
							zxvision_draw_window(&ventana);
						}
					break;

					case 'o':
						if (!editando_en_zona_ascii)  {
							menu_debug_hexdump_copy();
							//menu_debug_hexdump_ventana();
							zxvision_draw_window(&ventana);
						}
					break;					

					case 'h':
						if (!editando_en_zona_ascii)  {
							menu_debug_hexdump_with_ascii_modo_ascii++;
							if (menu_debug_hexdump_with_ascii_modo_ascii==3) menu_debug_hexdump_with_ascii_modo_ascii=0;
						}
					break;

					case 'i':
						if (!editando_en_zona_ascii) valor_xor ^= 255;
					break;

					case 't':
						if (!editando_en_zona_ascii) {
							menu_hexdump_edit_mode ^= 1;
							menu_espera_no_tecla();
							tecla=0; //para no enviar dicha tecla al editor
						}

						//Si zona de filemem
						if (menu_hexdump_edit_mode && menu_debug_memory_zone==MEMORY_ZONE_NUM_FILE_ZONE) {
							menu_debug_hexdump_aviso_edit_filezone(&ventana);				
						}
					break;					

					//case 'l':
					//	menu_debug_hex_shows_inves_low_ram.v ^=1;
					//break;

					case 'z':

						if (!editando_en_zona_ascii) {
							menu_debug_change_memory_zone();
							asked_about_writing_rom=0;
						}

						break;

					//Salir con ESC si no es modo edit
					case 2:
						if (menu_hexdump_edit_mode) {
							menu_hexdump_edit_mode=0;
						}
						else salir=1;
					break;

					//Enter tambien sale de modo edit
					case 13:
						if (menu_hexdump_edit_mode) menu_hexdump_edit_mode=0;
					break;



				}

				//Y ahora para el caso de edit_mode y pulsar tecla hexa o ascii segun la zona
				int editar_byte=0;
				if (menu_hexdump_edit_mode) {
					//Para la zona ascii
					if (cursor_en_zona_ascii && tecla>=32 && tecla<=126) editar_byte=1; 

					//Para la zona hexa
					if (
						!cursor_en_zona_ascii && 
						( (tecla>='0' && tecla<='9') || (tecla>='a' && tecla<='f') )
						) {
						editar_byte=1; 
					}
				}

				//Ver si vamos a editar en zona de rom
				if (editar_byte) {
					int attribute_zone;
					//Si zona por defecto mapped memory, asumimos lectura/escritura
					if (menu_debug_memory_zone==-1) attribute_zone=3;
					else machine_get_memory_zone_attrib(menu_debug_memory_zone, &attribute_zone);

					//printf ("Attrib zone %d asked %d\n",attribute_zone,asked_about_writing_rom);

					//Attrib: bit 0: read, bit 1: write
					//Si no tiene atributo de escritura y no se ha pedido antes si se quiere escribir en rom
					if ( (attribute_zone&2)==0 && !asked_about_writing_rom) {
						if (menu_confirm_yesno_texto("Memory zone is ROM","Sure you want to edit?")==0) {
							editar_byte=0;
						}
						else {
							asked_about_writing_rom=1;
						}

						//Volver a dibujar ventana, pues se ha borrado al pregutar confirmacion
						//menu_debug_hexdump_ventana();
						zxvision_draw_window(&ventana);
					}

 
				}


				//asked_about_writing_rom

				if (editar_byte) {
						//Obtener direccion puntero
						menu_z80_moto_int direccion_cursor=menu_debug_hexdump_direccion;

						//int si_zona_hexa=0; //en zona hexa o ascii
						//if (menu_hexdump_edit_position_x<bytes_por_linea*2) si_zona_hexa=1;


						if (!cursor_en_zona_ascii) {
							//Sumar x (cada dos, una posicion)
							direccion_cursor +=menu_hexdump_edit_position_x/2;
						}
						else {
							int indice_hasta_ascii=menu_hexdump_bytes_por_linea*2+1; //el hexa y el espacio
							direccion_cursor +=menu_hexdump_edit_position_x-indice_hasta_ascii;
						}

						//Sumar y. 
						direccion_cursor +=menu_hexdump_edit_position_y*menu_hexdump_bytes_por_linea;

						//Ajustar direccion a zona memoria
						direccion_cursor=adjust_address_memory_size(direccion_cursor);

						//TODO: ver si se sale de tamanyo zona memoria

						//printf ("Direccion edicion: %X\n",direccion_cursor);

						//Obtenemos byte en esa posicion
						z80_byte valor_leido=menu_debug_get_mapped_byte(direccion_cursor);


						//Estamos en zona hexa o ascii

						if (!cursor_en_zona_ascii) {
							//printf ("Zona hexa\n");
							//Zona hexa

							//Obtener valor nibble
							z80_byte valor_nibble;

							if (tecla>='0' && tecla<='9') valor_nibble=tecla-'0';
							else valor_nibble=tecla-'a'+10;

							//Ver si par o impar
							if ( (menu_hexdump_edit_position_x %2) ==0) {
								//par. alterar nibble alto
								valor_leido=(valor_leido&0x0F) | (valor_nibble<<4);
							}

							else {
								//impar. alterar nibble bajo
								valor_leido=(valor_leido&0xF0) | valor_nibble;
							}
						}

						else {
							//printf ("Zona ascii\n");
							valor_leido=tecla;

							//En el caso de zx80/81

							if (menu_debug_hexdump_with_ascii_modo_ascii==1) valor_leido=ascii_to_zx80(valor_leido);
							if (menu_debug_hexdump_with_ascii_modo_ascii==2) valor_leido=ascii_to_zx81(valor_leido);
						}



						//Escribimos valor
						menu_debug_write_mapped_byte(direccion_cursor,valor_leido);

						//Y mover cursor a la derecha
						menu_debug_hexdump_edit_cursor_derecha(1);

						//Si se llega a detecha de hexa o ascii, saltar linea

					
				}	


        } while (salir==0);

	cls_menu_overlay();
	zxvision_destroy_window(&ventana);
	

}



//Entrada seleccionada
int adventure_keyboard_selected_item=0;

//Posicion dentro del string
int adventure_keyboard_index_selected_item=0;

//z80_bit menu_osd_adventure_sending_keys={0};




int adventure_keyboard_pending_send_final_spc=1;

void menu_osd_adventure_kb_press_key_variable(char letra)
{
	if (letra==0) return; //pequenyo bug: si acaba texto con ~~ no se abrira luego de nuevo el menu. Bug???

	//printf ("Pulsar tecla entrada %d indice en entrada: %d letra: %c\n",adventure_keyboard_selected_item,adventure_keyboard_index_selected_item,letra);
	//osd_adv_kbd_list
	debug_printf (VERBOSE_DEBUG,"Pressing key %c of word %s",letra,osd_adv_kbd_list[adventure_keyboard_selected_item]);

	//Espacio no la gestiona esta funcion de convert_numeros_...
	if (letra==' ') util_set_reset_key(UTIL_KEY_SPACE,1);
	//else convert_numeros_letras_puerto_teclado_continue(letra,1);
	else ascii_to_keyboard_port(letra);

	//Lanzar pulsar tecla 
	timer_on_screen_adv_key=adventure_keyboard_key_length; 
}

void menu_osd_adventure_kb_press_key(void)
{

	//Aunque el usuario haya puesto alguna mayuscula, metemos minusculas
	char letra;

	//Ignorar ~~

	do {
		letra=letra_minuscula(osd_adv_kbd_list[adventure_keyboard_selected_item][adventure_keyboard_index_selected_item]);
		if (letra=='~') adventure_keyboard_index_selected_item++;
	} while (letra=='~' && letra!=0);

	menu_osd_adventure_kb_press_key_variable(letra);

	/*if (letra==0) return; //pequenyo bug: si acaba texto con ~~ no se abrira luego de nuevo el menu. Bug???

	//printf ("Pulsar tecla entrada %d indice en entrada: %d letra: %c\n",adventure_keyboard_selected_item,adventure_keyboard_index_selected_item,letra);
	//osd_adv_kbd_list
	debug_printf (VERBOSE_DEBUG,"Pressing key %c of word %s",letra,osd_adv_kbd_list[adventure_keyboard_selected_item]);

	//Espacio no la gestiona esta funcion de convert_numeros_...
	if (letra==' ') util_set_reset_key(UTIL_KEY_SPACE,1);
	//else convert_numeros_letras_puerto_teclado_continue(letra,1);
	else ascii_to_keyboard_port(letra);

	//Lanzar pulsar tecla 
	timer_on_screen_adv_key=adventure_keyboard_key_length; */

}


void menu_osd_adventure_keyboard_action(MENU_ITEM_PARAMETERS)
{
	//printf ("opcion seleccionada: %d\n",valor_opcion);
	adventure_keyboard_selected_item=valor_opcion;
	adventure_keyboard_index_selected_item=0;
	adventure_keyboard_pending_send_final_spc=1;


	//Estamos enviando teclas
	//menu_osd_adventure_sending_keys.v=1;

	menu_osd_adventure_kb_press_key();
}

//Retorno desde el core
void menu_osd_adventure_keyboard_next(void)
{

	//if (menu_osd_adventure_sending_keys.v==0 return;

	//Si final de string
	adventure_keyboard_index_selected_item++;
	if (osd_adv_kbd_list[adventure_keyboard_selected_item][adventure_keyboard_index_selected_item]==0) {
		//printf ("Fin texto\n");
		
		//Ver si habia que enviar un espacio al final
		if (adventure_keyboard_send_final_spc && adventure_keyboard_pending_send_final_spc) {
			menu_osd_adventure_kb_press_key_variable(' ');
			adventure_keyboard_pending_send_final_spc=0;
		}
		else {
			//En este caso reabrir el menu
			menu_osd_adventure_keyboard(0);
			return;
		}
	}

	//Siguiente tecla
	else menu_osd_adventure_kb_press_key();
}


#define ADVENTURE_KB_X 0
#define ADVENTURE_KB_Y 0
#define ADVENTURE_KB_ANCHO 32
#define ADVENTURE_KB_ALTO 24


void menu_osd_adventure_keyboard(MENU_ITEM_PARAMETERS)
{

	//Si estamos enviando teclas
	//if (menu_osd_adventure_sending_keys.v) {
	//	menu_osd_adventure_keyboard_next();
	//	return;
	//}

	//Si lista vacia, error
	if (osd_adv_kbd_defined==0) {
		debug_printf (VERBOSE_ERR,"Empty list");
		return;
	}

	//Si estamos enviando teclas, desactivar
	timer_on_screen_adv_key=0;



 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	zxvision_new_window(&ventana,ADVENTURE_KB_X,ADVENTURE_KB_Y,ADVENTURE_KB_ANCHO,ADVENTURE_KB_ALTO,
							ADVENTURE_KB_ANCHO-1,ADVENTURE_KB_ALTO-2,"OSD Adventure Keyboard");
	zxvision_draw_window(&ventana);		


       
        menu_item *array_menu_osd_adventure_keyboard;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

		int initial_test; //si es 1, haremos el calculo inicial del alto

		int alto_ventana=ADVENTURE_KB_ALTO;
		int y_ventana=ADVENTURE_KB_Y;

		for (initial_test=1;initial_test>=0;initial_test--) {


          //Hay que redibujar la ventana desde este bucle
          if (!initial_test) {
			  //menu_dibuja_ventana(ADVENTURE_KB_X,y_ventana,ADVENTURE_KB_ANCHO,alto_ventana,"OSD Adventure Keyboard");
			  zxvision_set_y_position(&ventana,y_ventana);
			  zxvision_set_visible_height(&ventana,alto_ventana);
			  
			  //Alteramos alto total para que coincida con alto ventana (siempre que sea menor que el alto actual)
			  //si fuese mayor el alto, estariamos necesitando mas memoria y seria un problema
			  //esto es un poco feo realmente, pero bueno, al reducir el tamaño no hay problema de que nos salgamos de la memoria
			  int current_height=ventana.total_height;
			  int desired_height=alto_ventana-2;

			  if (desired_height<current_height) {
				  ventana.total_height=desired_height;
			  }
		  }


                //Como no sabemos cual sera el item inicial, metemos este sin asignar, que se sobreescribe en el siguiente menu_add_item_menu
                menu_add_item_menu_inicial(&array_menu_osd_adventure_keyboard,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

	//if (osd_adv_kbd_list[adventure_keyboard_selected_item][adventure_keyboard_index_selected_item]==0) {
	//osd_adv_kbd_defined
		int i;
		int last_x=1;
		int last_y=0;
		int salir=0;

		//Asignar hotkeys, segun si se han asignado antes o no
		//int hotkeys_assigned[26]; //de la A a la Z
		//for (i=0;i<26;i++) hotkeys_assigned[i]=0;
		
		
		for (i=0;i<osd_adv_kbd_defined && !salir;i++) {
			int longitud_texto=strlen(osd_adv_kbd_list[i])+1; //Espacio para la entrada y 1 espacio
			if (last_x+longitud_texto>ADVENTURE_KB_ANCHO) {
				last_x=1;
				last_y++; 
			}

			//controlar maximo de alto
			if (last_y>=ADVENTURE_KB_ALTO-2) {
				debug_printf (VERBOSE_DEBUG,"Reached maximum window height");
				last_y--;
				salir=1;
			}

			else {
				//Si es cadena vacia, ignorarla. No deberia pasar pues se debe denegar desde donde se lee la configuracion, pero por si acaso
				if (osd_adv_kbd_list[i][0]==0) {
					debug_printf (VERBOSE_DEBUG,"Null string at %d",i);
				}

				else {

					int tiene_hotkey=0;

					char texto_opcion[64];
					strcpy(texto_opcion,osd_adv_kbd_list[i]);

					char hotkey;
					
					//Caracter de hotkey. Crearlo automaticamente
					//hotkey=letra_minuscula(osd_adv_kbd_list[i][0]);

					//Caracter de hotkey. Dejar que el usuario lo escriba en la cadena de texto. Ver si dicha cadena lo tiene

					int j;
					for (j=0;texto_opcion[j];j++) {
						if (texto_opcion[j]=='~' && texto_opcion[j+1]=='~') {
							//Si hay letra detras
							hotkey=letra_minuscula(texto_opcion[j+2]);
							if (hotkey) tiene_hotkey=1;
						}
					}
					

					//Caracter de hotkey. Crearlo automaticamente
					/*if (hotkey>='a' && hotkey<='z') {
						//Ver si no se ha usado antes
						int indice_hotkey=hotkey-'a';
						if (hotkeys_assigned[indice_hotkey]==0) {
							hotkeys_assigned[indice_hotkey]=1;
							sprintf (texto_opcion,"~~%s",osd_adv_kbd_list[i]);
							tiene_hotkey=1;
						}
					}*/

		                        menu_add_item_menu_format(array_menu_osd_adventure_keyboard,MENU_OPCION_NORMAL,menu_osd_adventure_keyboard_action,NULL,texto_opcion);
        		                menu_add_item_menu_tabulado(array_menu_osd_adventure_keyboard,last_x,last_y);
					menu_add_item_menu_valor_opcion(array_menu_osd_adventure_keyboard,i);

					if (tiene_hotkey) {
						menu_add_item_menu_shortcut(array_menu_osd_adventure_keyboard,hotkey);
						longitud_texto -=2;
					}

				}

				last_x+=longitud_texto;
			}

		}

		//Recalcular alto, y_inivial
		//del alto, se pierden 2 siempre
		//si tuvieramos el maximo de y, valdria 21. Y el maximo de alto es 24
		//printf ("ultima y: %d\n",last_y);
		alto_ventana=last_y+3;
		y_ventana=12-alto_ventana/2;
		if (y_ventana<0) y_ventana=0;	

                //int alto_ventana=last_y;
                //int y_ventana=ADVENTURE_KB_Y;


		}



//Nombre de ventana solo aparece en el caso de stdout
                retorno_menu=menu_dibuja_menu(&osd_adventure_keyboard_opcion_seleccionada,&item_seleccionado,array_menu_osd_adventure_keyboard,"OSD Adventure KB" );


        cls_menu_overlay();
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
				//printf ("Item seleccionado: %d\n",item_seleccionado.valor_opcion);
                                //printf ("actuamos por funcion\n");

	                        salir_todos_menus=1;

                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                cls_menu_overlay();
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);


        cls_menu_overlay();
		//menu_espera_no_tecla();

		//menu_abierto=1;
		//Si con control de joystick se ha salido con tecla ESCMenu, esa tecla de joystick lo que hace es ESC
		//pero luego fuerza a abrir el menu de nuevo. Por tanto, decimos que no hay que abrir menu
		menu_event_open_menu.v=0;

		//printf ("en final de funcion\n");
		zxvision_destroy_window(&ventana);

}


