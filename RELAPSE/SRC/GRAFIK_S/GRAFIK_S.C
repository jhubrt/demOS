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

#include "FX\COLPLANE\COLPLANE.H"

#include "DEMOSDK\PC\EMUL.H"

#include "EXTERN\RELOCATE.H"
#include "EXTERN\WIZZCAT\PRTRKSTE.H"

#include "RELAPSE\SRC\SCREENS.H"

#include "RELAPSE\RELAPSE1.H"


#define GRAFIKS_MODULE_MARGIN_SIZE 100002UL

#ifdef __TOS__

static GrafikSAsmImport* grafikSAsmImport(void* _prxbuffer)
{
    return (GrafikSAsmImport*)((DYNbootstrap)_prxbuffer)();
}

#else

void grafikSInit     (void* param_) {}
void grafikSUpdate   (void* param_) {}
void grafikSShutdown (void* param_) {}

static GrafikSAsmImport* grafikSAsmImport(void* _prxbuffer)
{
    static GrafikSAsmImport asmimport;

    asmimport.common.init = (DYNevent) grafikSInit;
    /*asmimport.common.update   = (DYNevent) grafikSUpdate;
    asmimport.common.shutdown = (DYNevent) grafikSInit;*/

    return &asmimport;
}

#endif


#define GRAFIKS_FRAMEBUFFER_SIZE 112000UL 
/*(74240UL + 256UL)*/


void grafikSInitModPeriodsTable(GrafikS* this)
{
    u8* temp = MEM_ALLOCTEMP(&sys.allocatorMem, MOD_PERIOD_BUFFERSIZE);
    u16 t;

    STDmset(temp, 0xFFFFFFFFUL, MOD_PERIOD_BUFFERSIZE);

    MODcreatePeriodConversionTable(temp);

    for (t = 0; t < MOD_PERIOD_BUFFERSIZE; t++)
    {
        u8 value = temp[t];

        if (value != 0xFF)
        {
            u8 octave = value >> 4;
            u8 key    = value & 0xF;

            switch (key)
            {
            case 0:  this->periodColors[t] = 0xF00; break;
            case 1:  this->periodColors[t] = 0xFF0; break; /* -- */
            case 2:  this->periodColors[t] = 0x4F0; break;
            case 3:  this->periodColors[t] = 0x0F0; break; /* -- */
            case 4:  this->periodColors[t] = 0x0F4; break;
            case 5:  this->periodColors[t] = 0x0FF; break;
            case 6:  this->periodColors[t] = 0x04F; break; /* -- */
            case 7:  this->periodColors[t] = 0x00F; break;
            case 8:  this->periodColors[t] = 0x40F; break; /* -- */
            case 9:  this->periodColors[t] = 0xF0F; break;
            case 10: this->periodColors[t] = 0xF4F; break; /* -- */
            case 11: this->periodColors[t] = 0xFF4; break;
            }

            this->equalizerPeriods[t] = (octave * 12 + key) << 1;
        }
        else
        {
            this->equalizerPeriods[t] = 0;
            this->periodColors[t] = 0;
        }
    }

    MEM_FREE(&sys.allocatorMem, temp);
}

void GrafikSEntry (FSM* _fsm)
{
    EMUL_STATIC GrafikS* this;

    u16 sndtrackindex    = g_screens.persistent.altmuziks ? RSC_RELAPSE1_ZIKS_GRAFIK_ARJX : RSC_RELAPSE1_ZIKS_NEW_TEC7_ARJX;
    u16 sndtrackmetadata = g_screens.persistent.altmuziks ? RSC_RELAPSE1_METADATA_ZIKS_GRAFIK_ARJX : RSC_RELAPSE1_METADATA_ZIKS_NEW_TEC7_ARJX;
    u8* temp;
    u8* temp2;
    u32 sndtracksize = LOADmetadataOriginalSize (&RSC_RELAPSE1, sndtrackmetadata) + GRAFIKS_MODULE_MARGIN_SIZE;
    u32 asmbufsize   = LOADmetadataOriginalSize (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_______OUTPUT_RELAPSE_GRAFIK_S_ARJX);
    u32 tempsize     = LOADresourceRoundedSize  (&RSC_RELAPSE1, sndtrackindex);


    EMUL_BEGIN_ASYNC_REGION
        
    g_screens.grafiks = this = MEM_ALLOC_STRUCT( &sys.allocatorMem, GrafikS );
    DEFAULT_CONSTRUCT(this);

    this->sndtrack    = MEM_ALLOC ( &sys.allocatorMem, sndtracksize );   
    this->framebuffer = MEM_ALLOC ( &sys.allocatorMem, GRAFIKS_FRAMEBUFFER_SIZE + 65535UL);
    this->asmbuf      = MEM_ALLOC ( &sys.allocatorMem, asmbufsize );

    temp  = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, tempsize ); 
    temp2 = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, LOADresourceRoundedSize (&RSC_RELAPSE1, RSC_RELAPSE1_______OUTPUT_RELAPSE_GRAFIK_S_ARJX));

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(4);

    g_screens.persistent.loadRequest[0] = LOADdata (&RSC_RELAPSE1, sndtrackindex, temp, LOAD_PRIORITY_INORDER);
    g_screens.persistent.loadRequest[1] = LOADdata (&RSC_RELAPSE1, RSC_RELAPSE1_______OUTPUT_RELAPSE_GRAFIK_S_ARJX, temp2, LOAD_PRIORITY_INORDER);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[0]);
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(3);

    STDfastmset (this->sndtrack, 0UL, sndtracksize);
    RELAPSE_UNPACK (this->sndtrack, temp);
    MX2MconvertDPCM2Pulse(this->sndtrack, sndtracksize - GRAFIKS_MODULE_MARGIN_SIZE);
    WIZmodInit (this->sndtrack, this->sndtrack + sndtracksize);

    {
        /*static u16 curve1_1regular[3] = {0x081, 0x023, 0x046};   */
        static u16 curve1_1regular[3] = {0x089, 0x823, 0x146}; 
        COLPinitColors3Praimbow(curve1_1regular, 0, this->colors3P);
    }


    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[1]);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(2);

    STDmset (this->asmbuf, 0UL, asmbufsize);
    RELAPSE_UNPACK (this->asmbuf, temp2);
    SYSrelocate (this->asmbuf);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(1);

    this->asmimport = grafikSAsmImport(this->asmbuf);
    
    this->asmimport->wizInfo = g_screens.playerInfo;
    this->asmimport->wizinfoCall = WIZgetInfo;
    this->asmimport->periodColors  = this->periodColors;
    this->asmimport->equalizerPeriods = this->equalizerPeriods;

    STDfastmset(this->framebuffer, 0UL, GRAFIKS_FRAMEBUFFER_SIZE + 65535UL);

    MEM_FREE (&sys.allocatorMem, temp);
    MEM_FREE (&sys.allocatorMem, temp2);

    grafikSInitModPeriodsTable(this);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(0);

    EMUL_DELAY(200);

    FSMgotoNextState (_fsm);

    EMUL_END_ASYNC_REGION
}


