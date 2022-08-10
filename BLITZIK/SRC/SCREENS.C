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
#include "DEMOSDK\FSM.H"

#define SCREENS_C

#include "BLITZIK\SRC\SCREENS.H"

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\LOAD.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\SOUND.H"

#include "DEMOSDK\PC\EMUL.H"

#include "EXTERN\ARJDEP.H"


/* Debug entries to save time... */
#define SCREEN_DEFAULT_BOOT_SCREEN BLZ_EP_INTRO
/*#define SCREEN_DEFAULT_BOOT_SCREEN BLZ_EP_MENU*/
/*#define SCREEN_DEFAULT_BOOT_SCREEN BLZ_EP_INFO*/
/*#define SCREEN_DEFAULT_BOOT_SCREEN BLZ_EP_SHADE_SOUND*/
/*#define SCREEN_DEFAULT_BOOT_SCREEN BLZ_EP_SAM_CURVE*/
/*#define SCREEN_DEFAULT_BOOT_SCREEN BLZ_EP_SAM_SCROLL*/
/*#define SCREEN_DEFAULT_BOOT_SCREEN BLZ_EP_LAYERZ*/
/*#define SCREEN_DEFAULT_BOOT_SCREEN BLZ_EP_SPACEWAV*/
/*#define SCREEN_DEFAULT_BOOT_SCREEN BLZ_EP_WAVHERO*/


#define DISK2_BOOT_CHECKSUM_DIFF 0x7d25


#ifdef DEMOS_DEBUG
void ScreensDumpRingAllocator(MEMallocator* _allocator, void* _address, FILE* _file)
{
    RINGallocatorDumpAlloc(_allocator->allocator, _address, _file);
}
#endif


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

static void doNothing(FSM* _fsm)	
{ 
	IGNORE_PARAM(_fsm); 
}

static void stepNext(FSM* _fsm)	
{ 
    FSMgotoNextState (_fsm);
    FSMgotoNextState (&g_stateMachineIdle);
}

static void SCRswitch50hz (FSM* _fsm)
{
    *HW_VIDEO_SYNC = HW_VIDEO_SYNC_50HZ;
    FSMgotoNextState (_fsm);
}

static void ScreensFade2Black (FSM* _fsm)
{
    EMUL_STATIC u16 source [16];
    EMUL_STATIC u16 dest   [16];
    u16 current[16];
    EMUL_STATIC u16 t;
    u16 i;

    EMUL_BEGIN_ASYNC_REGION

    for (i = 0 ; i < 16 ; i++)
        source[i] = PCENDIANSWAP16(HW_COLOR_LUT[i]);

    STDmset (dest, 0, sizeof(dest));
   
    for (t = 0 ; t <= 16 ; )
    {
        SYSvsync;
        
        EMUL_REENTER_POINT
        
        COLcomputeGradient16Steps (source, dest, 16, t, current);
        for (i = 0 ; i < 16 ; i++)
            HW_COLOR_LUT[i] = PCENDIANSWAP16(current[i]);
        
        t++;

        EMUL_EXIT_IF(t < 16)
    }
    
    FSMgotoNextState (_fsm);

    EMUL_END_ASYNC_REGION
}

#ifdef DEMOS_DEBUG
void ScreensLogFreeArea(char* _title)
{
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        RINGallocatorFreeArea freeArea;

        TRAClog(TRAC_LOG_MEMORY, _title, '\n');

        RINGallocatorFreeSize(&sys.coremem, &freeArea);
        TRAClog(TRAC_LOG_MEMORY, "RING ALLOCATOR - coremem", '\n');

        TRAClogNumberS(TRAC_LOG_MEMORY, "size: ", freeArea.size, 6, '\n');

        if (freeArea.nbareas >= 1)
            TRAClogNumberS(TRAC_LOG_MEMORY, "freearea1: ", freeArea.areasizes[0], 6, '\n');

        if (freeArea.nbareas >= 2)
            TRAClogNumberS(TRAC_LOG_MEMORY, "freearea2: ", freeArea.areasizes[1], 6, '\n');

        RINGallocatorFreeSize(&sys.mem, &freeArea);
        TRAClog(TRAC_LOG_MEMORY, "RING ALLOCATOR - mem", '\n');

        TRAClogNumberS(TRAC_LOG_MEMORY, "size: ", freeArea.size, 6, '\n');

        if (freeArea.nbareas >= 1)
            TRAClogNumberS(TRAC_LOG_MEMORY, "freearea1: ", freeArea.areasizes[0], 6, '\n');

        if (freeArea.nbareas >= 2)
            TRAClogNumberS(TRAC_LOG_MEMORY, "freearea2: ", freeArea.areasizes[1], 6, '\n');
    }
}
#endif

