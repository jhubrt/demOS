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


/*  FSM 
    
    Implements simple finite state machine.        
    FSM abstracts a list of states. Each state has an entry and exit action + activity routine
*/

#ifndef FSM_H
#define FSM_H

typedef void (*FSMfunction)(struct FSM_* _fsm);

struct FSMstate_
{
    FSMfunction func;

#   ifdef DEMOS_DEBUG
    char* name;
#   endif
};
typedef struct FSMstate_ FSMstate;

struct FSM_
{
	FSMstate*       states;
    u16             nbStates;
	u16	            activeState;

#   ifdef DEMOS_DEBUG
    char   name[8];
    char   trace[40];
    s16    traceCurrent;
    u16	   traceLastState;
#   endif
};
typedef struct FSM_ FSM;

void  FSMinit	(FSM* _m, FSMstate* _states, u16 _nbStates, u16 _startState, char* _name);
void  FSMgoto	(FSM* _m, u16 _newState);
s16   FSMlookForStateIndex(FSM* _m, FSMfunction _func);

#define FSMupdate(FSM)          (FSM)->states[(FSM)->activeState].func(FSM)
#define FSMgotoNextState(FSM)   FSMgoto((FSM),(FSM)->activeState+1)
#define	FSMgetCurrentState(FSM) (FSM)->activeState
#define FSMisLastState(FSM)     (((FSM)->activeState+1) == (FSM)->nbStates)

#ifdef DEMOS_DEBUG
#define FSM_STATE(FUNC) { FUNC, #FUNC }
u16 FSMtrace (FSM* _fsm, void* _image, u16 _pitch, u16 _planePitch, u16 _y);
#else
#define FSM_STATE(FUNC) { FUNC }
#endif

#endif
