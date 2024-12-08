/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2024 J.Hubert

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

#include "RELAPSE\SRC\SCREENS.H"

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\LOAD.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\SOUND.H"

#include "RELAPSE\RELAPSE1.H"
#include "RELAPSE\RELAPSE2.H"

#define RELAPSE_PRELOAD_ENABLE 1

Screens	g_screens;

FSM g_stateMachine;
FSM g_stateMachineIdle;

void decoden2(u8 *dst, u8 *data);

void RELAPSE_UNPACK(void* to, void* from)
{
    g_screens.persistent.dpackbase = (u8*) from+4;
    decoden2((u8*)to, (u8*)from+4);
}

static void doNothing(FSM* _fsm)	
{ 
	IGNORE_PARAM(_fsm); 
}

static void idle(FSM* _fsm)	
{ 
    IGNORE_PARAM(_fsm); 
    STDstop2300();
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
   
    COLcomputeGradient ((u16*)HW_COLOR_LUT, black, 16, 16, gradient[0]);

    for (t = 0 ; t < 16 ; t++)
    {
        SYSvsync;
        STDmcpy (HW_COLOR_LUT, gradient[t], 32);
    }
    
    FSMgotoNextState (_fsm);
}

void ScreenWrap(FSM* _fsm)
{
    FSMgoto (&g_stateMachineIdle, FSMlookForStateIndex(&g_stateMachineIdle, IntroEntry   ) - 2);
    FSMgoto (&g_stateMachine    , FSMlookForStateIndex(&g_stateMachine    , IntroActivity) - 3);
}


/* WHOLE DEMO FLOW -------------------------------------------*/

FSMstate states[] =
{
#if RELAPSE_PRELOAD_ENABLE
    FSM_STATE(doNothing),
    FSM_STATE(PreLoadActivity),
#endif

    FSM_STATE(doNothing),
    FSM_STATE(SplashActivity),

    FSM_STATE(doNothing),
    FSM_STATE(LoaderActivity),
    FSM_STATE(doNothing),
    FSM_STATE(IntroActivity),

    FSM_STATE(doNothing),
    FSM_STATE(LoaderActivity),
    FSM_STATE(doNothing),
    FSM_STATE(LiquidActivity),
    
    FSM_STATE(doNothing),
    FSM_STATE(LoaderActivity),
    FSM_STATE(doNothing),
    FSM_STATE(EgyptiaActivity),

    FSM_STATE(doNothing),
    FSM_STATE(LoaderActivity),
    FSM_STATE(doNothing),
    FSM_STATE(GrafikSActivity),

    FSM_STATE(doNothing),
    FSM_STATE(LoaderActivity),
    FSM_STATE(doNothing),
    FSM_STATE(InterludeActivity),

    FSM_STATE(doNothing),
    FSM_STATE(LoaderActivity),
    FSM_STATE(doNothing),
    FSM_STATE(CascadeActivity),

    FSM_STATE(doNothing),
    FSM_STATE(LoaderActivity),
    FSM_STATE(doNothing),
    FSM_STATE(ShadeVAActivity),

    FSM_STATE(doNothing),
    FSM_STATE(LoaderActivity),
    FSM_STATE(doNothing),
    FSM_STATE(SpaceFActivity),

    FSM_STATE(doNothing),
    FSM_STATE(LoaderActivity),
    FSM_STATE(doNothing),
    FSM_STATE(EndActivity),

    FSM_STATE(doNothing),
    FSM_STATE(LoaderActivity),
    FSM_STATE(doNothing),
    FSM_STATE(InfoActivity),

    FSM_STATE(doNothing),
    FSM_STATE(FastmenuActivity),
    FSM_STATE(doNothing),
};

