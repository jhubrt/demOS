/*------------------------------------------------------------------------------  -----------------
  The MIT License (MIT)

  Copyright (c) 2015-2018 J.Hubert

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
------------------------------------------------------------------------------------------------- */

#ifndef VISUALIZ_H
#define VISUALIZ_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"

void VisualizerEntry	(FSM* _fsm);
void VisualizerActivity	(FSM* _fsm);
void VisualizerExit		(FSM* _fsm);
void VisualizerBacktask (FSM* _fsm);
void VisualizerEntryFast(FSM* _fsm);

STRUCT(Visualizer)
{
	u16*	    tcbuffers[2];
    u8*         opAddBuffer;
    u8*         opAdd;
    u16*        startcolors;
	u16*        startcolflash;
	u16*        startflash[2];
	void*	    bitbuf;
	u8*		    bitmaps;
    u16*        animations;
    u16*        routines[3];
	RASinterupt rasterBootFunc;
	RASopVbl1	rasterBootOp;
    u16*        colors;
    u16         nbcolors;
	u16 	    colorscroll;
    u16         lastcount;
    s16         timmax;
    u16*        currentanimation;
    u16*        animateto;
    u16         animationframe;
    u16         animationframescount;
    bool        displaysample;
    u8		    lvol;
    u8		    rvol;
	u8		    trackcounter;
    bool        beat;
    u16         entryClientStep;
    u16         multable[409];  /* 65536UL / 160UL */
};

#endif
