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

#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\ALLOC.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\DATA\SYSTFNT.H"

#include "EXTERN\RELOCATE.H"

#include "RELAPSE\SRC\SCREENS.H"
#include "RELAPSE\SRC\FASTMENU\FASTMENU.H"

#include "RELAPSE\RELAPSE1.H"


#define FASTMENU_FRAMEBUFFER_SIZE 32256
#define FASTMENU_ASMSIZE 10000


#ifdef __TOS__

static FastmenuAsmImport* fastmenuAsmImport(void* _prxbuffer)
{
    return (FastmenuAsmImport*)((DYNbootstrap)_prxbuffer)();
}

#else

void fastmenuInit     (void* param_) {}
void fastmenuUpdate   (void* param_) {}

static FastmenuAsmImport* fastmenuAsmImport(void* _prxbuffer)
{
    static FastmenuAsmImport asmimport;

    asmimport.init     = (DYNevent)    fastmenuInit;
    asmimport.update   = (DYNevent)    fastmenuUpdate;

    return &asmimport;
}

#endif


void FastmenuEntry (FSM* _fsm)
{
    Fastmenu* this;
    FastmenuInitParam params;   
   

#   ifndef __TOS__
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRAClog(TRAC_LOG_MEMORY,"Intro memallocator dump", '\n');

        TRACmemDump(&sys.allocatorCoreMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorCoreMem.allocator, stdout);

        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif

    g_screens.fastmenu = this = MEM_ALLOC_STRUCT( &sys.allocatorMem, Fastmenu );
    DEFAULT_CONSTRUCT(this);

    this->framebuffer = MEM_ALLOC ( &sys.allocatorMem, FASTMENU_FRAMEBUFFER_SIZE);
    this->asmbuf      = MEM_ALLOC ( &sys.allocatorMem, FASTMENU_ASMSIZE );

    ScreensSetVideoMode(HW_VIDEO_MODE_2P, 0, 0);

    STDmset (this->asmbuf, 0UL, FASTMENU_ASMSIZE);
    RELAPSE_UNPACK (this->asmbuf, g_screens.persistent.fastmenu);
    SYSrelocate (this->asmbuf);

    this->asmimport = fastmenuAsmImport(this->asmbuf);

    STDfastmset(this->framebuffer, 0UL, FASTMENU_FRAMEBUFFER_SIZE);

    params.font = SYSfont.data;
    params.fontmap = SYSfont.charsmap;
    params.framebuffer = (u8*)(((u32)this->framebuffer + 256) & 0xFFFFFF00UL);
    params.nbnops = 1 - sys.isMegaSTe;

    SYSwriteVideoBase((u32)params.framebuffer);

#   ifndef __TOS__
    SYSfastPrint("FASTMENU", params.framebuffer, 160, 8, (u32)&SYSfont);
    STDmset(HW_COLOR_LUT+1, 0xFFFFFFFFUL, 30);
#   endif

    this->asmimport->init(&params);

    g_screens.persistent.not1stinto = true;

    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState (_fsm);
}


void FastmenuBacktask (FSM* _fsm)
{
    Fastmenu* this = g_screens.fastmenu;

#   ifndef __TOS__
    if (this->asmimport->exitflag == false)
        return;
#   endif
    this->asmimport->update (NULL);

    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState(_fsm);
}


void FastmenuActivity (FSM* _fsm)
{       
    Fastmenu* this = g_screens.fastmenu;

    IGNORE_PARAM(_fsm);

    this->asmimport->exitflag = true;
    g_screens.persistent.altmuziks = false;

    switch (g_screens.scancodepressed)
    {
    case HW_KEY_F1:
    case HW_KEY_F2:
    case HW_KEY_F3:
    case HW_KEY_F4:
    case HW_KEY_F5:
    case HW_KEY_F6:
    case HW_KEY_F7:
    case HW_KEY_F8:
    case HW_KEY_F9:
    case HW_KEY_F10:
        g_screens.persistent.menumode = true;
        this->singlescreenselection = g_screens.scancodepressed - HW_KEY_F1;
        break;

    case HW_KEY_0:
    case HW_KEY_NUMPAD_0:
        g_screens.persistent.menumode = true;
        g_screens.persistent.altmuziks = true;
        this->singlescreenselection = 9;
        break;

    case HW_KEY_4:
    case HW_KEY_NUMPAD_4:
        g_screens.persistent.menumode = true;
        g_screens.persistent.altmuziks = true;
        this->singlescreenselection = 3;
        break;

    case HW_KEY_6:
    case HW_KEY_NUMPAD_6:
        g_screens.persistent.menumode = true;
        g_screens.persistent.altmuziks = true;
        this->singlescreenselection = 5;
        break;

    case HW_KEY_TAB:
        g_screens.persistent.altmuziks = true;
        /* NO BREAK */

    case HW_KEY_RETURN:
    case HW_KEY_NUMPAD_ENTER:
        g_screens.persistent.menumode = false;
        break;

    default:
        this->asmimport->exitflag = false;
        break;
    }
}

static void FastMenuGoto(FSMfunction _stateIdle, FSMfunction _state)
{
    FSMgoto (&g_stateMachineIdle, FSMlookForStateIndex(&g_stateMachineIdle, _stateIdle) - 2);
    FSMgoto (&g_stateMachine    , FSMlookForStateIndex(&g_stateMachine    , _state    ) - 3);
}


void FastmenuExit (FSM* _fsm)
{
    Fastmenu* this = g_screens.fastmenu;

    IGNORE_PARAM(_fsm);

    MEM_FREE ( &sys.allocatorMem, this->asmbuf );
    MEM_FREE ( &sys.allocatorMem, this->framebuffer );

    if (g_screens.persistent.menumode)
    {
        switch (this->singlescreenselection)
        {
        case 0:     FastMenuGoto (IntroEntry,       IntroActivity);     break;
        case 1:     FastMenuGoto (LiquidEntry,      LiquidActivity);    break;
        case 2:     FastMenuGoto (EgyptiaEntry,     EgyptiaActivity);   break;
        case 3:     FastMenuGoto (GrafikSEntry,     GrafikSActivity);   break;
        case 4:     FastMenuGoto (InterludeEntry,   InterludeActivity); break;
        case 5:     FastMenuGoto (CascadeEntry,     CascadeActivity);   break;
        case 6:     FastMenuGoto (ShadeVAEntry,     ShadeVAActivity);   break;
        case 7:     FastMenuGoto (SpaceFEntry,      SpaceFActivity);    break;
        case 8:     FastMenuGoto (EndEntry,         EndActivity);       break;
        case 9:     FastMenuGoto (InfoEntry,        InfoActivity);      break;
        default:
            ASSERT(0);
        }
    }
    else
    {
        FastMenuGoto(IntroEntry, IntroActivity);
        g_screens.persistent.menumode = false;
    }

    g_screens.gotomenu = false;

    MEM_FREE(&sys.allocatorMem, this);
    g_screens.fastmenu = NULL;

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );
}
