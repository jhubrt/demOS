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
#include "RELAPSE\SRC\SPACEF\SPACEF.H"

#include "RELAPSE\RELAPSE2.H"


#define SPACEF_MODULE_MARGIN_SIZE 50000UL
#define SPACEF_FRAMEBUFFER_SIZE (160UL * 256UL)


#ifdef __TOS__

static SpaceFAsmImport* spaceFAsmImport(void* _prxbuffer)
{
    return (SpaceFAsmImport*)((DYNbootstrap)_prxbuffer)();
}

#else

void spaceFInit     (void* param_) {}
void spaceFUpdate   (void* param_) {}
void spaceFShutdown (void* param_) {}

static SpaceFAsmImport* spaceFAsmImport(void* _prxbuffer)
{
    static SpaceFAsmImport asmimport;

    asmimport.common.init     = (DYNevent)    spaceFInit;
    asmimport.common.update   = (DYNevent)    spaceFUpdate;
    asmimport.common.shutdown = (DYNevent)    spaceFInit;

    return &asmimport;
}

#endif


void SpaceFEntry (FSM* _fsm)
{
    EMUL_STATIC SpaceF* this;
    u8* tempAsm;
    u8* tempCloud;
    u8* tempSndtrack;
    u32 sndtracksize = LOADmetadataOriginalSize (&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_ZIKS_FIRST6_ARJX) + SPACEF_MODULE_MARGIN_SIZE;
    u32 asmbufsize   = LOADmetadataOriginalSize (&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_______OUTPUT_RELAPSE_SPACEF_ARJX);
    u32 cloudsize    = LOADmetadataOriginalSize (&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_SPACEF_NUAGES2_ARJX);
    
    u32 tempSndtrackSize = LOADresourceRoundedSize(&RSC_RELAPSE2, RSC_RELAPSE2_ZIKS_FIRST6_ARJX);
    u32 tempAsmSize      = LOADresourceRoundedSize(&RSC_RELAPSE2, RSC_RELAPSE2_______OUTPUT_RELAPSE_SPACEF_ARJX);
    u32 tempCloudSize    = LOADresourceRoundedSize(&RSC_RELAPSE2, RSC_RELAPSE2_SPACEF_NUAGES2_ARJX);

    EMUL_BEGIN_ASYNC_REGION
        
    g_screens.spacef = this = MEM_ALLOC_STRUCT( &sys.allocatorMem, SpaceF );
    DEFAULT_CONSTRUCT(this);

    this->sndtrack    =       MEM_ALLOC ( &sys.allocatorMem, sndtracksize );   
    this->framebuffer =       MEM_ALLOC ( &sys.allocatorMem, SPACEF_FRAMEBUFFER_SIZE);
    this->asmbuf      =       MEM_ALLOC ( &sys.allocatorMem, asmbufsize );
    this->images      = (u8*) MEM_ALLOC ( &sys.allocatorMem, LOADresourceRoundedSize(&RSC_RELAPSE2, RSC_RELAPSE2_SPACEF_IMAGES_ARJX));
    this->cloud       = (u8*) MEM_ALLOC ( &sys.allocatorMem, cloudsize);
    this->temp        =       MEM_ALLOC ( &sys.allocatorMem, 33000UL);

    tempAsm      = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, tempAsmSize ); 
    tempCloud    = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, tempCloudSize ); 
    tempSndtrack = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, tempSndtrackSize ); 

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(4);

    g_screens.persistent.loadRequest[0] = LOADdata (&RSC_RELAPSE2, RSC_RELAPSE2_SPACEF_NUAGES2_ARJX, tempCloud, LOAD_PRIORITY_INORDER);
    g_screens.persistent.loadRequest[1] = LOADdata (&RSC_RELAPSE2, RSC_RELAPSE2_ZIKS_FIRST6_ARJX, tempSndtrack, LOAD_PRIORITY_INORDER);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[0]);
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(3);

    RELAPSE_UNPACK (this->cloud, tempCloud);
    BITpl2chunk(this->cloud, (u16)STDdivu(cloudsize,160), 20, 0, this->cloud);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(2);

    g_screens.persistent.loadRequest[2] = LOADdata (&RSC_RELAPSE2, RSC_RELAPSE2_______OUTPUT_RELAPSE_SPACEF_ARJX, tempAsm, LOAD_PRIORITY_INORDER);
    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[0]);
    g_screens.persistent.loadRequest[3] = LOADdata (&RSC_RELAPSE2, RSC_RELAPSE2_SPACEF_IMAGES_ARJX, this->images, LOAD_PRIORITY_INORDER);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[1]);

    STDfastmset (this->sndtrack, 0, sndtracksize);
    RELAPSE_UNPACK (this->sndtrack, tempSndtrack);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(1);

    MX2MconvertDPCM2Pulse(this->sndtrack, sndtracksize - SPACEF_MODULE_MARGIN_SIZE);
    WIZmodInit (this->sndtrack, this->sndtrack + sndtracksize);
   
    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[2]);               

    STDmset (this->asmbuf, 0UL, asmbufsize);
    RELAPSE_UNPACK (this->asmbuf, tempAsm);
    SYSrelocate (this->asmbuf);

    this->asmimport = spaceFAsmImport(this->asmbuf);

    STDfastmset(this->framebuffer, 0UL, SPACEF_FRAMEBUFFER_SIZE);

    MEM_FREE (&sys.allocatorMem, tempAsm);
    MEM_FREE (&sys.allocatorMem, tempCloud);
    MEM_FREE (&sys.allocatorMem, tempSndtrack);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[3]);
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(0);

    EMUL_DELAY(200);

    FSMgotoNextState (_fsm);

    EMUL_END_ASYNC_REGION
}

