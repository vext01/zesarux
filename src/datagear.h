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

#ifndef DATAGEAR_H
#define DATAGEAR_H

#include "cpu.h"

#define DATAGEAR_DMA_FIRST_PORT 0x0b
#define DATAGEAR_DMA_SECOND_PORT 0x6b

extern void datagear_reset(void);
extern void datagear_write_value(z80_byte value);

extern z80_bit datagear_dma_emulation;

extern void datagear_dma_disable(void);
extern void datagear_dma_enable(void);

#endif
