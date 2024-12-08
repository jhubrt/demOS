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


#ifndef LIQUID_H
#define LIQUID_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"

#include "RELAPSE\SRC\IDYNLIB.H"

void LiquidEntry	(FSM* _fsm);
void LiquidPostInit (FSM* _fsm);
void LiquidActivity	(FSM* _fsm);
void LiquidExit		(FSM* _fsm);

struct LiquidInitParam_
{
    void* framebuffer;
    void* waterdata;
    void* font;
};
typedef struct LiquidInitParam_ LiquidInitParam;

struct LiquidAsmImport_
{
    DYNinterface common;
    DYNevent     vbl;
    bool         exitflag;
};
typedef struct LiquidAsmImport_ LiquidAsmImport;

struct Liquid_
{
    void* framebuffer;
    void* water;  
    void* font;
    void* asmbuf;
    u8*   sndtrack;
    u8    endcount;
    u16   framecount;

    LiquidAsmImport* asmimport;
};
typedef struct Liquid_ Liquid;

#endif
