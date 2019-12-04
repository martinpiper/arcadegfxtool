#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <ctype.h>

#include "decoder.h"
#include "bmp.h"

#define SCREEN_W 128
#define SCREEN_H 128

#ifdef TIGR
//	skip including this to not use tigr
#include "tigr.h"
void tigrView(decoder_t *decode);
#endif

RGBA palette[256]={{0,0x66,0xbb},{0x00,0x00,0x00},{0xcc,0xaa,0x88},{0xaa,0x88,0x66},{0x77,0x55,0x00},{0x55,0x44,0x00},{48,48,48},{56,56,56},{63,63,63},{63,0,0},{0,63,0},{0,0,63},{0,16,0},{0,0,16},{0,0,16},{0,0,0}};

//	helper functions to make a tiled view of decoder cells

int page=0;
int pages=0;
int getTileID(codec_t *c,int sx,int sy)
{
	int tw = SCREEN_W/c->width;
	int th = SCREEN_H/c->height;
	if ((sx<0) || (sy<0) || (sx>SCREEN_W)) return 0;
	//	figure out TILE id 
	//	take screen and split into tiles of the codec 
	int id=((sx/c->width)&(tw-1)) + ((sy/c->height)*tw);
	//	include page offsets
	pages = (c->total) / (tw*th);

	if (pages>1)
		page%=pages;
	else 
	{
		pages=1;
		page=0;
	}
	int offset = page*(tw*th) % c->total;
	id += offset;
	if (id>c->total-1)	return 0;
	return id;
}

int getTiledPix(codec_t *c,int sx,int sy)
{
	return decoderGetPix(c,sx,sy,getTileID(c,sx,sy));
}

void setTiledPix(codec_t *c,int sx,int sy,int color)
{
	decoderSetPix(c,sx,sy,color,getTileID(c,sx,sy));
}

int main(int argc, const char **argv)
{
	decoder_t *decode=NULL;
	bool saveOnExit=true;
	bool View=true;
	for (int q=1;q<argc;q++)
	{
		if ((strstr(argv[q],".ini")) || (strstr(argv[q],".INI")))
		{
			printf("load %s\n",argv[q]);
			decode = decoderParse((char*)argv[q]);
		}

//	write roms back out
		if ((strcmp(argv[q],"-w")==0) && decode!=NULL)
			saveOnExit=true;

		if ((strcmp(argv[q],"-q")==0) && decode!=NULL)
			View=false;

//	-x export bmp for decoder or specified codec 
//	-x 0 ,-x 1 etc. 
		if ((strcmp(argv[q],"-x")==0) && decode!=NULL)
		{
			char string[256];
			int codecid = -1;
			if (argc>q+1)
			{
				if (isdigit(argv[q+1][0])==true)
				{
					codecid = atol(argv[q+1]);
					q+=1;
				}
			}
			for (int z=0;z<decode->ncodecs;z++)
			{
				//	create filename for decoder_codecid
				sprintf(string,"%s_%02X_%dx%d.bmp",decode->basename,z,decode->codecs[z].width,decode->codecs[z].height);
				if ((codecid==-1) || (codecid==z))
				{
					printf("write %s\n",string);
					page=0;
					bmpSave(string,&decode->codecs[z],SCREEN_W,decoderImageHeight(&decode->codecs[z],SCREEN_W));
				}
			}
		}

//	-i import bmp for decoder or specified codec 
//	-i 0 ,-i 1 etc. 
		if ((strcmp(argv[q],"-i")==0) && decode!=NULL)
		{
			char string[256];
			int codecid = -1;
			if (argc>q+1)
			{
				if (isdigit(argv[q+1][0])==true)
				{
					codecid = atol(argv[q+1]);
					q+=1;
				}
			}
			//	for each codec
			for (int z=0;z<decode->ncodecs;z++)
			{
				//	create filename for decoder_codecid
				sprintf(string,"%s_%02X_%dx%d.bmp",decode->basename,z,decode->codecs[z].width,decode->codecs[z].height);
				if ((codecid==-1) || (codecid==z))
				{
					printf("read %s\n",string);
					page = 0;
					bmpLoad(string,&decode->codecs[z],SCREEN_W,decoderImageHeight(&decode->codecs[z],SCREEN_W));
				}
			}
		}
	}

	if (decode==NULL)
	{
		printf("pass .ini file\n");
		exit(0);
	}

	page = 0;

#ifdef TIGR_FIXED
	if (View==true)
		tigrView(decode);
#endif
	if (saveOnExit==true)
		decoderWrite(decode);

	decoderFree(decode);

	return 0;
}

#ifdef TIGR
//	simple view logic 
void tigrView(decoder_t *decode)
{
Tigr *screen;
	int currentpen=4;
	int currentcodec=0;
	screen = tigrWindow(SCREEN_W, SCREEN_H, "test", TIGR_FIXED);
	tigrClear(screen, tigrRGB(0x80, 0x90, 0xa0));

	while (!tigrClosed(screen) && !tigrKeyDown(screen, TK_ESCAPE))
	{
		int mx,my,buttons;
		codec_t *codec=&decode->codecs[currentcodec];

		if (tigrKeyDown(screen,TK_TAB))
		{
			currentcodec++;
			currentcodec%=decode->ncodecs;
		}

		if (tigrKeyDown(screen,TK_SHIFT))
		{
			page++;
		}
		tigrMouse(screen,&mx,&my,&buttons);

		for (int y=0;y<SCREEN_H;y++)
		{
			for (int x=0;x<SCREEN_W;x++)
			{
				int index = getTiledPix(codec,x,y);
				RGBA col = palette[index];
				tigrPlot(screen,x,y,tigrRGB(col.r,col.g,col.b));
			}
		}

		int xoff =0;
		int tsize=8;
		int size=16;
		int hsize=size/2;

		int cx = (mx/codec->width)*codec->width;
		int cy = (my/codec->height)*codec->height;

		if (mx>(SCREEN_W/2))
			xoff = 0;
		else
			xoff = SCREEN_W-(tsize*size);
#if 0		
		for (int y=0;y<size;y++)
		{
			for (int x=0;x<size;x++)
			{
				int index = getTiledPix(codec,mx-hsize+x,my-hsize+y);
				RGBA col = palette[index];
				tigrFill(screen,xoff+(x*tsize),y*tsize,tsize,tsize,tigrRGB(col.r,col.g,col.b));
			}
		}
#endif
		for (int q=0;q<16;q++)
		{
				RGBA col = palette[q];
				tigrFill(screen,xoff+(q*8),size*tsize,8,8,tigrRGB(col.r,col.g,col.b));
		}

		int yoff = -12;
		if (cy<(SCREEN_H/2))
			yoff=codec->height;

		tigrRect(screen,cx,cy,codec->width,codec->height,tigrRGB(255,255,255));
		tigrPrint(screen,tfont,cx,cy+yoff,tigrRGB(255,255,255),"%04X",getTileID(codec,mx,my));

		tigrPrint(screen,tfont,0,SCREEN_H-12,tigrRGB(255,255,255),"Name:%s",decode->description);
		tigrPrint(screen,tfont,0,SCREEN_H-24,tigrRGB(255,255,255),"%d x %d @ %d bpp %04x page:%d/%d",codec->width,codec->height,codec->planes,codec->total,page+1,pages);
		//	left button , set pixel 
		if (buttons&1)
		{
			setTiledPix(codec,mx,my,currentpen);
		}

		//	right button, grab pixel 
		if (buttons&4)
		{
			currentpen = getTiledPix(codec,mx,my);
		}
		tigrUpdate(screen);
	}
	tigrFree(screen);
}
#endif
