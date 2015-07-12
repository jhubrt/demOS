/*------------------------------------------------------------------------------  -----------------
  Copyright J.Hubert 2015

  This file is part of demOS

  demOS is free software: you can redistribute it and/or modify it under the terms of 
  the GNU Lesser General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  demOS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY ;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with demOS.  
  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------- */

/*! @brief @ref FSM @file */

/*! @defgroup  FSM 
    
    Implements simple finite state machine.        
    FSM abstracts a list of states. Each state has an entry and exit action + activity routine
*/

#ifndef FSM_H
#define FSM_H

#include "DEMOSDK\BASTYPES.H"

STRUCT(FSM)
{
	struct FSMstate_* states;
    u16    nbStates;
	u16	   activeState;

#   ifdef DEMOS_DEBUG
    char   trace[40];
    s16    traceCurrent;
    u16	   traceLastState;
#   endif
};

typedef void (*FSMfunction)(FSM* _fsm);

STRUCT(FSMstate)
{
	FSMfunction entryAction;
	FSMfunction activity;
	FSMfunction exitAction;
};

void  FSMinit		 (FSM* _m, FSMstate* _states, u16 _nbStates, u16 _startState);
void  FSMupdate		(FSM* _m);
void  FSMgoto		(FSM* _m, u16 _newState);

#define FSMgotoNextState(FSM)   FSMgoto((FSM),(FSM)->activeState+1)
#define	FSMgetCurrentState(FSM) (FSM)->activeState
#define FSMisLastState(FSM)     (((FSM)->activeState+1) == (FSM)->nbStates)

#ifdef DEMOS_DEBUG
u16 FSMtrace (FSM* _fsm, void* _image, u16 _pitch, u16 _planePitch, u16 _y);
#endif

#endif
