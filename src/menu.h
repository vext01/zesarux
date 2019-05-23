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

#ifndef MENU_H
#define MENU_H

#include <dirent.h>
#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif


#include "cpu.h"

//Por el tema de usar PATH_MAX en windows
#include "utils.h"

//Valor para ninguna tecla pulsada
//Tener en cuenta que en spectrum y zx80/81 se usan solo 5 bits pero en Z88 se usan 8 bits
//en casos de spectrum y zx80/81 se agregan los 3 bits faltantes
#define MENU_PUERTO_TECLADO_NINGUNA 255

#define MENU_ITEM_PARAMETERS int valor_opcion GCC_UNUSED

//Usado en ver sprites y ver colores mapeados
#define MENU_TOTAL_MAPPED_PALETTES 16



extern int menu_overlay_activo;
extern void (*menu_overlay_function)(void);

extern char *esc_key_message;
extern char *openmenu_key_message;

extern z80_bit menu_desactivado;

extern void set_menu_overlay_function(void (*funcion)(void) );
extern void reset_menu_overlay_function(void);
extern void pruebas_texto_menu(void);
extern void cls_menu_overlay(void);
extern void menu_escribe_texto(z80_byte x,z80_byte y,z80_byte tinta,z80_byte papel,char *texto);
extern void normal_overlay_texto_menu(void);
extern int si_menu_mouse_en_ventana(void);
extern void menu_calculate_mouse_xy(void);

extern void menu_ventana_draw_vertical_perc_bar(int x,int y,int ancho,int alto,int porcentaje,int estilo_invertido);
extern void menu_ventana_draw_horizontal_perc_bar(int x,int y,int ancho,int alto,int porcentaje,int estilo_invertido);

extern void menu_espera_tecla(void);
extern void menu_espera_no_tecla_con_repeticion(void);
extern int menu_generic_message_final_abajo(int primera_linea,int alto_ventana,int indice_linea);
extern void menu_espera_tecla_timeout_window_splash(void);
extern void menu_cpu_core_loop(void);
extern z80_byte menu_get_pressed_key(void);

extern int menu_cond_zx8081(void);
extern int menu_cond_zx8081_realvideo(void);
extern int menu_cond_zx8081_no_realvideo(void);
extern int menu_cond_realvideo(void);
extern int menu_display_rainbow_cond(void);
extern int menu_cond_stdout(void);
extern int menu_cond_simpletext(void);
extern int menu_cond_curses(void);

extern int zxvision_drawing_in_background;

extern int menu_hardware_advanced_input_value(int minimum,int maximum,char *texto,int *variable);
extern void menu_interface_rgb_inverse_common(void);

extern z80_bit menu_writing_inverse_color;
extern int menu_dibuja_menu_permite_repeticiones_hotk;

extern z80_bit menu_symshift;
extern z80_bit menu_capshift;
extern z80_bit menu_backspace;
extern z80_bit menu_tab;


extern int menu_footer;

extern void enable_footer(void);
extern void disable_footer(void);
extern void menu_init_footer(void);
extern void menu_footer_z88(void);

extern int mouse_is_dragging;
extern int menu_mouse_left_double_click_counter;

struct s_overlay_screen {
	z80_byte tinta,papel,parpadeo;
	z80_byte caracter;
};

struct s_generic_message_tooltip_return {
	char texto_seleccionado[40];
	int linea_seleccionada;
	int estado_retorno; //Retorna 1 si sale con enter. Retorna 0 si sale con ESC
};

typedef struct s_generic_message_tooltip_return generic_message_tooltip_return;

#define OVERLAY_SCREEN_MAX_WIDTH 256
#define OVERLAY_SCREEN_MAX_HEIGTH 128

//Tamanyo inicial maximo. Aunque luego se puede hacer mas grande
#define ZXVISION_MAX_ANCHO_VENTANA 32
#define ZXVISION_MAX_ALTO_VENTANA 24


#define ZXVISION_MAX_X_VENTANA (scr_get_menu_width()-1)
#define ZXVISION_MAX_Y_VENTANA (scr_get_menu_height()-1)

//#define ZXVISION_MAX_X_VENTANA 31
//#define ZXVISION_MAX_Y_VENTANA 23


typedef struct s_overlay_screen overlay_screen;

//Nuevas ventanas zxvision
struct s_zxvision_window {
	overlay_screen *memory;
	int visible_width,visible_height;
	int x,y;

	int upper_margin;
	int lower_margin;

	//char *text_margin[20]; //Hasta 20 lineas de texto que se usan como texto que no se mueve. La ultima finaliza con 0

	int offset_x,offset_y;

	int total_width,total_height;
	char window_title[256];

	int can_be_resized;
	int is_minimized;

	int height_before_minimize;
	int width_before_minimize;
	int x_before_minimize;
	int y_before_minimize;


	int can_use_all_width; //Si tenemos usable también la ultima columna derecha

	//Posicion del cursor y si esta visible
	int visible_cursor;
	int cursor_line;

