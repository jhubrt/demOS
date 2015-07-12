/* -------------------------------------------------------------------
    MPP to BMP file converter.
    by Zerkman / Sector One
------------------------------------------------------------------- */

/* This program is free software. It comes without any warranty, to
* the extent permitted by applicable law. You can redistribute it
* and/or modify it under the terms of the Do What The Fuck You Want
* To Public License, Version 2, as published by Sam Hocevar. See
* http://sam.zoy.org/wtfpl/COPYING for more details. */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pixbuf.h"

typedef struct {
    int id;
    int ncolors;
    int nfixed;
    int border0;
    int x0;
    int (*xinc)(int);
    int width;
    int height;
} ModeD;

static int xinc0(int c) { return ((c==15)?88:((c==31)?12:((c==37)?100:4))); }
static int xinc1(int c) { return (((c)&1)?16:4); }
static int xinc2(int c) { return 8; }
static int xinc3(int c) { return ((c==15)?112:((c==31)?12:((c==37)?100:4))); }

static const ModeD modes[4] = {
    { 0, 54, 0, 1, 33, xinc0, 320, 199 },
    { 1, 48, 0, 1,  9, xinc1, 320, 199 },
    { 2, 58, 0, 1,  4, xinc2, 336, 199 },
    { 3, 54, 6, 0, 69, xinc3,  416, 273 },
};

