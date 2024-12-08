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

#include "DEMOSDK\PC\EMUL.H"

#include "EXTERN\RELOCATE.H"
#include "EXTERN\WIZZCAT\PRTRKSTE.H"

#include "RELAPSE\SRC\SCREENS.H"
#include "RELAPSE\SRC\CASCADE\CASCADE.H"

#include "RELAPSE\RELAPSE2.H"


#define CASCADE_PICTUREW 240
#define CASCADE_PICTUREH 251
#define CASCADE_PITCH 168
#define CASCADE_HEIGHT 274
#define CASCADE_FONTH 54
#define CASCADE_MODULE_MARGIN_SIZE 64000UL
#define CASCADE_FRAMEBUFFER_SIZE ((u32)CASCADE_PITCH * (u32)CASCADE_HEIGHT * 2UL)
#define CASCADE_FRAME_AND_TEMP_BUFFER_SIZE 134000UL

#ifdef __TOS__

static CascadeAsmImport* cascadeAsmImport(void* _prxbuffer)
{
    return (CascadeAsmImport*)((DYNbootstrap)_prxbuffer)();
}

#else

void cascadeInit (void* param_) {}

void cascadeUpdate (void* param_) 
{
    Cascade* this = (Cascade*)param_;

    static u32 offset = 0UL;

    offset ^= CASCADE_FRAMEBUFFER_SIZE / 2UL;

    EMULfbExStart (HW_VIDEO_MODE_4P, 80, 20, 80 + 168 * 2 - 1, 20 + 274 - 1, 168, 0);
    EMULfbExEnd();

    SYSwriteVideoBase((u32)(this->framebuffer + offset));
}

void cascadeShutdown (void* param_) {}
void cascadeVbl      (void)         {}

static CascadeAsmImport* cascadeAsmImport(void* _prxbuffer)
{
    static CascadeAsmImport asmimport;

    asmimport.common.init     = (DYNevent)    cascadeInit;
    asmimport.common.update   = (DYNevent)    cascadeUpdate;
    asmimport.common.shutdown = (DYNevent)    cascadeInit;
    asmimport.vbl             = (SYSinterupt) cascadeVbl;

    return &asmimport;
}

#endif

static void cascadeDisplayImage(u8* dest_, u8* source_)
{
    u16 t;


    dest_ += 24;

    for (t = 0; t < CASCADE_PICTUREH; t++)
    {
        STDmcpy2(dest_, source_, CASCADE_PICTUREW / 2);
        dest_ += CASCADE_PITCH;
        source_ += CASCADE_PICTUREW / 2;
    }
}