	//Ventana anterior. Se van poniendo una encima de otra
	struct s_zxvision_window *previous_window;

	//Ventana siguiente.
	struct s_zxvision_window *next_window;


	//Puntero a funcion de overlay
	void (*overlay_function) (void);
};

typedef struct s_zxvision_window zxvision_window;

extern void zxvision_new_window(zxvision_window *w,int x,int y,int visible_width,int visible_height,int total_width,int total_height,char *title);
extern void zxvision_destroy_window(zxvision_window *w);
extern void zxvision_draw_window(zxvision_window *w);
extern void zxvision_print_char(zxvision_window *w,int x,int y,overlay_screen *caracter);
extern void zxvision_print_char_simple(zxvision_window *w,int x,int y,int tinta,int papel,int parpadeo,z80_byte caracter);
extern void zxvision_draw_window_contents(zxvision_window *w);
extern void zxvision_draw_window_contents_no_speech(zxvision_window *ventana);
extern void zxvision_wait_until_esc(zxvision_window *w);

extern void zxvision_set_offset_x(zxvision_window *w,int offset_x);
extern void zxvision_set_offset_y(zxvision_window *w,int offset_y);
extern void zxvision_set_offset_y_visible(zxvision_window *w,int y);
extern void zxvision_set_x_position(zxvision_window *w,int x);
extern void zxvision_set_y_position(zxvision_window *w,int y);
extern void zxvision_set_visible_width(zxvision_window *w,int visible_width);
extern void zxvision_set_visible_height(zxvision_window *w,int visible_height);
extern void zxvision_print_string(zxvision_window *w,int x,int y,int tinta,int papel,int parpadeo,char *texto);
extern void zxvision_print_string_defaults(zxvision_window *w,int x,int y,char *texto);
extern void zxvision_print_string_defaults_fillspc(zxvision_window *w,int x,int y,char *texto);
extern void zxvision_handle_mouse_events(zxvision_window *w);
extern void zxvision_generic_message_tooltip(char *titulo, int return_after_print_text, int volver_timeout, int tooltip_enabled, int mostrar_cursor, generic_message_tooltip_return *retorno, int resizable, const char * texto_format , ...);

extern void zxvision_send_scroll_up(zxvision_window *w);
extern void zxvision_send_scroll_down(zxvision_window *w);
extern void zxvision_send_scroll_left(zxvision_window *w);
extern void zxvision_send_scroll_right(zxvision_window *w);

extern void zxvision_draw_below_windows(zxvision_window *w);
extern void zxvision_draw_below_windows_with_overlay(zxvision_window *w);

extern void zxvision_set_not_resizable(zxvision_window *w);
extern void zxvision_set_resizable(zxvision_window *w);

extern void zxvision_putpixel(zxvision_window *w,int x,int y,int color);
extern z80_byte zxvision_read_keyboard(void);
void zxvision_handle_cursors_pgupdn(zxvision_window *ventana,z80_byte tecla);
extern z80_byte zxvision_common_getkey_refresh(void);
extern z80_byte zxvision_common_getkey_refresh_noesperatecla(void);

extern zxvision_window *zxvision_current_window;

extern int zxvision_keys_event_not_send_to_machine;
//extern void zxvision_espera_tecla_timeout_window_splash(void);
extern void zxvision_espera_tecla_timeout_window_splash(int tipo);
extern int zxvision_key_not_sent_emulated_mach(void);
extern void menu_linea_zxvision(zxvision_window *ventana,int x,int y1,int y2,int color);
extern void zxvision_fill_width_spaces(zxvision_window *w,int y);

struct s_first_aid_list
{
	//enum first_aid_number_list indice_setting; //numero
	char config_name[100]; //nombre en la config
	int *puntero_setting;
	char *texto_opcion;
	int si_startup; //Si mensaje puede aparecer en startup del emulador
};

#define MAX_MENU_FIRST_AID 100

extern struct s_first_aid_list first_aid_list[];

extern int menu_first_aid_startup;
extern z80_bit menu_disable_first_aid;
extern void menu_first_aid_disable(char *texto);
extern int total_first_aid;
extern int menu_first_aid(char *key_setting);
extern void menu_first_aid_restore_all(void);
extern void menu_first_aid_init(void);
extern void menu_first_aid_random_startup(void);
extern int menu_first_aid_title(char *key_setting,char *title);

#define MAX_F_FUNCTIONS 20

enum defined_f_function_ids {
	//reset, hard-reset, nmi, open menu, ocr, smartload, osd keyboard, exitemulator.
	F_FUNCION_DEFAULT,   //1
	F_FUNCION_NOTHING,
	F_FUNCION_RESET,
	F_FUNCION_HARDRESET,
	F_FUNCION_NMI,  //5
	F_FUNCION_OPENMENU,
	F_FUNCION_OCR,
	F_FUNCION_SMARTLOAD,
	F_FUNCION_QUICKSAVE,
	F_FUNCION_LOADBINARY, //10
	F_FUNCION_SAVEBINARY,
	F_FUNCION_OSDKEYBOARD,
	F_FUNCION_OSDTEXTKEYBOARD,
    F_FUNCION_SWITCHBORDER,
	F_FUNCION_SWITCHFULLSCREEN,
	F_FUNCION_RELOADMMC, //16
	F_FUNCION_REINSERTTAPE, 
	F_FUNCION_DEBUGCPU,   
	F_FUNCION_PAUSE,      
 	F_FUNCION_EXITEMULATOR //20
};

