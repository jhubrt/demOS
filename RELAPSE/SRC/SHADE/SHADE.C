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
#include "DEMOSDK\BITMAP.H"
#include "DEMOSDK\MODX2MOD.H"
#include "DEMOSDK\CODEGEN.H"

#include "DEMOSDK\Pc\EMUL.H"

#include "EXTERN\RELOCATE.H"
#include "EXTERN\WIZZCAT\PRTRKSTE.H"

#include "RELAPSE\SRC\SCREENS.H"
#include "RELAPSE\SRC\SHADE\SHADE.H"

#include "RELAPSE\RELAPSE2.H"


#ifdef __TOS__
#define SHADE_RASTERIZE() 0
#else
#define SHADE_RASTERIZE() 0
#endif

#if SHADE_RASTERIZE()
#   define SHADE_RASTERIZE_COLOR(COLOR) *HW_COLOR_LUT=COLOR
#else
#   define SHADE_RASTERIZE_COLOR(COLOR)
#endif


#define SHADEVA_MODULE_MARGIN_SIZE 100000UL


#ifdef __TOS__

static ShadeVAAsmImport* shadeVAAsmImport(void* _prxbuffer)
{
    return (ShadeVAAsmImport*)((DYNbootstrap)_prxbuffer)();
}

#else

void shadeVAInit     (void* param_) {}
void shadeVAUpdate   (void* param_) {}
void shadeVAShutdown (void* param_) {}
void shadeDummy      (void)         {}

static ShadeVAAsmImport* shadeVAAsmImport(void* _prxbuffer)
{
    static ShadeVAAsmImport asmimport;

    asmimport.common.init     = (DYNevent)    shadeVAInit;
    asmimport.common.update   = (DYNevent)    shadeVAUpdate;
    asmimport.common.shutdown = (DYNevent)    shadeVAShutdown;
    asmimport.display         = shadeDummy;
    asmimport.erase           = NULL;

    return &asmimport;
}

#endif


#define SHADEVA_FRAMEBUFFER_SIZE 40000UL

void ShadeVAEntry (FSM* _fsm)
{
    EMUL_STATIC ShadeVA* this;

    u32 sndtracksize     = LOADmetadataOriginalSize (&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_ZIKS_DELOS_0C_ARJX) + SHADEVA_MODULE_MARGIN_SIZE;
    u32 asmbufsize       = 141000UL;    
    u32 codetempsize     = LOADresourceRoundedSize(&RSC_RELAPSE2, RSC_RELAPSE2_______OUTPUT_RELAPSE_SHADE_ARJX);
    u32 sndtracktempsize = LOADresourceRoundedSize(&RSC_RELAPSE2, RSC_RELAPSE2_ZIKS_DELOS_0C_ARJX);

    u8* sndtracktemp;
    u8* codetemp;

    EMUL_BEGIN_ASYNC_REGION
        
    g_screens.shade = this = MEM_ALLOC_STRUCT( &sys.allocatorMem, ShadeVA );
    DEFAULT_CONSTRUCT(this);

    this->asmbuf      = MEM_ALLOC ( &sys.allocatorMem, asmbufsize );
    this->sndtrack    = MEM_ALLOC ( &sys.allocatorMem, sndtracksize );   
    this->framebuffer = MEM_ALLOC ( &sys.allocatorMem, SHADEVA_FRAMEBUFFER_SIZE);

    sndtracktemp  = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, sndtracktempsize ); 
    codetemp      = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, codetempsize ); 

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(5);

    g_screens.persistent.loadRequest[0] = LOADdata (&RSC_RELAPSE2, RSC_RELAPSE2_______OUTPUT_RELAPSE_SHADE_ARJX, codetemp, LOAD_PRIORITY_INORDER);
    g_screens.persistent.loadRequest[1] = LOADdata (&RSC_RELAPSE2, RSC_RELAPSE2_ZIKS_DELOS_0C_ARJX, sndtracktemp, LOAD_PRIORITY_INORDER);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[0]);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(4);

    STDmset (this->asmbuf, 0UL, asmbufsize);
    RELAPSE_UNPACK (this->asmbuf, codetemp);
    SYSrelocate (this->asmbuf);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(3);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[1]);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(2);

    STDfastmset (this->sndtrack, 0, sndtracksize);
    RELAPSE_UNPACK (this->sndtrack, sndtracktemp);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(1);

    MX2MconvertDPCM2Pulse(this->sndtrack, sndtracksize - SHADEVA_MODULE_MARGIN_SIZE);
    WIZmodInit (this->sndtrack, this->sndtrack + sndtracksize);

    this->asmimport = shadeVAAsmImport(this->asmbuf);  
    WIZplay();

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(0);

    MEM_FREE (&sys.allocatorMem, sndtracktemp);
    MEM_FREE (&sys.allocatorMem, codetemp);

    EMUL_DELAY(1000);

    FSMgotoNextState (_fsm);

    EMUL_END_ASYNC_REGION
}


void ShadeVAPostInit (FSM* _fsm)
{
    ShadeVA* this = g_screens.shade;
    ShadeVAInitParam params;

    params.framebuffer = this->framebuffer;
    params.colorsdest = this->vbl.colors;

    this->asmimport->common.init(&params);   
    SYSwriteVideoBase((u32)this->framebuffer);

    this->vbl.scanLinesTo1stInterupt = 199;
    this->vbl.nextRasterRoutine = this->asmimport->erase;

    RASnextOpList = &this->vbl;

    SYSvsync;

    SYSvblroutines[0] = WIZrundma;
    SYSvblroutines[1] = RASvbl16;
    SYSvblroutines[2] = this->asmimport->display;
    SYSvblroutines[3] = WIZstereo;

    {
        WIZinfo info;

        do
        {
            WIZgetInfo(&info);
        }
        while (info.pattpos < 0xA);
    }

    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState(_fsm);
}


void ShadeVAActivity (FSM* _fsm)
{       
    ShadeVA* this = g_screens.shade;

    IGNORE_PARAM(_fsm);

    SHADE_RASTERIZE_COLOR(7);
    this->asmimport->common.update (NULL);
    SHADE_RASTERIZE_COLOR(0);

    if (g_screens.persistent.menumode == false)
    {
        WIZinfo info;

        WIZgetInfo(&info);

        if (info.songpos >= 0x3F)
            g_screens.next = true;
    }

    if (g_screens.next)
    {
        g_screens.next = false;

        FSMgotoNextState (&g_stateMachineIdle);
        FSMgotoNextState (_fsm);
    }
}


void ShadeVAExit (FSM* _fsm)
{
    ShadeVA* this = g_screens.shade;
    
    this->asmimport->common.shutdown(NULL);

    SYSvblroutines[1] = SYSvbldonothing;
    SYSvblroutines[2] = SYSvbldonothing;

    ScreenFadeOut();

    WIZstop();

    SYSvblroutines[0] = SYSvblend;
    SYSvblroutines[1] = SYSvblend;
    SYSvblroutines[2] = SYSvblend;
    SYSvblroutines[3] = SYSvblend;
    *HW_MFP_TIMER_B_CONTROL = 0;
    SYSvsync;
    RASnextOpList = NULL;

    MEM_FREE ( &sys.allocatorMem, this->asmbuf );
    MEM_FREE ( &sys.allocatorMem, this->framebuffer );
    MEM_FREE ( &sys.allocatorMem, this->sndtrack );

    MEM_FREE(&sys.allocatorMem, this);
    g_screens.shade = NULL;

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    ScreenNextState(_fsm);
}