void CascadeEntry (FSM* _fsm)
{
    EMUL_STATIC Cascade* this;
    u8* temp;
    u8* temp2;
    u8* temp3;
    u8  sndtrackindex = g_screens.persistent.altmuziks ? RSC_RELAPSE2_ZIKS_SHA_CASC_ARJX : RSC_RELAPSE2_ZIKS_DELOS_04_ARJX;
    u8  sndtrackmetadataindex = g_screens.persistent.altmuziks ? RSC_RELAPSE2_METADATA_ZIKS_SHA_CASC_ARJX : RSC_RELAPSE2_METADATA_ZIKS_DELOS_04_ARJX;
    u32 sntracksizenomargin = LOADmetadataOriginalSize (&RSC_RELAPSE2, sndtrackmetadataindex);
    u32 sndtracksize  = sntracksizenomargin + CASCADE_MODULE_MARGIN_SIZE;
    u32 asmbufsize    = 2000UL;
    u32 cascadetempsize = LOADmetadataOriginalSize(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_CASCADE_WATER1_ARJX) + LOADmetadataOriginalSize(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_CASCADE_WATER2_ARJX);

    
    sndtracksize &= 0xFFFFFFFCUL;

    EMUL_BEGIN_ASYNC_REGION
        
    g_screens.cascade = this = MEM_ALLOC_STRUCT( &sys.allocatorMem, Cascade );
    DEFAULT_CONSTRUCT(this);

    ASSERT(LOADresourceRoundedSize (&RSC_RELAPSE2, sndtrackindex) <= CASCADE_FRAME_AND_TEMP_BUFFER_SIZE);

    this->sndtrack    =       MEM_ALLOC ( &sys.allocatorMem, sndtracksize );   
    this->framebuffer =       MEM_ALLOC ( &sys.allocatorMem, CASCADE_FRAME_AND_TEMP_BUFFER_SIZE);
    this->font        = (u8*) MEM_ALLOC ( &sys.allocatorMem, LOADmetadataOriginalSize (&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_CASCADE_CYBERFNT_ARJX) );
    this->asmbuf      =       MEM_ALLOC ( &sys.allocatorMem, asmbufsize );

    temp  = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, LOADresourceRoundedSize (&RSC_RELAPSE2, RSC_RELAPSE2_CASCADE_DATA_ARJX) ); 
    temp2 = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, LOADresourceRoundedSize (&RSC_RELAPSE2, sndtrackindex) ); 
    temp3 = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, cascadetempsize);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(4);

    g_screens.persistent.loadRequest[0] = LOADdata (&RSC_RELAPSE2, RSC_RELAPSE2_CASCADE_DATA_ARJX, temp, LOAD_PRIORITY_INORDER);
    g_screens.persistent.loadRequest[1] = LOADdata (&RSC_RELAPSE2, sndtrackindex, temp2, LOAD_PRIORITY_INORDER);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[0]);
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(3);

    STDmcpy2(this->colors, temp, 32);

    RELAPSE_UNPACK (temp3, temp + LOADmetadataOffset(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_CASCADE_WATER1_ARJX));
    RELAPSE_UNPACK (temp3 + LOADmetadataOriginalSize(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_CASCADE_WATER1_ARJX), temp + LOADmetadataOffset(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_CASCADE_WATER2_ARJX));

    RELAPSE_UNPACK (this->font, temp + LOADmetadataOffset (&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_CASCADE_CYBERFNT_ARJX) );
    BITpl2chunk(temp3, (u16)(cascadetempsize / 8), 1, 0, temp3);
    BITpl2chunk(this->font, CASCADE_FONTH*26, 2, 0, this->font);
    
    STDfastmset (this->framebuffer, 0UL, CASCADE_FRAMEBUFFER_SIZE);
    cascadeDisplayImage(this->framebuffer + CASCADE_PITCH * 6, temp3);
    cascadeDisplayImage(this->framebuffer + CASCADE_PITCH * 6 + CASCADE_FRAMEBUFFER_SIZE / 2UL, temp3 + LOADmetadataOriginalSize(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_CASCADE_WATER1_ARJX));

    STDfastmset (this->sndtrack, 0, sndtracksize);

    g_screens.persistent.loadRequest[2] = LOADdata (&RSC_RELAPSE2, RSC_RELAPSE2_______OUTPUT_RELAPSE_CASCADE_ARJX, temp, LOAD_PRIORITY_INORDER);
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(2);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[1]);               
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(1);

    RELAPSE_UNPACK (this->sndtrack, temp2);
    MX2MconvertDPCM2Pulse(this->sndtrack, sntracksizenomargin);
    WIZmodInit (this->sndtrack, this->sndtrack + sndtracksize);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[2]);
    STDmset (this->asmbuf, 0UL, asmbufsize);
    RELAPSE_UNPACK (this->asmbuf, temp);
    SYSrelocate (this->asmbuf);

    this->asmimport = cascadeAsmImport(this->asmbuf);  

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(0);

    MEM_FREE (&sys.allocatorMem, temp);
    MEM_FREE (&sys.allocatorMem, temp2);
    MEM_FREE (&sys.allocatorMem, temp3);

    EMUL_DELAY(100);

    FSMgotoNextState (_fsm);

    EMUL_END_ASYNC_REGION
}


void CascadePostInit (FSM* _fsm)
{
    Cascade* this = g_screens.cascade;
    CascadeInitParam params;
    u16 t;


    ScreensSetVideoMode (HW_VIDEO_MODE_4P, 0, 1);

    params.framebuffer = this->framebuffer;
    params.font = this->font;
    params.pal = this->colors;

    this->asmimport->common.init(&params);

    WIZplay();

    SYSwriteVideoBase((u32)this->framebuffer);

    SYSvblroutines[0] = this->asmimport->vbl;
    SYSvblroutines[1] = WIZrundma;
    SYSvblroutines[2] = WIZstereo;

    for (t = 0; t <= 16; t++)
    {
        SYSvsync;
        COLcomputeGradient16Steps(this->black, this->colors, 16, t, HW_COLOR_LUT);
    }

    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState(_fsm);
}


void CascadeActivity (FSM* _fsm)
{       
    Cascade* this = g_screens.cascade;

    IGNORE_PARAM(_fsm);

    this->asmimport->common.update (this);

    if (g_screens.persistent.menumode == false)
    {
        WIZinfo info;

        WIZgetInfo(&info);
        
        if (info.songpos >= 0x8)
            g_screens.next = true;
    }

    if (g_screens.next)
    {
        FSMgotoNextState (&g_stateMachineIdle);
        FSMgotoNextState (_fsm);
    }
}


void CascadeExit (FSM* _fsm)
{
    Cascade* this = g_screens.cascade;


    ScreenFadeOut();

    this->asmimport->common.shutdown(NULL);

    WIZstop();

    SYSvblroutines[0] = SYSvblend;
    SYSvblroutines[1] = SYSvblend;
    SYSvblroutines[2] = SYSvblend;
    SYSvsync;
    RASnextOpList = NULL;

    MEM_FREE ( &sys.allocatorMem, this->asmbuf );
    MEM_FREE ( &sys.allocatorMem, this->font );
    MEM_FREE ( &sys.allocatorMem, this->framebuffer );
    MEM_FREE ( &sys.allocatorMem, this->sndtrack );

    MEM_FREE(&sys.allocatorMem, this);
    g_screens.cascade = NULL;

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    ScreenNextState(_fsm);
}
