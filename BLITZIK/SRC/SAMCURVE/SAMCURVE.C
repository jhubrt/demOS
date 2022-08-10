/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2022 J.Hubert

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

#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\TRACE.H"

#include "DEMOSDK\PC\EMUL.H"

#include "FX\SMPLCURV\SMPLCURV.H"

#include "BLITZIK\SRC\SCREENS.H"


#ifdef __TOS__
#define SAMCURV_RASTERIZE() 0
#else
#define SAMCURV_RASTERIZE() 0
#endif

#define SAMCURV_PITCH       168
#define SAMCURV_DISPLAY_H   245
#define SAMCURV_FRAMEBUFFER_H       (SAMCURV_DISPLAY_H + 200)
#define SAMCURV_FRAMEBUFFER_BASE    100

#define SAMCURV_CURVE0_Y    61
#define SAMCURV_CURVE1_Y    (SAMCURV_CURVE0_Y+SAMCURV_CURVES_H+1)
#define SAMCURV_CURVES_H    122

#define SAMCURV_XOR_PASS1_H 17
#define SAMCURV_XOR_PASS3_H 43

#define SAMCURV_YM_FREQMULTIPLIER 397000UL /* (32768 / 333) * (15 * 256 + 192) */



enum samCurveType
{
    samCurve_TWINS,
    samCurve_SOLO,
    samCurve_SIAMESE,
    samCurve_TWINS_SPARKS
};


static void samCurveDrawHYM (SmplCurveYMcurveState* state, void* drawroutineym, void* display, u8 shift)
{
    u16 level = state->level << shift;
    u8* disp  = (u8*) display;


    if (state->type & SmplCurveYMcurveType_NOISE)
    {
        u8 freqnoise = state->freqnoise;
        u16 mask = 4095;
        u16 inc  = 31000;
        u16 inc2;

        freqnoise >>= 4;

        mask >>= freqnoise;
        inc >>= freqnoise;
        inc2 = (inc >> 2) + (STDmfrnd() & mask);
        
        SmplCurveDrawHYMCurve(drawroutineym,
            333,
            (u8*) disp,
            (u8*) disp - SAMCURV_PITCH * level,
            STDmfrnd(), 
            inc, 
            inc2 );
    }
    else if (state->type == SmplCurveYMcurveType_SQUARE)
    {
        s16 inc = (s16) STDdivu(397000UL, state->freqsquare);

        SmplCurveDrawHYMCurve(drawroutineym,
            333,
            (u8*) disp,
            (u8*) disp - SAMCURV_PITCH * level,
            -inc, inc, inc);
    }
}


static void samCurveInitRasters(SamCurve* this)
{
    {
        RASopVbl1* op = &this->rasters1.vbl1;

        op->backgroundColor        = 0;
        op->scanLinesTo1stInterupt = 199;
        op->nextRasterRoutine      = (RASinterupt) RASlowB;
    }
    
    {
        RASopLowB* op = &this->rasters1.lowb;

        op->scanLineToNextInterupt = 200;
        op->nextRasterRoutine = NULL;
    }

    /* ---- */

    {
        RASopVbl1* op = &this->rasters2.vbl1;

        op->backgroundColor        = 0;
        op->scanLinesTo1stInterupt = SAMCURV_CURVES_H;
        op->nextRasterRoutine      = (RASinterupt) RASmid15;
    }

    {
        RASopMid15* op = &this->rasters2.mid15;

        op->scanLineToNextInterupt = 199 - SAMCURV_CURVES_H;
        op->nextRasterRoutine = RASlowB;
        STDmset (op->colors, 0, sizeof(op->colors));
        op->colors[14] = RASstopMask;
    }

    {
        RASopLowB* op = &this->rasters2.lowb;

        op->scanLineToNextInterupt = 200;
        op->nextRasterRoutine = NULL;
    }
}

void SamSetSampleDisplay(SamCurve* this)
{
    SmplCurveInitSampleOffsets (
        g_screens.samcurveStatic.drawcurveroutine, 
        g_screens.samcurveStatic.codesampleoffset, 
        ARRAYSIZE(g_screens.samcurveStatic.codesampleoffset),
        this->samrequest.inc1, 
        this->samrequest.inc2);

    this->samdisplay = this->samrequest;
}


