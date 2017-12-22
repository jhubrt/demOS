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

#ifndef SCREENS_H
#define SCREENS_H

#include "DEMOSDK\BASTYPES.H"

#include "REBIRTH\SRC\POLYZOOM.H"
#include "REBIRTH\SRC\VISUALIZ.H"
#include "REBIRTH\SRC\SLIDES.H"
#include "REBIRTH\SRC\FUGIT.H"
#include "REBIRTH\SRC\INTERLUD.H"

STRUCT(Screens)
{
    /* Effects sub objects */
	Cybervector* cybervector;
	Visualizer*	 visualizer;
    Slides*      slides;
    Fugit*       fugit;
    Interlude*   interlude;

    u8           visualizerIndex;
    u8           interludeTextIndex;
};

#ifndef SCREEN_C
extern Screens      g_screens;
extern FSM          g_stateMachine;
extern FSM          g_stateMachineIdle;
extern FSMfunction  states[];
extern FSMfunction  statesIdle[];
extern u16          statesSize;
extern u16          statesIdleSize;
#endif

void ScreensInit(void* _preload, u32 _preloadsize);
void ScreenWaitMainDonothing (void);

#endif
