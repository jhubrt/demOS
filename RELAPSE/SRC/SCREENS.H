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


#ifndef SCREENS_H
#define SCREENS_H

#include "DEMOSDK\FSM.H"

#include "EXTERN\WIZZCAT\PRTRKSTE.H"

#define RELAPSE_DEV() 1

#include "RELAPSE\SRC\LOADER\PRELOAD.H"
#include "RELAPSE\SRC\SPLASH\SPLASH.H"
#include "RELAPSE\SRC\INTRO\INTRO.H"
#include "RELAPSE\SRC\LIQUID\LIQUID.H"
#include "RELAPSE\SRC\EGYPTIA\EGYPTIA.H"
#include "RELAPSE\SRC\GRAFIK_S\GRAFIK_S.H"
#include "RELAPSE\SRC\INTERLUD\INTERLUD.H"
#include "RELAPSE\SRC\CASCADE\CASCADE.H"
#include "RELAPSE\SRC\SHADE\SHADE.H"
#include "RELAPSE\SRC\SPACEF\SPACEF.H"
#include "RELAPSE\SRC\END\END.H"
#include "RELAPSE\SRC\INFO\INFO.H"
#include "RELAPSE\SRC\LOADER\LOADER.H"
#include "RELAPSE\SRC\FASTMENU\FASTMENU.H"


#define demOS_COREHEAPSIZE      (8UL * 1024UL)
#define demOS_HEAPSIZE          (900UL * 1024UL)


#define RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(nb_) g_screens.persistent.initprogress = nb_
#define RELAPSE_WAIT_LOADREQUEST_COMPLETED(request) { LOADwaitRequestCompleted(request); request = NULL; }
#define RELAPSE_NBMAX_LOADREQUESTS 4

void RELAPSE_UNPACK(void* to, void* from);

struct Persistent_
{
    bool menumode;
    bool altmuziks;
    bool not1stinto;

    u8   initprogress;
    u8*  dpackbase;

    struct LOADrequest_* loadRequest[RELAPSE_NBMAX_LOADREQUESTS];

    void* loaderprx;
    void* fastmenu;

    u16   fsmFastmenuStateIndex;
    u16   fsmFastmenuIdleStateIndex;
};
typedef struct Persistent_ Persistent;


struct Screens_
{    
    PreLoad*    preload;
    Splash*     splash;
    Intro*      intro;
    Liquid*     liquid;
    Egyptia*    egyptia;
    Interlude*  interlude;
    Cascade*    cascade;
    ShadeVA*    shade;
    SpaceF*     spacef;
    End*        end;
    GrafikS*    grafiks;
    Info*       info;
    Loader*     loader;
    Fastmenu*   fastmenu;

    Persistent  persistent;

    WIZplayerInfo playerInfo;

    bool use2drives;

    bool gotomenu;
    bool next;
    u8   scancodepressed;
    bool justpressed;

    u8   bass;
    u8   treble;
};
typedef struct Screens_ Screens;

typedef void(*DynLibEvent)(void);

struct DynInterface_
{
    DynLibEvent    init;
    DynLibEvent    update;
    DynLibEvent    shutdown;
};
typedef struct DynInterface_ DynInterface;

typedef void(*DynLibBootstrap)(DynInterface* _interface);

#ifndef SCREEN_C
extern Screens      g_screens;
extern FSM          g_stateMachine;
extern FSM          g_stateMachineIdle;
extern FSMstate     states[];
extern FSMstate     statesIdle[];
extern u16          statesSize;
extern u16          statesIdleSize;
extern u8           RELdmaOn;
#endif

void ScreensInit(void);
void ScreenWaitMainDonothing (void);
void ScreensSetVideoMode (u8 vmode_, u8 voffset_, u16 xtrapix_);
void ScreenNextState (FSM* _fsm);
void ScreenFadeOut(void);
void ScreenFadeOutSound(void);
u8 ScreenGetch(void);


#ifdef DEMOS_DEBUG
void ScreensDumpRingAllocator(MEMallocator* _allocator, void* _address, FILE* _file);
#endif

#endif