u8 samcurve_g_interlaceincs[] = {4,8,  6,6,  3,9,  1,11};
u8 samcurve_g_xorfreeareah [] = {0, 5, 10, 20, 255};


static void SamCurveManageCommands(SamCurve* this, bool allownavigation_)
{
    while (BLZ_COMMAND_AVAILABLE)
    {
        u8 cmd = BLZ_CURRENT_COMMAND;
        u8 category = cmd & BLZ_CMD_CATEGORY_MASK;

        BLZ_ITERATE_COMMAND;

        switch (category >> 4)
        {
        case BLZ_CMD_VOICE1_CATEGORY >> 4:

            cmd &= BLZ_CMD_COMMAND_MASK;

            if (cmd < 7)
            {
                BLZ_TRAC_COMMAND_NUM("CRVvoice1_", cmd);
                this->voice = cmd;
            }
            else if (cmd >= 8)
            {
                cmd -= 8;

                BLZ_TRAC_COMMAND_NUM("CRVinterlace", cmd);

                cmd <<= 1;
                this->samrequest.inc1 = samcurve_g_interlaceincs[cmd];
                this->samrequest.inc2 = samcurve_g_interlaceincs[cmd + 1];
            }
            break;

        case BLZ_CMD_VOICE2_CATEGORY >> 4:

            if (cmd < BLZ_CMD_VOICE2_8)
            {
                BLZ_TRAC_COMMAND_NUM("CRVvoice2_", cmd - BLZ_CMD_VOICE2_1);
                this->voice2 = cmd - BLZ_CMD_VOICE2_1;
            }
            break;

        case BLZ_CMD_LINE1_CATEGORY >> 4:

            cmd &= BLZ_CMD_COMMAND_MASK;

            if (cmd < 4)
            {
                BLZ_TRAC_COMMAND_NUM("CRVfx_", cmd);
                this->curvetype  = cmd;
            }
            else if (cmd < 9)
            {
                BLZ_TRAC_COMMAND_NUM("curveSoloFill", cmd - 4);
                this->xorfreeareah = samcurve_g_xorfreeareah [cmd - 4];
            }
            else if (cmd == 9)
            {
                BLZ_TRAC_COMMAND("CRVcurveSoloBumpUp");
                this->ypos = 100;  
            }
            else if (cmd == 10)
            {
                BLZ_TRAC_COMMAND("CRVcurveSoloBumpDown");
                this->ypos = -100;
            }
            break;

        case BLZ_CMD_LINE2_CATEGORY >> 4:
            
            cmd &= BLZ_CMD_COMMAND_MASK;

            if (cmd < 4)
            {
                BLZ_TRAC_COMMAND_NUM("CRVCRVcurveColor_", cmd);
                this->colorchoice = cmd;
            }
            break;

        default: 
            if (allownavigation_)
                if (ScreensManageScreenChoice(BLZ_EP_SAM_CURVE, cmd))
                    return;
        }
    }
}


static void samCurveInitColors(SamCurve* this)
{
    /* static u16 curve1[3] = {0x089, 0x013, 0x026};
       static u16 curve2[3] = {0x908, 0x301, 0x602};   */

    static u16 curve1_1regular[3] = {0x081, 0x023, 0x046};   
    static u16 curve2_1regular[3] = {0x189, 0x384, 0x612}; 

    static u16 curve1_2regular[3] = {0x080, 0x020, 0x040};   
    static u16 curve2_2regular[3] = {0x918, 0x328, 0x741};

    static u16 curve1_1sparks[3] = {0x023, 0x46,  0xFFF};
    static u16 curve2_1sparks[3] = {0x384, 0x612, 0xFFF};

    static u16 curve1_2sparks[3] = {0x020, 0x040, 0xFFF};   
    static u16 curve2_2sparks[3] = {0x328, 0x741, 0xFFF};


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "samCurveInitColors", '\n');

    COLPinitColors3Praimbow(curve1_1regular, 0, this->colors [0][0]);
    COLPinitColors3Praimbow(curve1_2regular, 0, this->colors [0][1]);
    COLPinitColors3Praimbow(curve2_1regular, 0, this->colors [0][2]);
    COLPinitColors3Praimbow(curve2_2regular, 0, this->colors [0][3]);

    COLPinitColors3Praimbow(curve1_1sparks,  0, this->colors [1][0]);
    COLPinitColors3Praimbow(curve1_2sparks,  0, this->colors [1][1]);
    COLPinitColors3Praimbow(curve2_1sparks,  0, this->colors [1][2]);
    COLPinitColors3Praimbow(curve2_2sparks,  0, this->colors [1][3]); 
    
    /*for (t = 0 ; t < 16*4 ; t++)
    this->coloranimation.colors[t] ^= 0xFFFF;*/
}


