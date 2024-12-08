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

#include "DEMOSDK\Pc\EMUL.H"

#include "EXTERN\RELOCATE.H"
#include "EXTERN\WIZZCAT\PRTRKSTE.H"

#include "RELAPSE\SRC\SCREENS.H"
#include "RELAPSE\SRC\END\END.H"

#include "RELAPSE\RELAPSE2.H"


#define END_MODULE_MARGIN_SIZE 34002UL
#define END_ZOOM_BUFFERSIZE 228000UL
#define END_FRAMEBUFFER_SIZE (132UL * 256UL * 3UL + 256UL)


#ifdef __TOS__

static EndAsmImport* endAsmImport(void* _prxbuffer)
{
    return (EndAsmImport*)((DYNbootstrap)_prxbuffer)();
}

#else

void endInit     (void* param_) {}
void endUpdate   (void* param_) {}
void endShutdown (void* param_) {}

static EndAsmImport* endAsmImport(void* _prxbuffer)
{
    static EndAsmImport asmimport;

    asmimport.common.init     = (DYNevent)    endInit;
    asmimport.common.update   = (DYNevent)    endUpdate;
    asmimport.common.shutdown = (DYNevent)    endInit;

    return &asmimport;
}

#endif

static void endUnpackImages(End* this, u8* temp2)
{
    u8* d = this->images[0];
    u16 t, i = 0;

    for (t = RSC_RELAPSE2_METADATA_END_INTRO_ARJX; t <= RSC_RELAPSE2_METADATA_END_THE_END_ARJX; t++)
    {
        RELAPSE_UNPACK(d, temp2 + LOADmetadataOffset(&RSC_RELAPSE2, t));
        ASSERT(i < END_NBIMAGES);
        this->images[i++] = d;
        BITpl2chunk(d + 32, 200, 20, 0, d + 32);
        d += LOADmetadataOriginalSize(&RSC_RELAPSE2, t);
    }
}


void EndEntry (FSM* _fsm)
{
    EMUL_STATIC End* this;
    u8* temp;
    u8* temp2;
    u32 sndtracksize = LOADmetadataOriginalSize (&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_ZIKS_ENDMOD3_ARJX) + END_MODULE_MARGIN_SIZE;
    u32 asmbufsize   = LOADmetadataOriginalSize (&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_______OUTPUT_RELAPSE_END_ARJX);
    u16 imagesize    = (u16) LOADmetadataOriginalSize (&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_END_BARLOAD_ARJX);
    
    u32 tempsize     = LOADresourceRoundedSize(&RSC_RELAPSE2, RSC_RELAPSE2_______OUTPUT_RELAPSE_END_ARJX);
    u32 tempsize2    = LOADresourceRoundedSize(&RSC_RELAPSE2, RSC_RELAPSE2_END_END_ARJX);

    EMUL_BEGIN_ASYNC_REGION
        
    ASSERT(LOADresourceRoundedSize(&RSC_RELAPSE2, RSC_RELAPSE2_ZIKS_ENDMOD3_ARJX) <= END_FRAMEBUFFER_SIZE);

    g_screens.end = this = MEM_ALLOC_STRUCT( &sys.allocatorMem, End );
    DEFAULT_CONSTRUCT(this);

    this->sndtrack    =      MEM_ALLOC ( &sys.allocatorMem, sndtracksize );   
    this->framebuffer =      MEM_ALLOC ( &sys.allocatorMem, END_FRAMEBUFFER_SIZE);
    this->asmbuf      =      MEM_ALLOC ( &sys.allocatorMem, asmbufsize );
    this->images[0]   = (u8*)MEM_ALLOC ( &sys.allocatorMem, STDmulu(imagesize, END_NBIMAGES));

    temp  = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, tempsize ); 
    temp2 = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, tempsize2 );

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(4);

    g_screens.persistent.loadRequest[0] = LOADdata (&RSC_RELAPSE2, RSC_RELAPSE2_END_END_ARJX, temp2, LOAD_PRIORITY_INORDER);
    g_screens.persistent.loadRequest[1] = LOADdata (&RSC_RELAPSE2, RSC_RELAPSE2_ZIKS_ENDMOD3_ARJX, this->framebuffer, LOAD_PRIORITY_INORDER);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[0]);
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(3);

    endUnpackImages(this, temp2);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(2);

    STDfastmset (this->sndtrack, 0UL, sndtracksize);

    g_screens.persistent.loadRequest[2] = LOADdata (&RSC_RELAPSE2, RSC_RELAPSE2_______OUTPUT_RELAPSE_END_ARJX, temp, LOAD_PRIORITY_INORDER);
    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[1]);

    RELAPSE_UNPACK (this->sndtrack, this->framebuffer);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(1);

    MX2MconvertDPCM2Pulse(this->sndtrack, sndtracksize - END_MODULE_MARGIN_SIZE);
    WIZmodInit (this->sndtrack, this->sndtrack + sndtracksize);
   
    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[2]);               

    STDmset (this->asmbuf, 0UL, asmbufsize);
    RELAPSE_UNPACK (this->asmbuf, temp);
    SYSrelocate (this->asmbuf);

    this->asmimport = endAsmImport(this->asmbuf);

    STDfastmset(this->framebuffer, 0UL, END_FRAMEBUFFER_SIZE);

    MEM_FREE (&sys.allocatorMem, temp);
    MEM_FREE (&sys.allocatorMem, temp2);

    this->zoom = (u8*) MEM_ALLOC(&sys.allocatorMem, END_ZOOM_BUFFERSIZE);
    STDfastmset(this->zoom, 0UL, END_ZOOM_BUFFERSIZE);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(0);

    EMUL_DELAY(200);

    FSMgotoNextState (_fsm);

    EMUL_END_ASYNC_REGION
}


void EndPostInit (FSM* _fsm)
{
    End* this = g_screens.end;
    EndInitParam params;
    u16 t;


    SYSwriteVideoBase((u32)this->framebuffer);

    for (t = 0 ; t < END_NBIMAGES ; t++)
    {
        params.pictures[t] = this->images[t];
    }

    params.framebuffer = this->framebuffer;
    params.zoom = this->zoom;
    params.wizinfoCall = WIZgetInfo;

    this->asmimport->common.init(&params);

    WIZplay();

    SYSvsync;

    SYSvblroutines[0] = WIZrundma;
    SYSvblroutines[1] = WIZstereo;

    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState(_fsm);
}


void EndBacktask (FSM* _fsm)
{
    End* this = g_screens.end;

    this->asmimport->common.update (NULL);

    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState(_fsm);
}


void EndActivity (FSM* _fsm)
{       
    End* this = g_screens.end;

    IGNORE_PARAM(_fsm);

    if (g_screens.next)
        this->asmimport->exitflag = true;
}


void EndExit (FSM* _fsm)
{
    End* this = g_screens.end;


    ScreenFadeOut();

    this->asmimport->common.shutdown(NULL);

    WIZstop();

    SYSvblroutines[0] = SYSvblend;
    SYSvblroutines[1] = SYSvblend;
    SYSvblroutines[2] = SYSvblend;
    SYSvsync;
    RASnextOpList = NULL;

    MEM_FREE ( &sys.allocatorMem, this->images[0] );
    MEM_FREE ( &sys.allocatorMem, this->asmbuf );
    MEM_FREE ( &sys.allocatorMem, this->framebuffer );
    MEM_FREE ( &sys.allocatorMem, this->sndtrack );
    MEM_FREE ( &sys.allocatorMem, this->zoom );

    MEM_FREE(&sys.allocatorMem, this);
    g_screens.end = NULL;

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    ScreenNextState(_fsm);
}
