/* -------------------------------------------------------------------
 Management of a basic pixel buffer with export to BMP file format.
 by Zerkman / Sector One
------------------------------------------------------------------- */

/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef _PIXBUF_H_
#define _PIXBUF_H_

#include <stdint.h>

typedef union pixel 
{
  unsigned int rgb;
  struct { uint8_t a, r, g, b; } cp;
} 
Pixel;

typedef struct _pixbuf {
  int width;
  int height;
  Pixel *array;
  void *_ptr;
} PixBuf;


PixBuf *pixbuf_new(int width, int height);
void pixbuf_delete(PixBuf *pb);
void pixbuf_export_bmp(const PixBuf *pb, const char *filename);

#endif
