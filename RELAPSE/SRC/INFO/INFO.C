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
#include "RELAPSE\SRC\INFO\INFO.H"

#include "RELAPSE\RELAPSE2.H"


#define INFO_PICTUREW 240
#define INFO_PICTUREH 251
#define INFO_PITCH 168
#define INFO_SPACING 11
#define INFO_HEIGHT 274
#define INFO_FONTH 54
#define INFO_MODULE_MARGIN_SIZE 64000UL
#define INFO_FRAMEBUFFER_SIZE ((u32)INFO_PITCH * (u32)INFO_HEIGHT * 2UL)
#define INFO_SCROLLSTEP 8

#ifdef __TOS__

static InfoAsmImport* infoAsmImport(void* _prxbuffer)
{
    return (InfoAsmImport*)((DYNbootstrap)_prxbuffer)();
}

#else

void infoInit (void* param_) {}

void infoUpdate (void* param_) 
{
    Info* this = (Info*)param_;

    static u32 offset = 0UL;

    offset ^= INFO_FRAMEBUFFER_SIZE / 2UL;

    EMULfbExStart (HW_VIDEO_MODE_4P, 80, 20, 80 + 168 * 2 - 1, 20 + 274 - 1, 168, 0);
    EMULfbExEnd();

    SYSwriteVideoBase((u32)(this->textbuffer + offset));
}

void infoShutdown (void* param_) {}
void infoVbl      (void)         {}

static InfoAsmImport* infoAsmImport(void* _prxbuffer)
{
    static InfoAsmImport asmimport;

    asmimport.init = (DYNevent)    infoInit;
    asmimport.vbl  = (SYSinterupt) infoVbl;

    return &asmimport;
}

#endif

static u32 infoComputeTextBitmapSize(Info* this)
{
    char* p = this->text;

    this->nblines = 0;

    while (*p)
    {
        if (*p == 13)
            this->nblines++;

        p++;
    }

    this->nblines *= INFO_SPACING;
    return STDmulu (INFO_PITCH, this->nblines);
}


void InfoEntry (FSM* _fsm)
{
    EMUL_STATIC Info* this;
    u8* temp;
    u8* temp2;
    u8  sndtrackindex = g_screens.persistent.altmuziks ? RSC_RELAPSE2_ZIKS_EAGAN_ARJX : RSC_RELAPSE2_ZIKS_INFO_ARJX;
    u8  sndtrackmetadataindex = g_screens.persistent.altmuziks ? RSC_RELAPSE2_METADATA_ZIKS_EAGAN_ARJX : RSC_RELAPSE2_METADATA_ZIKS_INFO_ARJX;
    u32 sntracksizenomargin = LOADmetadataOriginalSize (&RSC_RELAPSE2, sndtrackmetadataindex);
    u32 sndtracksize  = sntracksizenomargin + INFO_MODULE_MARGIN_SIZE;
    u32 fontsize      = LOADmetadataOriginalSize (&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_INFO_FONT_ARJX);
    u32 textsize      = LOADmetadataOriginalSize(&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_INFO_INFO_ARJX);
    u32 asmbufsize    = 2000UL;


    sndtracksize &= 0xFFFFFFFCUL;

    EMUL_BEGIN_ASYNC_REGION
        
    g_screens.info = this = MEM_ALLOC_STRUCT( &sys.allocatorMem, Info );
    DEFAULT_CONSTRUCT(this);

    this->sndtrack    =       MEM_ALLOC ( &sys.allocatorMem, sndtracksize );   
    this->font        = (u8*) MEM_ALLOC ( &sys.allocatorMem, fontsize );
    this->asmbuf      =       MEM_ALLOC ( &sys.allocatorMem, asmbufsize );
    this->text        = (u8*) MEM_ALLOC ( &sys.allocatorMem, textsize + 1);

    temp  = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, LOADresourceRoundedSize (&RSC_RELAPSE2, RSC_RELAPSE2_INFO_INFO_ARJX) ); 
    temp2 = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, LOADresourceRoundedSize (&RSC_RELAPSE2, sndtrackindex) ); 

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(4);

    g_screens.persistent.loadRequest[0] = LOADdata (&RSC_RELAPSE2, RSC_RELAPSE2_INFO_INFO_ARJX, temp, LOAD_PRIORITY_INORDER);
    g_screens.persistent.loadRequest[1] = LOADdata (&RSC_RELAPSE2, sndtrackindex, temp2, LOAD_PRIORITY_INORDER);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[0]);
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(3);

    STDmcpy2(this->colors, temp, 32);
    RELAPSE_UNPACK (this->font, temp + LOADmetadataOffset (&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_INFO_FONT_ARJX) );
    BITpl2chunk(this->font, (u16)(STDdivu(fontsize, 160) & 0xFFFF), 20, 0, this->font);

    RELAPSE_UNPACK (this->text, temp + LOADmetadataOffset (&RSC_RELAPSE2, RSC_RELAPSE2_METADATA_INFO_INFO_ARJX) );
    this->text[textsize] = 0;
    {
        u32 size = infoComputeTextBitmapSize(this);
        this->textbuffer = (u8*) MEM_ALLOC ( &sys.allocatorMem, size);
        STDfastmset (this->textbuffer, 0, size);
    }

    STDfastmset (this->sndtrack, 0, sndtracksize);

    g_screens.persistent.loadRequest[2] = LOADdata (&RSC_RELAPSE2, RSC_RELAPSE2_______OUTPUT_RELAPSE_INFO_ARJX, temp, LOAD_PRIORITY_INORDER);
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

    this->asmimport = infoAsmImport(this->asmbuf);  

    this->asmimport->init(NULL);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(0);

    MEM_FREE (&sys.allocatorMem, temp);
    MEM_FREE (&sys.allocatorMem, temp2);

    EMUL_DELAY(100);

    FSMgotoNextState (_fsm);

    EMUL_END_ASYNC_REGION
}


