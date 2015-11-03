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

#ifndef INTERLUD_H
#define INTERLUD_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"

#define INTERLUDE_MAX_MESSAGES 16
#define INTERLUDE_GRADIENTLEN  26

void InterludeEntry		(FSM* _fsm);
void InterludeActivity	(FSM* _fsm);
void InterludeExit		(FSM* _fsm);
void InterludeBacktask	(FSM* _fsm);

STRUCT(InterludeText)
{
    u8      y;
    u8      col;
    char*   text;
};

STRUCT(InterludeMessage)
{
    InterludeText* texts;
    u16            nbTexts;
};

STRUCT(Interlude)
{
    s16                 index;
	u8*		            framebuffers[2];
    u8*                 morphcode;
    u32*                horitable;
    u16*                verttable;
    u16*                startpos;
    u16*                endpos;
    u16*                pos;
	u16*		        printbuffer;
    u8*                 rasters;   
    u16                 gradient[INTERLUDE_GRADIENTLEN];          
    InterludeMessage*   sequence;
    u8                  sequenceLen;
	volatile u8	        animationStep;
    u16*                rastersColors[INTERLUDE_MAX_MESSAGES];
    s16                 gradientIndex[INTERLUDE_MAX_MESSAGES];
};

#endif
