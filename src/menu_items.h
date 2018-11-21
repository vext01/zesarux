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

#ifndef MENU_ITEMS_H
#define MENU_ITEMS_H

#include "cpu.h"


extern void menu_poke(MENU_ITEM_PARAMETERS);
extern void menu_settings_debug(MENU_ITEM_PARAMETERS);
extern void menu_settings_audio(MENU_ITEM_PARAMETERS);
extern void menu_zxvision_test(MENU_ITEM_PARAMETERS);
extern void menu_mem_breakpoints(MENU_ITEM_PARAMETERS);
extern int menu_cond_ay_chip(void);
extern int last_debug_poke_dir;
extern void menu_debug_poke(MENU_ITEM_PARAMETERS);
extern void menu_debug_cpu_resumen_stats(MENU_ITEM_PARAMETERS);
extern void menu_about_core_statistics(MENU_ITEM_PARAMETERS);
extern void menu_ay_registers(MENU_ITEM_PARAMETERS);
extern void menu_debug_tsconf_tbblue_videoregisters(MENU_ITEM_PARAMETERS);
extern void menu_debug_tsconf_tbblue_spritenav(MENU_ITEM_PARAMETERS);
extern void menu_debug_tsconf_tbblue_tilenav(MENU_ITEM_PARAMETERS);
extern void menu_audio_new_waveform(MENU_ITEM_PARAMETERS);

#endif

