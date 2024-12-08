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
#include "DEMOSDK\CODEGEN.H"
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
#include "RELAPSE\SRC\INTRO\INTRO.H"

#include "RELAPSE\RELAPSE1.H"


#ifdef __TOS__
#define INTRO_RASTERIZE() 0
#else
#define INTRO_RASTERIZE() 0
#endif

#if INTRO_RASTERIZE()
#   define INTRO_RASTERIZE_COLOR(COLOR) *HW_COLOR_LUT=COLOR
#else
#   define INTRO_RASTERIZE_COLOR(COLOR)
#endif


#define INTRO_MODULE_MARGIN_SIZE 28002UL

#define INTRO_SCREEN_PITCH (168+24)
#define INTRO_FRAMEBUFFER_SIZE ((u32)INTRO_SCREEN_PITCH * 200UL)
#define INTRO_SCREEN_H 200
#define INTRO_SPRITE_H 90
#define INTRO_SPRITE_PITCH 8
#define INTRO_SPRITE_COLOR 0x824

static void introAnim3D(Intro* this)
{
    u32 source = (u32) this->anim3d + this->anim3doffset;
    u32 dest   = (u32) this->backbuffer + 8 + 72 * INTRO_SCREEN_PITCH;

    *HW_BLITTER_ADDR_SOURCE = source;
    *HW_BLITTER_XINC_SOURCE = 6;
    *HW_BLITTER_YINC_SOURCE = 6;
    *HW_BLITTER_ENDMASK1 = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1;
    *HW_BLITTER_ADDR_DEST   = dest;
    *HW_BLITTER_XINC_DEST   = 8;
    *HW_BLITTER_YINC_DEST   = INTRO_SCREEN_PITCH - (19*8) + 8;
    *HW_BLITTER_XSIZE       = 19;
    *HW_BLITTER_YSIZE       = 56;
    *HW_BLITTER_HOP         = HW_BLITTER_HOP_SOURCE;
    *HW_BLITTER_OP          = HW_BLITTER_OP_S;
    *HW_BLITTER_CTRL2       = 0;
    *HW_BLITTER_CTRL1       = 0xC0;

    EMULblit();

    *HW_BLITTER_ADDR_SOURCE = source+2;
    *HW_BLITTER_ADDR_DEST   = dest+2;
    *HW_BLITTER_YSIZE       = 56;
    *HW_BLITTER_CTRL1       = 0xC0;

    EMULblit();

    *HW_BLITTER_ADDR_SOURCE = source+4;
    *HW_BLITTER_ADDR_DEST   = dest+4;
    *HW_BLITTER_YSIZE       = 56;
    *HW_BLITTER_CTRL1       = 0xC0;

    EMULblit();
}


static void introAnim3Dinc(Intro* this)
{
    this->anim3doffset += 56*19*6;
    if (this->anim3doffset >= this->anim3dsize)
        this->anim3doffset = 0;
}


