#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tigr.h"
#include "decoder.h"

#pragma pack(1)
typedef struct
{
	uint16_t magic;
	uint32_t fileSize;
	uint32_t reserved0;
	uint32_t bitmapDataOffset;

	uint32_t bitmapHeaderSize;
	uint32_t width;
	uint32_t height;
	uint16_t planes;
	uint16_t bitsPerPixel;
	uint32_t compression;
	uint32_t bitmapDataSize;
	uint32_t hRes;
	uint32_t vRes;
	uint32_t colors;
	uint32_t importantColors;
} BMPHeader_t;


int getTiledPix(codec_t *c,int sx,int sy);
void setTiledPix(codec_t *c,int sx,int sy,int color);

extern RGBA palette[256];
void bmpSave(const char *fname,codec_t *codec,int w,int h)
{
BMPHeader_t header;
	memset(&header,0,sizeof(BMPHeader_t));
	header.width = w;
	header.height = h;
	header.planes = 1;
	header.bitsPerPixel = 8;
	header.magic = 0x4d42;
	header.fileSize = 0x400 + 40 + (w*h);
	header.colors = 256;
	header.bitmapDataOffset = 0x436;
	header.bitmapHeaderSize = 40;
	FILE *fp=fopen(fname,"wb");
	fwrite(&header,sizeof(BMPHeader_t),1,fp);

	for (int y=0;y<256;y++)
	{
		uint8_t bl=0xff;
		fwrite(&palette[y].r,1,1,fp);
		fwrite(&palette[y].g,1,1,fp);
		fwrite(&palette[y].b,1,1,fp);
		fwrite(&bl,1,1,fp);
	}
	
	//	write upside down 
	for (int y=h-1;y>=0;y--)
	{
		for (int x=0;x<w;x++)
		{
			uint8_t index = getTiledPix(codec,x,y);
			fwrite(&index,1,1,fp);
		}
	}
	//

	fclose(fp);
}

void bmpLoad(const char *fname,codec_t *codec,int w,int h)
{
BMPHeader_t header;
	
	FILE *fp=fopen(fname,"rb");
	if (fp==NULL)
	{
		printf("%s failed to load\n",fname);
		return;
	}
	fread(&header,sizeof(BMPHeader_t),1,fp);

	for (int y=0;y<header.colors;y++)
	{
		uint8_t bl=0xff;
		fread(&palette[y].r,1,1,fp);
		fread(&palette[y].g,1,1,fp);
		fread(&palette[y].b,1,1,fp);
		fread(&bl,1,1,fp);
	}

	printf("%dx%d x %dbpp\n",header.width,header.height,header.bitsPerPixel);
	//	read upside down 
	for (int y=h-1;y>=0;y--)
	{
		if (header.bitsPerPixel==8)
		{
			for (int x=0;x<w;x++)
			{
				uint8_t r;
				fread(&r,1,1,fp);
				setTiledPix(codec,x,y,r);
			}
		}
		else if (header.bitsPerPixel==4)
		{
			for (int x=0;x<w>>1;x++)
			{
				uint8_t r;
				fread(&r,1,1,fp);
				setTiledPix(codec,x<<1,y,r>>4);
				setTiledPix(codec,(x<<1)+1,y,r&0xf);
			}
		}
	}
	fclose(fp);
}