//Define teclas F que se pueden mapear a acciones
struct s_defined_f_function {
	char texto_funcion[20];
	enum defined_f_function_ids id_funcion;
};

typedef struct s_defined_f_function defined_f_function;


extern defined_f_function defined_f_functions_array[];


#define MAX_F_FUNCTIONS_KEYS 15

//Array de teclas F mapeadas
extern enum defined_f_function_ids defined_f_functions_keys_array[];

extern int menu_define_key_function(int tecla,char *funcion);


extern overlay_screen overlay_screen_array[];
//extern overlay_screen second_overlay_screen_array[];

//definiciones para funcion menu_generic_message
#define MAX_LINEAS_VENTANA_GENERIC_MESSAGE 20

//#define MAX_LINEAS_TOTAL_GENERIC_MESSAGE 1000

//archivo LICENSE ocupa 1519 lineas ya parseado
#define MAX_LINEAS_TOTAL_GENERIC_MESSAGE 2000

#define MAX_ANCHO_LINEAS_GENERIC_MESSAGE 32
#define MAX_TEXTO_GENERIC_MESSAGE (MAX_LINEAS_TOTAL_GENERIC_MESSAGE*MAX_ANCHO_LINEAS_GENERIC_MESSAGE)

extern int menu_generic_message_aux_wordwrap(char *texto,int inicio, int final);
extern void menu_generic_message_aux_copia(char *origen,char *destino, int longitud);
extern int menu_generic_message_aux_filter(char *texto,int inicio, int final);

//Posiciones de texto mostrado en second overlay
#define WINDOW_FOOTER_ELEMENT_X_JOYSTICK 0
#define WINDOW_FOOTER_ELEMENT_X_FPS 0
#define WINDOW_FOOTER_ELEMENT_X_PRINTING 10
#define WINDOW_FOOTER_ELEMENT_X_FLASH 10
#define WINDOW_FOOTER_ELEMENT_X_MDFLP 10
#define WINDOW_FOOTER_ELEMENT_X_GENERICTEXT 10
#define WINDOW_FOOTER_ELEMENT_X_MMC 11
#define WINDOW_FOOTER_ELEMENT_X_IDE 11
#define WINDOW_FOOTER_ELEMENT_X_ESX 11
#define WINDOW_FOOTER_ELEMENT_X_DISK 11
#define WINDOW_FOOTER_ELEMENT_X_TEXT_FILTER 10
#define WINDOW_FOOTER_ELEMENT_X_ZXPAND 10
#define WINDOW_FOOTER_ELEMENT_X_TAPE 11
#define WINDOW_FOOTER_ELEMENT_X_FLAP 11
#define WINDOW_FOOTER_ELEMENT_X_CPUSTEP 11
#define WINDOW_FOOTER_ELEMENT_X_CPU_TEMP 16
#define WINDOW_FOOTER_ELEMENT_X_CPU_USE 7
//#define WINDOW_FOOTER_ELEMENT_X_BATERIA 30

//#define WINDOW_FOOTER_ELEMENT_X_ACTIVITY 24

#define WINDOW_FOOTER_ELEMENT_Y_CPU_USE 1
#define WINDOW_FOOTER_ELEMENT_Y_CPU_TEMP 1
#define WINDOW_FOOTER_ELEMENT_Y_FPS 1

#define WINDOW_FOOTER_ELEMENT_Y_F5MENU 2
#define WINDOW_FOOTER_ELEMENT_Y_ZESARUX_EMULATOR 2

/*

Como quedan los textos:

01234567890123456789012345678901
50 FPS 100% CPU 29.3C   -SPEECH- 

           -TAPE-
          -FLASH-
          -PRINT-
	   MMC
          -ZXPAND-
*/


#define MENU_OPCION_SEPARADOR 0
#define MENU_OPCION_NORMAL 1
#define MENU_OPCION_ESC 2


//item de menu con memoria asignada pero item vacio
//si se va a agregar un item, se agrega en el primero con este tipo de opcion
#define MENU_OPCION_UNASSIGNED 254




#define MENU_RETORNO_NORMAL 0
#define MENU_RETORNO_ESC -1
#define MENU_RETORNO_F1 -2
#define MENU_RETORNO_F2 -3
#define MENU_RETORNO_F10 -4

extern void menu_footer_activity(char *texto);
extern void menu_delete_footer_activity(void);

