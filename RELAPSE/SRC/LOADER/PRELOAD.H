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

#ifndef PRELOAD_H
#define PRELOAD_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\LOADDEFS.H"

void PreLoadEntry	    (FSM* _fsm);
void PreLoadActivity	(FSM* _fsm);
void PreLoadExit		(FSM* _fsm);

#define PRELOAD_NBPLANES 4
#define PRELOAD_NBCOLORS 16

struct PreLoad_
{
    u8* framebuffer;

    u8  lastprogress;
    u16 lastnbsectors;
    u32 lastdpakprogress;

    u16 nbresources;
    u16 y;
    u8  printcolor;

    u16 gradient[17];
    s16 counter[PRELOAD_NBCOLORS / 2 - 1];
    s16 inc[PRELOAD_NBCOLORS / 2 - 1];

    u16 black[16];
    char text[16];

    u16 line[8*160/sizeof(u16)];

    u16 temp[LOAD_SECTORSIZE / sizeof(u16)];

#   if RELAPSE_DEV() 
    u16 nbframes;
    u16 lastsec;
#   endif
};
typedef struct PreLoad_ PreLoad;

#endif