static void ScreensLoadBaseStatic (FSM* _fsm)
{
    LOADrequest* loadRequest;
    LOADrequest* loadRequest2;
    u8* temp;
    LayerZBmp* bmp;
    u16 t;


    ScreensLogFreeArea("ScreensLoadBaseStatic");

    g_screens.base.sin  = (s16*) MEM_ALLOC (&sys.allocatorCoreMem, 1024 + 256);
    loadRequest  = LOADdata (&RSC_BLITZWAV, RSC_BLITZWAV_POLYZOOM_SIN_BIN, g_screens.base.sin, LOAD_PRIORITY_INORDER);
    LOADwaitRequestCompleted ( loadRequest );

    temp = (u8*) MEM_ALLOCTEMP(&sys.allocatorMem, LOADresourceRoundedSize(&RSC_BLITZWAV, RSC_BLITZWAV_PICS_BLITZ_ARJX));
    loadRequest2 = LOADdata (&RSC_BLITZWAV, RSC_BLITZWAV_PICS_BLITZ_ARJX, temp, LOAD_PRIORITY_INORDER);
    LOADwaitRequestCompleted ( loadRequest2 );
    
    bmp = g_screens.layerzStatic.bmps;

    for (t = RSC_BLITZWAV_METADATA_PICS_BLITZ_ARJX ; t <= RSC_BLITZWAV_METADATA_PICS_CYBER6_ARJX ; t++)
    {
        bmp->blitzbmp = MEM_ALLOC (&sys.allocatorCoreMem, LOADmetadataOriginalSize(&RSC_BLITZWAV, t) + 4);
        ARJdepack(bmp->blitzbmp, temp + LOADmetadataOffset(&RSC_BLITZWAV, t));

        bmp->blitzbmpoffset = *(u16*) bmp->blitzbmp;
        bmp->blitzbmpoffset = PCENDIANSWAP16(bmp->blitzbmpoffset);
        bmp->blitzbmp += 2;
        
        bmp++;
    }

    MEM_FREE(&sys.allocatorMem, temp);
    
    STDmcpy2 (g_screens.base.sin + 512, g_screens.base.sin, 256);
    g_screens.base.cos = g_screens.base.sin + 128;

    FSMgotoNextState (_fsm);
}

/* WHOLE DEMO -------------------------------------------*/

FSMstate states[] =
{
    
    FSM_STATE(doNothing),                  /* 0 */
    FSM_STATE(BlitZLoaderActivity),        /* 1 */
    FSM_STATE(stepNext),                   /* 2 */
                                
    FSM_STATE(doNothing),                  /* 3 */
    FSM_STATE(CybervectorActivity),        /* 4 */
    FSM_STATE(CybervectorActivity2),       /* 5 */
    FSM_STATE(doNothing),                  /* 6 */

    FSM_STATE(doNothing),                  /* 7  */
    FSM_STATE(BlitZMenuActivity),          /* 8  */
    FSM_STATE(doNothing),                  /* 9  */

    FSM_STATE(BlitZLoadModuleIdleActivity),/* 10 */
    FSM_STATE(doNothing),                  /* 11 */

    FSM_STATE(doNothing),                  /* 12 */
    FSM_STATE(InfoActivity),               /* 13 */
    FSM_STATE(doNothing),                  /* 14 */

    FSM_STATE(doNothing),                  /* 15 */
    FSM_STATE(SamCurveActivity),           /* 16 */
    FSM_STATE(doNothing),                  /* 17 */

    FSM_STATE(doNothing),                  /* 18 */
    FSM_STATE(AllCurveActivity),           /* 19 */    
    FSM_STATE(doNothing),                  /* 20 */

    FSM_STATE(doNothing),                  /* 21 */
    FSM_STATE(SamScrollActivity),          /* 22 */
    FSM_STATE(doNothing),                  /* 23 */
    
    FSM_STATE(doNothing),                  /* 24 */
    FSM_STATE(SShadeActivity),             /* 25 */
    FSM_STATE(doNothing),                  /* 26 */

    FSM_STATE(doNothing),                  /* 27 */
    FSM_STATE(LayerZActivity),             /* 28 */
    FSM_STATE(doNothing),                  /* 29 */

    FSM_STATE(doNothing),                  /* 30 */
    FSM_STATE(SpaceWavActivity),           /* 31 */
    FSM_STATE(doNothing),                  /* 32 */

    FSM_STATE(doNothing),                  /* 33 */
    FSM_STATE(WavHeroActivity),            /* 34 */
    FSM_STATE(doNothing)                   /* 35 */
};

