/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2018 J.Hubert

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


#include "DEMOSDK\FSM.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\SYSTEM.H"

void FSMinit (FSM* _m, FSMfunction* _states, u16 _nbStates, u16 _startState)
{
	_m->states		= _states;
    _m->nbStates    = _nbStates;
	_m->activeState = _startState;

#   ifdef DEMOS_DEBUG
    {
        STDmset(_m->trace, 0x20202020UL, sizeof(_m->trace));
        _m->trace[sizeof(_m->trace) - 1] = 0;
        _m->traceCurrent = -1;
        _m->traceLastState = 0xFFFF;
    }
#   endif
}

void FSMgoto (FSM* _m, u16 _newState)
{
    ASSERT(_newState < _m->nbStates);
    _m->activeState = _newState;
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