void SamCurveEnter (FSM* _fsm)
{
    SamCurve* this;
    u32 framebuffersize;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "SamCurveEnter", '\n');

    this = g_screens.samcurve = MEM_ALLOC_STRUCT(&sys.allocatorMem, SamCurve);;
    DEFAULT_CONSTRUCT(this);

    framebuffersize = (u32)SAMCURV_PITCH * (u32)SAMCURV_FRAMEBUFFER_H;

    this->framebuffer = (u8*) MEM_ALLOC(&sys.allocatorMem, framebuffersize);
    STDfastmset(this->framebuffer, 0, framebuffersize);

    samCurveInitColors(this);
       
#   ifdef __TOS__
    samCurveInitRasters(this);
#   endif

    SmplCurveInitOffset(SAMCURV_CURVE0_Y, this->offsets , SAMCURV_PITCH);
    SmplCurveInitOffset(SAMCURV_CURVES_H, this->offsets2, SAMCURV_PITCH);

    this->voice2 = 1;
    this->samrequest.inc1 = this->samrequest.inc2 = 6;

    this->lastimage[0] = this->lastimage[1] = this->framebuffer;

/*  TODO : unroll orders
    
    switch (g_screens.runscreen)
    {
    case BLZ_EP_SAM_CURVE:
        break;
    
    case BLZ_EP_SAM_CURVE2:
        this->samrequest.inc1 = 4;
        this->samrequest.inc2 = 8;
        break;

    default:
        ASSERT(0);
    }*/

    SamSetSampleDisplay(this);    
    
    this->oldcurvetype = this->curvetype;
    this->coloranimation.colormode = COLP_RAIMBOW_MODE;

    BlitZsetVideoMode(HW_VIDEO_MODE_4P, 0, BLITZ_VIDEO_16XTRA_PIXELS);

    SamCurveManageCommands(this, false);  /* hack => do this here in sync to avoid complexifying to put it on main thread like it should be */

    if ((this->curvetype == samCurve_TWINS) || (this->curvetype == samCurve_TWINS_SPARKS))
        RASnextOpList = &this->rasters2;
    else
        RASnextOpList = &this->rasters1;

    SYSvblroutines[1] = (SYSinterupt)RASvbl1;

    if (this->curvetype == samCurve_TWINS_SPARKS)
        this->coloranimation.colors = this->colors[1][this->colorchoice & 1];
    else
        this->coloranimation.colors = this->colors[0][this->colorchoice & 1];

    TRACsetVideoMode (168);

    SYSwriteVideoBase ((u32) this->framebuffer + (SAMCURV_FRAMEBUFFER_BASE * SAMCURV_PITCH));

#   ifndef __TOS__
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRAClog(TRAC_LOG_MEMORY,"SamCurve memallocator dump", '\n');
        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif

    FSMgotoNextState(_fsm);
    FSMgotoNextState(&g_stateMachine);
}


