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

#include "REBIRTH\SRC\SCREENS.H"
#include "REBIRTH\SRC\SNDTRACK.H"

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\LOAD.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\SOUND.H"

#include "REBIRTH\REBIRTH1.H"
#include "REBIRTH\REBIRTH2.H"


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
   
    COLcomputeGradient ((u16*)HW_COLOR_LUT, black, 16, 16, &gradient[0][0]);

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
    LOADunitTestInit, 
    LOADunitTestUpdate
};

FSMstate states[] =
{
	doNothing
};
*/

/* CYBER VECTOR ----------------------------------------------*/

/*
FSMstate statesIdle[] =
{
	CybervectorEntry, 
	CybervectorBacktask
};

FSMstate states[] =
{
	doNothing, 
    SCRswitch60hz, 
    SCRswitch4P, 
	CybervectorActivity
};
*/

/* VISUALIZER ----------------------------------------------*/
/*
FSMstate states[] =
{
	doNothing, 
    SCRswitch50hz, 
    SCRswitch2P, 
	VisualizerActivity
};

FSMstate statesIdle[] =
{
	VisualizerEntry, 
	VisualizerBacktask
};
*/

/* SLIDES ----------------------------------------------*/
/*
FSMstate states[] =
{
    SCRswitch50hz, 
    SCRswitch4P, 
    SCRswitch336, 
    doNothing, 
    SlidesInitActivity, 
	SlidesActivity
};

FSMstate statesIdle[] =
{
	SlidesEntry, 
	SlidesBacktask
};
*/


/* FUGIT ----------------------------------------------*/
/*
FSMstate statesIdle[] =
{
	FugitEntry, 
    FugitBacktask
};

FSMstate states[] =
{
    SCRswitch50hz, 
    SCRswitch2P, 
	doNothing, 
	FugitActivity
};
*/

/* INTERLUDE ----------------------------------------------*/
/*
FSMstate statesIdle[] =
{
	InterludeEntry,    
	InterludeBacktask
};

FSMstate states[] =
{
    SCRswitch50hz    , 
    SCRswitch4P      , 
	doNothing        , 
	InterludeActivity
};
*/

/* WHOLE DEMO -------------------------------------------*/

FSMstate states[] =
{
	FSM_STATE(doNothing), 
    FSM_STATE(SCRswitch60hz), 
    FSM_STATE(SCRswitch4P), 
	FSM_STATE(CybervectorActivity), 
	FSM_STATE(stepNext), 
    
	FSM_STATE(doNothing), 
    FSM_STATE(SCRswitch50hz), 
    FSM_STATE(SCRswitch2P), 
	FSM_STATE(VisualizerActivity), 
    FSM_STATE(stepNext), 
    
	FSM_STATE(doNothing), 
    FSM_STATE(SCRswitch50hz), 
    FSM_STATE(SCRswitch4P), 
	FSM_STATE(InterludeActivity), 
    FSM_STATE(stepNext), 
    
	FSM_STATE(doNothing), 
    FSM_STATE(SCRswitch50hz), 
    FSM_STATE(SCRswitch2P), 
	FSM_STATE(VisualizerActivity), 
    FSM_STATE(stepNext), 
    
	FSM_STATE(doNothing), 
    FSM_STATE(SCRswitch50hz), 
    FSM_STATE(SCRswitch4P), 
	FSM_STATE(InterludeActivity), 
    FSM_STATE(stepNext), 
    
	FSM_STATE(doNothing), 
    FSM_STATE(SCRswitch50hz), 
    FSM_STATE(SCRswitch2P), 
	FSM_STATE(VisualizerActivity), 
    FSM_STATE(stepNext), 
    
    FSM_STATE(doNothing), 
    FSM_STATE(SCRswitch4P), 
    FSM_STATE(SCRswitch336), 
    FSM_STATE(SlidesInitActivity), 
	FSM_STATE(SlidesActivity), 
	FSM_STATE(stepNext), 
    
	FSM_STATE(doNothing), 
    FSM_STATE(SCRswitch320), 
    FSM_STATE(SCRswitch2P), 
	FSM_STATE(VisualizerActivity), 
    FSM_STATE(stepNext), 
    
    FSM_STATE(doNothing), 
    FSM_STATE(SCRswitch320), 
    FSM_STATE(SCRswitch2P), 
	FSM_STATE(FugitActivity), 
    FSM_STATE(stepNext), 
    
	FSM_STATE(doNothing)
};


