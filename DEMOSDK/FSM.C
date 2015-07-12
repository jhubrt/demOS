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

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\SYSTEM.H"

void FSMinit (FSM* _m, FSMstate* _states, u16 _nbStates, u16 _startState)
{
	_m->states		= _states;
    _m->nbStates    = _nbStates;
	_m->activeState = _startState;

	{
		FSMfunction entryAction = _states[_startState].entryAction;

		if ( entryAction != NULL )
		{
			entryAction (_m);
		}
	}

#   ifdef DEMOS_DEBUG
    {
        STDmset(_m->trace, 0x20202020UL, sizeof(_m->trace));
        _m->trace[sizeof(_m->trace) - 1] = 0;
        _m->traceCurrent = -1;
        _m->traceLastState = 0xFFFF;
    }
#   endif
}

void FSMupdate (FSM* _m)
{
    _m->states [_m->activeState].activity (_m);
}

void FSMgoto (FSM* _m, u16 _newState)
{
	FSMfunction exitAction	= _m->states[_m->activeState].exitAction;
	FSMfunction entryAction	= _m->states[_newState].entryAction;


    ASSERT(_newState < _m->nbStates);

    if ( exitAction != NULL )
	{
		exitAction (_m);
	}

    _m->activeState = _newState;

	if ( entryAction != NULL )
	{
		entryAction (_m);
	}
}

#ifdef DEMOS_DEBUG
u16 FSMtrace (FSM* _m, void* _image, u16 _pitch, u16 _planePitch, u16 _y)
{
    if (_m->traceLastState != _m->activeState)
    {
        if (_m->traceCurrent == -1)
        {
            _m->traceCurrent = 0;
        }
        else
        {
            _m->trace[_m->traceCurrent  ] = ' ';
            _m->trace[_m->traceCurrent+3] = ' ';

            _m->traceCurrent += 4;
            if (_m->traceCurrent >= (sizeof(_m->trace) - 4))
            {
                _m->traceCurrent = 0;
            }
        }

        _m->trace[_m->traceCurrent]   = '[';
        STDuxtoa (&_m->trace[_m->traceCurrent+1], _m->activeState, 2);
        _m->trace[_m->traceCurrent+3] = ']';
        
        _m->traceLastState = _m->activeState;
    }

    SYSdebugPrint ( _image, _pitch, _planePitch, 0, _y, _m->trace);

	return 8;
}
#endif
