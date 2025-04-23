/******************************************************************************
**
** wap2lep
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
unsigned char buf[MAXBUFFER];
long pos;


long FileLength (FILE* fh);
void Error (char *errstr);
void ChangeFileExtension (char *str, char *ext);
void writeword(unsigned int i, FILE *fp);
void writestring(char *mystring, FILE *fp);
void writelong(unsigned long i, FILE *fp);
static void lepb(int dir, int delay);

static int lep_count = 0;
static int lep_curr_sample;
static int lep_last_sample = -1;

#define UNIT 50 /* resolution: 50 µs */
static int samplerate;

int main (int argc, char *argv[])
{

  if (argc < 2 || argc > 3)
    {
    printf ("\nUsage: wav2lep input.wav \n");
    exit (0);
    }

  if (argc == 2) 
    {  
    strcpy (buf, argv[1]); 
    ChangeFileExtension (buf, "lep");
    }
  else      
    strcpy (buf, argv[2]);

  if ((fhi = fopen (argv[1], "rb")) == NULL) 
    Error ("Can't read file!");

  if ((fho = fopen (buf, "wb")) == NULL) 
    Error ("Can't create file!");

  flen = FileLength (fhi);
	
  fseek (fhi, 24, SEEK_SET);fread(&samplerate, 1, 2, fhi);
  //printf("%d\n", samplerate);

  fseek (fhi, 44, SEEK_SET);	//skip wav file header (44bytes)

  pos =44;
  int i;
  int blocklength=0;
  while (pos < flen)
  {
  	blocklength = fread(buf, 1, MAXBUFFER, fhi);
	
	for (i=0;i<blocklength;i++) {  
		lep_count++;
		lep_curr_sample = (buf[i] >= 0x80) ? 1 : 0;
		if (lep_curr_sample != lep_last_sample) {
			//if (lep_last_sample != -1) lepb(lep_last_sample, lep_count * 10000 / 441 / UNIT); //nsamples * 1000000/44100 * 1/50 us
			if (lep_last_sample != -1) lepb(lep_last_sample, (int)(0.5 +(lep_count * 10000.0 / 441 / UNIT))); //nsamples * 1000000/44100 * 1/50 us
			lep_last_sample = lep_curr_sample;
			lep_count = 0;
		}
  	}
	pos=pos + blocklength;  	 
  }

  // last LEP byte
  //lepb(lep_last_sample, lep_count * 10000 / 441 / UNIT);
  lepb(lep_last_sample, (int)(0.5 +(lep_count * 10000.0 / 441 / UNIT)));

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


static void lepb(int boollevel, int lepvalue)
{
	int i;
	int lepbytes;
	int byte;
	lepbytes = lepvalue / 127;
	byte = lepvalue - (127 * lepbytes);
	if (byte == 0) byte++;
	if (boollevel == 0) byte = -byte;
	fputc(byte, fho);
	for (i = 0; i < lepbytes; i++) fputc(0, fho);
}