//funcion a la que salta al darle al enter. valor_opcion es un valor que quien crea el menu puede haber establecido,
//para cada item de menu, un valor diferente
//al darle enter se envia el valor de ese item seleccionado a la opcion de menu
typedef void (*t_menu_funcion)(MENU_ITEM_PARAMETERS);

//funcion que retorna 1 o 0 segun si opcion activa
typedef int (*t_menu_funcion_activo)(void);



//Aunque en driver xwindows no cabe mas de 30 caracteres, en stdout, por ejemplo, cabe mucho mas.
#define MAX_TEXTO_OPCION 60

struct s_menu_item {
	//texto de la opcion
	//char *texto;

	//Aunque en driver xwindows no cabe mas de 30 caracteres, en stdout, por ejemplo, cabe mucho mas
	char texto_opcion[MAX_TEXTO_OPCION];

	//texto de ayuda
	char *texto_ayuda;

	//texto de tooltip
	char *texto_tooltip;

	//atajo de teclado
	z80_byte atajo_tecla;

	//un valor enviado a la opcion, que puede establecer la funcion que agrega el item
	int valor_opcion;

	//Para tipos de menus "tabulados", aquellos en que:
	//-no se crea ventana al abrir
	//-las opciones tienen coordenadas X e Y relativas a la ventana activa
	//-se puede mover tambien usando teclas izquierda, derecha
	//-el texto de las opciones no se rellena con espacios por la derecha, se muestra tal cual en las coordenadas X e Y indicadas

	int es_menu_tabulado;
	int menu_tabulado_x;
	int menu_tabulado_y;

	//tipo de la opcion
	int tipo_opcion;
	//funcion a la que debe saltar
	t_menu_funcion menu_funcion;
	//funcion que retorna 1 o 0 segun si opcion activa
	t_menu_funcion_activo menu_funcion_activo;

	//funcion a la que debe saltar al pulsar espacio
	t_menu_funcion menu_funcion_espacio;

	//siguiente item
	struct s_menu_item *next;
};

typedef struct s_menu_item menu_item;



extern void menu_ventana_scanf(char *titulo,char *texto,int max_length);
extern int menu_filesel(char *titulo,char *filtros[],char *archivo);
extern int zxvision_menu_filesel(char *titulo,char *filtros[],char *archivo);

extern void menu_add_item_menu_inicial(menu_item **m,char *texto,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo);
extern void menu_add_item_menu_inicial_format(menu_item **p,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo,const char * format , ...);
extern void menu_add_item_menu(menu_item *m,char *texto,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo);
extern void menu_add_item_menu_format(menu_item *m,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo,const char * format , ...);
extern void menu_add_item_menu_ayuda(menu_item *m,char *texto_ayuda);
extern void menu_add_item_menu_tooltip(menu_item *m,char *texto_tooltip);
extern void menu_add_item_menu_shortcut(menu_item *m,z80_byte tecla);
extern void menu_add_item_menu_valor_opcion(menu_item *m,int valor_opcion);
extern void menu_add_item_menu_tabulado(menu_item *m,int x,int y);


extern void menu_warn_message(char *texto);
extern void menu_error_message(char *texto);

extern void menu_generic_message(char *titulo, const char * texto);
extern void menu_generic_message_format(char *titulo, const char * format , ...);
extern void menu_generic_message_splash(char *titulo, const char * texto);
extern void menu_generic_message_warn(char *titulo, const char * texto);




extern void menu_generic_message_tooltip(char *titulo, int volver_timeout, int tooltip_enabled, int mostrar_cursor, generic_message_tooltip_return *retorno, const char * texto_format , ...);

#define TOOLTIP_SECONDS 4
#define WINDOW_SPLASH_SECONDS 3

extern void menu_add_ESC_item(menu_item *array_menu_item);
extern int menu_dibuja_menu(int *opcion_inicial,menu_item *item_seleccionado,menu_item *m,char *titulo);
extern void menu_tape_settings_trunc_name(char *orig,char *dest,int max);

extern int menu_confirm_yesno(char *texto_ventana);
extern int menu_confirm_yesno_texto(char *texto_ventana,char *texto_interior);
extern int menu_ask_no_append_truncate_texto(char *texto_ventana,char *texto_interior);

extern void menu_refresca_pantalla(void);

extern int menu_debug_sprites_total_colors_mapped_palette(int paleta);
extern int menu_debug_sprites_return_index_palette(int paleta, z80_byte color);
extern int menu_debug_sprites_return_color_palette(int paleta, z80_byte color);
extern int menu_debug_sprites_max_value_mapped_palette(int paleta);
extern void menu_dibuja_rectangulo_relleno(zxvision_window *w,int x, int y, int ancho, int alto, int color);
extern void menu_debug_sprites_get_palette_name(int paleta, char *s);

extern int menu_debug_get_total_digits_dec(int valor);

extern void menu_debug_dissassemble_una_instruccion(char *dumpassembler,menu_z80_moto_int dir,int *longitud_final_opcode);
extern void menu_debug_dissassemble_una_inst_sino_hexa(char *dumpassembler,menu_z80_moto_int dir,int *longitud_final_opcode,int sino_hexa,int full_hexa_dump_motorola);
extern menu_z80_moto_int menu_debug_disassemble_last_ptr;