FSMstate statesIdle[] =
{
    FSM_STATE(ScreensFade2Black),          /* 0 */
    FSM_STATE(SCRswitch50hz),              /* 1 */

    FSM_STATE(BlitZMenuInitStatic),        /* 2 */
    FSM_STATE(ScreensLoadBaseStatic),      /* 3 */
    FSM_STATE(SShadeInitStatic),           /* 4 */
    FSM_STATE(SamCurveInitStatic),         /* 5 */
    FSM_STATE(AllCurveInitStatic),         /* 6 */
    FSM_STATE(SamScrollInitStatic),        /* 7 */ 
    FSM_STATE(LayerZStaticInit),           /* 8 */
    FSM_STATE(WavHeroInitStatic),          /* 9 */

    FSM_STATE(BlitZPreloadEnter),          /* 10 */
    FSM_STATE(ScreensFade2Black),          /* 11 */
    FSM_STATE(BlitZPreloadExit),           /* 12 */

    FSM_STATE(BlitZLoadModuleEnter),       /* 13 */
    FSM_STATE(BlitZLoadModule),            /* 14 */
    FSM_STATE(BlitZLoaderBacktask),        /* 15 */
    FSM_STATE(ScreensFade2Black),          /* 16 */
    FSM_STATE(BlitZLoadModuleExit),        /* 17 */
                                
    FSM_STATE(CybervectorEnter),           /* 18 */
    FSM_STATE(CybervectorBacktask),        /* 19 */
	FSM_STATE(CybervectorExit),            /* 20 */
                                
    FSM_STATE(BlitZMenuEnter),             /* 21 */
    FSM_STATE(BlitZMenuBacktask),          /* 22 */
    FSM_STATE(BlitZMenuExiting),           /* 23 */
    FSM_STATE(BlitZMenuExit),              /* 24 */

    FSM_STATE(BlitZLoadModuleIdleBacktask),/* 25 */
    FSM_STATE(BlitZLoadModuleIdleExit),    /* 26 */ 

    FSM_STATE(InfoEnter),                  /* 27 */ 
    FSM_STATE(InfoBacktask),               /* 28 */ 
    FSM_STATE(InfoExit),                   /* 29 */

    FSM_STATE(SamCurveEnter),              /* 30 */
    FSM_STATE(SamCurveBacktask),           /* 31 */
    FSM_STATE(SamCurveExit),               /* 32 */
                                
    FSM_STATE(AllCurveEnter),              /* 33 */
    FSM_STATE(AllCurveBacktask),           /* 34 */
    FSM_STATE(AllCurveExit),               /* 35 */
                                
    FSM_STATE(SamScrollEnter),             /* 36 */
    FSM_STATE(SamScrollBacktask),          /* 37 */
    FSM_STATE(SamScrollExit),              /* 38 */

    FSM_STATE(SShadeEnter),                /* 39 */
    FSM_STATE(doNothing),                  /* 40 */
    FSM_STATE(SShadeExit),                 /* 41 */

    FSM_STATE(LayerZEnter),                /* 42 */
    FSM_STATE(LayerZBacktask),             /* 43 */
    FSM_STATE(LayerZExit),                 /* 44 */

    FSM_STATE(SpaceWavEnter),              /* 45 */
    FSM_STATE(SpaceWavBacktask),           /* 46 */
    FSM_STATE(SpaceWavExit),               /* 47 */

    FSM_STATE(WavHeroEnter),               /* 48 */
    FSM_STATE(WavHeroBacktask),            /* 49 */
    FSM_STATE(WavHeroExit),                /* 50 */
                                         
    FSM_STATE(doNothing)                   /* 51 */
};

u16 statesSize     = (u16) ARRAYSIZE(states);
u16 statesIdleSize = (u16) ARRAYSIZE(statesIdle);


void ScreensGotoLoader (ScreensEntryPoints _returnEntryPoint)
{
    g_screens.runscreen = _returnEntryPoint;

    FSMgoto(&g_stateMachineIdle , g_screens.fsm.stateIdleEntryPoints[BLZ_EP_LOADER]);
    FSMgoto(&g_stateMachine     , g_screens.fsm.stateEntryPoints    [BLZ_EP_LOADER]);
}


