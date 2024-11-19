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


#ifndef WIZPLAY_H
#define WIZPLAY_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"

#include "EXTERN\WIZZCAT\PRTRKSTE.H"

void PlayerEntry    (void);

void PlayerActivity (FSM* _fsm);
void PlayerExit	    (FSM* _fsm);
void PlayerBacktask (FSM* _fsm);

void PlayerSetParam (int argc, char** argv);

struct Player_
{
    u8*   modulebuffer;
    u8*   modulebufferend;

    void* framebuffer;
    s8*   pcmcopy;
    char  filename[256];
    u16   rastermax;

    bool  play;

    u8    panel;
    u8    lastpanel;
    
    WIZplayerInfo playerInfo;
};
typedef struct Player_ Player;

#endif