static void samCurveActivityType1(SamCurve* this)
{
    bool      sparkmode         = this->curvetype == samCurve_TWINS_SPARKS;
    u8*       image             = (u8*) this->framebuffer + (SAMCURV_FRAMEBUFFER_BASE * SAMCURV_PITCH);
    u16       colorindex        = this->coloranimation.currentplane << 4;
    u16*      colors2           = &this->colors[sparkmode][(this->colorchoice & 1) + 2][colorindex];
    u8*       imagetoxor;
    u8*       imagetoxor2;


#   ifndef __TOS__
    EMULfbExStart (HW_VIDEO_MODE_4P, 80, 40, 80 + SAMCURV_PITCH * 2 - 1, 40 + SAMCURV_DISPLAY_H - 1, SAMCURV_PITCH, 0);

    {
        u16 t;
        u32 cycles;

        cycles = EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, 40 + SAMCURV_CURVES_H);

        for (t = 0 ; t < 16 ; t++)
        {
            EMULfbExSetColor (cycles, t, colors2[t]);
        }

}
    EMULfbExEnd();
#   endif
    
    this->ypos = 0;

    SYSwriteVideoBase((u32)this->framebuffer + (SAMCURV_FRAMEBUFFER_BASE * SAMCURV_PITCH));

    this->coloranimation.colors = this->colors[sparkmode][this->colorchoice & 1];
    image += COLPanimate3P(&this->coloranimation);
    STDmcpy2(this->rasters2.mid15.colors, colors2+1, 30);
    this->rasters2.mid15.colors[14] |= RASstopMask;

    if (sparkmode)
    {
        imagetoxor  = this->lastimage[0];
        imagetoxor2 = this->lastimage[1];

#       ifndef __TOS__
        imagetoxor  = imagetoxor2;
#       endif
    }
    else
    {
        imagetoxor = imagetoxor2 = image;
    }

    /* --------------------------------------------------------------------------- */
    /* Clear plane                                                                 */
    /* --------------------------------------------------------------------------- */
    *HW_BLITTER_ADDR_DEST = (u32) image;
    *HW_BLITTER_ENDMASK1  = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1;
    *HW_BLITTER_XSIZE     = SAMCURV_PITCH / 8;
    *HW_BLITTER_YSIZE     = ((SAMCURV_CURVES_H * 2) + 1);
    *HW_BLITTER_HOP       = HW_BLITTER_HOP_BIT1;
    *HW_BLITTER_OP        = HW_BLITTER_OP_BIT0;
    *HW_BLITTER_XINC_DEST = 8;
    *HW_BLITTER_YINC_DEST = 8;
    *HW_BLITTER_CTRL2     = 0;

#   if SAMCURV_RASTERIZE()
    *HW_COLOR_LUT = 0x4;
#   endif

    *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */

    EMULblit();

    /*----------------------------------
      Draw sample curves
    ----------------------------------*/

#   if SAMCURV_RASTERIZE()
    *HW_COLOR_LUT = 0x300;
#   endif

    if (this->voice < 4)
    {
        SmplCurveDrawHCurve(
            g_screens.samcurveStatic.drawcurveroutine,
            (void*)(g_screens.player.dmabufstart + this->voice),
            333,
            this->samdisplay.inc1,
            image + SAMCURV_CURVE0_Y * SAMCURV_PITCH,
            this->offsets);
    }
    else
    {
        samCurveDrawHYM (&this->ymstates[this->voice - 4], g_screens.samcurveStatic.drawcurveroutineym, image + (SAMCURV_CURVE0_Y - 1) * SAMCURV_PITCH, 2);
    }

#   if SAMCURV_RASTERIZE()
    *HW_COLOR_LUT = 0x400;
#   endif

    if (this->voice2 < 4)
    {
        SmplCurveDrawHCurve(
            g_screens.samcurveStatic.drawcurveroutine,
            (void*)(g_screens.player.dmabufstart + this->voice2),
            333,
            this->samdisplay.inc1,
            image + SAMCURV_CURVE1_Y * SAMCURV_PITCH,
            this->offsets);
    }
    else
    {
        samCurveDrawHYM (&this->ymstates[this->voice2 - 4], g_screens.samcurveStatic.drawcurveroutineym, image + (SAMCURV_CURVE1_Y - 1) * SAMCURV_PITCH, 2);
    }

#   if SAMCURV_RASTERIZE()
    *HW_COLOR_LUT = 0x40;
#   endif

    /* --------------------------------------------------------------------------- */
    /* XOR PASS                                                                    */ 
    /* --------------------------------------------------------------------------- */

