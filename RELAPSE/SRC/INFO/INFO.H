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

#ifndef INFO_H
#define INFO_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\SYSTEM.H"

#include "RELAPSE\SRC\IDYNLIB.H"

void InfoEntry	  (FSM* _fsm);
void InfoPostInit (FSM* _fsm);
void InfoActivity (FSM* _fsm);
void InfoExit	  (FSM* _fsm);

struct InfoInitParam_
{
    void* textbuffer;
    void* text;
    void* font;
};
typedef struct InfoInitParam_ InfoInitParam;

struct InfoAsmImport_
{
    DYNevent     init;
    DYNevent     display;
    SYSinterupt  vbl;
};
typedef struct InfoAsmImport_ InfoAsmImport;

struct Info_
{
    u8*   font;
    u8*   textbuffer;
    u8*   text;
    u16	  colors[16];
    u16   black[16];
    u8*   sndtrack;
    void* asmbuf;
    InfoAsmImport* asmimport;
    u8    flip;
    s16   targetline;
    s16   displayedline;
    u16   nblines;
};
typedef struct Info_ Info;

#endif
