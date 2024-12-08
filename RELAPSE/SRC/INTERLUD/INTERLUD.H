/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2024 J.Hubert

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

#ifndef INTERLUD_H
#define INTERLUD_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\SYSTEM.H"

#include "RELAPSE\SRC\IDYNLIB.H"


#define INTERLUDE_BALLHEIGHT 128

void InterludeEntry	   (FSM* _fsm);
void InterludePostInit (FSM* _fsm);
void InterludeActivity (FSM* _fsm);
void InterludeExit	   (FSM* _fsm);

struct InterludeInitParam_
{
    void* textbuffer;
    void* text;
    void* font;
};
typedef struct InterludeInitParam_ InterludeInitParam;

typedef u8* (*SNDblitzDecodeYMCall)(u8* data);

struct InterludeAsmImport_
{
    DYNevent             init;
    SYSinterupt          vbl;
    SNDblitzDecodeYMCall ymdecode;
    DYNevent gencode;
    u16      tun1;
    u16      tun2;
    void*    framebuffer;
    u16*     colorsbuffer;
    u8*      briksaddresses;
    u8*      frame2nbwords;
    u32      plane3colors;
    u8*      flexisource;
    u8*      flexidest;
};
typedef struct InterludeAsmImport_ InterludeAsmImport;

struct InterludeGen_
{
    u16  run;
    u16  lost;

    s16  xs;
    s16  ys;
    s16  xd;
    s16  yd;

    u16  offset;
    u8   bit;
};
typedef struct InterludeGen_ InterludeGen;

struct Interlude_
{
    u8*   alignedbuffer;
    u8*   framebuffer;
    u8*   text;
    u16	  colors[16];
    u16   black[16];
    u16*  briks;
    u8*   briksmoveleft;
    u8*   briksmoveright;
    u8*   frame2ndwords;
    s8*   sample;
    u16   samplesize;
    u16*  gencode;
    void* asmbuf;
    u8*   sounds[2];
    u16   soundssize[2];
    InterludeAsmImport* asmimport;
    u8    flip;
    u16   scrollpos;
    u16   scrollen;
    u16   ballx;
    s16   ballincx;
    u16   bally;
    s16   ballincy;
    u16   rotationcount;
    u32   rotationindex;
    u16   tun2;
    u8    nbsteps;
    u8    shokcount;
    u8*   shokadr;
    u8*   soundcurrent;
    u8*   soundend;
    u16   soundplayframe;
    u16   framecount;
    s16   sin[512+128];
    InterludeGen gen;
};
typedef struct Interlude_ Interlude;

#endif
