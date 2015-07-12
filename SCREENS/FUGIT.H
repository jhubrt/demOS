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

#ifndef FUGIT_H
#define FUGIT_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"

void FugitEntry	    (FSM* _fsm);
void FugitActivity	(FSM* _fsm);
void FugitExit		(FSM* _fsm);
void FugitBacktask  (FSM* _fsm);

STRUCT(FugitRasters)
{
    RASinterupt     bootFunc;
	RASopVbl1	    bootOp;
    RASopTop1	    topOp;
    u16             colors[97];
    RASopMid1       midLastOp;
};

STRUCT(Fugit)
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

    FugitRasters rasters[2];
};

#endif
