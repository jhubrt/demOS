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


#ifndef SCREENS_H
#define SCREENS_H

#include "DEMOSDK\FSM.H"

#include "DEMOSDK\BLSSND.H"

#include "BLITZIK\BLITZWAV.H"
#include "BLITZIK\SRC\COMMANDS.H"

#include "BLITZIK\SRC\INTRO\INTRO.H"
#include "BLITZIK\SRC\SSHADE\SSHADE.H"
#include "BLITZIK\SRC\SAMSCROL\SAMSCROL.H"
#include "BLITZIK\SRC\SAMCURVE\SAMCURVE.H"
#include "BLITZIK\SRC\ALLCURVE\ALLCURVE.H"
#include "BLITZIK\SRC\LAYERZ\LAYERZ.H"
#include "BLITZIK\SRC\SPACEWAV\SPACEWAV.H"
#include "BLITZIK\SRC\WAVHERO\WAVHERO.H"
#include "BLITZIK\SRC\MENU\MENU.H"
#include "BLITZIK\SRC\LOADER\LOADER.H"
#include "BLITZIK\SRC\INFO\INFO.H"

#define BLZ_DEVMODE()       0
#define BLZ_NBMAX_COMMANDS  (1<<5)

struct Base_
{
    s16* sin;
    s16* cos;
};
typedef struct Base_ Base;

struct ScreensInputs_
{    
    u8            commands[BLZ_NBMAX_COMMANDS];
    u8            firstcommand;
    u8            lastcommand;
};
typedef struct ScreensInputs_ ScreensInputs;


enum ScreensPlaymode_
{
    BLZ_PLAY_SCOREDRIVEN,
    BLZ_PLAY_INTERACTIVE,
    BLZ_PLAY_FROM_MENU,
    BLZ_PLAY_AUTORUN
};
typedef enum ScreensPlaymode_ ScreensPlaymode;

enum ScreensEntryPoints_
{
    BLZ_EP_SHADE_SOUND, /* screen num should be 0 based */
    BLZ_EP_SAM_CURVE,
    BLZ_EP_ALL_CURVE,
    BLZ_EP_SAM_SCROLL,
    BLZ_EP_LAYERZ,
    BLZ_EP_SPACEWAV,
    BLZ_EP_WAVHERO,

    BLZ_EP_LOADER,
    BLZ_EP_INTRO,
    BLZ_EP_MENU,
    BLZ_EP_INFO,
    BLZ_EP_WAIT_FOR_FX,
    BLZ_EP_NBENTRYPOINTS
};
typedef enum ScreensEntryPoints_ ScreensEntryPoints;

enum ScreensPhase_
{
    BLZ_PHASE_INTRO,
    BLZ_PHASE_MENU,
    BLZ_PHASE_FX,
    BLZ_PHASE_INFOSCREEN
};
typedef enum ScreensPhase_ ScreensPhase;


struct ScreensFSM_
{
    u8 stateEntryPoints[BLZ_EP_NBENTRYPOINTS];
    u8 stateIdleEntryPoints[BLZ_EP_NBENTRYPOINTS];
};
typedef struct ScreensFSM_ ScreensFSM;

struct ScreensPersistentData_
{
    BlitZMenuPersistentData menu;
};
typedef struct ScreensPersistentData_ ScreensPersistentData;


struct FXSequence_
{
    u16* framesmap;
    u8*  seq;
    u8*  current;
    u16  size;
};
typedef struct FXSequence_ FXSequence;


struct Screens_
{
    Cybervector*  cybervector;
    BlitZLoader*  loader;
    SShade*       sshade;
    SamScroll*    samscroll;
    SamCurve*     samcurve;
    SpaceWav*     spacewav;
    WavHero*      wavhero;
    AllCurve*     allcurve;
    LayerZ*       layerz;
    BlitZMenu*    menu;
    Info*         info;
                  
    Base            base;
    SShadeStatic    sshadeStatic;
    LayerZStatic    layerzStatic;
    WavHeroStatic   wavheroStatic;
    AllCurveStatic  allcurveStatic;
    SamCurveStatic  samcurveStatic;
    SamScrollStatic samscrollStatic;
                  
