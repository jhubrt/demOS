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

#ifndef GRAFIKS_H
#define GRAFIKS_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\SYSTEM.H"

#include "EXTERN\WIZZCAT\PRTRKSTE.H"

#include "FX\MOD\MOD.H"

#include "RELAPSE\SRC\IDYNLIB.H"
#include "RELAPSE\RELAPSE2.H"

void GrafikSEntry	   (FSM* _fsm);
void GrafikSPostInit   (FSM* _fsm);
void GrafikSActivity   (FSM* _fsm);
void GrafikSBacktask   (FSM* _fsm);
void GrafikSExit	   (FSM* _fsm);

struct GrafikSInitParam_
{
    bool ismegaste;
    bool altmuziks;
};
typedef struct GrafikSInitParam_ GrafikSInitParam;

typedef void(*GrafikSWizInfoCall)(WIZinfo*);

struct GrafikSAsmImport_
{
    DYNinterface common;
    SYSinterupt vbl;
    u16         exit;
    void*       wizfrontbufptr;
    u8*         bufferec;
    u16*        colors3P;
    WIZplayerInfo wizInfo;
    GrafikSWizInfoCall wizinfoCall;
    u16*        periodColors;
    u8*         equalizerPeriods;
    u16         wait;
    u16         wait2;
    u8          starttimer;
    bool        haslooped;
};
typedef struct GrafikSAsmImport_ GrafikSAsmImport;


struct GrafikS_
{
    void* framebuffer;
    u8*   sndtrack;
    void* asmbuf;
    GrafikSAsmImport* asmimport;
    u16   colors3P[64];
    u16   periodColors[MOD_PERIOD_BUFFERSIZE];
    u8    equalizerPeriods[MOD_PERIOD_BUFFERSIZE];
};
typedef struct GrafikS_ GrafikS;

#endif
