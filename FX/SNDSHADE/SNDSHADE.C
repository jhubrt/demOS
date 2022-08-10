/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2022 J.Hubert

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

#define SNDSHADE_C

#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\CODEGEN.H"
#include "DEMOSDK\PC\EMUL.H"
#include "DEMOSDK\PC\WINDOW.H"

#include "FX\SNDSHADE\SNDSHADE.H"

#ifdef __TOS__
extern SNSASMimportTable SNSimportTable;
#else
static u16* SNSgenerateCode (u8* poff, u16* p, u16 _height );
static void SNSinit         (void* _table, u16 _tcWidth, u16 _tcHeight);
static void SNSfade         (void* _source, void* _table, u32 _dest, s32 _pitch, u32 _count);
static void SNSfade3        (void* _source, void* _table, u32 _dest, s32 _pitch, u32 _count);
static void SNSfilsampl (void* _sample, void* _startcolors, u32 _dest, u16 _width, s16 _sampleoffset);

SNSASMimportTable SNSimportTable = 
{
    SNSgenerateCode,
    SNSinit, 
    SNSfade,
    SNSfade3,
    SNSfilsampl,
    NULL,           // displayColorsInterupt

    NULL,           // lines
    NULL,           // table
    NULL,           // empty
       
    0,              // reservedh
    1,
    0,              // pitch
    0,              // goblitter
    0,              // tunX3
    0,              // sampleinc
    0,              // voice2
    0xFFFF,         // mask
    
    0,              // pushregopcode
    0,              // popregopcode
    0,              // iregopcode
    0,              // loadopcode
    0,
    0,              // storeopcode
    
    0,              // reserved2
    0,              // reserved3

    NULL,           // srcAdr

    0,              // reserved4
    0,              // reserved5
    0,              // reserved6
    0,              // reserved7
    0,              // reserved8
    
    NULL,           // reserved9

    0,              // xwidth
    0,              // reserved10
    0               // hopop
};
#endif

void SNStc (void) PCSTUB;

void SNSinit(void* _table, u16 _tcWidth, u16 _tcHeight)
{
    SNSimportTable.table     = _table;
    SNSimportTable.xwidth    = _tcWidth;
    SNSimportTable.reservedh = _tcHeight;
}

u8* SNSprecomputeCoordsZoom(u8* p, s16 _mul, u16 _width, u16 _height)
{
    u32  w;
    u8   count = 0;

    {
        s16 x, y;
        s16 halfw        = _width >> 1;
        s16 halfh        = _height >> 1;
        s16 destoffindex = 0;

    
        for (y = -halfh ; y < halfh ; y++)
        {
            s16 srcy = y * _mul;

            /* srcy /= 64; */
            srcy >>= 6;
            if (srcy < 0)
                srcy++;

            srcy += halfh;
           
            for (x = -halfw ; x < halfw ; x++)
            {
                s16 srcoffsetindex, doffset;
                s16 srcx = x * 61;

                /*srcx /= 64;*/

                srcx >>= 6;
                if (srcx < 0)
                    srcx++;

                srcx += halfw;

                srcoffsetindex = srcx + (srcy * VIS_WIDTH);
                doffset = destoffindex - srcoffsetindex;

                ASSERT(doffset <   1024);                
                ASSERT(doffset >= -1024);

                switch (count)
                {
                case 0:
                    w = doffset & 0x3FF;
                    count = 1;
                    break;
                case 1:
                    w |= ((u32) doffset << 10) & 0xFFC00UL;
                    count = 2;
                    break;
                case 2:
                    w |= ((u32) doffset << 22) & 0xFFC00000UL;
                    count = 0;
                    *(u32*)p = w;
                    p += 4;
                    break;
                }

                destoffindex++;
            }
        }

        switch (count)
        {
        case 1:
        case 2:
            *(u32*)p = w;
            p += 4;
            break;
        }
    }
   
    return p;
}


