#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif

int main (void)
{

	unsigned char buffer_lectura[1024];

	char *file_orig="input.hdf";
	char *file_dest="output.ide";

        FILE *ptr_inputfile;
        ptr_inputfile=fopen(file_orig,"rb");

        if (ptr_inputfile==NULL) {
                printf ("Error opening %s\n",file_orig);
                exit(1);
        }

	FILE *ptr_outputfile;
	ptr_outputfile=fopen(file_dest,"wb");

        if (ptr_outputfile==NULL) {
                printf ("Error opening %s\n",file_dest);
                exit(1);
        }



	//Saltamos primero los 128 bytes del inicio

        int leidos=fread(buffer_lectura,1,128,ptr_inputfile);

	//Y vamos leyendo bloques de 1024

	do {
	        leidos=fread(buffer_lectura,1,1024,ptr_inputfile);
		if (leidos>0) fwrite(buffer_lectura,1,leidos,ptr_outputfile);
	} while (leidos>0);

	fclose (ptr_inputfile);

                        fclose(ptr_outputfile);

	return 0;

}
