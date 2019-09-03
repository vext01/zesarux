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
#include <sys/time.h>

#include "stats.h"

/*
	                gettimeofday(&z80_interrupts_timer_antes, NULL);
        	        randomize_noise[i]=z80_interrupts_timer_antes.tv_sec & 0xFFFF;
                	//printf ("randomize vale: %d\n",randomize_noise);
					*/

char stats_uuid[128];

void generate_stats_uuid(void)
{
	struct timeval fecha;

	gettimeofday(&fecha, NULL);


	int secs=fecha.tv_sec;
	int microsecs=fecha.tv_usec;

	printf ("secs %d microsecs %d\n",secs,microsecs);
	//tv_usec

	//El uuid del usuario consta de los segundos.microsegundos cuando se genera

	sprintf(stats_uuid,"%d.%d",secs,microsecs);
}