#   if SAMCURV_RASTERIZE()
    *HW_COLOR_LUT = 0x30;
#   endif

    /* --------------------------------------------------------------------------- */
    /* Common setup for xor pass */

    /* *HW_BLITTER_ENDMASK1    = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1; */ /* Already set by clear pass */
                                                                                      /* *HW_BLITTER_XSIZE       = SAMCURV_PITCH / 8; */
    *HW_BLITTER_HOP         = HW_BLITTER_HOP_SOURCE;
    *HW_BLITTER_OP          = HW_BLITTER_OP_S_XOR_D;
    *HW_BLITTER_XINC_SOURCE = 8;
    /* *HW_BLITTER_XINC_DEST   = 8; */                                                /* Already set by clear pass */
                                                                                      /* *HW_BLITTER_CTRL2       = 0; */

    /* --------------------------------------------------------------------------- */
    /* Curve 2 pass                                                                */

    imagetoxor2 += (SAMCURV_CURVE1_Y - (SAMCURV_CURVES_H / 2)) * SAMCURV_PITCH;

    *HW_BLITTER_ADDR_SOURCE = ((u32) imagetoxor2);
    *HW_BLITTER_ADDR_DEST   = ((u32) imagetoxor2) + SAMCURV_PITCH;
    *HW_BLITTER_YSIZE       = SAMCURV_XOR_PASS1_H;
    *HW_BLITTER_YINC_SOURCE = 8;
    *HW_BLITTER_YINC_DEST   = 8;
    *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */

    EMULblit();

#   ifdef __TOS__

#   if SAMCURV_RASTERIZE()
    *HW_COLOR_LUT = 0;
#   endif

    while (*HW_VECTOR_TIMERB != (u32) RASlowB);

#   if SAMCURV_RASTERIZE()
    *HW_COLOR_LUT = 0x20;
#   endif

#   endif

    *HW_BLITTER_YSIZE       = (SAMCURV_CURVES_H / 2) - 1 - SAMCURV_XOR_PASS1_H;
    *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */

    EMULblit();

#   if SAMCURV_RASTERIZE()
    *HW_COLOR_LUT = 0x40;
#   endif

    *HW_BLITTER_ADDR_SOURCE = ((u32) imagetoxor2) + (SAMCURV_CURVES_H - 1) * SAMCURV_PITCH;
    *HW_BLITTER_ADDR_DEST   = ((u32) imagetoxor2) + (SAMCURV_CURVES_H - 2) * SAMCURV_PITCH;
    *HW_BLITTER_YSIZE       = (SAMCURV_CURVES_H / 2) - 1;
    *HW_BLITTER_YINC_SOURCE = -SAMCURV_PITCH*2 + 8;
    *HW_BLITTER_YINC_DEST   = -SAMCURV_PITCH*2 + 8;
    *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */

    EMULblit();

#   if SAMCURV_RASTERIZE()
    *HW_COLOR_LUT = 0x30;
#   endif

    /* --------------------------------------------------------------------------- */
    /* Curve 1 pass                                                                */

    *HW_BLITTER_ADDR_SOURCE =  (u32) imagetoxor;
    *HW_BLITTER_ADDR_DEST   = ((u32) imagetoxor) + SAMCURV_PITCH;
    *HW_BLITTER_YSIZE       = SAMCURV_XOR_PASS3_H;
    *HW_BLITTER_YINC_SOURCE = 8;
    *HW_BLITTER_YINC_DEST   = 8;
    *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */

    EMULblit();

#   ifdef __TOS__

#   if SAMCURV_RASTERIZE()
    *HW_COLOR_LUT = 0x0;
#   endif

    while (*HW_VECTOR_TIMERB != 0);
#   endif

#   if SAMCURV_RASTERIZE()
    *HW_COLOR_LUT = 0x30;
#   endif

    *HW_BLITTER_YSIZE       = (SAMCURV_CURVES_H / 2) - 1 - SAMCURV_XOR_PASS3_H;
    *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */

    EMULblit();

#   if SAMCURV_RASTERIZE()
    *HW_COLOR_LUT = 0x40;