    SYSinterupt   blzupdateroutine;
    BLSplayer     player;
    FXSequence    fxsequence;
    
    u8*           preload;
    u8*           sampleToSourceSample;
    u8            sndtrackKeyMin;
    u8            sndtrackKeyMax;

    u8            bass;
    u8            treble;
                  
    u8            runningmodule;
                  
    u8            runscreen;
    u8            runningphase;
    
    bool          compomode;
    bool          dmaplayoncemode;

    ScreensInputs inputs;
    ScreensFSM    fsm;
    ScreensPersistentData persistent;
};
typedef struct Screens_ Screens;


#define BLZ_COMMAND_AVAILABLE   (g_screens.inputs.firstcommand != g_screens.inputs.lastcommand)
#define BLZ_CURRENT_COMMAND     g_screens.inputs.commands[g_screens.inputs.firstcommand]
#define BLZ_ITERATE_COMMAND     TRAClogNumberS(TRAC_LOG_COMMANDS, "popcmd", g_screens.inputs.commands[g_screens.inputs.firstcommand], 2, '\n'); g_screens.inputs.firstcommand++; g_screens.inputs.firstcommand &= BLZ_NBMAX_COMMANDS - 1;
#define BLZ_FLUSH_COMMANDS      g_screens.inputs.firstcommand = g_screens.inputs.lastcommand

/*#ifdef __TOS__
#define BLZ_PUSH_COMMAND(CMD)   { g_screens.inputs.commands[g_screens.inputs.lastcommand++] = (CMD); g_screens.inputs.lastcommand &= BLZ_NBMAX_COMMANDS - 1; ASSERT(g_screens.inputs.firstcommand != g_screens.inputs.lastcommand); }
#else*/
#define BLZ_PUSH_COMMAND(CMD)   { TRAClogNumberS(TRAC_LOG_COMMANDS, "pushcmd", CMD, 2, '\n'); g_screens.inputs.commands[g_screens.inputs.lastcommand++] = CMD; g_screens.inputs.lastcommand &= BLZ_NBMAX_COMMANDS - 1; ASSERT(g_screens.inputs.firstcommand != g_screens.inputs.lastcommand); }
/*#endif*/

#ifdef __TOS__
#define BLZ_TRAC_COMMAND(NAME)
#define BLZ_TRAC_COMMAND_NUM(NAME,NUM)
#else
#define BLZ_TRAC_COMMAND(NAME)  TRAClog(TRAC_LOG_COMMANDS, NAME, '\n')
#define BLZ_TRAC_COMMAND_NUM(NAME,NUM)  TRAClogNumber10S(TRAC_LOG_COMMANDS, NAME, NUM, 2, '\n')
#endif

#ifndef SCREEN_C
extern Screens      g_screens;
extern FSM          g_stateMachine;
extern FSM          g_stateMachineIdle;
extern FSMstate     states[];
extern FSMstate     statesIdle[];
extern u16          statesSize;
extern u16          statesIdleSize;
#endif


#define BLITZ_VIDEO_NOXTRA_PIXEL    0
#define BLITZ_VIDEO_16XTRA_PIXELS   0x100

void BlitZsetVideoMode (u8 vmode_, u8 voffset_, u16 xtrapix_);

void ScreensInit (void);
void ScreensGotoLoader (ScreensEntryPoints _returnEntryPoint);
void ScreensGotoScreen (void);
bool ScreensManageScreenChoice (ScreensEntryPoints _currentScreen, u8 cmd_);
void BlitZturnOffDisplay(void);

void BlitZmanageKey (void);
void BlitZrestartMod (void);

#ifdef DEMOS_DEBUG
void ScreensLogFreeArea(char* _title);
void ScreensDumpRingAllocator(MEMallocator* _allocator, void* _address, FILE* _file);
#else
#define ScreensLogFreeArea(TITLE)
#endif

#endif
