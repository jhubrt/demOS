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

#ifndef SNDSHADE_H
#define SNDSHADE_H

#include "DEMOSDK/RASTERS.H"

#define VIS_WIDTH 48

u8*  SNSprecomputeCoordsZoom   (u8* p, s16 _mul, u16 _width, u16 _height);
u8*  SNSprecomputeCoordsRotate (s16* sin, u8* p, u8 orientation, s16 _mul, s16 _rot, u16 _width, u16 _height);

void SNSfadeGCode (void* _source, void* _table, void* _code, u16 _height, u32 _dest);

#ifndef __TOS__
void SNSPCdisplay(u16 _height);
#endif

typedef u16*(*SNSgenerateCodeCall)(u8* poff, u16* p, u16 _height);
typedef void(*SNSinitCall)        (void* _table, u16 _tcWidth, u16 _tcHeight);
typedef void(*SNSfadeCall)        (void* _source, void* _table, u32 _dest, s32 _pitch, u32 _count);
typedef void(*SNSfilsamplCall)    (void* _sample, void* _startcolors, u32 _dest, u16 _width, s16 _sampleoffset);
typedef void(*SNStcCall)          (void);

struct SNSASMimportTable_
{
    SNSgenerateCodeCall generateCodeCall;
    SNSinitCall         initCall;
    SNSfadeCall         fadeCall;
    SNSfadeCall         fade3Call;
    SNSfilsamplCall     fillsamplCall;
    RASinterupt         displayColorsInterupt;

    void**              lines;
    void*               table;
    void*               empty;

    u16                 reservedh;
    u16                 backcolor;
    u16                 reservedpitch;
    u16                 goblitter;
    u16                 tunX3;
    u16                 sampleinc;
    u16                 voice2;
    u16                 mask;

    u16                 pushregopcode;
    u16                 popregopcode;
    u16                 iregopcode;
    u16                 loadopcode;
    u16                 loadopcodereserved;
    u32                 storeopcode;

    u16                 reserved2;
    u16                 reserved3;

    void*               srcAdr;

    u16                 reserved4;
    u16                 reserved5;
    u16                 reserved6;

    u16                 reserved7;
    u16                 reserved8;

    void*               reserved9;
    
    u16                 xwidth;
    u16                 reserved10;
    u16                 hopop;
};
typedef struct SNSASMimportTable_ SNSASMimportTable;

#ifndef SNDSHADE_C
extern SNSASMimportTable SNSimportTable;
#endif

#endif

