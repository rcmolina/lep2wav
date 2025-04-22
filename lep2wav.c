/******************************************************************************
**
** lep2wav
**                                                                       
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

FILE *fhi, *fho;
long flen, folen;
#define MAXBUFFER	256
char buf[MAXBUFFER];
long pos;


long FileLength (FILE* fh);
void Error (char *errstr);
void ChangeFileExtension (char *str, char *ext);
void writeword(unsigned int i, FILE *fp);
void writestring(char *mystring, FILE *fp);
void writelong(unsigned long i, FILE *fp);

int main (int argc, char *argv[])
{

  if (argc < 2 || argc > 3)
    {
    printf ("\nUsage: lep2raw input.lep \n");
    exit (0);
    }

  if (argc == 2) 
    {  
    strcpy (buf, argv[1]); 
    ChangeFileExtension (buf, "wav");
    }
  else      
    strcpy (buf, argv[2]);

  if ((fhi = fopen (argv[1], "rb")) == NULL) 
    Error ("Can't read file!");

  if ((fho = fopen (buf, "wb")) == NULL) 
    Error ("Can't create file!");

  flen = FileLength (fhi);
	
	// Now let's think at the WAV file
	writestring("RIFF",fho);
	writelong(folen+36,fho);	 	 	 // replace later
	writestring("WAVEfmt ",fho);
	writelong(0x10,fho);
	writeword(1,fho);
	writeword(1,fho);
		
	writelong(20000,fho);
	writelong(20000,fho);
	
	writeword(1,fho);
	writeword(8,fho);
	writestring("data",fho);
	writelong(folen,fho);	  	  	  // replace later
	  
  pos = 0;
  int i, j, nsamples;
  unsigned char nivel = 255;	// valor de inicio
  #define SAMPLPERIOD	50		// 50us , ie 20 KHz
  int blocklength=0;
  while (pos < flen)
  {
  	blocklength = fread(buf, 1, MAXBUFFER, fhi);
	
	for (i=0;i<blocklength;i++) {  

		nsamples = abs(buf[i]);
		if (buf[i] == 0) nsamples=127;
		
		if (buf[i] > 0) nivel = 255;
		if (buf[i] < 0) nivel = 0;
		
		for (j=0; j< nsamples; j++) fwrite(&nivel, 1, 1, fho);
	}

	pos=pos + blocklength;  	 
  }

  folen = FileLength (fho);

  /* replace wav header with raw file length */
  fseek (fho, 4, SEEK_SET);  writelong(folen-44+36,fho);
  fseek (fho, 40, SEEK_SET);  writelong(folen-44,fho);
 
  fclose (fhi);
  fclose (fho);  
  //system("pause");
  return (0);
}

/* Changes the File Extension of String *str to *ext */
void ChangeFileExtension (char *str, char *ext)
{
  int n;
  
  n = strlen(str); 

  while (str[n] != '.') 
    n--;

  n++; 
  str[n] = 0;
  strcat (str, ext);
}

/* Determine length of file */
long FileLength (FILE* fh)
{
  long curpos, size;
  
  curpos = ftell (fh);
  fseek (fh, 0, SEEK_END);
  size = ftell (fh);
  fseek (fh, curpos, SEEK_SET);
  return (size);
}

/* exits with an error message *errstr */
void Error (char *errstr)
{
  printf ("\n-- Error: %s ('%s')\n", errstr, strerror (errno));
  exit (0);
}

/* Writing routines */
void writebyte(unsigned char c, FILE *fp)
{
        fputc(c,fp);
}



void writeword(unsigned int i, FILE *fp)
{
    fputc(i%256,fp);
    fputc(i/256,fp);
}


void writestring(char *mystring, FILE *fp)
{
    size_t c;

    for (c=0; c < strlen(mystring); c++) {
        writebyte(mystring[c],fp);
    }
}


void writelong(unsigned long i, FILE *fp)
{
    writeword(i%65536,fp);
    writeword(i/65536,fp);
}



