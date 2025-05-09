/******************************************************************************
**
** lep2wav [freq] input.lep [output.wav]
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
#define UNIT	50.0	  	   	   	   	   	   	   // 50us , ie 20 KHz for lep
static int samplerate = (int)(1000000/UNIT);

long FileLength (FILE* fh);
void Error (char *errstr);
void ChangeFileExtension (char *str, char *ext);
void writeword(unsigned int i, FILE *fp);
void writestring(char *mystring, FILE *fp);
void writelong(unsigned long i, FILE *fp);

int getnumber(char *s)
{
// Returns the INT number contained in string *s
int i;

sscanf(s,"%d",&i); return(i);
}

int main (int argc, char *argv[])
{
int k; // argc position for read file

  if (argc < 2 || argc > 4) {
    printf ("\nUsage: lep2wav [freq] input.lep [output.wav]\n");
    exit (0);
  }
  else if (argc == 2) {
  	k = 1;  
    strcpy (buf, argv[1]); 
    ChangeFileExtension (buf, "wav");
  }
  else if (argc == 4) {
	samplerate=getnumber(argv[1]);
	k = 2;
    strcpy (buf, argv[3]); 
  }
  else if ( (argc == 3) && ((argv[1][0] >= 48) && (argv[1][0] <= 57)) ) {
		samplerate=getnumber(argv[1]);
		k = 2;
    	strcpy (buf, argv[2]);
    	ChangeFileExtension (buf, "wav");		
	}
  else if ( (argc == 3) && !((argv[1][0] >= 48) && (argv[1][0] <= 57)) ) {
		k = 1;
    	strcpy (buf, argv[2]); 	
  }

  if ((fhi = fopen (argv[k], "rb")) == NULL) 
    Error ("Can't read file!");

  if ((fho = fopen (buf, "wb")) == NULL) 
    Error ("Can't create file!");

  flen = FileLength (fhi);
	
	// write wav header (44 bytes)
	writestring("RIFF",fho);			// offset 0: File description header
	writelong(folen+36,fho);	 	 	// offset 4: (rawfilesize + 36) = (wavfilesize - 8). REPLACE later.
	writestring("WAVEfmt ",fho);		// offset 8: "WAVEfmt " Description header.
	writelong(0x10,fho);				// offset 16: Size of the rest of the Subchunk which follows this number: 16 for PCM
	writeword(1,fho);					// offset 20: Wave type format. PCM = 1 for Linear quantization
	writeword(1,fho);					// offset 22: Numchannels: 1 for Mono or 2 for stereo
		
	writelong(samplerate,fho);				// offset 24: Sample rate
	writelong(samplerate,fho);				// offset 28: Byte Rate = Bytes per second:  SampleRate * NumChannels * BitsPerSample/8
	
	writeword(1,fho);					// offset 32: Block alignment = NumChannels * BitsPerSample/8
	writeword(8,fho);					// offset 34: Bits per sample: 8 bits = 8, 16 bits = 16, etc
	writestring("data",fho);			// offset 36: "data" Description header
	writelong(folen,fho);				// offset 40: Size of data chunk = rawfilesize = (wavfilesize - 44). REPLACE later.
	  
  pos = 0;
  int i, j, lepsamples, nsamples;
  unsigned char nivel = 255;	// valor de inicio
  int blocklength=0;
  while (pos < flen)
  {
  	blocklength = fread(buf, 1, MAXBUFFER, fhi);
	
	for (i=0;i<blocklength;i++) {  

		lepsamples = abs(buf[i]);
		if (buf[i] == 0) lepsamples=127;
		
		if (buf[i] > 0) nivel = 255;
		if (buf[i] < 0) nivel = 0;
		
		nsamples= (int)(0.5 + (lepsamples * samplerate * UNIT /1000000));
		for (j=0; j< nsamples; j++) fwrite(&nivel, 1, 1, fho);
	}

	pos=pos + blocklength;  	 
  }

  folen = FileLength (fho);

  /* replace wav header with raw file length */
  fseek (fho, 4, SEEK_SET);  writelong(folen-44+36,fho);	// offset 4: (rawfilesize + 36) = (wavfilesize - 8)
  fseek (fho, 40, SEEK_SET);  writelong(folen-44,fho);		// offset 40: Size of data chunk = rawfilesize = = (wavfilesize - 44)
 
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



