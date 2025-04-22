/******************************************************************************
**
** lep2raw
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
long flen;
#define MAXBUFFER	256
char buf[MAXBUFFER];
long pos;

long FileLength (FILE* fh);
void Error (char *errstr);
void ChangeFileExtension (char *str, char *ext);


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
    ChangeFileExtension (buf, "raw");
    }
  else      
    strcpy (buf, argv[2]);

  if ((fhi = fopen (argv[1], "rb")) == NULL) 
    Error ("Can't read file!");

  if ((fho = fopen (buf, "wb")) == NULL) 
    Error ("Can't create file!");

  flen = FileLength (fhi);


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


