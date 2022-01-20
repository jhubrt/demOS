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


#ifndef SLIDES_H
#define SLIDES_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"

void SlidesEntry	    (FSM* _fsm);
void SlidesInitActivity (FSM* _fsm);
void SlidesActivity	    (FSM* _fsm);
void SlidesExit		    (FSM* _fsm);
void SlidesBacktask     (FSM* _fsm);

STRUCT(Slides)
{
	RASinterupt rasterBootFunc;
	RASopVbl1	rasterBootOp;
	u8*		    framebuffers[4];
	u8*			bitmaps[2];
    u8*         palettes[4];
    u8*         greyscaler;
    u16*        gradients;
    s8          multable[17][32];
    u8*         morphcode;
    u32*        horitable;
    u16*        verttable;
    u16*        startpos;
    u16*        endpos;
    u16*        pos;
    u16*        points[5];
    u16*        pointspal[7];
    u16         pal[16];
    u16*        deployer[2];
    u32         palettecurrentflip;
    u8*         palettefliped;
    u8*         pointsgradient;
    volatile s8 pulse;
    s8          pulseInc;
};

#endif
