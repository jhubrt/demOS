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

#ifndef END_H
#define END_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\SYSTEM.H"

#include "RELAPSE\SRC\IDYNLIB.H"
#include "RELAPSE\RELAPSE2.H"

#define END_NBIMAGES (RSC_RELAPSE2_METADATA_END_THE_END_ARJX-RSC_RELAPSE2_METADATA_END_INTRO_ARJX+1)

void EndEntry	   (FSM* _fsm);
void EndPostInit   (FSM* _fsm);
void EndActivity   (FSM* _fsm);
void EndBacktask   (FSM* _fsm);
void EndExit	   (FSM* _fsm);

struct EndInitParam_
{
    void* framebuffer;
    void* zoom;
    GrafikSWizInfoCall wizinfoCall;
    void* pictures[END_NBIMAGES];
};
typedef struct EndInitParam_ EndInitParam;

struct EndAsmImport_
{
    DYNinterface  common;
    u16           exitflag;
};
typedef struct EndAsmImport_ EndAsmImport;


struct End_
{
    void* framebuffer;
    u8*   zoom;
    u8*   images[END_NBIMAGES];
    u16   black[16];
    s16   fadecount;
    u8*   sndtrack;
    void* asmbuf;
    EndAsmImport* asmimport;
};
typedef struct End_ End;

#endif