static void introAnim(Intro* this)
{
    u8* display = this->backbuffer + 6;
    u8* sprite = this->logo;
    u16 s;

    /* Erase screen 1 plane */
    *HW_BLITTER_ADDR_DEST = (u32) display;
    *HW_BLITTER_XSIZE     = 21;
    *HW_BLITTER_YSIZE     = INTRO_SCREEN_H;
    *HW_BLITTER_XINC_DEST = 8;
    *HW_BLITTER_YINC_DEST = 8 + 24;
    *HW_BLITTER_HOP       = HW_BLITTER_HOP_BIT1;
    *HW_BLITTER_OP        = HW_BLITTER_OP_BIT0;
    *HW_BLITTER_CTRL2     = 0;
    *HW_BLITTER_CTRL1     = 0xC0;

    EMULblit();

    /* Draw sprites */
    *HW_BLITTER_HOP         = HW_BLITTER_HOP_SOURCE;
    *HW_BLITTER_XSIZE       = 4;
    *HW_BLITTER_XINC_DEST   = 8;
    *HW_BLITTER_YINC_DEST   = INTRO_SCREEN_PITCH-32+8;
    *HW_BLITTER_XINC_SOURCE = 2;
    *HW_BLITTER_YINC_SOURCE = 2;

    for (s = 0; s < 7; s++, sprite += INTRO_SPRITE_H*INTRO_SPRITE_PITCH)
    {
        s16 x, y, h, offsetx, offsety = 0;

        *HW_BLITTER_OP = s == 0 ? HW_BLITTER_OP_S : HW_BLITTER_OP_S_OR_D;

        x = this->x[s] >> 16;
        y = this->y[s] >> 16;

        if (y > (INTRO_SCREEN_H - INTRO_SPRITE_H))
            h = INTRO_SCREEN_H - y;
        else
            h = INTRO_SPRITE_H;

        if (y < 0)
        {
            h += y;
            offsety = -y;
            y = 0;
        }

        if ((x <= -48) || (x >= 336) || (h <= 0))
        {
            continue;
        }

        offsetx = x & 0xfff0;
        offsetx >>= 1;

        *HW_BLITTER_YSIZE = h;
        *HW_BLITTER_ADDR_SOURCE = (u32)(sprite + (offsety << 3)); /* INTRO_SPRITE_PITCH */
        *HW_BLITTER_ADDR_DEST = (u32)(display + STDmulu(y,INTRO_SCREEN_PITCH) + offsetx);
        *HW_BLITTER_CTRL2 = x & 0xF;
        *HW_BLITTER_CTRL1 = 0xC0;

        EMULblit();
    }
}


static void introSetSpritePos1(Intro* this)
{
    s16 t;

    for (t = 0 ; t < INTRO_NBSPRITES  ; t++)
    {
        this->x[t] = -128;
        this->y[t] = t & 1 ? 200 : -90;
    }

    /*for (t = 4 ; t < INTRO_NBSPRITES ; t++)
    {
    this->x[t] = 336+128;
    this->y[t] = t & 1 ? 200 : -90;
    }*/

    for (t = 0 ; t < INTRO_NBSPRITES  ; t++)
    {
        this->xend[t] = t * 48;
        this->yend[t] = (INTRO_SCREEN_H - INTRO_SPRITE_H) >> 1;
    }

    for (t = 0 ; t < INTRO_NBSPRITES  ; t++)
    {
        this->dx[t] = (this->xend[t] - this->x[t]) << (16-7);
        this->dy[t] = (this->yend[t] - this->y[t]) << (16-7);
        this->x[t] <<= 16;
        this->y[t] <<= 16;
    }
}

static void introSetSpritePos2(Intro* this)
{
    s16 t;

    /*for (t = 4 ; t < INTRO_NBSPRITES ; t++)
    {
    this->x[t] = 336+128;
    this->y[t] = t & 1 ? 200 : -90;
    }*/

    for (t = 0 ; t < INTRO_NBSPRITES  ; t++)
    {
        this->x[t] = t * 48;
        this->y[t] = (INTRO_SCREEN_H - INTRO_SPRITE_H) >> 1;
    }

    for (t = 0 ; t < INTRO_NBSPRITES  ; t++)
    {
        this->xend[t] = 336+128+100;
        this->yend[t] = t & 1 ? 150 : -40;
    }

    for (t = 0 ; t < INTRO_NBSPRITES  ; t++)
    {
        this->dx[t] = (this->xend[t] - this->x[t]) << (16-7);
        this->dy[t] = (this->yend[t] - this->y[t]) << (16-7);
        this->x[t] <<= 16;
        this->y[t] <<= 16;
    }
}