void ScreensInit (void)
{   
    g_screens.bass = g_screens.treble = 6 ;

    *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME_BASS   | g_screens.bass;

    g_screens.fsm.stateEntryPoints      [BLZ_EP_LOADER]       = (u8) FSMlookForStateIndex(&g_stateMachine    , BlitZLoaderActivity) - 1;
    g_screens.fsm.stateIdleEntryPoints  [BLZ_EP_LOADER]       = (u8) FSMlookForStateIndex(&g_stateMachineIdle, BlitZLoadModuleEnter);
                                                        
    *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME_TREBLE | g_screens.treble;

    g_screens.fsm.stateEntryPoints      [BLZ_EP_INTRO]        = (u8) FSMlookForStateIndex(&g_stateMachine    , CybervectorActivity) - 1;
    g_screens.fsm.stateIdleEntryPoints  [BLZ_EP_INTRO]        = (u8) FSMlookForStateIndex(&g_stateMachineIdle, CybervectorEnter);
                                                        
    g_screens.fsm.stateEntryPoints      [BLZ_EP_MENU]         = (u8) FSMlookForStateIndex(&g_stateMachine    , BlitZMenuActivity) - 1;
    g_screens.fsm.stateIdleEntryPoints  [BLZ_EP_MENU]         = (u8) FSMlookForStateIndex(&g_stateMachineIdle, BlitZMenuEnter);

    g_screens.fsm.stateEntryPoints      [BLZ_EP_INFO]         = (u8) FSMlookForStateIndex(&g_stateMachine    , InfoActivity) - 1;
    g_screens.fsm.stateIdleEntryPoints  [BLZ_EP_INFO]         = (u8) FSMlookForStateIndex(&g_stateMachineIdle, InfoEnter);

    g_screens.fsm.stateEntryPoints      [BLZ_EP_SHADE_SOUND]  = (u8) FSMlookForStateIndex(&g_stateMachine    , SShadeActivity) - 1;
    g_screens.fsm.stateIdleEntryPoints  [BLZ_EP_SHADE_SOUND]  = (u8) FSMlookForStateIndex(&g_stateMachineIdle, SShadeEnter);

    g_screens.fsm.stateEntryPoints      [BLZ_EP_SAM_CURVE]    = (u8) FSMlookForStateIndex(&g_stateMachine    , SamCurveActivity) - 1;
    g_screens.fsm.stateIdleEntryPoints  [BLZ_EP_SAM_CURVE]    = (u8) FSMlookForStateIndex(&g_stateMachineIdle, SamCurveEnter);

    g_screens.fsm.stateEntryPoints      [BLZ_EP_ALL_CURVE]    = (u8) FSMlookForStateIndex(&g_stateMachine     , AllCurveActivity) - 1;
    g_screens.fsm.stateIdleEntryPoints  [BLZ_EP_ALL_CURVE]    = (u8) FSMlookForStateIndex(&g_stateMachineIdle , AllCurveEnter);

    g_screens.fsm.stateEntryPoints      [BLZ_EP_SAM_SCROLL]   = (u8) FSMlookForStateIndex(&g_stateMachine    , SamScrollActivity) - 1;
    g_screens.fsm.stateIdleEntryPoints  [BLZ_EP_SAM_SCROLL]   = (u8) FSMlookForStateIndex(&g_stateMachineIdle, SamScrollEnter);

    g_screens.fsm.stateEntryPoints      [BLZ_EP_LAYERZ]       = (u8) FSMlookForStateIndex(&g_stateMachine    , LayerZActivity) - 1;
    g_screens.fsm.stateIdleEntryPoints  [BLZ_EP_LAYERZ]       = (u8) FSMlookForStateIndex(&g_stateMachineIdle, LayerZEnter);

    g_screens.fsm.stateEntryPoints      [BLZ_EP_SPACEWAV]     = (u8) FSMlookForStateIndex(&g_stateMachine    , SpaceWavActivity) - 1;
    g_screens.fsm.stateIdleEntryPoints  [BLZ_EP_SPACEWAV]     = (u8) FSMlookForStateIndex(&g_stateMachineIdle, SpaceWavEnter);

    g_screens.fsm.stateEntryPoints      [BLZ_EP_WAVHERO]      = (u8) FSMlookForStateIndex(&g_stateMachine    , WavHeroActivity) - 1;
    g_screens.fsm.stateIdleEntryPoints  [BLZ_EP_WAVHERO]      = (u8) FSMlookForStateIndex(&g_stateMachineIdle, WavHeroEnter);

    g_screens.fsm.stateEntryPoints      [BLZ_EP_WAIT_FOR_FX]  = (u8) FSMlookForStateIndex(&g_stateMachine    , BlitZLoadModuleIdleActivity);
    g_screens.fsm.stateIdleEntryPoints  [BLZ_EP_WAIT_FOR_FX]  = (u8) FSMlookForStateIndex(&g_stateMachineIdle, BlitZLoadModuleIdleBacktask);

    g_screens.runscreen               = SCREEN_DEFAULT_BOOT_SCREEN;

#   ifndef __TOS__ 
    g_screens.persistent.menu.playmode = BLZ_PLAY_INTERACTIVE;
#   endif

    if (g_screens.compomode)
        g_screens.persistent.menu.playmode = BLZ_PLAY_AUTORUN;

    if (g_screens.dmaplayoncemode)
    {
        TRAClog(TRAC_LOG_FLOW, "DMA play once", '\n');
        g_screens.blzupdateroutine = aBLZ2vbl;
    }
    else
    {
        g_screens.blzupdateroutine = aBLZvbl;
    }
}

