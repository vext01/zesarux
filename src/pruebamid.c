#include <stdio.h>
#include <string.h>

unsigned char midi_file[2048];

//http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html

void main(void)
{
    //cabecera
    memcpy(midi_file,"MThd",4);

    //Valor 6
    midi_file[4]=0;
    midi_file[5]=0;
    midi_file[6]=0;
    midi_file[7]=6;

    //Formato
    midi_file[8]=0;
    midi_file[9]=1;

    //Pistas
    midi_file[10]=0;
    midi_file[11]=1;   

 
    //Division. TODO
    midi_file[12]=0x00;
    midi_file[13]=0x78;   

    //Pista
    memcpy(&midi_file[14],"MTrk",4);
    int indice=18;

    int notas=2;

    int longitud_evento=(1+3)*notas;//longitud <delta-time><event> evento note on

    int deltatime=0x7f; //prueba



    //Nota 1
    

    //Valor longitud_evento
    midi_file[indice++]=(longitud_evento>>24) & 0xFF;
    midi_file[indice++]=(longitud_evento>>16) & 0xFF;
    midi_file[indice++]=(longitud_evento>>8) & 0xFF;
    midi_file[indice++]=(longitud_evento  ) & 0xFF;

    midi_file[indice++]=deltatime;

    //Evento note on
    int canal_midi=0;
    unsigned char noteonevent=(128+16) | canal_midi;

    unsigned char keynote=60; //C octava 4
    unsigned char velocity=1;

    midi_file[indice++]=noteonevent;
    midi_file[indice++]=keynote & 127;
    midi_file[indice++]=velocity & 127;


    //Nota 2
    

    //Valor longitud_evento
    midi_file[indice++]=(longitud_evento>>24) & 0xFF;
    midi_file[indice++]=(longitud_evento>>16) & 0xFF;
    midi_file[indice++]=(longitud_evento>>8) & 0xFF;
    midi_file[indice++]=(longitud_evento  ) & 0xFF;

    midi_file[indice++]=deltatime;

    //Evento note on
    canal_midi=0;
    noteonevent=(128+16) | canal_midi;

    keynote=61; //D octava 4
    velocity=1;

    midi_file[indice++]=noteonevent;
    midi_file[indice++]=keynote & 127;
    midi_file[indice++]=velocity & 127;



         FILE *ptr_configfile;

     ptr_configfile=fopen("salida.mid","wb");
     if (!ptr_configfile) {
                        printf("can not write midi file\n");
                        return;
      }

    fwrite(midi_file, 1, indice, ptr_configfile);


      fclose(ptr_configfile);



}