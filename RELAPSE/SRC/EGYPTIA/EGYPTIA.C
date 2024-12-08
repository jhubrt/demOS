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
#include "RELAPSE\SRC\EGYPTIA\EGYPTIA.H"

#include "RELAPSE\RELAPSE1.H"


#define EGYPTIA_MODULE_MARGIN_SIZE 100002UL
#define EGYPTIA_FRAMEBUFFER_SIZE   ((u32)(EGYPTIA_TUBE_H * 160) * 3UL + 32000UL * 2UL + 256UL + 256UL)


#ifdef __TOS__

static EgyptiaAsmImport* egyptiaAsmImport(void* _prxbuffer)
{
    return (EgyptiaAsmImport*)((DYNbootstrap)_prxbuffer)();
}

#else

void egyptiaInit     (void* param_) {}
void egyptiaUpdate   (void* param_) {}
void egyptiaShutdown (void* param_) {}
void egyptiaVbl      (void)         {}

static EgyptiaAsmImport* egyptiaAsmImport(void* _prxbuffer)
{
    static EgyptiaAsmImport asmimport;

    asmimport.common.init     = (DYNevent)    egyptiaInit;
    asmimport.common.update   = (DYNevent)    egyptiaUpdate;
    asmimport.common.shutdown = (DYNevent)    egyptiaInit;
    asmimport.vbl             = (SYSinterupt) egyptiaVbl;

    return &asmimport;
}

#endif

static void egyptiaInitTube(Egyptia* this, u8* temp_)
{
    s16* sin = (s16*)(temp_ + LOADmetadataOffset (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_EGYPTIA_SIN_BIN));
    u16 t;
    u16 last, offset;

    PCENDIANSWAPBUFFER16(sin, 512);
    
    this->tubeinc[0] = 160;

    last = 0;
    
    for (t = 1; t < (ARRAYSIZE(this->tubeinc) - 1) ; t++)
    {
        u16 index = (t * 256 / (u16)ARRAYSIZE(this->tubeinc)) + 384;
        s32 value = STDmuls (sin[index & 511], ARRAYSIZE(this->tubeinc) / 2);

        value >>= 15;
        value +=  ARRAYSIZE(this->tubeinc) / 2;
        
        offset = (u16)value * 160;

        PCONLY (printf("%d: %u %u %u\n", t, value, offset, offset - last));

        this->tubeinc[t] = offset - last;
        last = offset;

    }

    offset = (ARRAYSIZE(this->tubeinc) - 1) * 160;

    PCONLY (printf("%d: %u %u %u\n", t, ARRAYSIZE(this->tubeinc) - 1, offset, offset - last));

    this->tubeinc[ARRAYSIZE(this->tubeinc) - 1] = offset - last;
}


