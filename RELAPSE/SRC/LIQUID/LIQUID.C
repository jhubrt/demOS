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

#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"

#include "EXTERN\WIZZCAT\PRTRKSTE.H"
#include "EXTERN\RELOCATE.H"

#include "RELAPSE\RELAPSE1.H"

#include "RELAPSE\SRC\SCREENS.H"
#include "RELAPSE\SRC\LIQUID\LIQUID.H"


#ifdef __TOS__

static LiquidAsmImport* liquidAsmImport(void* _prxbuffer)
{
    return (LiquidAsmImport*)((DYNbootstrap)_prxbuffer)();
}

#else

void liquidInit    (void* param_) {}
void liquidUpdate  (void* param_) {}
void liquidShutdown(void* param_) {}
void liquidVbl     (void)         {}

static LiquidAsmImport* liquidAsmImport(void* _prxbuffer)
{
    static LiquidAsmImport asmimport;

    asmimport.common.init     = (DYNevent) liquidInit;
    asmimport.common.update   = (DYNevent) liquidUpdate;
    asmimport.common.shutdown = (DYNevent) liquidInit;
    asmimport.vbl             = (DYNevent) liquidVbl;
    asmimport.exitflag        = false;

    return &asmimport;
}

#endif

#define LIQUID_MOD_MARGIN_SIZE 80002UL

void LiquidEntry (FSM* _fsm)
{
    EMUL_STATIC Liquid* this;
    void* temp, *temp2;
    u32 watersize    = LOADmetadataOriginalSize (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_LIQUID_WATER_ARJX);
    u32 fontsize     = LOADmetadataOriginalSize (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_LIQUID_FONT_ARJX);
    u32 sndtracksize = LOADmetadataOriginalSize (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_ZIKS_HYDRO_ARJX) + LIQUID_MOD_MARGIN_SIZE;
    u32 framesize    = 65536UL + 245UL*256UL;
    u32 asmbufsize   = 20000UL;

    EMUL_BEGIN_ASYNC_REGION
        
    IGNORE_PARAM(_fsm);

    ASSERT(framesize >= LOADresourceRoundedSize (&RSC_RELAPSE1, RSC_RELAPSE1_ZIKS_HYDRO_ARJX));
    ASSERT(LOADresourceRoundedSize (&RSC_RELAPSE1, RSC_RELAPSE1_LIQUID_FONT_ARJX) <= LOADresourceRoundedSize (&RSC_RELAPSE1, RSC_RELAPSE1_______OUTPUT_RELAPSE_LIQUID_ARJX));

    g_screens.liquid = this = MEM_ALLOC_STRUCT( &sys.allocatorMem, Liquid );
    DEFAULT_CONSTRUCT(this);

    this->font        =       MEM_ALLOC ( &sys.allocatorMem, fontsize );    
    this->asmbuf      =       MEM_ALLOC ( &sys.allocatorMem, asmbufsize );
    this->sndtrack    = (u8*) MEM_ALLOC ( &sys.allocatorMem, sndtracksize );
    this->framebuffer =       MEM_ALLOC ( &sys.allocatorMem, framesize );
    this->water       =       MEM_ALLOC ( &sys.allocatorMem, watersize );   

    temp  = MEM_ALLOCTEMP ( &sys.allocatorMem, LOADresourceRoundedSize (&RSC_RELAPSE1, RSC_RELAPSE1_LIQUID_WATER_ARJX) ); 
    temp2 = MEM_ALLOCTEMP ( &sys.allocatorMem, LOADresourceRoundedSize (&RSC_RELAPSE1, RSC_RELAPSE1_______OUTPUT_RELAPSE_LIQUID_ARJX) ); 

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(8);

    g_screens.persistent.loadRequest[0] = LOADdata (&RSC_RELAPSE1, RSC_RELAPSE1_LIQUID_FONT_ARJX , temp2, LOAD_PRIORITY_INORDER);
    g_screens.persistent.loadRequest[1] = LOADdata (&RSC_RELAPSE1, RSC_RELAPSE1_ZIKS_HYDRO_ARJX , this->framebuffer, LOAD_PRIORITY_INORDER);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED(g_screens.persistent.loadRequest[0]);
    RELAPSE_UNPACK (this->font, temp2);   
    BITpl2chunk(this->font, (u16)(STDdivu(fontsize, 160) & 0xFFFF), 20, 0, this->font);
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(7);

    g_screens.persistent.loadRequest[2] = LOADdata (&RSC_RELAPSE1, RSC_RELAPSE1_LIQUID_WATER_ARJX, temp , LOAD_PRIORITY_INORDER);
    RELAPSE_WAIT_LOADREQUEST_COMPLETED(g_screens.persistent.loadRequest[1]);
    g_screens.persistent.loadRequest[3] = LOADdata (&RSC_RELAPSE1, RSC_RELAPSE1_______OUTPUT_RELAPSE_LIQUID_ARJX, temp2, LOAD_PRIORITY_INORDER);

    STDfastmset (this->sndtrack, 0UL, sndtracksize);
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(6);
    RELAPSE_UNPACK (this->sndtrack, this->framebuffer);
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(5);
    MX2MconvertDPCM2Pulse(this->sndtrack, sndtracksize - LIQUID_MOD_MARGIN_SIZE);
    WIZmodInit (this->sndtrack, this->sndtrack + sndtracksize);
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(4);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[2]);
    
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(3);
    RELAPSE_UNPACK (this->water, temp);
    
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(2);
    BITpl2chunk(this->water, (u16)(STDdivu(watersize, 160) & 0xFFFF), 20, 0, this->water);
    
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(1);
    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[3]);
    STDfastmset (this->asmbuf, 0UL, asmbufsize);
    RELAPSE_UNPACK (this->asmbuf, temp2);
    SYSrelocate (this->asmbuf);   
    this->asmimport = liquidAsmImport(this->asmbuf);
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(0);

    /* TODO => try to put water unpack into hydro.mod loading */
    MEM_FREE (&sys.allocatorMem, temp);
    MEM_FREE (&sys.allocatorMem, temp2);

    EMUL_DELAY(1000);

    FSMgotoNextState (_fsm);

    EMUL_END_ASYNC_REGION
}