FSMstate statesIdle[] =
{
    FSM_STATE(SCRfade2black), 

    FSM_STATE(CybervectorEntry), 
	FSM_STATE(CybervectorBacktask), 
	FSM_STATE(CybervectorExit), 
    
    FSM_STATE(VisualizerEntry), 
    FSM_STATE(VisualizerEntryFast), 
	FSM_STATE(VisualizerBacktask), 
	
    FSM_STATE(InterludeEntry), 
	FSM_STATE(InterludeBacktask), 
	FSM_STATE(InterludeExit), 
    
    FSM_STATE(VisualizerEntryFast), 
	FSM_STATE(VisualizerBacktask), 
    
    FSM_STATE(InterludeEntry), 
	FSM_STATE(InterludeBacktask), 
	FSM_STATE(InterludeExit), 
    
    FSM_STATE(VisualizerEntryFast), 
	FSM_STATE(VisualizerBacktask), 
	FSM_STATE(VisualizerExit), 
    
    FSM_STATE(SlidesEntry), 
	FSM_STATE(SlidesBacktask), 
	FSM_STATE(SlidesExit), 
    
	FSM_STATE(VisualizerEntry), 
    FSM_STATE(VisualizerEntryFast), 
	FSM_STATE(VisualizerBacktask), 
	FSM_STATE(VisualizerExit), 
    
    FSM_STATE(FugitEntry), 
	FSM_STATE(FugitBacktask), 
	FSM_STATE(FugitExit), 
	
    FSM_STATE(doNothing)
};

u16 statesSize     = (u16) ARRAYSIZE(states);
u16 statesIdleSize = (u16) ARRAYSIZE(statesIdle);


void ScreenWaitMainDonothing (void)
{
    while (g_stateMachine.states[FSMgetCurrentState(&g_stateMachine)].func != doNothing);
}


#define SCREEN_PRELOAD_H    160

void PreloadDisplay (u16 _preloadResourceIndex, LOADrequest* _request, void* _clientData)
{
    static char line[] = "preloading    -      ";


    STDuxtoa(&line[11], RSC_REBIRTH1_NBENTRIES - RSC_REBIRTH1_POLYZOOM_CYBERVECTOR_BIN - _preloadResourceIndex - 1, 2);
    STDuxtoa(&line[16], _request->nbsectors, 4);
    
    SYSfastPrint (line, _clientData, 160, 4, (u32)&SYSfont);
}


