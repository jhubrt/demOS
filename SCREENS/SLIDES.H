/*------------------------------------------------------------------------------  -----------------
  Copyright J.Hubert 2015

  This file is part of rebirth demo

  rebirth demo is free software: you can redistribute it and/or modify it under the terms of 
  the GNU Lesser General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  rebirth demo is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY ;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with rebirth demo.  
  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------- */

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
