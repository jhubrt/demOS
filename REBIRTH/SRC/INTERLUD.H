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