void decoden2(u8 *dst, u8* data);

void SpaceFPostInit (FSM* _fsm)
{
    SpaceF* this = g_screens.spacef;
    SpaceFInitParam params;

    params.depackroutine = (void*)decoden2;
    params.p2croutine    = (void*)BITpl2chunk;
    params.temp          = this->temp;
    params.cloud         = this->cloud;

    params.font = this->images + LOADmetadataOffset(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_SPACEF_IMAGE2_ARJX);

    params.pictures[0]   = this->images + LOADmetadataOffset(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_SPACEF_IMAGE4_ARJX);
    params.pictures[1]   = this->images + LOADmetadataOffset(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_SPACEF_IMAGE5_ARJX);
    params.pictures[2]   = this->images + LOADmetadataOffset(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_SPACEF_IMAGE3_ARJX);
    params.pictures[3]   = this->images + LOADmetadataOffset(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_SPACEF_IMAGE6_ARJX);
    params.pictures[4]   = this->images + LOADmetadataOffset(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_SPACEF_IMAGE7_ARJX);
    params.pictures[5]   = this->images + LOADmetadataOffset(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_SPACEF_IMAGE1_ARJX);
    params.pictures[6]   = this->images + LOADmetadataOffset(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_SPACEF_IMAGE8_ARJX);
    params.pictures[7]   = this->images + LOADmetadataOffset(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_SPACEF_IMAGE11_ARJX);
    params.pictures[8]   = this->images + LOADmetadataOffset(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_SPACEF_IMAGE9_ARJX);
    params.pictures[9]   = this->images + LOADmetadataOffset(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_SPACEF_IMAGE0_ARJX);
    params.pictures[10]  = this->images + LOADmetadataOffset(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_SPACEF_IMAGE10_ARJX);

    params.framebuffer = this->framebuffer;

    this->asmimport->common.init(&params);

    WIZplay();

    SYSvsync;

    SYSvblroutines[0] = WIZrundma;
    SYSvblroutines[1] = this->asmimport->vbl;
    SYSvblroutines[2] = WIZstereo;

    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState(_fsm);
}


void SpaceFBacktask (FSM* _fsm)
{
    SpaceF* this = g_screens.spacef;

    this->asmimport->common.update (NULL);

    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState(_fsm);
}


void SpaceFActivity (FSM* _fsm)
{       
    SpaceF* this = g_screens.spacef;

    IGNORE_PARAM(_fsm);

    if (g_screens.persistent.menumode == false)
        if (this->asmimport->haslooped)
            g_screens.next = true;

    if (g_screens.next)
        this->asmimport->exitflag = true;
}


void SpaceFExit (FSM* _fsm)
{
    SpaceF* this = g_screens.spacef;

    this->asmimport->common.shutdown(NULL);

    SYSvblroutines[1] = SYSvbldonothing;
    STDmset(HW_COLOR_LUT, 0UL, 32);

    ScreenFadeOutSound();

    WIZstop();

    SYSvblroutines[0] = SYSvblend;
    SYSvblroutines[1] = SYSvblend;
    SYSvblroutines[2] = SYSvblend;
    SYSvsync;

    MEM_FREE ( &sys.allocatorMem, this->images );
    MEM_FREE ( &sys.allocatorMem, this->cloud );
    MEM_FREE ( &sys.allocatorMem, this->asmbuf );
    MEM_FREE ( &sys.allocatorMem, this->framebuffer );
    MEM_FREE ( &sys.allocatorMem, this->sndtrack );
    MEM_FREE ( &sys.allocatorMem, this->temp );

    MEM_FREE(&sys.allocatorMem, this);
    g_screens.spacef = NULL;

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    ScreenNextState(_fsm);
}
