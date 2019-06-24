#include <stdio.h>
#include <string.h>


//Convierte valor entero en variable length. Se devuelve en el orden tal cual tiene que ir en destino
//Devuelve longitud en bytes
int util_int_variable_length(unsigned int valor,unsigned char *destino)
{

    //Controlar rango maximo
    if (valor>0x0FFFFFFF) valor=0x0FFFFFFF;

    //Inicializarlo a cero
    //destino[0]=destino[1]=destino[2]=destino[3]=0;
    int longitud=1;

    //Lo metemos temporalmente ahi
    unsigned int final=0;

    final=valor & 0x7F;

    do {
        valor=valor>>7;
        if (valor) {
            final=final<<8;
            final |=(valor & 0x7f);
            final |=128;
            longitud++;
        }
    } while (valor);

    //Y lo metemos en buffer destino
    int i;
    for (i=0;i<longitud;i++) { //maximo 4 bytes, por si acaso
        unsigned char valor_leer=final & 0xFF;
        final=final>>8;

        destino[i]=valor_leer;
    }

    return longitud;


}

void dump_variable_length(unsigned char *buffer,int longitud)
{
    int i;
    for (i=0;i<longitud;i++) {
        printf ("%02X ",buffer[i]);
    }
    printf ("\n");
}

//http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html


//Devuelve longitud en bytes
int mete_nota(unsigned char *mem,int duracion,int canal_midi,int keynote,int velocity)
{

    int indice=0;

    unsigned int deltatime=duracion;


    //Evento note on. al momento
    mem[indice++]=0;
   

    //int canal_midi=0;
    unsigned char noteonevent=(128+16) | (canal_midi & 0xf);


    mem[indice++]=noteonevent;
    mem[indice++]=keynote & 127;
    mem[indice++]=velocity & 127;



    //Evento note off
    int longitud_delta=util_int_variable_length(deltatime,&mem[indice]);
    indice +=longitud_delta;


    unsigned char noteoffevent=(128) | (canal_midi & 0xf);

    mem[indice++]=noteoffevent;
    mem[indice++]=keynote & 127;
    mem[indice++]=velocity & 127;

    return indice;
}

