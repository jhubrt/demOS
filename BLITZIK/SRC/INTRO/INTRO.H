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


#ifndef POLYZOOM_H
#define POLYZOOM_H

#include "DEMOSDK\FSM.H"

#include "FX\VECTOR\VECTOR.H"
#include "FX\COLPLANE\COLPLANE.H"

void CybervectorEnter 		(FSM* _fsm);
void CybervectorActivity	(FSM* _fsm);
void CybervectorActivity2   (FSM* _fsm);
void CybervectorExit		(FSM* _fsm);
void CybervectorBacktask	(FSM* _fsm);


struct PZanimationState_
{
    VECanimationState animState;

    s16 ic;
    s16 icx;   
    u8  state;
    s16 count;
};
typedef struct PZanimationState_ PZanimationState;

#define PZ_NBSCENES 6

struct Cybervector_
{
	VECscene                scenes[PZ_NBSCENES];
    VECanimationCallback    animationCallback[PZ_NBSCENES];
    PZanimationState        animationState[PZ_NBSCENES];
    u8                      colormodes[PZ_NBSCENES];
    COLPanimation           coloranimation;

    u16     *colors[4];
    u16     black[15];

	u8*	    framebuffers[2];
    u16*    erase[8];

    u16*	displist;
	u16*    displistp;
    u16*    polylinestmp;

	u16	    currentFrame;
    u16     currentScene;
    u8      repeatcount;
    u8      cycling;
    u16     cyclingcolors[15];

    u8      scene0State;
    u8      scene0FXframe;

    bool    exit;
};
typedef struct Cybervector_ Cybervector;

#endif