FSMstate statesIdle[] =
{
	FSM_STATE(SCRfade2black), 
    FSM_STATE(SCRswitch50hz), 
    FSM_STATE(SCRswitch4P), 
   
#if RELAPSE_PRELOAD_ENABLE
    FSM_STATE(PreLoadEntry),
    FSM_STATE(PreLoadExit),
#endif

    FSM_STATE(SplashEntry),
    FSM_STATE(idle),
    FSM_STATE(SplashExit),

    FSM_STATE(LoaderEntry), 
    FSM_STATE(LoaderCheckDisk1Inserted),
    FSM_STATE(IntroEntry), 
    FSM_STATE(LoaderExit), 
    FSM_STATE(IntroPostInit), 
    FSM_STATE(idle), 
    FSM_STATE(IntroExit), 

    FSM_STATE(LoaderEntry), 
    FSM_STATE(LoaderCheckDisk1Inserted),
    FSM_STATE(LiquidEntry), 
    FSM_STATE(LoaderExit), 
    FSM_STATE(LiquidPostInit), 
    FSM_STATE(idle), 
    FSM_STATE(LiquidExit), 

    FSM_STATE(LoaderEntry), 
    FSM_STATE(LoaderCheckDisk1Inserted),
    FSM_STATE(EgyptiaEntry), 
    FSM_STATE(LoaderExit), 
    FSM_STATE(EgyptiaPostInit), 
    FSM_STATE(idle), 
    FSM_STATE(EgyptiaExit),

    FSM_STATE(LoaderEntry), 
    FSM_STATE(LoaderCheckDisk1Inserted),
    FSM_STATE(GrafikSEntry), 
    FSM_STATE(LoaderExit), 
    FSM_STATE(GrafikSPostInit), 
    FSM_STATE(GrafikSBacktask), 
    FSM_STATE(GrafikSExit),

    FSM_STATE(LoaderEntry), 
    FSM_STATE(LoaderCheckDisk1Inserted),
    FSM_STATE(InterludeEntry), 
    FSM_STATE(LoaderExit), 
    FSM_STATE(InterludePostInit), 
    FSM_STATE(doNothing), 
    FSM_STATE(InterludeExit),

    FSM_STATE(LoaderEntry), 
    FSM_STATE(LoaderCheckDisk2Inserted),
    FSM_STATE(CascadeEntry), 
    FSM_STATE(LoaderExit), 
    FSM_STATE(CascadePostInit), 
    FSM_STATE(idle), 
    FSM_STATE(CascadeExit),

    FSM_STATE(LoaderEntry), 
    FSM_STATE(LoaderCheckDisk2Inserted),
    FSM_STATE(ShadeVAEntry), 
    FSM_STATE(LoaderExit), 
    FSM_STATE(ShadeVAPostInit), 
    FSM_STATE(idle), 
    FSM_STATE(ShadeVAExit),

    FSM_STATE(LoaderEntry), 
    FSM_STATE(LoaderCheckDisk2Inserted),
    FSM_STATE(SpaceFEntry), 
    FSM_STATE(LoaderExit), 
    FSM_STATE(SpaceFPostInit), 
    FSM_STATE(SpaceFBacktask), 
    FSM_STATE(SpaceFExit),

    FSM_STATE(LoaderEntry), 
    FSM_STATE(LoaderCheckDisk2Inserted),
    FSM_STATE(EndEntry), 
    FSM_STATE(LoaderExit), 
    FSM_STATE(EndPostInit), 
    FSM_STATE(EndBacktask), 
    FSM_STATE(EndExit),

    FSM_STATE(LoaderEntry), 
    FSM_STATE(LoaderCheckDisk2Inserted),
    FSM_STATE(InfoEntry), 
    FSM_STATE(LoaderExit), 
    FSM_STATE(InfoPostInit), 
    FSM_STATE(idle), 
    FSM_STATE(InfoExit),

    FSM_STATE(ScreenWrap),

    FSM_STATE(FastmenuEntry), 
    FSM_STATE(FastmenuBacktask), 
    FSM_STATE(FastmenuExit)
};

u16 statesSize     = (u16) ARRAYSIZE(states);
u16 statesIdleSize = (u16) ARRAYSIZE(statesIdle);


void ScreenWaitMainDonothing (void)
{
    while (g_stateMachine.states[FSMgetCurrentState(&g_stateMachine)].func != doNothing);
}