u8* SNSprecomputeCoordsRotate(s16* sin, u8* p, u8 orientation, s16 _mul, s16 _rot, u16 _width, u16 _height)
{
    s16* cos = sin + 128;
    u32  w;
    u8   count = 0;
    s16  cs;
    s16  sn;
    s16  x, y;
    s16  halfw = _width >> 1;
    s16  halfh = _height >> 1;
    s16  destoffindex = 0; /* offset / 2 */


    count = 0;

    if (orientation == 0)
    {
        cs = cos[_rot];
        sn = sin[_rot];
    }
    else
    {
        cs = cos[512 - _rot];
        sn = sin[512 - _rot];
    }

    cs = PCENDIANSWAP16 ( cs );
    sn = PCENDIANSWAP16 ( sn );

    for (y = -halfh ; y < halfh ; y++)
    {
        for (x = -halfw ; x < halfw ; x++)
        {
            s32 x2, y2;
            u16 srcx, srcy;
            s16 srcoffsetindex, doffset;


            x2  = STDmuls(x , cs);
            x2 -= STDmuls(y , sn);
            y2  = STDmuls(x , sn);
            y2 += STDmuls(y , cs);

            x2 += 16384;
            y2 += 16384;

            x2 >>= 6;
            y2 >>= 6;

            x2 = STDmuls((s16)x2, _mul);
            y2 = STDmuls((s16)y2, _mul);

            x2 >>= 15;
            y2 >>= 15;

            x2 += halfw;
            y2 += halfh;

            if ( x2 < 0 )
                x2 = 0;

            if ( x2 >= _width )
                x2 = _width - 1;

            if ( y2 < 0 )
                y2 = 0;

            if ( y2 >= _height)
                y2 = _height - 1;

            srcx = (u16) x2;
            srcy = (u16) y2;

            srcoffsetindex = srcx + (srcy * _width);
            doffset = destoffindex - srcoffsetindex;

            ASSERT(doffset <   1024);                
            ASSERT(doffset >= -1024);

            switch (count)
            {
            case 0:
                w = doffset & 0x3FF;
                count = 1;
                break;
            case 1:
                w |= ((u32) doffset << 10) & 0xFFC00UL;
                count = 2;
                break;
            case 2:
                w |= ((u32) doffset << 22) & 0xFFC00000UL;
                count = 0;
                *(u32*)p = w;
                p += 4;
                break;
            }

            destoffindex++;

#           ifndef __TOS__
            if (orientation)
            {
                WINsetColor  ( EMULgetWindow(), 128 + (rand() & 127), 128 + (rand() & 127), 128 + (rand() & 127));
                WINrectangle ( EMULgetWindow(), srcx * 10 - 1, srcy * 10 - 1, srcx * 10 + 1, srcy * 10 + 1);
                WINline ( EMULgetWindow(), srcx * 10, srcy * 10, (x + halfw) * 10, (y + halfh) * 10);
                //WINrender ( EMULgetWindow() );
            }
#           endif
        }
    }

#   ifndef __TOS__
    WINrender ( EMULgetWindow() );
#   endif

    switch (count)
    {
    case 1:
    case 2:
        *(u32*)p = w;
        p += 4;
        break;
    }

    return p;
}



#ifndef __TOS__

#define SNS_WIDTH   48

static u16* SNSgenerateCode ( u8* poff, u16* p, u16 _height )
{
    u16 i;
    s16 destoffindex = 0;
    u8  count = 0;
    s16 offset;
    s16 instructionoffset;
    u32 w;
    u16 nbloops = _height * SNS_WIDTH;


    *p++ = SNSimportTable.pushregopcode;
    *p++ = SNSimportTable.iregopcode;

    for (i = 0 ; i < nbloops ; i++ )
    {
        switch (count)
        {
        case 0:
            w = *(u32*) poff;
            poff += 4;
            offset = (s16)(w << 6);
            offset >>= 6;
            count = 1;
            break;
        case 1:
            offset = (s16)(w >> 4);
            offset >>= 6;
            count = 2;
            break;
        case 2:
            offset = (s16)(w >> 16);
            offset >>= 6;
            count = 0;
            break;
        }

        instructionoffset = (destoffindex - offset ) << 1;

        *p++ = SNSimportTable.loadopcode;
        *p++ = instructionoffset;
        *((u32*)p) = SNSimportTable.storeopcode;
        p += 2;

        destoffindex++;
    }

    *p++ = SNSimportTable.popregopcode;
    *p++ = CGEN_OPCODE_RTS;

    return p;
}

