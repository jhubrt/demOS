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

#include "DEMOSDK\BASTYPES.H"

#define SCREENS_C

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\SOUND.H"

#include "WIZPLAY\SRC\SCREENS.H"

Player	g_player;

FSM g_stateMachine;
FSM g_stateMachineIdle;


static void doNothing(FSM* _fsm)	
{ 
	IGNORE_PARAM(_fsm); 
    STDstop2300();
}

static void stepNext(FSM* _fsm)	
{ 
    HW_COLOR_LUT[0] = 0;
    HW_COLOR_LUT[1] = PCENDIANSWAP16(0xFFF);
    HW_COLOR_LUT[2] = PCENDIANSWAP16(0x261);
    HW_COLOR_LUT[3] = PCENDIANSWAP16(0xFFF);

    FSMgotoNextState (_fsm);
    FSMgotoNextState (&g_stateMachine);
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

static void SCRfade2black (FSM* _fsm)
{
    u16 black[16];
    u16 gradient[16][16];
    u16 t;


    STDmset (black, 0, sizeof(black));
   
    COLcomputeGradient ((u16*)HW_COLOR_LUT, black, 16, 16, &gradient[0][0]);

    for (t = 0 ; t < 16 ; t++)
    {
        SYSvsync;
        STDmcpy2 (HW_COLOR_LUT, gradient[t], 32);
    }
    
    FSMgotoNextState (_fsm);
}

/*----------------------------------------------*/

FSMstate statesPlay[] =
{
	FSM_STATE(doNothing), 
    FSM_STATE(PlayerActivity),
    FSM_STATE(doNothing)
};

FSMstate statesIdle[] =
{
    FSM_STATE(SCRswitch50hz), 
    FSM_STATE(SCRfade2black), 
    FSM_STATE(stepNext),
    FSM_STATE(PlayerBacktask),
    FSM_STATE(PlayerExit),
    FSM_STATE(doNothing)
};

u16 statesPlaySize = (u16) ARRAYSIZE(statesPlay);
u16 statesIdleSize = (u16) ARRAYSIZE(statesIdle);

void SYScheckHWRequirements (void)
{
}