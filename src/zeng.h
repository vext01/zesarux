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

#ifndef ZENG_H
#define ZENG_H

#include "utils.h"

//Estructura para la FIFO de eventos de teclas

struct s_zeng_key_presses {
	enum util_teclas tecla;
	int pressrelease;
};

typedef struct s_zeng_key_presses zeng_key_presses;

//50 teclas en cola, que es una barbaridad
#define ZENG_FIFO_SIZE 50


extern int zeng_fifo_add_element(zeng_key_presses *elemento);

extern int zeng_fifo_read_element(zeng_key_presses *elemento);

extern z80_bit zeng_enabled;

#endif
