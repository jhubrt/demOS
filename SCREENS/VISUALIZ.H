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