extern menu_z80_moto_int menu_debug_disassemble_subir(menu_z80_moto_int dir_inicial);
extern menu_z80_moto_int menu_debug_disassemble_bajar(menu_z80_moto_int dir_inicial);
extern void menu_debug_registers_show_scan_position(void);

//Fin funciones basicas que se suelen usar desde menu_items.c






extern void putchar_menu_overlay(int x,int y,z80_byte caracter,z80_byte tinta,z80_byte papel);
extern void putchar_menu_overlay_parpadeo(int x,int y,z80_byte caracter,z80_byte tinta,z80_byte papel,z80_byte parpadeo);
//extern void putchar_menu_second_overlay(int x,int y,z80_byte caracter,z80_byte tinta,z80_byte papel);
extern void new_menu_putchar_footer(int x,int y,z80_byte caracter,z80_byte tinta,z80_byte papel);
extern void menu_putstring_footer(int x,int y,char *texto,z80_byte tinta,z80_byte papel);
extern void cls_menu_overlay(void);
extern int menu_multitarea;
extern int menu_abierto;

extern z80_bit menu_hide_vertical_percentaje_bar;
extern z80_bit menu_hide_minimize_button;
extern z80_bit menu_hide_close_button;
extern z80_bit menu_invert_mouse_scroll;

extern int if_pending_error_message;
extern char pending_error_message[];

extern void menu_footer_bottom_line(void);


extern void menu_inicio(void);

extern void set_splash_text(void);
extern void reset_splash_text(void);
extern z80_bit menu_splash_text_active;
extern int menu_splash_segundos;
extern z80_byte menu_da_todas_teclas(void);
extern void menu_espera_tecla_o_joystick(void);
extern void menu_espera_no_tecla(void);
extern void menu_get_dir(char *ruta,char *directorio);

extern int menu_tooltip_counter;

extern int menu_window_splash_counter;
extern int menu_window_splash_counter_ms;

extern z80_bit tooltip_enabled;

extern z80_bit mouse_menu_disabled;

extern char *quickfile;
extern char quickload_file[];

extern z80_bit menu_button_quickload;
extern z80_bit menu_button_osdkeyboard;
extern z80_bit menu_button_osdkeyboard_return;
extern z80_bit menu_button_osd_adv_keyboard_return;
extern z80_bit menu_button_osd_adv_keyboard_openmenu;
extern z80_bit menu_button_exit_emulator;
extern z80_bit menu_event_drag_drop;
extern z80_bit menu_event_new_version_show_changes;
//extern char menu_event_drag_drop_file[PATH_MAX];
extern z80_bit menu_event_remote_protocol_enterstep;
extern z80_bit menu_button_f_function;
extern int menu_button_f_function_index;

//numero maximo de entradas 
#define MAX_OSD_ADV_KEYB_WORDS 1000
//longitud maximo de cada entrada
#define MAX_OSD_ADV_KEYB_TEXT_LENGTH 31

extern int osd_adv_kbd_defined;
extern char osd_adv_kbd_list[MAX_OSD_ADV_KEYB_WORDS][MAX_OSD_ADV_KEYB_TEXT_LENGTH];




extern int menu_contador_teclas_repeticion;
extern int menu_segundo_contador_teclas_repeticion;

extern int menu_speech_tecla_pulsada;

extern char binary_file_load[];
extern char binary_file_save[];

extern int menu_ask_file_to_save(char *titulo_ventana,char *filtro,char *file_save);

extern char menu_buffer_textspeech_filter_program[];

extern char menu_buffer_textspeech_stop_filter_program[];

extern void screen_print_splash_text(int y,z80_byte tinta,z80_byte papel,char *texto);
extern void screen_print_splash_text_center(z80_byte tinta,z80_byte papel,char *texto);

extern char menu_realtape_name[];

extern z80_bit menu_force_writing_inverse_color;

extern void menu_filesel_chdir(char *dir);

extern z80_bit force_confirm_yes;

extern void draw_middle_footer(void);

extern z80_int menu_mouse_frame_counter;

struct s_estilos_gui {
        char nombre_estilo[20];
        int papel_normal;
        int tinta_normal;

        int muestra_cursor; //si se muestra cursor > en seleccion de opcion
                                        //Esto es asi en ZXSpectr
                                        //el cursor entonces se mostrara con los colores indicados a continuacion

        int muestra_recuadro; //si se muestra recuadro en ventana

        int muestra_rainbow;  //si se muestra rainbow en titulo

        int solo_mayusculas; //para ZX80/81


        int papel_seleccionado;
        int tinta_seleccionado;

        int papel_no_disponible; //Colores cuando una opción no esta disponible
        int tinta_no_disponible;

        int papel_seleccionado_no_disponible;    //Colores cuando una opcion esta seleccionada pero no disponible
        int tinta_seleccionado_no_disponible;