static int read32(const void *ptr) {
    const unsigned char *p = ptr;
    return (p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
}

PixBuf *decode_mpp(const ModeD *mode, int ste, int extra, FILE *in, int raw_palette) 
{
    int bits = 3 * (3 + ste + extra);
    int palette_size = (mode->ncolors-mode->nfixed)*mode->height;
    int packed_palette_size = ((bits*(mode->ncolors-mode->nfixed-mode->border0*2)*mode->height+15)/8)&-2;
    int image_size = mode->width/2*mode->height;
    Pixel *pal = malloc(palette_size*sizeof(Pixel));
    unsigned char *img = malloc(image_size);
	unsigned char *pal_buf = (unsigned char*) malloc(raw_palette ? palette_size * 2 : packed_palette_size);
    int bitbuf = 0, bbnbits = 0;
    int i;


	if (raw_palette)
	{
		unsigned short* rc = (unsigned short*) pal_buf;

		fread(pal_buf, 2, palette_size - 2 * mode->height, in);

		for (i = 0; i < palette_size; i++) 
		{
			int x = i % mode->ncolors;

            if (mode->border0 && (x == 0 || x == ((mode->ncolors-1) & -16))) 
			{
				pal[i].rgb = 0;
				continue;
			}

			{
        		unsigned short c = *rc++;
				Pixel p;

                p.cp.a = 0;

                c = (c >> 8) | (c << 8);    // endian swap on pc
				
				switch (bits) 				
				{
				case 9:
					p.cp.r = (c>>6);
					p.cp.g = (c>>3)&7;
					p.cp.b = c&7;
					break;
				case 12:
				case 15:
					p.cp.r = ((c>>7)&0xe) | ((c>>11)&1);
					p.cp.g = ((c>>3)&0xe) | ((c>>7)&1);
					p.cp.b = ((c<<1)&0xe) | ((c>>3)&1);
					if (bits == 15) 
					{
						p.cp.r = (p.cp.r << 1) | ((c>>14)&1);
						p.cp.g = (p.cp.g << 1) | ((c>>13)&1);
						p.cp.b = (p.cp.b << 1) | ((c>>12)&1);
					}
					break;
				default:
					abort();
				}
				p.cp.r <<= 8 - (bits/3);
				p.cp.g <<= 8 - (bits/3);
				p.cp.b <<= 8 - (bits/3);
				pal[i] = p;
			}
		}
	}
	else
	{
    	unsigned char *p = pal_buf;

		fread(pal_buf, packed_palette_size, 1, in);

		for (i = 0; i < palette_size; i++) 
		{
			int x = i % mode->ncolors;
			if (mode->border0 && (x == 0 || x == ((mode->ncolors-1) & -16))) {
				pal[i].rgb = 0;
				continue;
			}
			while (bbnbits < bits) 
			{
				bitbuf = (bitbuf << 8) | *p++;
				bbnbits += 8;
			}

			bbnbits -= bits;

			{
				unsigned short c = (bitbuf >> bbnbits) & ((1<<bits)-1);
				Pixel p;
				p.cp.a = 0;
				switch (bits) {
				case 9:
					p.cp.r = (c>>6);
					p.cp.g = (c>>3)&7;
					p.cp.b = c&7;
					break;
				case 12:
				case 15:
					p.cp.r = ((c>>7)&0xe) | ((c>>11)&1);
					p.cp.g = ((c>>3)&0xe) | ((c>>7)&1);
					p.cp.b = ((c<<1)&0xe) | ((c>>3)&1);
					if (bits == 15) {
						p.cp.r = (p.cp.r << 1) | ((c>>14)&1);
						p.cp.g = (p.cp.g << 1) | ((c>>13)&1);
						p.cp.b = (p.cp.b << 1) | ((c>>12)&1);
					}
					break;
				default:
					abort();
				}
				p.cp.r <<= 8 - (bits/3);
				p.cp.g <<= 8 - (bits/3);
				p.cp.b <<= 8 - (bits/3);
				pal[i] = p;
			}
		}
	}

	fread(img, image_size, 1, in);

    {
        PixBuf *pix = pixbuf_new(mode->width, mode->height);
        int k=0, l=0;
        int x, y;
        unsigned short b0=0, b1=0, b2=0, b3=0;
        Pixel palette[16];

        memset(pix->array, 0, mode->width * mode->height * sizeof(Pixel));
        memset(palette, 0, sizeof(Pixel));

        for (y=0; y<mode->height; ++y) 
        {
            Pixel *ppal = pal + y * (mode->ncolors-mode->nfixed);
            int nextx = mode->x0;
            int nextc = 0;
            for (i=mode->nfixed; i<16; ++i)
                palette[i] = *ppal++;

            for (x=0; x<mode->width; ++x) 
            {
                if (x==nextx) 
                {
                    palette[nextc&0xf] = *ppal++;
                    nextx += mode->xinc(nextc);
                    ++nextc;
                }
                if ((x&0xf) == 0) 
                {
                    b0 = (img[k+0]<<8) | (img[k+1]);
                    b1 = (img[k+2]<<8) | (img[k+3]);
                    b2 = (img[k+4]<<8) | (img[k+5]);
                    b3 = (img[k+6]<<8) | (img[k+7]);
                    k += 8;
                }
                i = ((b3>>12)&8) | ((b2>>13)&4) | ((b1>>14)&2) | ((b0>>15)&1);
                pix->array[l++] = palette[i];
                b0 <<= 1;
                b1 <<= 1;
                b2 <<= 1;
                b3 <<= 1;
            }
        }
        free(pal);
        free(img);
		free(pal_buf);

        return pix;
    }
}

void mpp2bmp(const char* _mppfilename, int _mode, int ste, int extra, int doubl, const char* bmpfilename, int raw_palette)
{
    const ModeD *mode = &modes[_mode];
    FILE* in = fopen (_mppfilename, "rb");

    PixBuf *pix = decode_mpp(mode, ste, extra, in, raw_palette);

    if (doubl) 
    {
        PixBuf *pix2 = decode_mpp(mode, ste, extra, in, raw_palette);
        int k;
        for (k = 0; k < mode->width*mode->height; ++k) 
        {
            Pixel a = pix->array[k], b = pix2->array[k];
            a.cp.r = ((int)a.cp.r + b.cp.r)/2;
            a.cp.g = ((int)a.cp.g + b.cp.g)/2;
            a.cp.b = ((int)a.cp.b + b.cp.b)/2;
            pix->array[k] = a;
        }
        pixbuf_delete(pix2);
    }

    fclose(in);

    pixbuf_export_bmp(pix, bmpfilename);
    pixbuf_delete(pix);
}