void LiquidPostInit (FSM* _fsm)
{
    Liquid* this = g_screens.liquid;
    LiquidInitParam init;

    init.framebuffer = (u8*)this->framebuffer + 65536UL;
    init.waterdata = this->water;
    init.font = this->font;

    this->asmimport->common.init(&init);

    WIZplay();

    SYSvsync;
    SYSvblroutines[0] = WIZrundma;
    SYSvblroutines[1] = (SYSinterupt) this->asmimport->vbl;
    SYSvblroutines[2] = WIZstereo;

    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState (_fsm);
}


void LiquidActivity (FSM* _fsm)
{       
    Liquid* this = g_screens.liquid;
    u16 vblcount = SYSvblLcount;

    IGNORE_PARAM(_fsm);

    g_screens.liquid->asmimport->common.update (NULL);

#   ifdef __TOS__
    while (SYSvblLcount == vblcount);
#   endif

    if (this->asmimport->exitflag == false)
    {
        if (g_screens.persistent.menumode == false)
        {
            this->framecount++;
            if (this->framecount >= 1300)
                g_screens.next = true;
        }

        if (g_screens.next)
        {
            g_screens.next = false;
            this->asmimport->exitflag = true;
            this->endcount = 64;
        }

        ASSERT(SYSvblLcount != (vblcount+2));
    }
    else 
    {
        this->endcount--;
        if (this->endcount == 0)
        {
            FSMgotoNextState(&g_stateMachineIdle);
            FSMgotoNextState(_fsm);
        }
    }
}

void LiquidExit (FSM* _fsm)
{
    Liquid* this = g_screens.liquid;


    ScreenFadeOutSound();

    WIZstop();

    SYSvblroutines[0] = SYSvblend;
    SYSvblroutines[1] = SYSvblend;
    SYSvblroutines[2] = SYSvblend;

    SYSvsync;

    /* important to call this after disabling overscan routine else it can end in infinite loop (because of line offset set to 0) */
    this->asmimport->common.shutdown(NULL); 

    MEM_FREE ( &sys.allocatorMem, this->sndtrack );
    MEM_FREE ( &sys.allocatorMem, this->asmbuf );
    MEM_FREE ( &sys.allocatorMem, this->font );
    MEM_FREE ( &sys.allocatorMem, this->water );
    MEM_FREE ( &sys.allocatorMem, this->framebuffer );

    MEM_FREE (&sys.allocatorMem, this);
    g_screens.liquid = NULL; 

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    ScreenNextState(_fsm);
}
