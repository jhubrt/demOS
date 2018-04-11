/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2017 J.Hubert

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
  and associated documentation files (the "Software"), 
  to deal in the Software without restriction, including without limitation the rights to use, 
  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies 
  or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-------------------------------------------------------------------------------------------------*/

#include "DEMOSDK\BASTYPES.H"

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\PC\BITCONVR.H"


void BITfrom8bTox888 (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    u8* source = (u8*) _source;
    u8* dest   = (u8*) _dest;
    u16 x,y;


    for (y = 0 ; y < _h ; y++)
    {
        u8*  s = source;
        u32* d = (u32*) dest;


        for (x = 0 ; x < _w ; x++)
        {
            *d++ = _lut[*s++];
        }

        source += _pitchSource;
        dest   += _pitchDest;
    }
}


void BITfrom8bTo888 (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    u8* source = (u8*) _source;
    u8* dest   = (u8*) _dest;
    u16 x,y;


    for (y = 0 ; y < _h ; y++)
    {
        u8* s = source;
        u8* d = dest;


        for (x = 0 ; x < _w ; x++)
        {
            u32 c = _lut[*s++];
            
            *d++ = (u8)(c >> 16);
            *d++ = (u8)(c >> 8);
            *d++ = (u8)(c);
        }

        source += _pitchSource;
        dest   += _pitchDest;
    }
}


void BITfromx888To8b (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    u8* source = (u8*) _source;
    u8* dest   = (u8*) _dest;
    u16 x,y;


    for (y = 0 ; y < _h ; y++)
    {
        u8* s = source;
        u8* d = dest;


        for (x = 0 ; x < _w ; x++)
        {
            u16 grey = (s[0] * 14) + (s[2] * 37) + (s[1] * 77);
            
            grey >>= 7;

            *d++ = (u8) grey;
            s += 4;
        }

        source += _pitchSource;
        dest   += _pitchDest;
    }
}


void BITfrom888Tox888 (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    u8* source = (u8*) _source;
    u8* dest   = (u8*) _dest;
    u16 x,y;


    for (y = 0 ; y < _h ; y++)
    {
        u8* s = source;
        u8* d = dest;


        for (x = 0 ; x < _w ; x++)
        {
            d[0] = s[0];
            d[1] = s[1];
            d[2] = s[2];
            d[3] = 0;

            s += 3;
            d += 4;
        }

        source += _pitchSource;
        dest   += _pitchDest;
    }
}

void BITfrom4bTo8b (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    u8* source = (u8*) _source;
    u8* dest   = (u8*) _dest;
    u16 x,y;


    for (y = 0 ; y < _h ; y++)
    {
        u8* s = source;
        u8* d = dest;


        for (x = 0 ; x < _w ; x += 2)
        {
            *d++ = *s >> 4;
            *d++ = *s & 0xF;

            s++;
        }

        source += _pitchSource;
        dest   += _pitchDest;
    }
}

void BITfrom8bTo4b (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    u8* source = (u8*) _source;
    u8* dest   = (u8*) _dest;
    u16 x,y;


    for (y = 0 ; y < _h ; y++)
    {
        u8* s = source;
        u8* d = dest;


        for (x = 0 ; x < _w ; x += 2)
        {
            u8 p1 = *s++;
            u8 p2 = *s++;

            ASSERT (p1 < 16);
            ASSERT (p2 < 16);

            p1 &= 0xF;
            p2 &= 0xF;

            *d++ = (p1 << 4) | p2;
        }

        source += _pitchSource;
        dest   += _pitchDest;
    }
}

static void from4bToChunk (void* _source, u16 _pitchSource, void* _dest, u16 _w, u16 _h, u16 _pitchDest, u16 _nbP)
{
    u8* source = (u8*) _source;
    u8* dest   = (u8*) _dest;
	u16 planes[4];
    u16 y;
    u8  colormax = 1 << _nbP;


    for (y = 0 ; y < _h ; y++)
    {
        u8*  s = source;
        u16* d = (u16*) dest;
        u16 x;


        for (x = 0 ; x < _w ; x += 16)
        {
            u16 i,p;


            for (p = 0 ; p < _nbP ; p++)
            {
                planes[p] = 0;
            }

            for (i = 0 ; i < 8 ; i++)
            {
                u16 p;
                u8 pix1 = *s >> 4;
                u8 pix2 = *s & 0xF;

                s++;

                ASSERT(pix1 < colormax);
                ASSERT(pix2 < colormax);

                for (p = 0 ; p < _nbP ; p++)
                {
                    planes[p] <<= 1;
                    planes[p] |= pix1 & 1;
                    pix1 >>= 1;

                    planes[p] <<= 1;
                    planes[p] |= pix2 & 1;
                    pix2 >>= 1;
                }
            }

            for (p = 0 ; p < _nbP ; p++)
            {
                *d++ = PCENDIANSWAP16(planes[p]);
            }
        }

        source += _pitchSource;
        dest   += _pitchDest;
    }
}