#   endif

    *HW_BLITTER_ADDR_SOURCE = ((u32) imagetoxor) + (SAMCURV_CURVES_H - 1) * SAMCURV_PITCH;
    *HW_BLITTER_ADDR_DEST   = ((u32) imagetoxor) + (SAMCURV_CURVES_H - 2) * SAMCURV_PITCH;
    *HW_BLITTER_YSIZE       = (SAMCURV_CURVES_H / 2) - 1;
    *HW_BLITTER_YINC_SOURCE = -SAMCURV_PITCH*2 + 8;
    *HW_BLITTER_YINC_DEST   = -SAMCURV_PITCH*2 + 8;
    *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */

    EMULblit();

    this->lastimage[1] = this->lastimage[0];
    this->lastimage[0] = image;
}


static void samCurveActivityType2(SamCurve* this)
{
    u8* image = (u8*)this->framebuffer  + (SAMCURV_FRAMEBUFFER_BASE * SAMCURV_PITCH);
    
    static u8 movecurve3[] = { 1, 2, 3, 5, 8 };

    s16 speed = -(s16)this->ypos;

    if (speed < 0)
    {
        speed = -speed;
        speed >>= 3;
        if (speed >= ARRAYSIZE(movecurve3))
            speed = ARRAYSIZE(movecurve3) - 1;
        speed = movecurve3[speed];
        this->ypos -= speed;
    }
    else if (speed > 0)
    {
        speed >>= 3;
        if (speed >= ARRAYSIZE(movecurve3))
            speed = ARRAYSIZE(movecurve3) - 1;
        speed = movecurve3[speed];
        this->ypos += speed;
    }

    SYSwriteVideoBase((u32)(this->framebuffer + (SAMCURV_FRAMEBUFFER_BASE + this->ypos) * SAMCURV_PITCH));

#   ifndef __TOS__
    EMULfbExStart(HW_VIDEO_MODE_4P, 80, 40, 80 + SAMCURV_PITCH * 2 - 1, 40 + SAMCURV_DISPLAY_H - 1, SAMCURV_PITCH, 0);
    EMULfbExEnd();
#   endif

    this->coloranimation.colors = this->colors[0][this->colorchoice];
    image += COLPanimate3P(&this->coloranimation);

    /* ----------------------------------
        Clear plane
    ----------------------------------*/
    *HW_BLITTER_ADDR_DEST = (u32)image;
    *HW_BLITTER_ENDMASK1 = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1;
    *HW_BLITTER_XSIZE = SAMCURV_PITCH / 8;
    *HW_BLITTER_YSIZE = ((SAMCURV_CURVES_H * 2) + 1);
    *HW_BLITTER_HOP = HW_BLITTER_HOP_BIT1;
    *HW_BLITTER_OP = HW_BLITTER_OP_BIT0;
    *HW_BLITTER_XINC_DEST = 8;
    *HW_BLITTER_YINC_DEST = 8;
    *HW_BLITTER_CTRL2 = 0;

#   if SAMCURV_RASTERIZE() && defined(__TOS__)
    if (this->curvetype == samCurve_SIAMESE)
    {
        register u8 counter = *HW_VIDEO_COUNT_L;
        while (*HW_VIDEO_COUNT_L == counter);
    }
#   endif

#   if SAMCURV_RASTERIZE()
    * HW_COLOR_LUT = 0x4;
#   endif

    * HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */

    EMULblit();

    /* ----------------------------------
    Draw sample curves
    ----------------------------------*/

#   if SAMCURV_RASTERIZE()
    * HW_COLOR_LUT = 0x300;
#   endif

    if (this->curvetype == samCurve_SOLO)
    {
        if (this->voice < 4)
        {
            SmplCurveDrawHCurve(g_screens.samcurveStatic.drawcurveroutine,
                (void*)(g_screens.player.dmabufstart + this->voice),
                333,
                this->samdisplay.inc1,
                image + SAMCURV_CURVES_H * SAMCURV_PITCH,
                this->offsets2);
        }
        else
        {
            samCurveDrawHYM (&this->ymstates[this->voice - 4], g_screens.samcurveStatic.drawcurveroutineym, image + (SAMCURV_CURVES_H - 1) * SAMCURV_PITCH, 3);
        }
    }
    else
    {
        if (this->voice < 4)
        {
            SmplCurveDrawHCurve(g_screens.samcurveStatic.drawcurveroutine,
                (void*)(g_screens.player.dmabufstart + this->voice),
                333, this->samdisplay.inc1,
                image + SAMCURV_CURVE0_Y * SAMCURV_PITCH,
                this->offsets);
        }
        else
        {
            samCurveDrawHYM (&this->ymstates[this->voice - 4], g_screens.samcurveStatic.drawcurveroutineym,  image + (SAMCURV_CURVE0_Y - 1) * SAMCURV_PITCH, 2);
        }

#       if SAMCURV_RASTERIZE()
        * HW_COLOR_LUT = 0x400;
#       endif

        if (this->voice2 < 4)
        {
            SmplCurveDrawHCurve(g_screens.samcurveStatic.drawcurveroutine,
                (void*)(g_screens.player.dmabufstart + this->voice2),
                333,
                this->samdisplay.inc1,
                image + SAMCURV_CURVE1_Y * SAMCURV_PITCH,
                this->offsets);
        }
        else
        {
            samCurveDrawHYM (&this->ymstates[this->voice2 - 4], g_screens.samcurveStatic.drawcurveroutineym, image + (SAMCURV_CURVE1_Y - 1) * SAMCURV_PITCH, 2);
        }
    }

    /* ----------------------------------
    Xor pass
    ----------------------------------*/

    if (this->xorfreeareah != 255)
    {
#       if SAMCURV_RASTERIZE()
        * HW_COLOR_LUT = 0x30;
#       endif

        /* Common setup for xor pass */

        /* *HW_BLITTER_ENDMASK1    = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1; */ /* Already set by clear pass */
                                                                                          /* *HW_BLITTER_XSIZE       = SAMCURV_PITCH / 8; */                                /* Already set by clear pass */
        *HW_BLITTER_HOP = HW_BLITTER_HOP_SOURCE;
        *HW_BLITTER_OP = HW_BLITTER_OP_S_XOR_D;
        *HW_BLITTER_XINC_SOURCE = 8;
        /* *HW_BLITTER_XINC_DEST   = 8; */                                                /* Already set by clear pass */
                                                                                          /* *HW_BLITTER_CTRL2       = 0; */                                                /* Already set by clear pass */

                                                                                                                                                                            /* draw curve 1 */
        if (this->curvetype == samCurve_SOLO)
        {
            *HW_BLITTER_ADDR_SOURCE = (u32)image;
            *HW_BLITTER_ADDR_DEST = ((u32)image) + SAMCURV_PITCH;
            *HW_BLITTER_YSIZE = SAMCURV_CURVES_H - 1 - this->xorfreeareah;
            *HW_BLITTER_YINC_SOURCE = 8;
            *HW_BLITTER_YINC_DEST = 8;
            *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */

            EMULblit();

#           if SAMCURV_RASTERIZE()
            * HW_COLOR_LUT = 0x20;
#           endif

            * HW_BLITTER_ADDR_SOURCE = ((u32)image) + (u32)(SAMCURV_CURVES_H * 2 - 1) * (u32)SAMCURV_PITCH;
            *HW_BLITTER_ADDR_DEST = ((u32)image) + (u32)(SAMCURV_CURVES_H * 2 - 2) * (u32)SAMCURV_PITCH;
            *HW_BLITTER_YSIZE = SAMCURV_CURVES_H - 1 - this->xorfreeareah;
            *HW_BLITTER_YINC_SOURCE = -SAMCURV_PITCH * 2 + 8;
            *HW_BLITTER_YINC_DEST = -SAMCURV_PITCH * 2 + 8;
            *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */

            EMULblit();
        }
        else
        {
            *HW_BLITTER_ADDR_SOURCE = (u32)image;
            *HW_BLITTER_ADDR_DEST = ((u32)image) + SAMCURV_PITCH;
            *HW_BLITTER_YSIZE = 168;
            *HW_BLITTER_YINC_SOURCE = 8;
            *HW_BLITTER_YINC_DEST = 8;
            *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */

            EMULblit();

#           if SAMCURV_RASTERIZE()
            * HW_COLOR_LUT = 0x0;
#           endif

#           ifdef __TOS__
            STDstop2300();
            /*while (*HW_VECTOR_TIMERB != 0);*/
#           endif

#           if SAMCURV_RASTERIZE()
            * HW_COLOR_LUT = 0x30;
#           endif

            *HW_BLITTER_YSIZE = SAMCURV_DISPLAY_H - 1 - 168;
            *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */

            EMULblit();
        }
    }
}


 typedef void(*samCurveActivitiesCall)(SamCurve* this);