void ScreensGotoScreen (void)
{
    aBLZbackground = 0;

    FSMgoto(&g_stateMachineIdle, g_screens.fsm.stateIdleEntryPoints[g_screens.runscreen]);
    FSMgoto(&g_stateMachine    , g_screens.fsm.stateEntryPoints    [g_screens.runscreen]);
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

#   ifdef DEMOS_LOAD_FROMHD
    /* Check you have 2mb if running from HD */
    if ( *OS_PHYTOP < 0x200000UL )
    {
        failed = true;
    }
#   else
    /* Check you have 1mb */
    if ( *OS_PHYTOP < 0x100000UL )
    {
        failed = true;
    }
#   endif

    if ( failed )
    {
        u8* frameBuffer = (u8*) SYSreadVideoBase();    
        
        STDcpuSetSR(0x2700);

        while ((*HW_VIDEO_BASE_L != *HW_VIDEO_COUNT_L) || (*HW_VIDEO_BASE_M != *HW_VIDEO_COUNT_M) || (*HW_VIDEO_BASE_H != *HW_VIDEO_COUNT_H));

        *HW_VIDEO_MODE = HW_VIDEO_MODE_4P;

        STDmset(frameBuffer, 0, 32000);

        STDmset (&HW_COLOR_LUT[1], 0xFFFFFFFFUL, 30);

        SYSfastPrint ("Hardware requirements:" , frameBuffer, 160, 8, (u32)&SYSfont);
        SYSfastPrint ("STe at least 1mb with floppy or 2mb with HD", &frameBuffer[160 * 8], 160, 8, (u32)&SYSfont);

        while(1)
        {
            (*HW_COLOR_LUT) = 0x300;
            (*HW_COLOR_LUT) = 0x400;
        }
    }
#   endif
}


#if BLZ_DEVMODE()
static void BlitZresyncSeq (u16 trackindex_)
{
    ASSERT(trackindex_ < g_screens.player.sndtrack->trackLen);
    
    if (g_screens.fxsequence.framesmap != NULL)
    {
        u16 frame = g_screens.fxsequence.framesmap[trackindex_];
        u8* current = g_screens.fxsequence.seq;


        frame = PCENDIANSWAP16(frame);
        g_screens.player.framenum = frame;

        while (PCENDIANSWAP16(*(u16*)current) < frame)
        {
            u8 nb;

            current += 2;
            nb = *current++;
            current += nb;
            if ((nb & 1) == 0)
                current++;
        }

        g_screens.fxsequence.current = current;

        TRAClogNumberS  (TRAC_LOG_COMMANDS, "trackindex"   , trackindex_, 2, '\n');
        TRAClogNumber10S(TRAC_LOG_COMMANDS, "resyncseqtgt" , frame, 5, '\n');
        TRAClogNumber10S(TRAC_LOG_COMMANDS, "resyncseqcur" , PCENDIANSWAP16(*(u16*)current), 5, '\n');
    }
}
#endif


void BlitZrestartMod (void)
{
    BLZgoto(&g_screens.player, 0);
    g_screens.fxsequence.current = g_screens.fxsequence.seq;
    g_screens.player.framenum = 0;

    TRAClogNumberS  (TRAC_LOG_COMMANDS, "trackindex"   , g_screens.player.trackindex, 2, '\n');
    TRAClogNumber10S(TRAC_LOG_COMMANDS, "resyncseqtgt" , g_screens.player.framenum, 5, '\n');
    TRAClogNumber10S(TRAC_LOG_COMMANDS, "resyncseqcur" , PCENDIANSWAP16(*(u16*)g_screens.fxsequence.current), 5, '\n');
}