void GrafikSPostInit (FSM* _fsm)
{
    GrafikS* this = g_screens.grafiks;
    u32 framebuffer = (u32)this->framebuffer + 65535UL;
    GrafikSInitParam param;

    framebuffer &= 0xFFFF0000UL;

    SYSwriteVideoBase(framebuffer);

    WIZplay();

    SYSvsync;

    SYSvblroutines[0] = WIZrundma;
    SYSvblroutines[1] = this->asmimport->vbl;
    SYSvblroutines[2] = WIZstereo;

    this->asmimport->bufferec = (u8*) framebuffer;
    this->asmimport->wizfrontbufptr = &WIZfrontbuf;
    this->asmimport->colors3P = this->colors3P;

    param.ismegaste = sys.isMegaSTe;
    param.altmuziks = g_screens.persistent.altmuziks;
    this->asmimport->common.init(&param);

    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState(_fsm);
}


void GrafikSBacktask (FSM* _fsm)
{
    GrafikS* this = g_screens.grafiks;

    this->asmimport->common.update (NULL);

    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState(_fsm);
}


void GrafikSActivity (FSM* _fsm)
{       
    GrafikS* this = g_screens.grafiks;

    IGNORE_PARAM(_fsm);

#   if RELAPSE_DEV()
    if (g_screens.justpressed)
    {
        switch (g_screens.scancodepressed)
        {
        case HW_KEY_LEFT:
            if (this->asmimport->wait > 0)
                this->asmimport->wait--;
            break;
        case HW_KEY_RIGHT:
            if (this->asmimport->wait < 100)
                this->asmimport->wait++;
            break;
        case HW_KEY_DOWN:
            if (this->asmimport->wait2 > 0)
                this->asmimport->wait2--;
            break;
        case HW_KEY_UP:
            if (this->asmimport->wait2 < 31)
                this->asmimport->wait2++;
            break;
        case HW_KEY_NUMPAD_4:
            if (this->asmimport->starttimer > 0)
                this->asmimport->starttimer--;
            break;
        case HW_KEY_NUMPAD_6:
            if (this->asmimport->starttimer < 50)
                this->asmimport->starttimer++;
            break;
        }
    }
#   endif

    if (g_screens.persistent.menumode == false)
        if (this->asmimport->haslooped)
            g_screens.next = true;

    if (g_screens.next)
        this->asmimport->exit = true;
}


void GrafikSExit (FSM* _fsm)
{
    GrafikS* this = g_screens.grafiks;


    SYSvblroutines[1] = SYSvbldonothing;

    ScreenFadeOutSound();

    this->asmimport->common.shutdown(NULL);

    WIZstop();

    SYSvblroutines[0] = SYSvblend;
    SYSvblroutines[1] = SYSvblend;
    SYSvblroutines[2] = SYSvblend;
    SYSvsync;
    RASnextOpList = NULL;

    MEM_FREE ( &sys.allocatorMem, this->asmbuf );
    MEM_FREE ( &sys.allocatorMem, this->framebuffer );
    MEM_FREE ( &sys.allocatorMem, this->sndtrack );

    MEM_FREE(&sys.allocatorMem, this);
    g_screens.grafiks = NULL;

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    ScreenNextState(_fsm);
}
