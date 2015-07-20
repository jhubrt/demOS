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

#define SCREENS_C

#include "SCREENS\SCREENS.H"
#include "SCREENS\SNDTRACK.H"

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\LOAD.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\SOUND.H"

#include "DISK1.H"
#include "DISK2.H"


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

	SNDwaitClientStep (STEP_SOUNDTRACK_STARTED);

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

/* CYBER VECTOR ----------------------------------------------*/

/*
FSMstate statesIdle[] =
{
	{NULL, CybervectorEntry, NULL},
	{NULL, CybervectorBacktask, NULL}
};

FSMstate states[] =
{
	{NULL, doNothing, NULL},
    {NULL, SCRswitch60hz, NULL},
    {NULL, SCRswitch4P, NULL},
	{NULL, CybervectorActivity, NULL}
};
*/

/* VISUALIZER ----------------------------------------------*/
/*
FSMstate states[] =
{
	{NULL, doNothing, NULL},
    {NULL, SCRswitch50hz, NULL},
    {NULL, SCRswitch2P, NULL},
	{NULL, VisualizerActivity, NULL}
};

FSMstate statesIdle[] =
{
	{NULL, VisualizerEntry, NULL},
	{NULL, VisualizerBacktask, NULL}
};
*/

/* SLIDES ----------------------------------------------*/
/*
FSMstate states[] =
{
    {NULL, SCRswitch50hz, NULL},
    {NULL, SCRswitch4P, NULL},
    {NULL, SCRswitch336, NULL},
    {NULL, doNothing, NULL},
    {NULL, SlidesInitActivity, NULL},
	{NULL, SlidesActivity, NULL}
};

FSMstate statesIdle[] =
{
	{NULL, SlidesEntry, NULL},
	{NULL, SlidesBacktask, NULL}
};
*/


/* FUGIT ----------------------------------------------*/
/*
FSMstate statesIdle[] =
{
	{NULL, FugitEntry, NULL},
    {NULL, FugitBacktask, NULL}
};

FSMstate states[] =
{
    {NULL, SCRswitch50hz, NULL},
    {NULL, SCRswitch2P, NULL},
	{NULL, doNothing, NULL},
	{NULL, FugitActivity, NULL}
};
*/

/* INTERLUDE ----------------------------------------------*/
/*
FSMstate statesIdle[] =
{
	{NULL, InterludeEntry,    NULL},
	{NULL, InterludeBacktask, NULL},
};

FSMstate states[] =
{
    {NULL, SCRswitch50hz    , NULL},
    {NULL, SCRswitch4P      , NULL},
	{NULL, doNothing        , NULL},
	{NULL, InterludeActivity, NULL},
};
*/

/* WHOLE DEMO -------------------------------------------*/

FSMstate states[] =
{
	{NULL, doNothing, NULL},
    {NULL, SCRswitch60hz, NULL},
    {NULL, SCRswitch4P, NULL},
	{NULL, CybervectorActivity, NULL},
	{NULL, stepNext, NULL},

	{NULL, doNothing, NULL},
    {NULL, SCRswitch50hz, NULL},
    {NULL, SCRswitch2P, NULL},
	{NULL, VisualizerActivity, NULL},
    {NULL, stepNext, NULL},
    
	{NULL, doNothing, NULL},
    {NULL, SCRswitch50hz, NULL},
    {NULL, SCRswitch4P, NULL},
	{NULL, InterludeActivity, NULL},
    {NULL, stepNext, NULL},

	{NULL, doNothing, NULL},
    {NULL, SCRswitch50hz, NULL},
    {NULL, SCRswitch2P, NULL},
	{NULL, VisualizerActivity, NULL},
    {NULL, stepNext, NULL},

	{NULL, doNothing, NULL},
    {NULL, SCRswitch50hz, NULL},
    {NULL, SCRswitch4P, NULL},
	{NULL, InterludeActivity, NULL},
    {NULL, stepNext, NULL},

	{NULL, doNothing, NULL},
    {NULL, SCRswitch50hz, NULL},
    {NULL, SCRswitch2P, NULL},
	{NULL, VisualizerActivity, NULL},
    {NULL, stepNext, NULL},

    {NULL, doNothing, NULL},
    {NULL, SCRswitch4P, NULL},
    {NULL, SCRswitch336, NULL},
    {NULL, SlidesInitActivity, NULL},
	{NULL, SlidesActivity, NULL},
	{NULL, stepNext, NULL},

	{NULL, doNothing, NULL},
    {NULL, SCRswitch320, NULL},
    {NULL, SCRswitch2P, NULL},
	{NULL, VisualizerActivity, NULL},
    {NULL, stepNext, NULL},
    
    {NULL, doNothing, NULL},
    {NULL, SCRswitch320, NULL},
    {NULL, SCRswitch2P, NULL},
	{NULL, FugitActivity, NULL},
    {NULL, stepNext, NULL},

	{NULL, doNothing, NULL}
};