void EgyptiaEntry (FSM* _fsm)
{
    EMUL_STATIC Egyptia* this;
    u8* temp;
    u8* temp2;

    u8  sndtrackindex = RSC_RELAPSE1_ZIKS_EGYPTZIK_ARJX;
    u8  sndtrackmetadataindex = RSC_RELAPSE1_METADATA_ZIKS_EGYPTZIK_ARJX;
    u32 sntracksizenomargin = LOADmetadataOriginalSize (&RSC_RELAPSE1, sndtrackmetadataindex);
    u32 sndtracksize    = sntracksizenomargin + EGYPTIA_MODULE_MARGIN_SIZE;
    u32 imagebuffersize = LOADmetadataOriginalSize (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_EGYPTIA_EGYPTIA_ARJX) + (u32)(EGYPTIA_TUBE_H * 160) * 2UL;
    u32 asmbufsize      = 50000UL;   
    u32 tempsize        = LOADresourceRoundedSize(&RSC_RELAPSE1, RSC_RELAPSE1_______OUTPUT_RELAPSE_EGYPTIA_ARJX);
    u32 tempsize2       = LOADresourceRoundedSize(&RSC_RELAPSE1, RSC_RELAPSE1_EGYPTIA_EGYPTIA_ARJX);

    
    sndtracksize &= 0xFFFFFFFCUL;

    EMUL_BEGIN_ASYNC_REGION
        
    if (tempsize < tempsize2)
        tempsize = tempsize2;

    ASSERT(LOADresourceRoundedSize(&RSC_RELAPSE1, sndtrackindex) <= EGYPTIA_FRAMEBUFFER_SIZE);

    g_screens.egyptia = this = MEM_ALLOC_STRUCT( &sys.allocatorMem, Egyptia );
    DEFAULT_CONSTRUCT(this);

    this->sndtrack    =       MEM_ALLOC ( &sys.allocatorMem, sndtracksize );   
    this->framebuffer =       MEM_ALLOC ( &sys.allocatorMem, EGYPTIA_FRAMEBUFFER_SIZE);
    this->imagebuffer = (u8*) MEM_ALLOC ( &sys.allocatorMem, imagebuffersize);
    this->font        = (u8*) MEM_ALLOC ( &sys.allocatorMem, LOADmetadataOriginalSize (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_EGYPTIA_FONT2_ARJX) );
    this->asmbuf      =       MEM_ALLOC ( &sys.allocatorMem, asmbufsize );
    this->code        =       MEM_ALLOC ( &sys.allocatorMem, 64000UL );
    
    this->image = this->imagebuffer + (EGYPTIA_TUBE_H * 160);

    temp = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, tempsize ); 
    temp2 = this->framebuffer;

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(5);

    g_screens.persistent.loadRequest[0] = LOADdata (&RSC_RELAPSE1, RSC_RELAPSE1_EGYPTIA_EGYPTIA_ARJX, temp, LOAD_PRIORITY_INORDER);
    g_screens.persistent.loadRequest[1] = LOADdata (&RSC_RELAPSE1, sndtrackindex, temp2, LOAD_PRIORITY_INORDER);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[0]);
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(4);

    STDfastmset(this->imagebuffer, 0UL, imagebuffersize);
    RELAPSE_UNPACK (this->image, temp);
    RELAPSE_UNPACK (this->font, temp + LOADmetadataOffset (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_EGYPTIA_FONT2_ARJX) );
    BITpl2chunk(this->image+32, 200, 20, 0, this->image+32);
    BITpl2chunk(this->font, 200, 20, 0, this->font);
    STDfastmset (this->sndtrack, 0, sndtracksize);

    egyptiaInitTube(this, temp);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(3);

    g_screens.persistent.loadRequest[2] = LOADdata (&RSC_RELAPSE1, RSC_RELAPSE1_______OUTPUT_RELAPSE_EGYPTIA_ARJX, temp, LOAD_PRIORITY_INORDER);
    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[2]);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(2);

    STDmset (this->asmbuf, 0UL, asmbufsize);
    RELAPSE_UNPACK (this->asmbuf, temp);
    SYSrelocate (this->asmbuf);

    this->asmimport = egyptiaAsmImport(this->asmbuf);  

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[1]);               
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(1);
    RELAPSE_UNPACK (this->sndtrack, temp2);
    MX2MconvertDPCM2Pulse(this->sndtrack, sntracksizenomargin);
    WIZmodInit (this->sndtrack, this->sndtrack + sndtracksize);
    STDfastmset(this->framebuffer, 0UL, EGYPTIA_FRAMEBUFFER_SIZE);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(0);

    MEM_FREE (&sys.allocatorMem, temp);

    EMUL_DELAY(1000);

    FSMgotoNextState (_fsm);

    EMUL_END_ASYNC_REGION
}