static void introInitRasters(Intro* this)
{
    u16* p;
    s16 t;
    u16 start = 0x111;
    u16 end   = 0x12F;
    u16 color;

    RASsetColReg(0x8242);

    this->vbl.nextRasterRoutine = (RASinterupt) this->rastersbuffer;
    this->vbl.scanLinesTo1stInterupt = 220;
   
    p = this->rastersbuffer;

    for (t = 0; t <= 16; t++)
    {       
        COLcomputeGradient16Steps(&start, &end, 1, t, &color);
        
        RAS_GEN_SET_COLOR_W(p, 1, color);

        if (t == 0)
        {
            RAS_GEN_STOP_TIMERB(p);
            RAS_GEN_SET_TIMERB_NBSCANLINES(p,1);
            RAS_GEN_SET_TIMERB_ADR(p,&p[7]);
            RAS_GEN_START_TIMERB(p);
        }
        else
        {
            RAS_GEN_SET_TIMERB_ADR(p,&p[4]);
        }

        *p++ = CGEN_OPCODE_RTE;
    }

    for (t = 15 ; t >= 0 ; t--)
    {
        COLcomputeGradient16Steps(&start, &end, 1, t, &color);

        RAS_GEN_SET_COLOR_W(p, 1, color);

        if (t > 0)
        {
            RAS_GEN_SET_TIMERB_ADR(p,&p[4]);
        }
        else
        {
            RAS_GEN_STOP_TIMERB(p);
        }

        *p++ = CGEN_OPCODE_RTE;
    }

    ASSERT((p - this->rastersbuffer) <= ARRAYSIZE(this->rastersbuffer));
}


void IntroEntry (FSM* _fsm)
{
    EMUL_STATIC Intro* this;

    u32 sndtracksize = LOADmetadataOriginalSize (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_ZIKS_DELOS_05_ARJX) + INTRO_MODULE_MARGIN_SIZE;
    u32 buffersize   = LOADresourceRoundedSize(&RSC_RELAPSE1, RSC_RELAPSE1_INTRO_ANIM3D_ARJX);


    {
        u32 size = LOADmetadataOriginalSize(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTRO_RELAPSE3_ARJX) + (INTRO_FRAMEBUFFER_SIZE * 3UL);

        if (size > buffersize)
            buffersize = size;
    }

    EMUL_BEGIN_ASYNC_REGION
    
    g_screens.intro = this = MEM_ALLOC_STRUCT( &sys.allocatorMem, Intro );
    DEFAULT_CONSTRUCT(this);

    this->anim3dsize  = LOADmetadataOriginalSize (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTRO_ANIM3D_ARJX);
    this->sndtrack    =       MEM_ALLOC ( &sys.allocatorMem, sndtracksize );   
    this->buffer      =       MEM_ALLOC ( &sys.allocatorMem, buffersize );
    this->anim3d      = (u8*) MEM_ALLOC ( &sys.allocatorMem, this->anim3dsize );
/*    this->asmbuf      =       MEM_ALLOC ( &sys.allocatorMem, asmbufsize );*/

    this->framebuffers[0] = this->buffer + INTRO_SCREEN_PITCH;
    this->framebuffers[1] = this->framebuffers[0] + INTRO_FRAMEBUFFER_SIZE;
    this->title           = this->framebuffers[1] + INTRO_FRAMEBUFFER_SIZE;
    this->logo            = this->title + LOADmetadataOriginalSize (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTRO_TITLE_ARJX);

    ASSERT( ((this->logo - this->buffer) + LOADmetadataOriginalSize (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTRO_RELAPSE3_ARJX)) <= buffersize);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(5);
    g_screens.persistent.loadRequest[0] = LOADdata (&RSC_RELAPSE1, RSC_RELAPSE1_ZIKS_DELOS_05_ARJX, this->anim3d, LOAD_PRIORITY_INORDER);
    g_screens.persistent.loadRequest[1] = LOADdata(&RSC_RELAPSE1, RSC_RELAPSE1_INTRO_ANIM3D_ARJX, this->buffer, LOAD_PRIORITY_INORDER);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[0]);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(4);

    STDfastmset(this->sndtrack, 0, sndtracksize);
    RELAPSE_UNPACK(this->sndtrack, this->anim3d);
    MX2MconvertDPCM2Pulse(this->sndtrack, sndtracksize - INTRO_MODULE_MARGIN_SIZE);
    WIZmodInit(this->sndtrack, this->sndtrack + sndtracksize);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(3);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED(g_screens.persistent.loadRequest[1]);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(2);

    {
        u8* temp3 = (u8*) MEM_ALLOCTEMP(&sys.allocatorMem, LOADresourceRoundedSize(&RSC_RELAPSE1, RSC_RELAPSE1_INTRO_TITLE_ARJX));

        g_screens.persistent.loadRequest[2] = LOADdata(&RSC_RELAPSE1, RSC_RELAPSE1_INTRO_TITLE_ARJX, temp3, LOAD_PRIORITY_INORDER);
        
        RELAPSE_UNPACK(this->anim3d, this->buffer);

        RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(1);

        RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[2]);