FSMstate statesIdle[] =
{
    {NULL, SCRfade2black, NULL},

    {NULL, CybervectorEntry, NULL},
	{NULL, CybervectorBacktask, NULL},
	{NULL, CybervectorExit, NULL},

    {NULL, VisualizerEntry, NULL},
    {NULL, VisualizerEntryFast, NULL},
	{NULL, VisualizerBacktask, NULL},
	
    {NULL, InterludeEntry, NULL},
	{NULL, InterludeBacktask, NULL},
	{NULL, InterludeExit, NULL},

    {NULL, VisualizerEntryFast, NULL},
	{NULL, VisualizerBacktask, NULL},

    {NULL, InterludeEntry, NULL},
	{NULL, InterludeBacktask, NULL},
	{NULL, InterludeExit, NULL},

    {NULL, VisualizerEntryFast, NULL},
	{NULL, VisualizerBacktask, NULL},
	{NULL, VisualizerExit, NULL},

    {NULL, SlidesEntry, NULL},
	{NULL, SlidesBacktask, NULL},
	{NULL, SlidesExit, NULL},

	{NULL, VisualizerEntry, NULL},
    {NULL, VisualizerEntryFast, NULL},
	{NULL, VisualizerBacktask, NULL},
	{NULL, VisualizerExit, NULL},

    {NULL, FugitEntry, NULL},
	{NULL, FugitBacktask, NULL},
	{NULL, FugitExit, NULL},
	
    {NULL, doNothing, NULL}
};

u16 statesSize     = (u16) ARRAYSIZE(states);
u16 statesIdleSize = (u16) ARRAYSIZE(statesIdle);


void ScreenWaitMainDonothing (void)
{
    while (g_stateMachine.states[g_stateMachine.activeState].activity != doNothing);
}


#define SCREEN_PRELOAD_H    160

void ScreensInit (void* _preload, u32 _preloadsize)
{
    STDmset (&g_screens, 0, sizeof(g_screens));
    snd.syncWithSoundtrack = statesSize >= 20;

    if ( _preload != NULL )
    {
        u8 disk1Preload[RSC_DISK1_NBENTRIES - RSC_DISK1_POLYZOOM__CYBERVECTOR_BIN];
        u8 t, i = 0;
        
        void* current = _preload;
        u8*   screen = RINGallocatorAlloc(&sys.mem, 32000);
        u8*   diplayarea = screen + SCREEN_PRELOAD_H * 160 + 2;


        STDmcpy(screen, (void*) SYSreadVideoBase(), 32000);
        SYSwriteVideoBase ((u32)screen);
        SYSvsync;

        for (t = RSC_DISK1_POLYZOOM__CYBERVECTOR_BIN ; t < RSC_DISK1_NBENTRIES ; t++)
        {
            disk1Preload[i++] = t;
        }

        STDmset (diplayarea, 0, (200 - SCREEN_PRELOAD_H) * 160);
        SYSfastPrint ("only 1 drive but extra memory...", diplayarea, 160, 4, (u32) sys.fontChars);

        current = LOADpreload (&diplayarea[160*10], 160, 4, _preload, _preloadsize, current, &RSC_DISK1, disk1Preload, ARRAYSIZE(disk1Preload));

        {
            bool goon = true;

            SYSfastPrint ("insert disk 2 and press space...", &diplayarea[160*20], 160, 4, (u32) sys.fontChars);

            do 
            {
                LOADrequest* request;

                do 
                {
                    if ( SYS_kbhit )
                    {
                        goon = (SYSgetKb() != (HW_KEY_SPACEBAR | HW_KEYBOARD_KEYRELEASE));
                    }
                }
                while (goon);

                request = LOADrequestLoad (&RSC_DISK1, 0, current, LOAD_PRIORITY_INORDER);  /* force sector 0 load */
                LOADwaitRequestCompleted (request);
                LOADfreeRequest (request);

                goon = ((u16*)current)[255] != DISK2_BOOT_CHECKSUM_DIFF;
            } 
            while (goon);

            {
                static u8 disk2Preload[] = 
                {
                    RSC_DISK2_ZIK__7569902_RAW,  
                    RSC_DISK2_ZIK__12465342_RAW, 
                    RSC_DISK2_ZIK__14245502_RAW, 
                    RSC_DISK2_ZIK__14646038_RAW, 
                    RSC_DISK1_VISUALIZ__PAL_BIN,
                    RSC_DISK2_SLIDES__MASKS_PT, 
                    RSC_DISK2_FUGIT__FONT_ARJX 
                };

                LOADpreload (&diplayarea[160*30], 160, 4, _preload, _preloadsize, current, &RSC_DISK2, disk2Preload, ARRAYSIZE(disk2Preload));
            }
        }

        RINGallocatorFree(&sys.mem, screen);
    }
}