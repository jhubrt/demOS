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
#include "SYNSOUND\SRC\SYNTH.H"

void SynthSoundEntry	(FSM* _fsm);
void SynthSoundActivity (FSM* _fsm);
void SynthSoundExit	    (FSM* _fsm);
void SynthSoundBacktask (FSM* _fsm);

struct SyntheticSound_
{
    void* framebuffer;
    void* pcm[2];
    s8*   pcmcopy;
    u8    channel;
    u8    volume[2];
    u8    keyb[2];
    s8    mask[2];
    u8    wtindex[2];
    u16   wavetable[16][16];
    u32   cyclicratio[2];
    u16   porta[2];

    SNDsquareSound squareSounds[2];
};
typedef struct SyntheticSound_ SyntheticSound;

#endif