void InfoPostInit (FSM* _fsm)
{
    Info* this = g_screens.info;


    ScreensSetVideoMode (HW_VIDEO_MODE_4P, 0, 1);
    /**HW_VIDEO_OFFSET = 84;*/
    SYSwriteVideoBase((u32)this->textbuffer);

    SYSvsync;

    STDmcpy2(HW_COLOR_LUT, this->colors, 32);

    WIZplay();

    SYSvsync;

    SYSvblroutines[0] = this->asmimport->vbl;
    SYSvblroutines[1] = WIZrundma;
    SYSvblroutines[2] = WIZstereo;

    {
        InfoInitParam params;

        params.text = this->text;
        params.font = this->font;
        params.textbuffer = this->textbuffer;

        this->asmimport->display(&params);
    }

    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState(_fsm);
}


void InfoActivity (FSM* _fsm)
{       
    Info* this = g_screens.info;

    IGNORE_PARAM(_fsm);

    this->flip++;
    if ((this->flip & 3) == 0)
    {
        this->targetline++;
    }

    if (this->targetline < 0)
    {
        this->targetline = this->displayedline = this->nblines - INFO_HEIGHT - INFO_SPACING;
    }
    else if ((this->targetline + INFO_HEIGHT) >= this->nblines)
    {
        this->displayedline = this->targetline = 0;

        if (g_screens.persistent.menumode == false)
            g_screens.next = true;
    }

    if (this->targetline != this->displayedline)
    {
        static u8 movecurve2[] = {1, 2, 3, 4};
        s16 speed = this->targetline - this->displayedline;

        if (speed < 0)
        {
            speed = -speed;
            speed >>= 3;
            if (speed >= ARRAYSIZE(movecurve2))
                speed = ARRAYSIZE(movecurve2) - 1;
            speed = movecurve2[speed];
            this->displayedline -= speed;
        }
        else
        {
            speed >>= 3;
            if (speed >= ARRAYSIZE(movecurve2))
                speed = ARRAYSIZE(movecurve2) - 1;
            speed = movecurve2[speed];
            this->displayedline += speed;
        }
    }

    SYSwriteVideoBase((u32)this->textbuffer + STDmulu(this->displayedline, INFO_PITCH));

    switch (g_screens.scancodepressed)
    {
    case HW_KEY_UP:
        this->targetline -= INFO_SCROLLSTEP;
        break;
    case HW_KEY_DOWN:
        this->targetline += INFO_SCROLLSTEP;
        break;
    }

    if (g_screens.next)
    {
        FSMgotoNextState (&g_stateMachineIdle);
        FSMgotoNextState (_fsm);
    }
}


void InfoExit (FSM* _fsm)
{
    Info* this = g_screens.info;


    ScreenFadeOut();
 
    *HW_VIDEO_OFFSET = 0;

    WIZstop();

    SYSvblroutines[0] = SYSvblend;
    SYSvblroutines[1] = SYSvblend;
    SYSvblroutines[2] = SYSvblend;
    
    SYSvsync;

    RASnextOpList = NULL;

    MEM_FREE ( &sys.allocatorMem, this->asmbuf );
    MEM_FREE ( &sys.allocatorMem, this->font );
    MEM_FREE ( &sys.allocatorMem, this->textbuffer );
    MEM_FREE ( &sys.allocatorMem, this->text );
    MEM_FREE ( &sys.allocatorMem, this->sndtrack );

    MEM_FREE(&sys.allocatorMem, this);
    g_screens.info = NULL;

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    ScreenNextState(_fsm);
}
