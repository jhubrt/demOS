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


#ifndef SHADE_H
#define SHADE_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\SYSTEM.H"

#include "RELAPSE\SRC\IDYNLIB.H"

void ShadeVAEntry	 (FSM* _fsm);
void ShadeVAPostInit (FSM* _fsm);
void ShadeVAActivity (FSM* _fsm);
void ShadeVAExit	 (FSM* _fsm);

struct ShadeVAInitParam_
{
    void* framebuffer;
    void* colorsdest;
};
typedef struct ShadeVAInitParam_ ShadeVAInitParam;

struct ShadeVAAsmImport_
{
    DYNinterface  common;
    SYSinterupt   display;
    SYSinterupt   erase;
};
typedef struct ShadeVAAsmImport_ ShadeVAAsmImport;

struct ShadeVA_
{
    void* framebuffer;
    u8*   sndtrack;
    void* asmbuf;
    ShadeVAAsmImport* asmimport;
    RASopVbl16 vbl;
    u16        rasterend[10];
};
typedef struct ShadeVA_ ShadeVA;

#endif