static u8 blitZkeysmap[] =
{
    0,
    0,                          /* 01 | ESC       */
    BLZ_CMD_VOICE1_1,           /* 02 | 1         */
    BLZ_CMD_VOICE1_2,           /* 03 | 2         */
    BLZ_CMD_VOICE1_3,           /* 04 | 3         */
    BLZ_CMD_VOICE1_4,           /* 05 | 4         */
    BLZ_CMD_VOICE1_5,           /* 06 | 5         */
    BLZ_CMD_VOICE1_6,           /* 07 | 6         */
    BLZ_CMD_VOICE1_7,           /* 08 | 7         */
    BLZ_CMD_VOICE1_8,           /* 09 | 8         */
    BLZ_CMD_VOICE1_9,           /* 0A | 9         */
    BLZ_CMD_VOICE1_10,          /* 0B | 0         */
    BLZ_CMD_VOICE1_11,          /* 0C | -         */
    BLZ_CMD_VOICE1_12,          /* 0D | =         */
    0,                          /* 0E | BACKSPACE */

    0,                          /* 0F | TAB       */   
    BLZ_CMD_Q,                  /* 10 | Q         */
    BLZ_CMD_W,                  /* 11 | W         */
    BLZ_CMD_E,                  /* 12 | E         */
    BLZ_CMD_R,                  /* 13 | R         */
    BLZ_CMD_T,                  /* 14 | T         */
    BLZ_CMD_Y,                  /* 15 | Y         */
    BLZ_CMD_U,                  /* 16 | U         */
    BLZ_CMD_I,                  /* 17 | I         */
    BLZ_CMD_O,                  /* 18 | O         */
    BLZ_CMD_P,                  /* 19 | P         */
    BLZ_CMD_BRACKL,             /* 1A | BRACKET L */
    BLZ_CMD_BRACKR,             /* 1B | BRACKET R */
    BLZ_CMD_SELECT,             /* 1C | RETURN    */

    0,                          /* 1D | CTRL      */   
    BLZ_CMD_A,                  /* 1E | A         */   
    BLZ_CMD_S,                  /* 1F | S         */
    BLZ_CMD_D,                  /* 20 | D         */
    BLZ_CMD_F,                  /* 21 | F         */
    BLZ_CMD_G,                  /* 22 | G         */
    BLZ_CMD_H,                  /* 23 | H         */
    BLZ_CMD_J,                  /* 24 | J         */
    BLZ_CMD_K,                  /* 25 | K         */
    BLZ_CMD_L,                  /* 26 | L         */
    BLZ_CMD_SEMICOLON,          /* 27 | ;         */
    BLZ_CMD_QUOTE,              /* 28 | '         */
    0,                          /* 29 | ' UPPER LINE */

    0,                          /* 2A | LEFT SHIFT  */
    0,                          /* 2B | #           */
    BLZ_CMD_Z,                  /* 2C | Z           */         
    BLZ_CMD_X,                  /* 2D | X           */
    BLZ_CMD_C,                  /* 2E | C           */
    BLZ_CMD_V,                  /* 2F | V           */
    BLZ_CMD_B,                  /* 30 | B           */
    BLZ_CMD_N,                  /* 31 | N           */
    BLZ_CMD_M,                  /* 32 | M           */
    BLZ_CMD_COMMA,              /* 33 | ,           */
    BLZ_CMD_DOT,                /* 34 | .           */
    BLZ_CMD_SLASH,              /* 35 | /           */
    0,                          /* 36 | RIGHT SHIFT */
    
    0,                          /* 37               */     
    0,                          /* 38 | ALTERNATE   */
    BLZ_CMD_SELECT,             /* 39 | SPACE       */  
    0,                          /* 3A | CAPS LOCK   */

    BLZ_CMD_F1,                 /* 3B | F1          */
    BLZ_CMD_F2,                 /* 3C | F2          */
    BLZ_CMD_F3,                 /* 3D | F3          */
    BLZ_CMD_F4,                 /* 3E | F4          */
    BLZ_CMD_F5,                 /* 3F | F5          */
    BLZ_CMD_F6,                 /* 40 | F6          */
    BLZ_CMD_F7,                 /* 41 | F7          */
    BLZ_CMD_F8,                 /* 42 | F8          */
    BLZ_CMD_F9,                 /* 43 | F9          */
    BLZ_CMD_F10,                /* 44 | F10         */

    0,                          /* 45               */    
    0,                          /* 46               */    
    0,                          /* 47 | CLR HOME    */
    BLZ_CMD_UP,                 /* 48 | UP          */     
    0,                          /* 49               */ 
    BLZ_CMD_BASS_MINUS,         /* 4A | - NUMPAD    */
    BLZ_CMD_LEFT,               /* 4B | LEFT        */
    0,                          /* 4C               */            
    BLZ_CMD_RIGHT,              /* 4D | RIGHT       */  
    BLZ_CMD_BASS_PLUS,          /* 4E | + NUMPAD    */
    0,                          /* 4F               */     
    BLZ_CMD_DOWN,               /* 50 | DOWN        */ 
    0,                          /* 51               */            
    0,                          /* 52 | INSERT      */
    0,                          /* 53 | DELETE      */
    
    0,                          /* 54               */ 
    0,                          /* 55               */ 
    0,                          /* 56               */ 
    0,                          /* 57               */ 
    0,                          /* 58               */ 
    0,                          /* 59               */ 
    0,                          /* 5A               */ 
    0,                          /* 5B               */ 
    0,                          /* 5C               */ 
    0,                          /* 5D               */ 
    0,                          /* 5E               */ 
    0,                          /* 5F               */ 
    
    BLZ_CMD_ANTISLASH,          /* 60 | \           */ 
    0,                          /* 61 | UNDO        */  
    0,                          /* 62 | HELP        */  

    0,                          /* 63 | ( NUMPAD    */  
    0,                          /* 64 | ) NUMPAD    */  
    BLZ_CMD_TREBLE_MINUS,       /* 65 | / NUMPAD    */  
    BLZ_CMD_TREBLE_PLUS,        /* 66 | * NUMPAD    */  
    BLZ_CMD_VOICE2_7,           /* 67 | 7 NUMPAD    */  
    BLZ_CMD_VOICE2_8,           /* 68 | 8 NUMPAD    */  
    BLZ_CMD_VOICE2_9,           /* 69 | 9 NUMPAD    */  
    BLZ_CMD_VOICE2_4,           /* 6A | 4 NUMPAD    */  
    BLZ_CMD_VOICE2_5,           /* 6B | 5 NUMPAD    */  
    BLZ_CMD_VOICE2_6,           /* 6C | 6 NUMPAD    */  
    BLZ_CMD_VOICE2_1,           /* 6D | 1 NUMPAD    */  
    BLZ_CMD_VOICE2_2,           /* 6E | 2 NUMPAD    */  
    BLZ_CMD_VOICE2_3,           /* 6F | 3 NUMPAD    */  
    BLZ_CMD_VOICE2_10,          /* 70 | 0 NUMPAD    */  
    BLZ_CMD_VOICE2_11,          /* 71 | . NUMPAD    */  
    BLZ_CMD_SELECT              /* 72 | ENTER NUMPAD*/  
};