void EgyptiaPostInit (FSM* _fsm)
{
    Egyptia* this = g_screens.egyptia;
    EgyptiaInitParam params;
    u16 t;


    params.code = this->code;
    params.font = this->font;
    params.framebuffer = this->framebuffer;
    params.image = this->image;
    params.screenmargin = EGYPTIA_TUBE_H * 160;

    this->asmimport->common.init(&params);

    WIZplay();

    this->asmimport->rasters = this->currentcolors;
    this->scroll = -(EGYPTIA_TUBE_H * 160);

    SYSvsync;
    SYSvblroutines[0] = WIZrundma;
    SYSvblroutines[1] = this->asmimport->vbl;
    SYSvblroutines[2] = WIZstereo;

    STDmcpy2(this->colors[0]   , this->image, 32);
    STDmcpy2(this->colors[1]   , this->image, 16);
    STDmcpy2(this->colors[1]+8 , this->image, 16);

    for (t = 0; t <= 16; t++)
    {
        SYSvsync;
        COLcomputeGradient16Steps(this->black, &this->colors[1][1], 15, t, this->currentcolors);
    }

    STDmset(this->image, 0UL, 32);

    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState(_fsm);
}

enum EgyptiaScrollState
{
    EgyptiaScrollState_WAIT1,
    EgyptiaScrollState_GODOWN,
    EgyptiaScrollState_WAIT2,
    EgyptiaScrollState_GODOWN2,
    EgyptiaScrollState_WAIT3
};


void EgyptiaActivity (FSM* _fsm)
{       
    Egyptia* this = g_screens.egyptia;

    IGNORE_PARAM(_fsm);
       
    this->asmimport->common.update (NULL);
    
    switch (this->scrollstate)
    {
    case EgyptiaScrollState_WAIT1:
        this->scrollcount++;
        if (this->scrollcount > 50*5)
        {
            this->scrollstate = EgyptiaScrollState_GODOWN;
            this->scroll = -(EGYPTIA_TUBE_H * 160);
            this->scrollcount = 0;
        }
        break;

    case EgyptiaScrollState_GODOWN:
    case EgyptiaScrollState_GODOWN2:
        this->scroll += 160;
        if (this->scroll >= 200 * 160)
            this->scrollstate++;
        break;

    case EgyptiaScrollState_WAIT2:
        this->scrollcount++;
        if (this->scrollcount > 50*5)
        {
            this->scrollstate = EgyptiaScrollState_GODOWN2;
            this->scroll = -(EGYPTIA_TUBE_H * 160);
            this->scrollcount = 0;
        }
        break;
    }

    this->asmimport->copycall(this->image + 32 + this->scroll, (u8*)(this->asmimport->currentbackbuffer) + this->scroll, (u32) this->tubeinc, ARRAYSIZE(this->tubeinc));
    
    if (this->asmimport->fade != 0)
    {
        this->fadecount = 64;
        this->fade = this->asmimport->fade;
        this->asmimport->fade = 0;
    }

    if (this->fadecount > 0)
    {
        if (this->fade > 0)
        {
            COLcomputeGradient16Steps(&this->colors[0][8], &this->colors[1][8], 8, this->fadecount >> 2, this->currentcolors + 7);
            if (this->fadecount == 1)
                this->scrollstate = EgyptiaScrollState_WAIT1;
        }
        else
        {
            COLcomputeGradient16Steps(&this->colors[1][8], &this->colors[0][8], 8, this->fadecount >> 2, this->currentcolors + 7);
            if (this->fadecount == 1)
                this->shapecount++;
        }

        this->fadecount--;
    }

    if (g_screens.persistent.menumode == false)
    {
        if (this->shapecount >= 4)
            g_screens.next = true;
    }

    if (g_screens.next)
    {
        FSMgotoNextState (&g_stateMachineIdle);
        FSMgotoNextState (_fsm);
    }

#   ifdef DEMOS_DEBUG
    *HW_COLOR_LUT = 0;
#   endif
}


void EgyptiaExit (FSM* _fsm)
{
    Egyptia* this = g_screens.egyptia;


    SYSvblroutines[1] = SYSvbldonothing;

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
    MEM_FREE ( &sys.allocatorMem, this->imagebuffer );
    MEM_FREE ( &sys.allocatorMem, this->code );
    MEM_FREE ( &sys.allocatorMem, this->sndtrack );

    MEM_FREE(&sys.allocatorMem, this);
    g_screens.egyptia = NULL;

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    ScreenNextState(_fsm);   
}
