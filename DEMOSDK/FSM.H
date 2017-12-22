/*------------------------------------------------------------------------------  -----------------
  The MIT License (MIT)

  Copyright (c) 2015-2017 J.Hubert

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
------------------------------------------------------------------------------------------------- */

/*! @brief @ref FSM @file */

/*! @defgroup  FSM 
    
    Implements simple finite state machine.        
    FSM abstracts a list of states. Each state has an entry and exit action + activity routine
*/

#ifndef FSM_H
#define FSM_H

#include "DEMOSDK\BASTYPES.H"

typedef struct FSM_ FSM;
typedef void (*FSMfunction)(FSM* _fsm);

struct FSM_
{
	FSMfunction*    states;
    u16    nbStates;
	u16	   activeState;

#   ifdef DEMOS_DEBUG
    char   trace[40];
    s16    traceCurrent;
    u16	   traceLastState;
#   endif
};

void  FSMinit	(FSM* _m, FSMfunction* _states, u16 _nbStates, u16 _startState);
void  FSMgoto		(FSM* _m, u16 _newState);

#define FSMupdate(FSM)          (FSM)->states[(FSM)->activeState](FSM)
#define FSMgotoNextState(FSM)   FSMgoto((FSM),(FSM)->activeState+1)
#define	FSMgetCurrentState(FSM) (FSM)->activeState
#define FSMisLastState(FSM)     (((FSM)->activeState+1) == (FSM)->nbStates)

#ifdef DEMOS_DEBUG
u16 FSMtrace (FSM* _fsm, void* _image, u16 _pitch, u16 _planePitch, u16 _y);
#endif

#endif