void BlitZmanageKey (void)
{
    if ((g_screens.compomode == false) || BLZ_DEVMODE())
    {
        bool pressed  = (sys.key & HW_KEYBOARD_KEYRELEASE) == 0;
        u8   scancode = sys.key & ~(HW_KEYBOARD_KEYRELEASE);


        /* Just filter screens fx / option keys when not interactive mode and not into menus */
        if (g_screens.persistent.menu.playmode != BLZ_PLAY_INTERACTIVE)
        {
            if (g_screens.runningphase != BLZ_PHASE_MENU)
            {
                switch (scancode)
                {
                default:
                    pressed = false;
                    scancode = 0;
                    break;

                case HW_KEY_SPACEBAR:
                case HW_KEY_RETURN:
                case HW_KEY_NUMPAD_ENTER:

#               if BLZ_DEVMODE()
                case HW_KEY_HOME:
                case HW_KEY_INSERT:
                case HW_KEY_BACKSPACE:
                case HW_KEY_TAB:
#               endif
                    break;
                }
            }
        }

        if (pressed)
        {       
            u8 blitzcmd = 0;
        

            if (scancode < ARRAYSIZE(blitZkeysmap))
                blitzcmd = blitZkeysmap[scancode];

            if (blitzcmd > 0)
            {
                BLZ_PUSH_COMMAND(blitzcmd);

                /* double key test ...
                switch (blitzcmd)
                {
                case BLZ_CMD_UP:
                case BLZ_CMD_RIGHT:
                case BLZ_CMD_LEFT:
                case BLZ_CMD_DOWN:
                    break;
                default:
                    BLZ_PUSH_COMMAND(blitzcmd);
                    BLZ_PUSH_COMMAND(blitzcmd);
                    break;
                }
                ... double key test */
            }

#           if BLZ_DEVMODE()
            else
            {
                switch (scancode)
                {
                case HW_KEY_HOME:
                    BlitZrestartMod();
                    break;

                case HW_KEY_INSERT:
                    BLZgoto(&g_screens.player, g_screens.player.trackindex);
                    BlitZresyncSeq(g_screens.player.trackindex);
                    break;

                case HW_KEY_BACKSPACE:
                {
                    s16 index = (s16)g_screens.player.trackindex - 1;
                    if (index < 0)
                        index = (s16)g_screens.player.sndtrack->trackLen - 1;
                    BLZgoto(&g_screens.player, (u8)index);
                    BlitZresyncSeq(index);
                    break;
                }

                case HW_KEY_TAB:
                {
                    u8 index = g_screens.player.trackindex + 1;
                    if (index < g_screens.player.sndtrack->trackLen)
                    {
                        BLZgoto(&g_screens.player, index);
                        BlitZresyncSeq(index);
                    }
                    break;
                }
                }
            }
#           endif /* BLZ_DEVMODE() */
        }
    }
}


