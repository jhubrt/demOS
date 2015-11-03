/*-------------------------------------------------------------------------------------------------
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

#ifndef SCREENS_H
#define SCREENS_H

#include "DEMOSDK\BASTYPES.H"
#include "DEMOSDK\FSM.H"

#include "SYNSOUND\SRC\PLAYER.H"

STRUCT(Screens)
{
    Intro*      intro;
};

typedef void(*DynLibEvent)(void);

STRUCT(DynInterface)
{
    DynLibEvent    init;
    DynLibEvent    update;
    DynLibEvent    shutdown;
};

typedef void(*DynLibBootstrap)(DynInterface* _interface);

#ifndef SCREEN_C
extern Screens  g_screens;
extern FSM      g_stateMachine;
extern FSM      g_stateMachineIdle;
extern FSMstate states[];
extern FSMstate statesIdle[];
extern u16      statesSize;
extern u16      statesIdleSize;
#endif

void ScreensInit(void* _preload, u32 _preloadsize);
void ScreenWaitMainDonothing (void);

#endif