/*    this->asmimport = introAsmImport(this->asmbuf);  */

        RELAPSE_UNPACK (this->logo,  temp3 + LOADmetadataOffset (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTRO_RELAPSE3_ARJX) );
        RELAPSE_UNPACK (this->title, temp3 + LOADmetadataOffset (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTRO_TITLE_ARJX) );
        BITpl2chunk(this->title+32, 200, 21, 0, this->title+32);
        STDfastmset(this->framebuffers[0], 0UL, INTRO_FRAMEBUFFER_SIZE * 2UL);

        introSetSpritePos1(this);
        introInitRasters(this);

        RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(0);

#       ifndef __TOS__
        if (TRACisSelected(TRAC_LOG_MEMORY))
        {
            TRAClog(TRAC_LOG_MEMORY,"Intro memallocator dump", '\n');
            TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
            RINGallocatorDump(sys.allocatorMem.allocator, stdout);
        }
#       endif

        MEM_FREE(&sys.allocatorMem, temp3);
    }

    EMUL_DELAY(100);

    FSMgotoNextState (_fsm);

    EMUL_END_ASYNC_REGION
}

void IntroPostInit (FSM* _fsm)
{
    Intro* this = g_screens.intro;
    u16 t;

/*    params.font = this->logo;
    params.framebuffer = this->framebuffers[0];
    params.image = this->anim3d;

    this->asmimport->common.init(&params);*/

    ScreensSetVideoMode (HW_VIDEO_MODE_4P, 12, 1);
    SYSwriteVideoBase((u32)this->framebuffers[0]);

    WIZplay();

    SYSvsync;

    STDmset(this->pal[0], 0UL, 16);
    this->pal[0][8]  = PCENDIANSWAP16(INTRO_SPRITE_COLOR);
    for (t = 1 ; t < 8 ; t++)
        this->pal[0][8+t] = PCENDIANSWAP16(t*0x111+0x888);

    STDmset(this->pal[1], 0UL, 16);
    STDmset(&this->pal[1][8], 0xFFFFFFFFUL, 16);

    STDmcpy(&this->pal[2][1], &this->pal[0][9], 14);
    STDmcpy(&this->pal[2][9], &this->pal[0][9], 14);
    this->pal[2][8] = PCENDIANSWAP16(INTRO_SPRITE_COLOR);

    STDmset(this->pal[3], 0xFFFFFFFFUL, 32);

    STDmcpy2(this->vbl.colors, this->pal[0], 32);

    RASnextOpList = &this->vbl;

/*    this->asmimport->rasters = this->colors;*/
    SYSvblroutines[0] = WIZrundma;
    SYSvblroutines[1] = RASvbl16;
    SYSvblroutines[2] = WIZstereo;

   /* for (t = 0; t <= 16; t++)
    {
        SYSvsync;
        COLcomputeGradient16Steps(this->black, (u16*)(this->anim3d + 2), 15, t, this->colors);
    }*/

    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState(_fsm);
}

