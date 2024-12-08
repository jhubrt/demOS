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


#ifndef EGYPTIA_H
#define EGYPTIA_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\SYSTEM.H"

#include "RELAPSE\SRC\IDYNLIB.H"

#define EGYPTIA_TUBE_H 24

void EgyptiaEntry	 (FSM* _fsm);
void EgyptiaPostInit (FSM* _fsm);
void EgyptiaActivity (FSM* _fsm);
void EgyptiaExit	 (FSM* _fsm);

struct EgyptiaInitParam_
{
    void* framebuffer;
    void* image;
    void* font;
    void* code;
    u16   screenmargin;
};
typedef struct EgyptiaInitParam_ EgyptiaInitParam;

typedef void (*EgyptCopyCall)(void* source, void* dest, u32 table, u16 height);

struct EgyptiaAsmImport_
{
    DYNinterface  common;
    SYSinterupt   vbl;
    EgyptCopyCall copycall;
    void*         rasters;
    s16           fade;
    void*         currentbackbuffer;
};
typedef struct EgyptiaAsmImport_ EgyptiaAsmImport;

struct Egyptia_
{
    void* framebuffer;
    u8*   image;
    u8*   imagebuffer;
    u8*   font;
    void* code;
    u16	  currentcolors[15];
    u16	  colors[2][16];
    u16   black[16];
    s16   fade;
    s16   fadecount;
    u8    shapecount;
    u8    scrollstate;
    s16   scroll;
    u16   scrollcount;
    u8*   sndtrack;
    void* asmbuf;
    EgyptiaAsmImport* asmimport;
    u16   tubeinc[EGYPTIA_TUBE_H];
};
typedef struct Egyptia_ Egyptia;

#endif
