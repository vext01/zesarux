#include <stdio.h>
#include <string.h>



//http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html

int mete_nota(unsigned char *mem,int duracion,int canal_midi,int keynote,int velocity)
{

    int indice=0;

    unsigned int deltatime=duracion & 0x7f; //de momento duracion variable de 1 byte

        //unsigned int deltatime=0x7f; //prueba

    //Evento note on. al momento
    mem[indice++]=0;
    //midi_file[indice++]=(deltatime>>8)&0xFF;
    

    //int canal_midi=0;
    unsigned char noteonevent=(128+16) | (canal_midi & 0xf);

    //unsigned char keynote=60; //C octava 4
    //unsigned char velocity=100;

    mem[indice++]=noteonevent;
    mem[indice++]=keynote & 127;
    mem[indice++]=velocity & 127;



    //Evento note off
    mem[indice++]=deltatime & 0xFF;
    //midi_file[indice++]=(deltatime>>8)&0xFF;

    unsigned char noteoffevent=(128) | (canal_midi & 0xf);

    mem[indice++]=noteoffevent;
    mem[indice++]=keynote & 127;
    mem[indice++]=velocity & 127;

    return indice;
}

int main(void)
{

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

 
    //Division. TODO
    midi_file[12]=0x00;
    midi_file[13]=0x78;   

    //Pista
    memcpy(&midi_file[14],"MTrk",4);
    int indice=18;

    int notas=7;

    int longitud_evento=((1+3)*2)*notas;//note on+off


    //longitud eventos
    midi_file[indice++]=(longitud_evento>>24) & 0xFF;
    midi_file[indice++]=(longitud_evento>>16) & 0xFF;
    midi_file[indice++]=(longitud_evento>>8) & 0xFF;
    midi_file[indice++]=(longitud_evento  ) & 0xFF;    

    
    //TODO: averiguar parametro deltatime segun duracion
    //TODO: convertir nota formato ZEsarUX a keynote
    //TODO: poder escribir mas pistas... hasta 3


    //Nota 1
    
    unsigned int deltatime=0x7f; //prueba
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