void ScreensInit (void* _preload, u32 _preloadsize)
{
    SYSvblroutines[0] = RASvbldonothing;

    STDmset (&g_screens, 0, sizeof(g_screens));
    snd.syncWithSoundtrack = statesSize >= 20;

#   ifndef DEMOS_LOAD_FROMHD
    if ( _preload != NULL )
    {
        u8 disk1Preload[RSC_REBIRTH1_NBENTRIES - RSC_REBIRTH1_POLYZOOM__CYBERVECTOR_BIN];
        u8 t, i = 0;
        
        void* current = _preload;
        u8*   screen = (u8*) RINGallocatorAlloc(&sys.mem, 32000);
        u8*   diplayarea = screen + SCREEN_PRELOAD_H * 160 + 2;


        STDmcpy(screen, (void*) SYSreadVideoBase(), 32000);
        SYSwriteVideoBase ((u32)screen);
        SYSvsync;

        for (t = RSC_REBIRTH1_POLYZOOM__CYBERVECTOR_BIN ; t < RSC_REBIRTH1_NBENTRIES ; t++)
        {
            disk1Preload[i++] = t;
        }

        STDmset (diplayarea, 0, (200 - SCREEN_PRELOAD_H) * 160);
        SYSfastPrint ("only 1 drive but extra memory...", diplayarea, 160, 4, (u32)&SYSfont);

        current = LOADpreload (_preload, _preloadsize, current, &RSC_REBIRTH1, disk1Preload, ARRAYSIZE(disk1Preload), PreloadDisplay, &diplayarea[160*10]);

        {
            bool goon = true;

            SYSfastPrint ("insert disk 2 and press space...", &diplayarea[160*20], 160, 4, (u32)&SYSfont);

            do 
            {
                LOADrequest* request;

                do 
                {
                    SYSkbAcquire;
                    if ( SYSkbHit )
                    {
                        goon = (sys.key != (HW_KEY_SPACEBAR | HW_KEYBOARD_KEYRELEASE));
                        SYSkbReset();
                    }
                }
                while (goon);

                request = LOADrequestLoad (&RSC_REBIRTH1, 0, current, LOAD_PRIORITY_INORDER);  /* force sector 0 load */
                LOADwaitRequestCompleted (request);
                LOADfreeRequest (request);

                goon = ((u16*)current)[255] != DISK2_BOOT_CHECKSUM_DIFF;
            } 
            while (goon);

            LOADinitFAT (0, &RSC_REBIRTH2, RSC_REBIRTH2_NBENTRIES, RSC_REBIRTH2_NBMETADATA);

            {
                static u8 disk2Preload[] = 
                {
                    RSC_REBIRTH2_ZIK__7569902_RAW,  
                    RSC_REBIRTH2_ZIK__12465342_RAW, 
                    RSC_REBIRTH2_ZIK__14245502_RAW, 
                    RSC_REBIRTH2_ZIK__14646038_RAW, 
                    RSC_REBIRTH1_VISUALIZ__PAL_BIN,
                    RSC_REBIRTH2_SLIDES__MASKS_PT, 
                    RSC_REBIRTH2_FUGIT__FONT_ARJX 
                };

                LOADpreload (_preload, _preloadsize, current, &RSC_REBIRTH2, disk2Preload, ARRAYSIZE(disk2Preload), PreloadDisplay, &diplayarea[160*10]);
            }
        }

        RINGallocatorFree(&sys.mem, screen);
    }
#   endif
}

void SYScheckHWRequirements (void)
{
#	ifdef __TOS__
    bool failed = false;

    /* Check computer is a STe */
    if ( (*((u16*)HW_VECTOR_INIT_PC)) != 0xE0 )
    {
        failed = true;
    }

    /* Check you have two drives or 2mb */
    if (( *OS_NFLOPS < 2 ) && ( *OS_PHYTOP < 0x200000UL ))
    {
        failed = true;
    }

    if ( failed )
    {
        u8* frameBuffer = (u8*) SYSreadVideoBase();    
        
        STDcpuSetSR(0x2700);

        while ((*HW_VIDEO_BASE_L != *HW_VIDEO_COUNT_L) || (*HW_VIDEO_BASE_M != *HW_VIDEO_COUNT_M) || (*HW_VIDEO_BASE_H != *HW_VIDEO_COUNT_H));

        *HW_VIDEO_MODE = HW_VIDEO_MODE_4P;

        STDmset(frameBuffer, 0, 32000);

        STDmset (&HW_COLOR_LUT[1], 0xFFFFFFFFUL, 30);

        SYSfastPrint ("Hardware requirements:" , frameBuffer, 160, 8, (u32)&SYSfont);
        SYSfastPrint ("STe - 1mb + 2 drives or at least 2mb", &frameBuffer[160 * 8], 160, 8, (u32)&SYSfont);

        while(1)
        {
            (*HW_COLOR_LUT) = 0x300;
            (*HW_COLOR_LUT) = 0x400;
        }
    }
#   endif
}
