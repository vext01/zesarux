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
#include <dirent.h>

//#if defined(__APPLE__)
//        #include <sys/syslimits.h>
//#endif


#include "hilow.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "ula.h"


z80_bit hilow_enabled={0};


//8 KB ROM, 2 KB RAM
//RAM mapeada en modo mirror en 2000h, 2800h, 3000h, 3800h
z80_byte *hilow_memory_pointer;



int hilow_nested_id_poke_byte;
int hilow_nested_id_poke_byte_no_time;
int hilow_nested_id_peek_byte;
int hilow_nested_id_peek_byte_no_time;
int hilow_nested_id_core;

z80_bit hilow_mapped_rom={0};
z80_bit hilow_mapped_ram={0};


int hilow_check_if_rom_area(z80_int dir)
{
    if (dir<8192 && hilow_mapped_rom.v) {
			return 1;
                }
	return 0;
}

int hilow_check_if_ram_area(z80_int dir)
{
    if (dir>=8192 && dir<16384 && hilow_mapped_ram.v) {
			return 1;
                }
	return 0;
}

z80_byte hilow_read_rom_byte(z80_int dir)
{
	return hilow_memory_pointer[dir];
}


z80_byte hilow_read_ram_byte(z80_int dir)
{

	dir &= 2047; 


	//La RAM esta despues de los 8kb de rom
	return hilow_memory_pointer[8192+dir];
}

void hilow_poke_ram(z80_int dir,z80_byte value)
{

	if (hilow_check_if_ram_area(dir) ) {
		dir &= 2047; 
		//La RAM esta despues de los 8kb de rom
		hilow_memory_pointer[8192+dir]=value;	
	}

}


z80_byte hilow_poke_byte(z80_int dir,z80_byte valor)
{

    //Llamar a anterior
    debug_nested_poke_byte_call_previous(hilow_nested_id_poke_byte,dir,valor);

	hilow_poke_ram(dir,valor);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;


}

z80_byte hilow_poke_byte_no_time(z80_int dir,z80_byte valor)
{
 
	//Llamar a anterior
	debug_nested_poke_byte_no_time_call_previous(hilow_nested_id_poke_byte_no_time,dir,valor);


	hilow_poke_ram(dir,valor);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;


}

z80_byte hilow_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(hilow_nested_id_peek_byte,dir);


	if (hilow_check_if_rom_area(dir)) {
		return hilow_read_rom_byte(dir);
	}

	return valor_leido;
}

z80_byte hilow_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(hilow_nested_id_peek_byte_no_time,dir);


	if (hilow_check_if_rom_area(dir)) {
                return hilow_read_rom_byte(dir);
        }

	return valor_leido;
}

void hilow_automap_unmap_memory(z80_int dir)
{
	//Si hay que mapear/desmapear memorias

	//Puntos de mapeo rom
	//Si no estaba mapeada
	if (hilow_mapped_rom.v==0) {
		if (dir==0x04C2 || dir==0x0556 || dir==0x0976) {
			hilow_mapped_rom.v=1;
		}
	}

	//Puntos de desmapeo rom
	//Si estaba mapeada
	if (hilow_mapped_rom.v==1) {
		if (dir==0x0052) {
			hilow_mapped_rom.v=0;
		}
	}	


	//Mapeo de ram de momento identico que rom
	//TODO: si realmente es identico, meter este codigo arriba
	//Puntos de mapeo ram
	//Si no estaba mapeada
	if (hilow_mapped_ram.v==0) {
		if (dir==0x04C2 || dir==0x0556 || dir==0x0976) {
			hilow_mapped_ram.v=1;
		}
	}

	//Puntos de desmapeo ram
	//Si estaba mapeada
	if (hilow_mapped_ram.v==1) {
		if (dir==0x0052) {
			hilow_mapped_ram.v=0;
		}
	}	

}

z80_byte cpu_core_loop_spectrum_hilow(z80_int dir, z80_byte value GCC_UNUSED)
{

        //Llamar a anterior
        debug_nested_core_call_previous(hilow_nested_id_core);


		hilow_automap_unmap_memory(dir);



        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;

}



//Establecer rutinas propias
void hilow_set_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Setting hilow poke / peek functions");

	//Asignar mediante nuevas funciones de core anidados
	hilow_nested_id_poke_byte=debug_nested_poke_byte_add(hilow_poke_byte,"Hilow poke_byte");
	hilow_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(hilow_poke_byte_no_time,"Hilow poke_byte_no_time");
	hilow_nested_id_peek_byte=debug_nested_peek_byte_add(hilow_peek_byte,"Hilow peek_byte");
	hilow_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(hilow_peek_byte_no_time,"Hilow peek_byte_no_time");


	hilow_nested_id_core=debug_nested_core_add(cpu_core_loop_spectrum_hilow,"Hilow Spectrum core");


}

//Restaurar rutinas de hilow
void hilow_restore_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before hilow");


	debug_nested_poke_byte_del(hilow_nested_id_poke_byte);
	debug_nested_poke_byte_no_time_del(hilow_nested_id_poke_byte_no_time);
	debug_nested_peek_byte_del(hilow_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(hilow_nested_id_peek_byte_no_time);


	debug_nested_core_del(hilow_nested_id_core);
}



void hilow_alloc_memory(void)
{
        int size=HILOW_MEM_SIZE;  

        debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for hilow emulation",size/1024);

        hilow_memory_pointer=malloc(size);
        if (hilow_memory_pointer==NULL) {
                cpu_panic ("No enough memory for hilow emulation");
        }


}

int hilow_load_rom(void)
{

        FILE *ptr_hilow_romfile;
        int leidos=0;

        debug_printf (VERBOSE_INFO,"Loading hilow rom %s",HILOW_ROM_FILE_NAME);

  			ptr_hilow_romfile=fopen(HILOW_ROM_FILE_NAME,"rb");
                if (!ptr_hilow_romfile) {
                        debug_printf (VERBOSE_ERR,"Unable to open ROM file");
                }

        if (ptr_hilow_romfile!=NULL) {

                leidos=fread(hilow_memory_pointer,1,HILOW_ROM_SIZE,ptr_hilow_romfile);
                fclose(ptr_hilow_romfile);

        }



        if (leidos!=HILOW_ROM_SIZE || ptr_hilow_romfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error reading hilow rom");
                return 1;
        }

        return 0;
}



void hilow_enable(void)
{

  if (!MACHINE_IS_SPECTRUM) {
    debug_printf(VERBOSE_INFO,"Can not enable hilow on non Spectrum machine");
    return;
  }

	if (hilow_enabled.v) {
		debug_printf (VERBOSE_DEBUG,"Already enabled");
		return;
	}


	hilow_alloc_memory();
	if (hilow_load_rom()) return;

	hilow_set_peek_poke_functions();

	hilow_enabled.v=1;

	hilow_press_button();




}

void hilow_disable(void)
{
	if (hilow_enabled.v==0) return;

	hilow_restore_peek_poke_functions();

	free(hilow_memory_pointer);

	hilow_enabled.v=0;
}


void hilow_press_button(void)
{

        if (hilow_enabled.v==0) {
                debug_printf (VERBOSE_ERR,"Trying to press Hilow button when it is disabled");
                return;
        }

	hilow_mapped_rom.v=0;
	hilow_mapped_ram.v=0;

	reset_cpu();


}
