/*------------------------------------------------------------------------------  -----------------
  Copyright J.Hubert 2015

  This file is part of Synthetic Sound

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

#define SCREENS_C

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\LOAD.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\SOUND.H"

#include "SYNSOUND\SRC\SCREENS.H"

#include "REBIRTH\DISK1.H"
#include "REBIRTH\DISK2.H"


#define DISK2_BOOT_CHECKSUM_DIFF 0x7d25

Screens	g_screens;

FSM g_stateMachine;
FSM g_stateMachineIdle;

/* ------------------------------------------------------------------------
    Main thread             Idle thread
   ------------------------------------------------------------------------

    doNothing               ScreenEntry 
    ...                     ...
    ...                     ... >> main >> idle    

    ScreenActivity          ScreenBackground
    ...                     ...
    ...                     ...
    ...                     ...
    ...                     ... >> main | vsync

    stepNext                ScreenExit
    ... >> main >> idle     >> idle
                            
    doNothing               Screen2Entry
    ...                     ...
    ...                     ... >> main >> idle

    Screen2Activity         Screen2Background
    ...                     ...
    ...                     ...
    ...                     ...
    ...                     ... >> main | vsync

------------------------------------------------------------------------*/

/*static void testfsm(FSM* _fsm)	
{ 
	IGNORE_PARAM(_fsm); 

    *HW_COLOR_LUT = 0x333;
}*/

static void doNothing(FSM* _fsm)	
{ 
	IGNORE_PARAM(_fsm); 
}

static void stepNext(FSM* _fsm)	
{ 
    STDmset(HW_COLOR_LUT,0,32);
    FSMgotoNextState (_fsm);
    FSMgotoNextState (&g_stateMachineIdle);
}

static void SCRswitch50hz (FSM* _fsm)
{
    *HW_VIDEO_SYNC = HW_VIDEO_SYNC_50HZ;
    FSMgotoNextState (_fsm);
}

static void SCRswitch2P (FSM* _fsm)
{
	*HW_VIDEO_MODE = HW_VIDEO_MODE_2P;
    TRACsetVideoMode (160);
    FSMgotoNextState (_fsm);
}

static void SCRswitch4P (FSM* _fsm)
{
	*HW_VIDEO_MODE = HW_VIDEO_MODE_4P;
    TRACsetVideoMode (160);
    FSMgotoNextState (_fsm);
}

static void SCRswitch336 (FSM* _fsm)
{
    *HW_VIDEO_PIXOFFSET = 1;
    *HW_VIDEO_PIXOFFSET_HIDDEN = 0;
    TRACsetVideoMode (168);
    FSMgotoNextState (_fsm);
}

static void SCRswitch320 (FSM* _fsm)
{
    *HW_VIDEO_PIXOFFSET = 0;
    *HW_VIDEO_PIXOFFSET_HIDDEN = 0;
    TRACsetVideoMode (160);
    FSMgotoNextState (_fsm);
}

static void SCRswitch60hz (FSM* _fsm)
{
    *HW_VIDEO_SYNC = HW_VIDEO_SYNC_60HZ;
    FSMgotoNextState (_fsm);
}

static void SCRfade2black (FSM* _fsm)
{
    u16 black[16];
    u16 gradient[16][16];
    u16 t;


    STDmset (black, 0, sizeof(black));
   
    COLcomputeGradient (HW_COLOR_LUT, black, 16, 16, &gradient[0][0]);

    for (t = 0 ; t < 16 ; t++)
    {
        SYSvsync;
        STDmcpy (HW_COLOR_LUT, gradient[t], 32);
    }
    
    FSMgotoNextState (_fsm);
}

/* LOAD unit test ----------------------------------------------*/

/*
FSMstate statesIdle[] =
{
    {NULL, LOADunitTestInit, NULL},
    {NULL, LOADunitTestUpdate, NULL}
};

FSMstate states[] =
{
	{NULL, doNothing, NULL}
};
*/

/* WHOLE DEMO -------------------------------------------*/

FSMstate states[] =
{
	{NULL, doNothing, NULL},

    {NULL, IntroActivity, NULL},
	{NULL, doNothing, NULL}
};

FSMstate statesIdle[] =
{
    {NULL, SCRswitch50hz, NULL},
	{NULL, SCRfade2black, NULL},
    {NULL, IntroEntry, NULL},
    {NULL, IntroBacktask, NULL},
    {NULL, IntroExit, NULL}
};

u16 statesSize     = (u16) ARRAYSIZE(states);
u16 statesIdleSize = (u16) ARRAYSIZE(statesIdle);


void ScreenWaitMainDonothing (void)
{
    while (g_stateMachine.states[g_stateMachine.activeState].activity != doNothing);
}


#define SCREEN_PRELOAD_H    160

void ScreensInit (void)
{
    STDmset (&g_screens, 0, sizeof(g_screens));
}

void SYScheckHWRequirements(void)
{
}