#ifdef __TOS__

ASMIMPORT u16 blitzvpix;
ASMIMPORT u8  blitzvoffset;
ASMIMPORT u8  blitzvmode;

void blitzvbl(void);
void blitz2vbl(void);

#endif

void BlitZsetVideoMode (u8 vmode_, u8 voffset_, u16 xtrapix_)
{
#   ifdef __TOS__

    ASSERT(SYSvblroutines[0] != blitzvbl);

    if (SYSvblroutines[0] != g_screens.blzupdateroutine)
    {
        SYSvsync;

        *(u16*)HW_VIDEO_PIXOFFSET_HIDDEN = xtrapix_;
        *HW_VIDEO_PIXOFFSET_HIDDEN       = xtrapix_;

        *HW_VIDEO_OFFSET                 = voffset_;

        *HW_VIDEO_MODE = !vmode_;
        *HW_VIDEO_MODE = vmode_;
    }
    else
    {
        blitzvpix = xtrapix_;
        blitzvmode = vmode_;
        blitzvoffset = voffset_;

        SYSvblroutines[0] = g_screens.dmaplayoncemode ? blitz2vbl : blitzvbl;
    }

#   else

    *HW_VIDEO_PIXOFFSET_HIDDEN = xtrapix_ >> 8;
    *HW_VIDEO_PIXOFFSET        = (u8)xtrapix_;
    *HW_VIDEO_OFFSET           = voffset_;
    *HW_VIDEO_MODE             = vmode_;

#   endif
}


void BlitZturnOffDisplay(void)
{
    SYSvsync;
    STDmset (HW_COLOR_LUT, 0UL, 32);
}

bool ScreensManageScreenChoice (ScreensEntryPoints _currentScreen, u8 cmd_)
{
    u8 category = cmd_ & BLZ_CMD_CATEGORY_MASK;

    if (category == BLZ_CMD_FUNC_CATEGORY)
    {
        u8 screen = cmd_ & BLZ_CMD_COMMAND_MASK;

        if (_currentScreen != screen)
        {
            if (screen < BLZ_EP_LOADER)
            {
                g_screens.runscreen = screen;
                STDmset(HW_COLOR_LUT, 0UL, 32);
                FSMgotoNextState(&g_stateMachineIdle);
                FSMgotoNextState(&g_stateMachine);

                return true;
            }
        }
    }
    else if (category == BLZ_CMD_CURSOR_CATEGORY)       
    {     
        if (cmd_ >= BLZ_CMD_SELECT)
        {
            if (cmd_ == BLZ_CMD_SELECT) /* if autorun mode and user press space/return => go back and cancel autorun */
                if (g_screens.compomode == false)
                    if (g_screens.persistent.menu.playmode == BLZ_PLAY_AUTORUN)
                        g_screens.persistent.menu.playmode = BLZ_PLAY_SCOREDRIVEN;

            g_screens.runscreen = BLZ_EP_MENU;
            STDmset (HW_COLOR_LUT, 0UL, 32);
            FSMgotoNextState(&g_stateMachineIdle);
            FSMgotoNextState(&g_stateMachine);

            return true;
        }
    }        

    return false;
}
