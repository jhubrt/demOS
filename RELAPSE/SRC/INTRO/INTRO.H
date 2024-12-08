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


#ifndef INTRO_H
#define INTRO_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\RASTERS.H"

#include "RELAPSE\SRC\IDYNLIB.H"

void IntroEntry	   (FSM* _fsm);
void IntroPostInit (FSM* _fsm); 
void IntroActivity (FSM* _fsm);
void IntroExit	   (FSM* _fsm);
void IntroBacktask (FSM* _fsm);

#define INTRO_NBSPRITES 7

struct Intro_
{
    u8*   buffer;
    u8*   framebuffers[2];
    u8*   backbuffer;
    u8*   anim3d;
    u32   anim3dsize;
    u8*   logo;
    u8*   title;
    u16	  colors[15];
    u16   black[16];
    u8*   sndtrack;
    s32   x    [INTRO_NBSPRITES];
    s32   y    [INTRO_NBSPRITES];
    s16   xend [INTRO_NBSPRITES];
    s16   yend [INTRO_NBSPRITES];
    s32   dx   [INTRO_NBSPRITES];
    s32   dy   [INTRO_NBSPRITES];
    u16   count;
    u32   anim3doffset;
    u8    flip;
    u8    state;
    u16   pal[4][16];
    
    RASopVbl16 vbl;
    u16        rastersbuffer[512];
};
typedef struct Intro_ Intro;

#endif