        int papel_titulo, tinta_titulo;

        int color_waveform;  //Color para forma de onda en view waveform
        int color_unused_visualmem; //Color para zona no usada en visualmem


	int papel_opcion_marcada; //Color para opcion marcada, de momento solo usado en osd keyboard
	int tinta_opcion_marcada; 

	char boton_cerrar; //caracter de cerrado de ventana

	int color_aviso; //caracter de aviso de volumen alto, cpu alto, etc. normalmente rojo
};

typedef struct s_estilos_gui estilos_gui;


#define ESTILOS_GUI 10

#define ESTILO_GUI_QL 7
#define ESTILO_GUI_MANSOFTWARE 6
#define ESTILO_GUI_SAM 5
#define ESTILO_GUI_CPC 4
#define ESTILO_GUI_Z88 3

extern void estilo_gui_retorna_nombres(void);

extern int estilo_gui_activo;

extern estilos_gui definiciones_estilos_gui[];

extern void set_charset(void);

extern void menu_draw_ext_desktop(void);

extern int menu_ext_desktop_fill;
extern int menu_ext_desktop_fill_solid_color;


#define ESTILO_GUI_PAPEL_NORMAL (definiciones_estilos_gui[estilo_gui_activo].papel_normal)
#define ESTILO_GUI_TINTA_NORMAL (definiciones_estilos_gui[estilo_gui_activo].tinta_normal)
#define ESTILO_GUI_PAPEL_SELECCIONADO (definiciones_estilos_gui[estilo_gui_activo].papel_seleccionado)
#define ESTILO_GUI_TINTA_SELECCIONADO (definiciones_estilos_gui[estilo_gui_activo].tinta_seleccionado)


#define ESTILO_GUI_PAPEL_NO_DISPONIBLE (definiciones_estilos_gui[estilo_gui_activo].papel_no_disponible)
#define ESTILO_GUI_TINTA_NO_DISPONIBLE (definiciones_estilos_gui[estilo_gui_activo].tinta_no_disponible)
#define ESTILO_GUI_PAPEL_SEL_NO_DISPONIBLE (definiciones_estilos_gui[estilo_gui_activo].papel_seleccionado_no_disponible)
#define ESTILO_GUI_TINTA_SEL_NO_DISPONIBLE (definiciones_estilos_gui[estilo_gui_activo].tinta_seleccionado_no_disponible)

#define ESTILO_GUI_PAPEL_OPCION_MARCADA (definiciones_estilos_gui[estilo_gui_activo].papel_opcion_marcada)
#define ESTILO_GUI_TINTA_OPCION_MARCADA (definiciones_estilos_gui[estilo_gui_activo].tinta_opcion_marcada)

#define ESTILO_GUI_PAPEL_TITULO (definiciones_estilos_gui[estilo_gui_activo].papel_titulo)
#define ESTILO_GUI_TINTA_TITULO (definiciones_estilos_gui[estilo_gui_activo].tinta_titulo)

#define ESTILO_GUI_COLOR_WAVEFORM (definiciones_estilos_gui[estilo_gui_activo].color_waveform)
#define ESTILO_GUI_COLOR_UNUSED_VISUALMEM (definiciones_estilos_gui[estilo_gui_activo].color_unused_visualmem)

#define ESTILO_GUI_MUESTRA_CURSOR (definiciones_estilos_gui[estilo_gui_activo].muestra_cursor)
#define ESTILO_GUI_MUESTRA_RECUADRO (definiciones_estilos_gui[estilo_gui_activo].muestra_recuadro)
#define ESTILO_GUI_MUESTRA_RAINBOW (definiciones_estilos_gui[estilo_gui_activo].muestra_rainbow)
#define ESTILO_GUI_SOLO_MAYUSCULAS (definiciones_estilos_gui[estilo_gui_activo].solo_mayusculas)

#define ESTILO_GUI_BOTON_CERRAR (definiciones_estilos_gui[estilo_gui_activo].boton_cerrar)

#define ESTILO_GUI_COLOR_AVISO (definiciones_estilos_gui[estilo_gui_activo].color_aviso)


#define MENU_ANCHO_FRANJAS_TITULO 5

//extern z80_bit menu_espera_tecla_no_cpu_loop_flag_salir;
extern int salir_todos_menus;

extern int si_valid_char(z80_byte caracter);

extern z80_bit menu_event_open_menu;

extern int menu_debug_memory_zone;

extern menu_z80_moto_int menu_debug_memory_zone_size;

extern int menu_debug_show_memory_zones;

#define MAX_LENGTH_ADDRESS_MEMORY_ZONE 6

extern int menu_get_current_memory_zone_name_number(char *s);

extern void menu_debug_set_memory_zone_attr(void);

extern z80_byte menu_debug_get_mapped_byte(int direccion);

extern void menu_debug_print_address_memory_zone(char *texto, menu_z80_moto_int address);

extern void menu_debug_write_mapped_byte(int direccion,z80_byte valor);