void ScreenNextState (FSM* _fsm)
{
    STDmset (HW_COLOR_LUT, 0UL, 32);

    g_screens.next = false;

    if (g_screens.gotomenu)
    {
        g_screens.gotomenu = false;
        g_screens.persistent.menumode = true;
    }

    if (g_screens.persistent.menumode)
    {
        FSMgoto (&g_stateMachineIdle, g_screens.persistent.fsmFastmenuIdleStateIndex);
        FSMgoto (&g_stateMachine, g_screens.persistent.fsmFastmenuStateIndex);
    }
    else
    {
        FSMgotoNextState (_fsm);
    }
}


#define SCREEN_PRELOAD_H    160

void ScreensInit (void)
{
    *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME | 40;

    STDmset (&g_screens, 0, sizeof(g_screens));

    g_screens.bass = 6 + 3;
    g_screens.treble = 6 + 2;

    *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME_BASS   | g_screens.bass;

    g_screens.persistent.fsmFastmenuStateIndex     = FSMlookForStateIndex(&g_stateMachine, FastmenuActivity) - 1;
    g_screens.persistent.fsmFastmenuIdleStateIndex = FSMlookForStateIndex(&g_stateMachineIdle, FastmenuEntry);

    *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME_TREBLE | g_screens.treble;
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

    /* Check you have 1mb */
    if ( *OS_PHYTOP < 0x100000UL )
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

        SYSfastPrint ("Minimum hardware requirement" , frameBuffer, 160, 8, (u32)&SYSfont);
        SYSfastPrint ("STe 1mb", &frameBuffer[160 * 8], 160, 8, (u32)&SYSfont);

        while(1)
        {
            (*HW_COLOR_LUT) = 0x300;
            (*HW_COLOR_LUT) = 0x400;
        }
    }
#   endif
}

void ScreensSetVideoMode (u8 vmode_, u8 voffset_, u16 xtrapix_)
{
#ifdef __TOS__
    SYSvsync;

    *HW_VIDEO_PIXOFFSET = xtrapix_;
    *HW_VIDEO_PIXOFFSET_HIDDEN = 0;

    *HW_VIDEO_OFFSET                 = voffset_;

    *HW_VIDEO_MODE = !vmode_;
    *HW_VIDEO_MODE = vmode_;
#else
    *HW_VIDEO_PIXOFFSET_HIDDEN = xtrapix_ >> 8;
    *HW_VIDEO_PIXOFFSET        = (u8)xtrapix_;
    *HW_VIDEO_OFFSET           = voffset_;
    *HW_VIDEO_MODE             = vmode_;

#   endif
}

u8 ScreenGetch(void)
{
    while (g_screens.justpressed == false);
    g_screens.justpressed = false;

    return g_screens.scancodepressed;
}


void ScreenFadeOutSound(void)
{
    u16 t;

    SYSvsync;
    STDmset(HW_COLOR_LUT, 0UL, 32);

    for (t = 0; t <= 32; t++)
    {
        u16 soundvolume = 32 - t;

        *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME | (soundvolume + 7);

        SYSvsync;
    }

    *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME;
}


void ScreenFadeOut(void)
{
    u16 start[16];
    u16 end[16];
    u16 colors[16];
    u16 t;
    
    STDmcpy2(start, HW_COLOR_LUT, sizeof(start));
    STDmset(end, 0UL, sizeof(end));

    for (t = 0; t <= 32; t++)
    {
        u16 soundvolume = 32 - t;

        *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME | (soundvolume + 7);

        COLcomputeGradient16Steps(start, end, ARRAYSIZE(start), (t >> 1), colors);
        SYSvsync;
        STDmcpy2(HW_COLOR_LUT, colors, sizeof(colors));
    }

    *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME;

    *HW_VIDEO_OFFSET = 0;
}


#ifdef DEMOS_DEBUG
void ScreensDumpRingAllocator(MEMallocator* _allocator, void* _address, FILE* _file)
{
    RINGallocatorDumpAlloc(_allocator->allocator, _address, _file);
}
#endif