static void SNSfade (void* _source, void* _table, u32 _dest, s32 _pitch, u32 _count)
{
    u16* m  = (u16*) _dest;
    s16* m2 = (s16*) _source;
    u16 t, i;


    _pitch >>= 1;

    for (t = 0 ; t < (u16)_count ; t++)
    {
        for (i = 0 ; i < VIS_WIDTH; i++)
        {
            s16 input = *m2;
            ASSERT((input & 1) == 0);
            *m = *(u16*)((u8*)_table + input);
            m++;
            m2++;
        }

        m  += _pitch;
        m2 += _pitch;
    }
}

void SNSfade3 (void* _source, void* _table, u32 _dest, s32 _pitch, u32 _count)
{
    u16* m  = (u16*) _dest;
    s16* m2 = (s16*) _source;
    u16 t, i;


    _pitch >>= 1;

    for (t = 0 ; t < (u16)_count ; t++)
    {
        for (i = 0 ; i < (VIS_WIDTH / 2); i++)
        {
            s16 input = *m2;
            ASSERT((input & 1) == 0);
            *m = *(u16*)((u8*)_table + input);
            m++;
            m2++;
        }

        m  += _pitch + (VIS_WIDTH / 2);
        m2 += _pitch + (VIS_WIDTH / 2);
    }
}

void SNSfadeGCode (void* _source, void* _table, void* _code, u16 _height, u32 _dest)
{
    s16 x, y;
    u16* p = (u16*) _code;
    u16* source = (u16*) _source;
    u16* dest   = (u16*) _dest;
    s16* table  = (s16*) _table;


    p += 2;

    for (y = 0 ; y < _height ; y++)
    {
        for (x = 0 ; x < VIS_WIDTH ; x++)
        {
            u16  offset;
            s16  oldcol;
            s16  newcol;

            p++;        //  = SNSload;
            offset = *p++;
            p += 2;     // SNSstore

            offset >>= 1;

            ASSERT(offset < (VIS_WIDTH * _height));

            oldcol = source[offset];
            ASSERT((oldcol & 1) == 0);
            newcol = table[oldcol >> 1];

            *dest++ = newcol;
        }
    }
}


void SNSfilsampl (void* _sample, void* _startcolors, u32 _dest, u16 _width, s16 _sampleoffset)
{
    s8* p = (s8*)_sample;
    u16 i;
    u16* m = (u16*) _dest;
    u16* startcolors = (u16*) _startcolors;


    for (i = 0 ; i < _width ; i++)
    {
        s16  sample = *p;
        s16  t;
        u16* pix = (u16*) m;
        u16 color = startcolors[i];


        if ( sample < 0 )
        {
            sample = -sample;
            sample >>= 3;

            for (t = 0 ; t < sample ; t++)
            {
                *pix = color;
                pix += VIS_WIDTH;
            }
        }
        else
        {
            sample >>= 3;

            for (t = 0 ; t < sample ; t++)
            {
                *pix = color;
                pix -= VIS_WIDTH;
            }
        }

        p += _sampleoffset;
        m ++;
    }
}
 

void SNSPCdisplay(u16 _height)
{
    u16* p = (u16*) SNSimportTable.srcAdr;
    u16 x,y;

    EMULfbExStart(HW_VIDEO_MODE_2P, 32, 63, 32 + 319, 63 + 199, 160, 0);

    for (y = 0 ; y < _height ; y++)
    {
        u16 j;

        for (j = 0 ; j < 4 ; j++)
        {
            for (x = 0 ; x < VIS_WIDTH ; x++)
            {
                EMULfbExSetColor( EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 32 + x * 8, 63 + y * 4 + j), 0, PCENDIANSWAP16(p[x]));
            } 

            EMULfbExSetColor( EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 32 + x * 8, 63 + y * 4 + j), 0, 0);
        }

        p += VIS_WIDTH;
    }

    EMULfbExEnd();
}

#endif
