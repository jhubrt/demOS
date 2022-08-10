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


#ifndef PLAYER_H
#define PLAYER_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"
#include "DEMOSDK\SYNTHYM.H"

void SynthYMEntry	 (void);
void SynthYMActivity (void);
void SynthYMBacktask (void);


struct PlayerControl_
{
    u8      currentchannel;
    bool    channels[SND_YM_NB_CHANNELS];
    s8      keyb;
    u8      pressed[SND_YM_NB_CHANNELS];
    s8      transpose[SND_YM_NB_CHANNELS];
};
typedef struct PlayerControl_ PlayerControl;

struct Player_
{
    SNDYMsoundSet     soundSet;
    SNDYMplayer       player;
    PlayerControl     ctrl;
    void*             framebuffer;
    bool              flip;
    bool              curvesync;
    u32               framenum;
    u8                keyboard[256];
    bool              updated;
    char              filename[256];
};
typedef struct Player_ Player;

#ifndef PLAYER_C
extern Player       g_player;
extern FSM          g_stateMachine;
extern FSM          g_stateMachineIdle;
extern FSMfunction  states[];
extern FSMfunction  statesIdle[];
extern u16          statesSize;
extern u16          statesIdleSize;
#endif

#endif