enum IntroState
{
    IS_START,
    IS_RELAPSE_COMING,
    IS_FILL_OTHER_FRAME,
    IS_WAIT1,
    IS_FADE_1,
    IS_FADE_2,
    IS_FADE_3,
    IS_FADE_4,
    IS_WAIT2,
    IS_RELAPSE_LEAVING,
    IS_FILL_OTHER_FRAME2,
    IS_WAIT3,
    IS_WAIT4,
    IS_FLASH
};


void IntroActivity (FSM* _fsm)
{       
    Intro* this = g_screens.intro;
    s16 t;

    IGNORE_PARAM(_fsm);
    
    EMULfbExStart (HW_VIDEO_MODE_4P, 80, 40, 80 + 168 * 2 - 1, 40 + 200 - 1, 168 + (*HW_VIDEO_OFFSET * 2), 0);
    EMULfbExEnd();

    this->backbuffer = this->framebuffers[this->flip];
    this->flip ^= 1;

    SYSwriteVideoBase((u32)this->backbuffer);

    INTRO_RASTERIZE_COLOR(7);

    switch(this->state)
    {
    case IS_START:
    {
        WIZinfo info;

        WIZgetInfo(&info);

        if (info.songpos >= 2)
            this->state = IS_RELAPSE_COMING;
        break;
    }

    case IS_RELAPSE_COMING:
        introAnim3D(this);
        introAnim3Dinc(this);
        INTRO_RASTERIZE_COLOR(5);
        introAnim(this);

        for (t = 0; t < INTRO_NBSPRITES; t++)
        {
            this->x[t] += this->dx[t];
            this->y[t] += this->dy[t];
        }
        this->count++;
        if (this->count >= 128)
        {
            this->state = IS_FILL_OTHER_FRAME;
            this->count = 0;
        }
        break;

    case IS_FILL_OTHER_FRAME:
    case IS_FILL_OTHER_FRAME2:
        introAnim3D(this);
        introAnim3Dinc(this);
        INTRO_RASTERIZE_COLOR(5);
        introAnim(this);

        this->count++;
        if (this->count >= 2)
        {
            this->state++;
            this->count = 0;
        }
        break;

    case IS_WAIT1:
    {
        WIZinfo info;


        WIZgetInfo(&info);

        introAnim3D(this);
        introAnim3Dinc(this);
        INTRO_RASTERIZE_COLOR(5);

        if (info.songpos >= 3)
        {
            this->state = IS_FADE_1;
            this->count = 0;
        }           
        break;
    }

    case IS_FADE_1:
    {
        WIZinfo info;


        WIZgetInfo(&info);

        introAnim3D(this);
        introAnim3Dinc(this);
        INTRO_RASTERIZE_COLOR(5);

        if (info.songpos >= 3)
        {
            this->count++;
            if ((this->count >= 16) && (this->count <= 32))
            {
                COLcomputeGradient16Steps(&this->pal[0][1], &this->pal[1][1], 15, this->count - 16, this->vbl.colors + 1);
            }
            else if (this->count >= 40)
            {
                this->state = IS_FADE_2;
                this->count = 0;
            }
        }
        break;
    }

    case IS_FADE_2:
    {
        introAnim3D(this);
        introAnim3Dinc(this);
        INTRO_RASTERIZE_COLOR(5);

        this->count++;
        COLcomputeGradient16Steps(&this->pal[1][1], &this->pal[2][1], 15, this->count, this->vbl.colors+1);

        if (this->count >= 16)
        {
            this->state = IS_WAIT2;
            this->count = 0;
        }           
        break;
    }

    case IS_WAIT2:
    {
        WIZinfo info;

        WIZgetInfo(&info);

        introAnim3D(this);
        introAnim3Dinc(this);
        INTRO_RASTERIZE_COLOR(5);

        if (info.songpos >= 3)
        { 
            this->count++;

            if (this->count >= 134)
            {
                introSetSpritePos2(this);

                this->state = IS_RELAPSE_LEAVING;
                this->count = 0;
            }
        }
        break;
    }

    case IS_RELAPSE_LEAVING:
        introAnim3D(this);
        introAnim3Dinc(this);
        INTRO_RASTERIZE_COLOR(5);
        introAnim(this);

        for (t = 0; t < INTRO_NBSPRITES; t++)
        {
            this->x[t] += this->dx[t];
            this->y[t] += this->dy[t];
        }
        this->count++;
        if (this->count >= 128)
        {
            this->state = IS_FILL_OTHER_FRAME2;
            this->count = 0;
        }
        break;

    case IS_WAIT3:
        introAnim3D(this);
        introAnim3Dinc(this);
        INTRO_RASTERIZE_COLOR(5);

        this->count++;
        if ((this->count >= 250) && (this->anim3doffset == 0))
        {
            WIZjump(0x10);
            this->state = IS_WAIT4;
            this->count = 0;
        }
        break;

    case IS_WAIT4:
        introAnim3D(this);
        INTRO_RASTERIZE_COLOR(5);
        this->state = IS_FADE_3;
        this->count = 0;
        break;

    case IS_FADE_3:

        introAnim3D(this);
        this->count++;

        COLcomputeGradient16Steps(this->pal[2], this->pal[3], 16, this->count, this->vbl.colors);
        
        if (this->count >= 16)
        {
            this->state = IS_FADE_4;
            this->count = 0;
            SYSwriteVideoBase((u32)(this->title+32));
            *HW_VIDEO_OFFSET = 0;
            this->vbl.scanLinesTo1stInterupt = 165;
        }           
        break;

    case IS_FADE_4:
        SYSwriteVideoBase((u32)(this->title+32));

        this->count++;
        if (this->count <= 16)
        {
            COLcomputeGradient16Steps(this->pal[3], (u16*)this->title, 16, this->count, this->vbl.colors);
        }
        else
        {
            if (g_screens.persistent.menumode == false)
            {
                WIZinfo info;

                WIZgetInfo(&info);

                if ((info.songpos == 0x0C) && (info.pattpos == 0x3F))
                    g_screens.next = true;
            }

            if (this->count & 1)
            {
                this->vbl.scanLinesTo1stInterupt--;
                if (this->vbl.scanLinesTo1stInterupt < 35)
                    this->vbl.scanLinesTo1stInterupt = 165;
            }
        }

        break;
    }

    INTRO_RASTERIZE_COLOR(0);

/*    this->asmimport->common.update (NULL);*/

    if (g_screens.next)
    {
        g_screens.next = false;

        FSMgotoNextState (&g_stateMachineIdle);
        FSMgotoNextState (_fsm);
    }
}


void IntroExit (FSM* _fsm)
{
    Intro* this = g_screens.intro;

/*    this->asmimport->common.shutdown(NULL);*/

    SYSvblroutines[1] = SYSvbldonothing;

    ScreenFadeOut();

    WIZstop();

    SYSvblroutines[0] = SYSvblend;
    SYSvblroutines[1] = SYSvblend;
    SYSvblroutines[2] = SYSvblend;
    RASnextOpList = NULL;
    *HW_MFP_TIMER_B_CONTROL = 0;

    SYSvsync;

/*    MEM_FREE ( &sys.allocatorMem, this->asmbuf ); */
    MEM_FREE ( &sys.allocatorMem, this->buffer );
    MEM_FREE ( &sys.allocatorMem, this->anim3d );
    MEM_FREE ( &sys.allocatorMem, this->sndtrack );

    MEM_FREE(&sys.allocatorMem, this);
    g_screens.intro = NULL;

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    ScreenNextState(_fsm);
}
