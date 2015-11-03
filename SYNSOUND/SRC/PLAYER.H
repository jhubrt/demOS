/*------------------------------------------------------------------------------  -----------------
  Copyright J.Hubert 2015

  This file is part of relapse HD demo

  relapse HD demo is free software: you can redistribute it and/or modify it under the terms of 
  the GNU Lesser General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  relapse HD demo is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY ;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with relapse HD
  demo.  
  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------- */

#ifndef PLAYER_H
#define PLAYER_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"

void IntroEntry	   (FSM* _fsm);
void IntroActivity (FSM* _fsm);
void IntroExit	   (FSM* _fsm);
void IntroBacktask (FSM* _fsm);

STRUCT(Intro)
{
    void* framebuffer;
    void* pcm[2];
    s8*   pcmcopy;
    u8    channel;
    u8    volume;
    u8    keyb[2];
    s8    mask[2];
};

#endif