static samCurveActivitiesCall samCurve_g_activities[] = {samCurveActivityType1, samCurveActivityType2, samCurveActivityType2, samCurveActivityType1};


void SamCurveActivity(FSM* _fsm)
{
    SamCurve* this = g_screens.samcurve;

    IGNORE_PARAM(_fsm);

    if (this->curvetype != this->oldcurvetype)
    {
        STDfastmset(this->framebuffer, 0UL, (u32)SAMCURV_PITCH * (u32)SAMCURV_FRAMEBUFFER_H);

        if (samCurve_g_activities[this->curvetype] == samCurveActivityType1)
            RASnextOpList = &this->rasters2;
        else
            RASnextOpList = &this->rasters1;

        this->oldcurvetype = this->curvetype;
    }
    else
    {
        SmplCurveYMgetStates (this->ymstates);

        if (this->ymstates[0].type == SmplCurveYMcurveType_SQUARE)
           this->ymincs[0] = (s16) STDdivu(SAMCURV_YM_FREQMULTIPLIER, this->ymstates[0].freqsquare); 

        if (this->ymstates[1].type == SmplCurveYMcurveType_SQUARE)
            this->ymincs[1] = (s16) STDdivu(SAMCURV_YM_FREQMULTIPLIER, this->ymstates[1].freqsquare);

        if (this->ymstates[2].type == SmplCurveYMcurveType_SQUARE)
            this->ymincs[2] = (s16) STDdivu(SAMCURV_YM_FREQMULTIPLIER, this->ymstates[2].freqsquare);

        samCurve_g_activities[this->curvetype](this);
    }

#   if SAMCURV_RASTERIZE()
    *HW_COLOR_LUT = 0x0;
#   endif

    SamCurveManageCommands(this, true);
}


