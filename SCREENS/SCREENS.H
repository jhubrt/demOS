/*------------------------------------------------------------------------------  -----------------
  Copyright J.Hubert 2015

  This file is part of rebirth demo

  rebirth demo is free software: you can redistribute it and/or modify it under the terms of 
  the GNU Lesser General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  rebirth demo is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY ;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with rebirth demo.  
  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------- */

#ifndef SCREENS_H
#define SCREENS_H

#include "DEMOSDK\BASTYPES.H"

#include "SCREENS\POLYZOOM.H"
#include "SCREENS\VISUALIZ.H"
#include "SCREENS\SLIDES.H"
#include "SCREENS\FUGIT.H"
#include "SCREENS\INTERLUD.H"

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
extern Screens  g_screens;
extern FSM      g_stateMachine;
extern FSM      g_stateMachineIdle;
extern FSMstate states[];
extern FSMstate statesIdle[];
extern u16      statesSize;
extern u16      statesIdleSize;
#endif

void ScreensInit(void);

#endif
