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


#ifndef FUGIT_H
#define FUGIT_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"

void FugitEntry	    (FSM* _fsm);
void FugitActivity	(FSM* _fsm);
void FugitExit		(FSM* _fsm);
void FugitBacktask  (FSM* _fsm);

struct FugitRasters_
{
	RASopVbl1	    bootOp;
    RASopTop1	    topOp;
    u16             colors[97];
    RASopMid1       midLastOp;
};
typedef struct FugitRasters_ FugitRasters;

struct Fugit_
{
    u8* framebuffers[2];
    u8* auxbuffer;
    u8* deltapacked;
    
    u8* font;
    u8* fontscan;
    u8* fontoffset;

    u8* asmbuf;

    u16 currentframebuffer;
    u16 currentrasterbuffer;

    volatile u8* currentdeltabuffer;

    RASinterupt  bootFunc;
    FugitRasters rasters[2];
};
typedef struct Fugit_ Fugit;

#endif
