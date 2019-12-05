
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include "decoder.h"

//	pretty big . may need more ? 
static uint8_t rom[16*65536];
//	load a file into a buffer 
//	used by the rom loader
static void decoderLoadTo(uint8_t *buffer,const char *fname)
{
FILE *fp;
int len;
	fp=fopen(fname,"rb");
	if (fp==NULL)
	{
		printf("%s failed\n",fname);
		return;
	}

	fseek(fp,0,SEEK_END);
	len = ftell(fp);
	fseek(fp,0,SEEK_SET);
	fread(buffer,len,1,fp);
	fclose(fp);
}

//	save from a buffer of len bytes
static void decoderSaveFrom(uint8_t *buffer,int len,const char *fname)
{
FILE *fp;
	fp=fopen(fname,"wb");
	if (fp==NULL)
	{
		printf("%s failed\n",fname);
		return;
	}
	fwrite(buffer,len,1,fp);
	fclose(fp);
}

//	give us a height for the entire codec based on the output width
int decoderImageHeight(codec_t *c,int width)
{
	int tw = width/c->width;
	int sh=((c->total/tw)*c->height);
	printf("calculated height = %d\n",sh);
	return sh;
}

// get a single bit
static int decoderGetBit( char * buffer, long bitnumber, long bufsize )
{
	long nbyte = bitnumber >> 3;
	int nbit = bitnumber & 7;    
	int mask = 0x80 >> nbit;
		//	nothing out of bounds
	if( nbyte < 0 || nbyte > bufsize ) return( 0 );
	return ((buffer[nbyte] & mask) == mask);
}

//	set a single bit 
static void decoderSetBit( char * buffer, long bitnumber, int value, long bufsize )
{
    long nbyte = bitnumber >> 3;
    int nbit = bitnumber & 7;
    int mask = 0x80 >> nbit;
		//	nothing out of bounds
    if( nbyte < 0 || nbyte > bufsize ) return;
    buffer[nbyte] &= ~mask;
    if (value)
        buffer[nbyte] |= mask;
}

//	just scan a number from the token
//	for parsing the .INI files

static int scandigit()
{
char *next = strtok(NULL," \r\n=");
	if (next==NULL)	return 0;
	return (atoi(next));
}

//	parse the .ini files 
decoder_t *decoderParse(char *fname)
{
int len;	
decoder_t *dec = NULL;
codec_t *codec = NULL;
char *ext = NULL;
char *buffer=NULL;

	//	read the entire file
	FILE *fp=fopen(fname,"rb");
	if (fp==NULL)
	{
		printf("File %s failed to load\n");
		return NULL;
	}
	fseek(fp,0,SEEK_END);
	len = ftell(fp)+1;
	fseek(fp,0,SEEK_SET);
	buffer = (char*)malloc(len);
	memset(buffer,0,len);
	fread(buffer,len-1,1,fp);
	fclose(fp);

	//	make a new decoder
	dec = (decoder_t*)malloc(sizeof(decoder_t));
	dec->ncodecs=0;
	dec->description = NULL;
	dec->codecs = NULL;
	dec->roms = NULL;

	//	start tokenizing
	char* token = strtok(buffer, " \n\r\t="); 
	char *next=NULL;

	//	check for / or \\ in the ini name 
	char *slash = strchr(fname,'/');
	if (slash==NULL)
		slash = strchr(fname,'\\');
	if (slash!=NULL)		
	{
		dec->basename=_strdup(slash+1);
	}
	else
		dec->basename=_strdup(fname);

	//	lowercase it 
	for (int q=0;q<strlen(dec->basename);q++)
		dec->basename[q]=tolower(dec->basename[q]);

	//	basename = pacman if input is drivers/PACMAN.ini 
	ext = strstr(dec->basename,".ini");
	if (ext!=NULL)
		*ext = 0;

	//	parse the info 
	while(token!=NULL)
	{
		if (_stricmp(token,"Description")==0)
		{
			next = strtok(NULL,"\r\n=");
			//	skip spaces
			while(*next==0x20) next++;
			dec->description = _strdup(next);
		}
		else if (_stricmp(token,"GfxDecodes")==0)
		{
			dec->ncodecs = scandigit();
			dec->codecs = (codec_t*)malloc(sizeof(codec_t)*dec->ncodecs);
			codec = NULL;
		}
		else if (_stricmp(token,"Orientation")==0)
		{
			//	haven't encountered this , so isn't handled yet 
			dec->orientation = scandigit();
		}
		else if (strstr(token,"[Decode")!=NULL)
		{
			if (codec==NULL)
				codec = dec->codecs;
			else 
				codec++;
		}
		else if (_stricmp(token,"start")==0)
			codec->startaddress = scandigit();
		else if (_stricmp(token,"width")==0)
			codec->width = scandigit();
		else if (_stricmp(token,"height")==0)
			codec->height = scandigit();
		else if (_stricmp(token,"total")==0)
			codec->total = scandigit();
		else if (_stricmp(token,"planes")==0)
			codec->planes = scandigit();
		else if (_stricmp(token,"planeoffsets")==0)
		{
			for (int q=0;q<codec->planes;q++)
				codec->planeoffsets[q] = scandigit();
		}
		else if (_stricmp(token,"xoffsets")==0)
		{
			for (int q=0;q<codec->width;q++)
				codec->xoffsets[q] = scandigit();
		}
		else if (_stricmp(token,"yoffsets")==0)
		{
			for (int q=0;q<codec->height;q++)
				codec->yoffsets[q] = scandigit();
		}
		else if (_stricmp(token,"charincrement")==0)
			codec->charincrement = scandigit();
		else if (_stricmp(token,"[GraphicsRoms]")==0)
		{
			//	do nothing just exclude it :)
		}
		else if (strstr(token,"Rom")!=NULL)
		{
			rom_t *nrom;
			char string[256];
			int loadaddress = scandigit();
			int size = scandigit();
			token = strtok(NULL," \n\r\t");

			sprintf(string,"roms/%s/%s",dec->basename,token);
//			printf("load %s\n",string);
			decoderLoadTo(&rom[loadaddress],string);
			nrom = (rom_t*)malloc(sizeof(rom_t));
			nrom->loadaddress = loadaddress;
			nrom->size = size;
			nrom->name = _strdup(string);
			nrom->next = dec->roms;
			dec->roms = nrom;
//			printf("%04x %04X [%s]\n",loadaddress,size,token);
		}
		else 
		{
//			printf("\"%s\"\n",token);
		}
		token = strtok(NULL," \n\r\t=");
	}
	free(buffer);
	
	return dec;
}

