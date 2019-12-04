typedef struct 
{
	int startaddress; 
	int width;
	int height;
	int total;
	int planes;
	int planeoffsets[8];
	int xoffsets[32];
	int yoffsets[32];
	int charincrement;
	unsigned int palette[256];
} codec_t;

typedef struct _rom_t_
{
	char *name;
	unsigned int loadaddress;
	unsigned int size;
	struct	_rom_t_ *next;
} rom_t;

typedef struct 
{
	char *description;
	char *basename;
	int orientation;

	int ncodecs;
	codec_t *codecs;

	rom_t		*roms;
} decoder_t;

typedef struct 
{
	uint8_t r,g,b,a;
} RGBA;

decoder_t *decoderParse(char *fname);
int decoderImageHeight(codec_t *c,int width);
int decoderGetPix(codec_t *c,int sx,int sy,int tile);
void decoderSetPix(codec_t *c,int sx,int sy,int color,int tile);
void decoderFree(decoder_t *dec);
void decoderWrite(decoder_t *dec);
