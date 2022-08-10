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

#ifndef SSHADE_H
#define SSHADE_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"

void SShadeInitStatic   (FSM* _fsm);
void SShadeEnter	    (FSM* _fsm);
void SShadeActivity	    (FSM* _fsm);
void SShadeExit		    (FSM* _fsm);


struct SShadeStatic_
{
    /* Loaded */
    u16*    colors;
    u16     nbcolors;

    /* Precomputed */
    u16*    routines[3];
};
typedef struct SShadeStatic_ SShadeStatic;

struct SShade_
{
	u16*	     tcbuffers[2];
    u8*          opAddBuffer;
    u8*          opAdd;
    u16*         startcolors;
	u16*         startcolflash;
	u16*         startflash[2];
	u8*	         bitbuf;
	u8*		     bitmaps;
    u8*          empty;
    u16*         routines[3];
	RASinterupt  rasterBootFunc;
	RASopVbl1	 rasterBootOp;
                
	u16 	     colorscroll;
    u16 	     colorscroll2;
    
    bool         displaysample;                
    bool         displayverticalsample;
    u8           flip;
    u8           fadetype;
    u8           voice;
    u8           flash;
    u8           rotationcounter;
                
    u16          lines[200];
    u16          table[256];
};
typedef struct SShade_ SShade;

#endif
