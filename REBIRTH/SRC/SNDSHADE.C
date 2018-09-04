/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2018 J.Hubert

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

#define SNDSHADE_C

#include "DEMOSDK\HARDWARE.H"

#include "FX\SNDSHADE\SNDSHADE.H"

ASMIMPORT u16 SNScolor;
ASMIMPORT u32 SNStrace;

ASMIMPORT u16 SNStunX1;
ASMIMPORT u16 SNStunX2;
ASMIMPORT u16 SNStunX3;
ASMIMPORT u16 SNStunX4;

ASMIMPORT u16 SNSireg;
ASMIMPORT u16 SNSload;
ASMIMPORT u32 SNSstore;

ASMIMPORT void* SNSlines;

void SNSinit     (void* _tcbuf, void* _lines, u16 _tcWidth, u16 _tcHeight) PCSTUB;
void SNStc       (void) PCSTUB;
void SNSfade3	 (void* _source, void* _table, u32 _dest, s32 _pitch, u32 _count) PCSTUB;
void SNScopy     (void* _source, void* _dest, u16 _nbwords) PCSTUB;

#ifndef __TOS__

void SNSfade (void* _source, void* _table, u32 _dest, s32 _pitch, u32 _count)
{
    u16* m  = (u16*) _dest;
    s16* m2 = (s16*) _source;
    u16 t, i;


    _pitch >>= 1;

    for (t = 0 ; t < (u16)_count ; t++)
    {
        for (i = 0 ; i < VIS_WIDTH; i++)
        {
            *m = *(u16*)((u8*)_table + *m2);
            m++;
            m2++;
        }

        m  += _pitch;
        m2 += _pitch;
    }
}

u16 SNSfilsampl (void* _sample, void* _startcolors, u32 _dest, u16 _width, s16 _sampleoffset)
{
    s8* p = (s8*)_sample;
    u16 i;
    u16* m = (u16*) _dest;
    u16* startcolors = (u16*) _startcolors;
    u16 total = 0;


    for (i = 0 ; i < _width ; i++)
    {
        s8   sample = *p;
        s16  t;
        u16* pix = (u16*) m;
        u16 color = startcolors[i];


        if ( sample < 0 )
        {
            sample = -sample;
            sample >>= 3;

            total += sample;

            for (t = 0 ; t < sample ; t++)
            {
                *pix = color;
                pix += VIS_WIDTH;
            }
        }
        else
        {
            sample >>= 3;

            total += sample;

            for (t = 0 ; t < sample ; t++)
            {
                *pix = color;
                pix -= VIS_WIDTH;
            }
        }

        p += _sampleoffset;
        m += 2;
    }

    return total;
}

#endif