//	write all the GFX roms back out 
void decoderWrite(decoder_t *dec)
{
rom_t *rm = dec->roms;
	while(rm!=NULL)
	{
		rom_t *nxt=rm->next;
		decoderSaveFrom(&rom[rm->loadaddress],rm->size,rm->name);
		rm = nxt;
	}
}

//	clean up 
void decoderFree(decoder_t *dec)
{
rom_t *rm;

	rm = dec->roms;
	while(rm!=NULL)
	{
		rom_t *nxt=rm->next;
		free(rm->name);
		free(rm);
		rm = nxt;
	}
	if (dec->codecs!=NULL)
		free(dec->codecs);
	if (dec->basename!=NULL)
		free(dec->basename);
	if (dec->description!=NULL)
		free(dec->description);
	free(dec);
}

//
//	get a pixel from a tile. 
//
int decoderGetPix(codec_t *c,int sx,int sy,int tile)
{
if (tile>c->total) return 0;
int startOffsetBits = ( c->startaddress * 8 ) + (c->charincrement * tile );
int	tx = sx%c->width;
int	ty = sy%c->height;
int xoffs = c->xoffsets[tx];
int yoffs = c->yoffsets[ty];
	int bits = 0;
	for (int p=0;p<c->planes;p++)
	{
		int poffs = c->planeoffsets[p];    /* the offset for the plane */
		int ret = decoderGetBit((char*)rom,	startOffsetBits + poffs + xoffs + yoffs, sizeof(rom) );
		bits|=ret<<(c->planes-1-p);
	}
	return bits;
}

//
//	set a pixel a tile. 
//

void decoderSetPix(codec_t *c,int sx,int sy,int color,int tile)
{
if (tile>c->total) return;

int startOffsetBits = ( c->startaddress * 8 ) + (c->charincrement * tile );
int	tx = sx%c->width;
int	ty = sy%c->height;
int xoffs = c->xoffsets[tx];
int yoffs = c->yoffsets[ty];

	int bits = 0;
	for (int p=0;p<c->planes;p++)
	{
		int pmask = 1<<(c->planes-1-p);
		int bit = 0;
		int poffs = c->planeoffsets[p];    /* the offset for the plane */
		if ((color&pmask)==0)
			bit=0;
		else 
			bit=1;

		decoderSetBit((char*)rom,	startOffsetBits + poffs + xoffs + yoffs, bit, sizeof(rom) );
	}
}