extern menu_z80_moto_int adjust_address_memory_size(menu_z80_moto_int direccion);

extern int menu_debug_get_total_digits_hexa(int valor);

extern int menu_debug_hexdump_with_ascii_modo_ascii;

extern menu_z80_moto_int menu_debug_hexdump_adjusta_en_negativo(menu_z80_moto_int dir,int linesize);

extern int menu_debug_hexdump_change_pointer(int p);

extern void menu_debug_change_memory_zone(void);

extern void menu_chdir_sharedfiles(void);

extern void menu_debug_registers_dump_hex(char *texto,menu_z80_moto_int direccion,int longitud);

extern void menu_debug_registers_dump_ascii(char *texto,menu_z80_moto_int direccion,int longitud,int modoascii,z80_byte valor_xor);

extern int menu_gui_zoom;

extern int menu_escribe_linea_startx;

extern z80_bit menu_disable_special_chars;

extern void menu_onscreen_keyboard(MENU_ITEM_PARAMETERS);

extern int timer_osd_keyboard_menu;

extern char snapshot_load_file[];

extern int menu_char_width;

extern int overlay_usado_screen_array[];
#define ZESARUX_ASCII_LOGO_ANCHO 26
#define ZESARUX_ASCII_LOGO_ALTO 26
extern char *zesarux_ascii_logo[ZESARUX_ASCII_LOGO_ALTO];

extern int return_color_zesarux_ascii(char c);

extern z80_bit menu_filesel_posicionar_archivo;
extern char menu_filesel_posicionar_archivo_nombre[];

extern z80_bit menu_limit_menu_open;

extern z80_bit menu_filesel_hide_dirs;

extern int osd_kb_no_mostrar_desde_menu;

extern void menu_fire_event_open_menu(void);

extern int menu_button_f_function_action;

//extern z80_bit screen_bw_no_multitask_menu;

extern int menu_hardware_autofire_cond(void);

extern void menu_file_mmc_browser_show_file(z80_byte *origen,char *destino,int sipuntoextension,int longitud);

extern void menu_dsk_getoff_block(z80_byte *dsk_file_memory,int longitud_dsk,int bloque,int *offset1,int *offset2);

extern int menu_dsk_get_start_filesystem(z80_byte *dsk_file_memory,int longitud_dsk);

extern int menu_dsk_getoff_track_sector(z80_byte *dsk_memoria,int total_pistas,int pista_buscar,int sector_buscar);

extern int menu_decae_ajusta_valor_volumen(int valor_decae,int valor_volumen);

extern int menu_decae_dec_valor_volumen(int valor_decae,int valor_volumen);

extern void menu_reset_counters_tecla_repeticion(void);

extern void menu_debug_cpu_stats_diss_complete_no_print (z80_byte opcode,char *buffer,z80_byte preffix1,z80_byte preffix2);

extern void menu_string_volumen(char *texto,z80_byte registro_volumen,int indice_decae);

extern void menu_copy_clipboard(char *texto);

#define MAX_LAST_FILESUSED 18

//#define ZXVISION_MAX_WINDOW_WIDTH 32
//#define ZXVISION_MAX_WINDOW_HEIGHT 24


extern void last_filesused_clear(void);
extern void last_filesused_insert(char *s);
extern char last_files_used_array[MAX_LAST_FILESUSED][PATH_MAX];
extern void lastfilesuser_scrolldown(int posicion_up,int posicion_down);

extern void menu_debug_daad_init_flagobject(void);
extern void menu_draw_last_fps(void);
extern void menu_draw_cpu_use_last(void);

extern void putchar_footer_array(int x,int y,z80_byte caracter,z80_byte tinta,z80_byte papel,z80_byte parpadeo);
extern void redraw_footer(void);
extern void cls_footer(void);

extern int menu_center_x(void);
extern int menu_origin_x(void);
extern int menu_center_y(void);


//"[VARIABLE][VOP][CONDITION][VALUE] [OPERATOR] [VARIABLE][VOP][CONDITION][VALUE] [OPERATOR] .... where: \n" 