void BITfrom4bToChunk1P (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    from4bToChunk (_source, _pitchSource, _dest, _w, _h, _pitchDest, 1);
}

void BITfrom4bToChunk2P (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    from4bToChunk (_source, _pitchSource, _dest, _w, _h, _pitchDest, 2);
}

void BITfrom4bToChunk3P (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    from4bToChunk (_source, _pitchSource, _dest, _w, _h, _pitchDest, 3);
}

void BITfrom4bToChunk4P (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    from4bToChunk (_source, _pitchSource, _dest, _w, _h, _pitchDest, 4);
}

static void fromChunkTob (void* _source, u16 _pitchSource, void* _dest, u16 _w, u16 _h, u16 _pitchDest, u16 _nbP, u8 _nbB)
{
    u8* source = (u8*) _source;
    u8* dest   = (u8*) _dest;
    u16 y;
    u8  val = 1 << _nbP;


    ASSERT(_nbP <= 4);
    ASSERT((_nbB == 4) || (_nbB == 8));

    for (y = 0 ; y < _h ; y++)
    {
        u16 planes[4];
        u16 x;
        u16* s = (u16*) source;
        u8*  d = dest;

        for (x = 0 ; x < _w ; x += 16)
        {
            u16 p,i;

            for (p = 0 ; p < _nbP ; p++)
            {
                planes[p] = PCENDIANSWAP16( *s );
                s++;
            }

            for (i = 0 ; i < 8 ; i++)
            {
                u8  pix1 = 0;
                u8  pix2 = 0;

                for (p = 0 ; p < _nbP ; p++)
                {
                    pix1 |= (planes[p] & 0x8000) != 0 ? val : 0;
                    pix1 >>= 1;
                    planes[p] <<= 1;
                }

                for (p = 0 ; p < _nbP ; p++)
                {
                    pix2 |= (planes[p] & 0x8000) != 0 ? val : 0;
                    pix2 >>= 1;
                    planes[p] <<= 1;
                }

                if ( _nbB == 4 )
                {
                    *d++ = (pix1 << 4) | pix2;
                }
                else
                {
                    *d++ = pix1;
                    *d++ = pix2;
                }
            }
        }

        source += _pitchSource;
        dest   += _pitchDest;
    }
}

void BITfromChunk1PTo4b (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    fromChunkTob (_source, _pitchSource, _dest, _w, _h, _pitchDest, 1, 4);
}

void BITfromChunk2PTo4b (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    fromChunkTob (_source, _pitchSource, _dest, _w, _h, _pitchDest, 2, 4);
}

void BITfromChunk3PTo4b (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    fromChunkTob (_source, _pitchSource, _dest, _w, _h, _pitchDest, 3, 4);
}

void BITfromChunk4PTo4b (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    fromChunkTob (_source, _pitchSource, _dest, _w, _h, _pitchDest, 4, 4);
}

void BITfromChunk1PTo8b (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    fromChunkTob (_source, _pitchSource, _dest, _w, _h, _pitchDest, 1, 8);
}

void BITfromChunk2PTo8b (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    fromChunkTob (_source, _pitchSource, _dest, _w, _h, _pitchDest, 2, 8);
}

void BITfromChunk3PTo8b (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    fromChunkTob (_source, _pitchSource, _dest, _w, _h, _pitchDest, 3, 8);
}

void BITfromChunk4PTo8b (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest)
{
    fromChunkTob (_source, _pitchSource, _dest, _w, _h, _pitchDest, 4, 8);
}


void BITignorePlanes (void *_source, bool* _planes, u16 _nbchunks, u16 _pitch, u16 _height, void* _dest)
{
	u16* buf  = (u16*) _source;
	u16* dest = (u16*) _dest;
	u16 x, y, p;
	

	_pitch >>= 1;

	for (y = 0 ; y < _height ; y++)
	{
		u16* line = buf;

		buf += _pitch;

		for (x = 0 ; x < _nbchunks ; x++)
		{
			for (p = 0 ; p < 4 ; p++)
			{
				if ( _planes[p] )
				{
					*dest++ = *line;
				}

				line++;
			}
		}
	}
}