void SamCurveBacktask(FSM* _fsm)
{
    SamCurve* this = g_screens.samcurve;

    IGNORE_PARAM(_fsm);

    if ((this->samdisplay.inc1  != this->samrequest.inc1) ||
        (this->samdisplay.inc2  != this->samrequest.inc2))
    {
        SamSetSampleDisplay(this);
    }

    /*STDstop2300();*/
}


void SamCurveExit(FSM* _fsm)
{
    SamCurve* this = g_screens.samcurve;

    IGNORE_PARAM(_fsm);

    SYSvblroutines[1] = RASvbldonothing;
    BlitZturnOffDisplay();

    MEM_FREE(&sys.allocatorMem, this->framebuffer);
    MEM_FREE(&sys.allocatorMem, this);

    this = g_screens.samcurve = NULL;

#   if DEMOS_MEMDEBUG
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    ScreensGotoScreen();
}


void SamCurveInitStatic (FSM* _fsm)
{    
    ScreensLogFreeArea("SamCurveInitStatic");

    g_screens.samcurveStatic.drawcurveroutine   = (u8*) MEM_ALLOC(&sys.allocatorCoreMem, SMPLCURV_CODESIZE);
    g_screens.samcurveStatic.drawcurveroutineym = (u8*) MEM_ALLOC(&sys.allocatorCoreMem, SMPLCURV_CODESIZE_YM);

#   ifdef __TOS__
    SmplCurveGenerateDrawHCurves(g_screens.samcurveStatic.drawcurveroutine, g_screens.samcurveStatic.drawcurveroutineym, g_screens.samcurveStatic.codesampleoffset, ARRAYSIZE(g_screens.samcurveStatic.codesampleoffset));
#   endif

    FSMgotoNextState(_fsm);
}

