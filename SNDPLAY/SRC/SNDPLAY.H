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


#ifndef BPLAYER_H
#define BPLAYER_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"

void PlayerEntry    (void);
void PlayerTest     (u8 _mode);

void PlayerActivity (void);
void PlayerExit	    (void);
void PlayerBacktask (void);

void PlayerSetParam (int argc, char** argv);


struct PlayerText_
{
    u8    state;
    u8    currentpos;
    u8    currentline;
    s16   animatecount;
};
typedef struct PlayerText_ PlayerText;


struct Player_
{
    PlayerText          text;
    
    void* framebuffer;
    u8*   data;
    u32   datasize;
    u8    testMode;
    char  filename[256];
    u32   dataindex;

    u16   playframe;
    bool  play;
    bool  run;
    bool  ymemuldisplaysync;

    u8    panel;
    u8    lastpanel;

	s32	  allocatedbytes;
};
typedef struct Player_ Player;

#ifndef SNDPLAY_C
extern 
#endif
Player g_player;

#endif