int main(void)
{

    //prueba variable length
    unsigned char buffer_var_length[4];
    int longitud;
    longitud=util_int_variable_length(0,buffer_var_length);
    dump_variable_length(buffer_var_length,longitud);

    longitud=util_int_variable_length(0x01,buffer_var_length);
    dump_variable_length(buffer_var_length,longitud);

    longitud=util_int_variable_length(0x40,buffer_var_length);
    dump_variable_length(buffer_var_length,longitud);    

    longitud=util_int_variable_length(0x7f,buffer_var_length);
    dump_variable_length(buffer_var_length,longitud);    

    longitud=util_int_variable_length(0x80,buffer_var_length);
    dump_variable_length(buffer_var_length,longitud);    

    longitud=util_int_variable_length(0x00002000,buffer_var_length);
    dump_variable_length(buffer_var_length,longitud);    

    longitud=util_int_variable_length(0x00003FFF,buffer_var_length);
    dump_variable_length(buffer_var_length,longitud);    

    longitud=util_int_variable_length(0x00004000,buffer_var_length);
    dump_variable_length(buffer_var_length,longitud);    

    longitud=util_int_variable_length(0x00100000,buffer_var_length);
    dump_variable_length(buffer_var_length,longitud);    

    longitud=util_int_variable_length(0x001FFFFF,buffer_var_length);
    dump_variable_length(buffer_var_length,longitud);    

    longitud=util_int_variable_length(0x00200000,buffer_var_length);
    dump_variable_length(buffer_var_length,longitud);    

    longitud=util_int_variable_length(0x08000000,buffer_var_length);
    dump_variable_length(buffer_var_length,longitud);                            


    longitud=util_int_variable_length(0x0FFFFFFF,buffer_var_length);
    dump_variable_length(buffer_var_length,longitud);    


    longitud=util_int_variable_length(0xFFFFFFFF,buffer_var_length);
    dump_variable_length(buffer_var_length,longitud);  

  

    unsigned char midi_file[2048];

    //cabecera
    memcpy(midi_file,"MThd",4);

    int pistas=1;

    //Valor 6
    midi_file[4]=0;
    midi_file[5]=0;
    midi_file[6]=0;
    midi_file[7]=6;

    //Formato
    midi_file[8]=0;
    midi_file[9]=1;

    //Pistas. This is a 16-bit binary number, MSB first.
    midi_file[10]=(pistas>>8) & 0xFF;
    midi_file[11]=pistas & 0xFF;   

 
    //Division. Ticks per quarter note (redonda?)
    //Haremos 400 (=50*4*2->redonda)

    int division=100;

    midi_file[12]=0x00;
    midi_file[13]=12;   

    //Pista
    memcpy(&midi_file[14],"MTrk",4);
    int indice=18;

    int notas=7;

    //int longitud_evento=((1+3)*2)*notas;//note on+off

    int puntero_longitud_pista=indice;

    //longitud eventos. meter al final
    /*midi_file[indice++]=(longitud_evento>>24) & 0xFF;
    midi_file[indice++]=(longitud_evento>>16) & 0xFF;
    midi_file[indice++]=(longitud_evento>>8) & 0xFF;
    midi_file[indice++]=(longitud_evento  ) & 0xFF;    */

    indice +=4;

    
    //TODO: averiguar parametro deltatime segun duracion
    //TODO: convertir nota formato ZEsarUX a keynote
    //TODO: poder escribir mas pistas... hasta 3


    //Nota 1
    
    unsigned int deltatime=(division/2); ///2=redonda


    deltatime=(division/2)/2; //blanca

    deltatime=(division/2)/4; //negra

    deltatime=(division/2)/8; //corchea

    int canal_midi=0;

    unsigned char keynote=60; //C octava 4
    unsigned char velocity=0x40; //Devices which are not velocity sensitive should send vv=40....


    indice +=mete_nota(&midi_file[indice],deltatime,canal_midi,keynote,velocity);


   //Nota 2
 
    keynote=62; //D octava 4
    indice +=mete_nota(&midi_file[indice],deltatime,canal_midi,keynote,velocity);    


    //Nota 3
    keynote=64; //E octava 4
    indice +=mete_nota(&midi_file[indice],deltatime,canal_midi,keynote,velocity);   

    //Nota 4
    keynote=65; //E octava 4
    indice +=mete_nota(&midi_file[indice],deltatime,canal_midi,keynote,velocity);      


    //Nota 5
    keynote=67; //G octava 4
    indice +=mete_nota(&midi_file[indice],deltatime,canal_midi,keynote,velocity);       

    //Nota 6
    keynote=69; //A octava 4
    indice +=mete_nota(&midi_file[indice],deltatime,canal_midi,keynote,velocity);       

    //Nota 7
    keynote=71; //B octava 4
    indice +=mete_nota(&midi_file[indice],deltatime,canal_midi,keynote,velocity);         


    //Meter longitud eventos
    int longitud_eventos=indice-puntero_longitud_pista-4; //evitar los 4 bytes que indican precisamente longitud

    //longitud eventos. meter al final
    midi_file[puntero_longitud_pista++]=(longitud_eventos>>24) & 0xFF;
    midi_file[puntero_longitud_pista++]=(longitud_eventos>>16) & 0xFF;
    midi_file[puntero_longitud_pista++]=(longitud_eventos>>8) & 0xFF;
    midi_file[puntero_longitud_pista++]=(longitud_eventos  ) & 0xFF;    

        



         FILE *ptr_configfile;

     ptr_configfile=fopen("salida.mid","wb");
     if (!ptr_configfile) {
                        printf("can not write midi file\n");
                        return 1;
      }

    fwrite(midi_file, 1, indice, ptr_configfile);


      fclose(ptr_configfile);

    return 0;

}