#define HELP_MESSAGE_CONDITION_BREAKPOINT \
"A condition breakpoint has the following format: \n" \
"[EXPRESSION][CONDITION][EXPRESSION]  [OPERATOR]  [EXPRESSION][CONDITION][EXPRESSION]  [OPERATOR] ... where: \n" \
"[EXPRESSION] can be a COMPLEXVARIABLE or a VALUE  \n" \
"[COMPLEXVARIABLE] is formed by [VARIABLE][VOP] \n" \
"[VARIABLE] can be a CPU register or some pseudo variables: A,B,C,D,E,F,H,L,AF,BC,DE,HL,A',B',C',D',E',F',H',L',AF',BC',DE',HL',I,R,SP,PC,IX,IY\n" \
"FS,FZ,FP,FV,FH,FN,FC: Flags\n" \
"(BC),(DE),(HL),(SP),(PC),(IX),(IY), (NN), IFF1, IFF2, OPCODE,\n" \
"RAM: RAM mapped on 49152-65535 on Spectrum 128 or Prism,\n" \
"ROM: ROM mapped on 0-16383 on Spectrum 128,\n" \
"SEG0, SEG1, SEG2, SEG3: memory banks mapped on each 4 memory segments on Z88\n" \
"MRV: value returned on read memory operation\n" \
"MWV: value written on write memory operation\n" \
"MRA: address used on read memory operation\n" \
"MWA: address used on write memory operation\n" \
"PRV: value returned on read port operation\n" \
"PWV: value written on write port operation\n" \
"PRA: address used on read port operation\n" \
"PWA: address used on write port operation\n" \
"\n" \
"OUTFIRED: returns 1 if last Z80 opcode was an OUT operation\n" \
"INFIRED: returns 1 if last Z80 opcode was an IN operation\n" \
"INTFIRED: returns 1 when an interrupt has been generated\n" \
"ENTERROM: returns 1 the first time PC register is on ROM space (0-16383)\n" \
"EXITROM: returns 1 the first time PC register is out ROM space (16384-65535)\n" \
"Note: The last two only return 1 the first time the breakpoint is fired, or a watch is shown, " \
"it will return 1 again only exiting required space address and entering again\n" \
"\n" \
"TSTATES: t-states total in a frame\n" \
"TSTATESL: t-states in a scanline\n" \
"TSTATESP: t-states partial\n" \
"SCANLINE: scanline counter\n" \
"\n" \
"[VOP] is optional is made of a Variable Operator and Variable Value, joined together with no space. Variable Operator can be:\n" \
"& : bitwise AND\n" \
"| : bitwise OR\n" \
"^ : bitwise XOR\n" \
"Variable Value is any value you want to apply with the operator\n" \
"Examples of [VOP]: \n" \
"|3 : Makes a bitwise OR with 3 to the Variable value\n" \
"&FH : Makes a bitwise AND with FH to the Variable value\n" \
"\n" \
"[CONDITION] must be one of: <,>,=,/  (/ means not equal)\n" \
"[VALUE] must be a numeric value\n" \
"[OPERATOR] must be one of the following: and, or, xor\n" \
"\n" \
"Examples of conditions:\n" \
"SP<32768 : it will match when SP register is below 32768\n" \
"PWA&FFH=FEH : it will match when last port write address, doing an AND bitwise (&) with FFH, is equal to FEH\n" \
"A|1=255 : it will match when register A, doing OR bitwise (|), it equal to 255\n" \
"(32768)&0FH=3 : it will match when memory address 32768 has the low 4 bits set to value 3\n" \
"OUTFIRED=1 AND PWA&00FFH=FEH AND PWV&7=1 : it will match when changing border color to blue\n" \
"HL=DE : it will mach when HL is equal to DE register\n" \
"32768>PC : it will match when PC<32768\n" \
"1=1 : it will match when 1=1, so always ;) \n" \
"FS=1: it will match when flag S is set\n" \
"A=10 and BC<33 : it will match when A register is 10 and BC is below 33\n" \
"OPCODE=ED4AH : it will match when running opcode ADC HL,BC\n" \
"OPCODE=21H : it will match when running opcode LD HL,NN\n" \
"OPCODE=210040H : it will match when running opcode LD HL,4000H\n" \
"SEG2=40H : when memory bank 40H is mapped to memory segment 2 (49152-65535 range) on Z88\n" \
"MWA<16384 : it will match when attempting to write in ROM\n" \
"ENTERROM=1 : it will match when entering ROM space address\n" \
"TSTATESP>69888 : it will match when partial counter has executed a 48k full video frame (you should reset it before)\n" \
"\nNote 1: Any condition in the whole list can trigger a breakpoint" \
"\n\nNote 2: When writing values, is faster for the breakpoint parser if the number starts with a digit and uses less cpu, " \
"so, having this breakpoint: \n" \
"DE=3FFFH \n" \
"is faster than\n" \
"DE=FFFFH \n" \
"So I recommend you to add a 0 at the beginning of the number, if it does not start with a digit. So using the same example, this expression will be faster: \n" \
"DE=0FFFFH \n" \
"The technical explanation for that is, if the parser sees a starting digit, it doesn't have to try to find a match on every register/variable\n" \
"\nNote 3: Breakpoint types PC=XXXX, MWA=XXXX and MRA=XXXX are a lot faster than the rest, because they use a breakpoint optimizer"


#define HELP_MESSAGE_BREAKPOINT_ACTION \
"Action can be one of the following: \n" \
"menu or break or empty string: Breaks current execution of program\n" \
"call address: Calls memory address\n" \
"printc c: Print character c to console\n" \
"printe expression: Print expression following the same syntax as watches and evaluate condition\n" \
"prints string: Prints string to console\n" \
"quicksave: Saves a quick snapshot\n" \
"set-register string: Sets register indicated on string. Example: set-register PC=32768\n" \
"write address value: Write memory address with indicated value\n" \

#endif
