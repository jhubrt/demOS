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
#define ALLCURV_RASTERIZE() 0
#else
#define ALLCURV_RASTERIZE() 0
#endif

#define ALLCURV_PITCH       168
#define ALLCURV_DISPLAY_H   245

#define ALLCURV_CODESIZE1   5930
#define ALLCURV_CODESIZE2   15300
#define ALLCURV_RASTERSIZE  4000
#define ALLCURV_YM_FREQMULTIPLIER 540000UL /* (32768 / DISPLAY_H) * (15 * 256 + 192) */


ASMIMPORT AllCurveASMimport BACrImportTable;


static u8* allCurveGenerateDrawVCurves(u16* codesampleoffset, u16* nbcodesampleoffsets, u8* output)
{
    CGENdesc* code = BACrImportTable.opcodes;
    u8*       base = output;
    s16       t,i;
    u16       offsetsdisplay[3];
    u16       offsetssample [1];
    s16       offset = -128 * ALLCURV_PITCH; /* 250 * 168 > 32k */
    u16       result;
    u8*       p;
    u16       nbsamoffs = 0;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "allCurveGenerateDrawVCurves", '\n');

    CGENgenerateSimple(&code[ASCrOp_DrawVBegin], output);

    result = CGENfindWords(&code[ASCrOp_DrawV], offsetssample, 0xABA);
    ASSERT(result == ARRAYSIZE(offsetssample));

    result = CGENfindWords(&code[ASCrOp_DrawV], offsetsdisplay, 0xABC);
    ASSERT(result == ARRAYSIZE(offsetsdisplay));

    for (t = 0 ; t < ALLCURV_DISPLAY2_H ; t++)
    {
        p = (u8*) output;

        CGENgenerateSimple(&code[ASCrOp_DrawV], output);

        for (i = 0 ; i < ARRAYSIZE(offsetsdisplay) ; i++)
        {
            *(u16*)(p + offsetsdisplay[i]) = offset + (i << 3);
        }

        offset += ALLCURV_PITCH;

        codesampleoffset[nbsamoffs++] = (u16)(p - base) + offsetssample[0];
    }

    CGENgenerateSimple(&code[ASCrOp_DrawVEnd], output);

    ASSERT(nbsamoffs == t);

    *nbcodesampleoffsets = nbsamoffs;

    IGNORE_PARAM(result);

    return output;
}



static u8* allCurveGenerateDrawVymCurves(u8* output)
{
    CGENdesc* code = BACrImportTable.opcodes;
    s16       t;
    u16       offsetsdisplay[12];
    s16       offset = -128 * ALLCURV_PITCH; /* 250 * 168 > 32k */
    u16       result;
    u8*       p;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "allCurveGenerateDrawVymCurves", '\n');

    CGENgenerateSimple(&code[ASCrOp_DrawVymBegin], output);

    result = CGENfindWords(&code[ASCrOp_DrawVym], offsetsdisplay, 0xABC);
    ASSERT(result == ARRAYSIZE(offsetsdisplay));

    for (t = 0 ; t < ALLCURV_DISPLAY2_H / 2 ; t++)
    {
        s16 o;

        p = (u8*) output;

        CGENgenerateSimple(&code[ASCrOp_DrawVym], output);

        o = offset;

        *(u16*)(p + offsetsdisplay[0]) = o;
        *(u16*)(p + offsetsdisplay[6]) = o;
        o += 8;
        
        *(u16*)(p + offsetsdisplay[1]) = o;
        *(u16*)(p + offsetsdisplay[7]) = o;
        o += 8;

        *(u16*)(p + offsetsdisplay[2]) = o;
        *(u16*)(p + offsetsdisplay[8]) = o;

        offset += ALLCURV_PITCH;
        o = offset;

        *(u16*)(p + offsetsdisplay[3]) = o;
        *(u16*)(p + offsetsdisplay[9]) = o;
        o += 8;

        *(u16*)(p + offsetsdisplay[4])  = o;
        *(u16*)(p + offsetsdisplay[10]) = o;
        o += 8;

        *(u16*)(p + offsetsdisplay[5])  = o;
        *(u16*)(p + offsetsdisplay[11]) = o;

        offset += ALLCURV_PITCH;
    }

    CGENgenerateSimple(&code[ASCrOp_DrawVymEnd], output);

    return output;
}


static u8* allCurveGenerateDrawVymNoiseCurves(u8* output)
{
    CGENdesc* code = BACrImportTable.opcodes;
    s16       t;
    u16       offsetsdisplay[12];
    s16       offset = -128 * ALLCURV_PITCH; /* 250 * 168 > 32k */
    u16       result;
    u8*       p;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "allCurveGenerateDrawVymNoiseCurves", '\n');

    CGENgenerateSimple(&code[ASCrOp_DrawVymBegin], output);

    result = CGENfindWords(&code[ASCrOp_DrawVymNoise], offsetsdisplay, 0xABC);
    ASSERT(result == ARRAYSIZE(offsetsdisplay));

    for (t = 0 ; t < ALLCURV_DISPLAY2_H / 2 ; t++)
    {
        s16 o;

        p = (u8*) output;

        CGENgenerateSimple(&code[ASCrOp_DrawVymNoise], output);

        o = offset;

        *(u16*)(p + offsetsdisplay[0]) = o;
        *(u16*)(p + offsetsdisplay[6]) = o;
        o += 8;

        *(u16*)(p + offsetsdisplay[1]) = o;
        *(u16*)(p + offsetsdisplay[7]) = o;
        o += 8;

        *(u16*)(p + offsetsdisplay[2]) = o;
        *(u16*)(p + offsetsdisplay[8]) = o;

        offset += ALLCURV_PITCH;
        o = offset;

        *(u16*)(p + offsetsdisplay[3]) = o;
        *(u16*)(p + offsetsdisplay[9]) = o;
        o += 8;

        *(u16*)(p + offsetsdisplay[4])  = o;
        *(u16*)(p + offsetsdisplay[10]) = o;
        o += 8;

        *(u16*)(p + offsetsdisplay[5])  = o;
        *(u16*)(p + offsetsdisplay[11]) = o;

        offset += ALLCURV_PITCH;
    }

    CGENgenerateSimple(&code[ASCrOp_DrawVymEnd], output);

    return output;
}


#ifdef __TOS__

typedef void (*allCurveDrawVCall)(void* sample, void* display, u32 pixeltable);

#if ALLCURV_RASTERIZE()
#   define allCurveDrawVCurve(this, _sample, _nbsamples, _incx, _screen) *HW_COLOR_LUT ^= 0x100; ((allCurveDrawVCall)this->drawcurveroutine[0])(_sample, ((u8*)(_screen)) + (128 * ALLCURV_PITCH), (u32)this->offsets)
#else
#   define allCurveDrawVCurve(this, _sample, _nbsamples, _incx, _screen)                         ((allCurveDrawVCall)this->drawcurveroutine[0])(_sample, ((u8*)(_screen)) + (128 * ALLCURV_PITCH), (u32)this->offsets)
#endif 

typedef void (*allCurveDrawVymCall)     (void* display, u16 inc, u32 curve1, u16 curve2);
typedef void (*allCurveDrawVymNoiseCall)(void* display, void* noisyreg, u16 bittotest, u32 curve1, u16 curve2);

#else

static void allCurveDrawVCurve (AllCurve* this, void* _sample, u16 _nbsamples, u16 _incx, void* _screen)
{
    s8*  s = (s8*) _sample;
    u16* d = (u16*) _screen;
    u16  t;


    for (t = 0 ; t < _nbsamples ; t++)
    {
        s16 index = *s;
        u16* p;

        s += _incx;

        index &= 0xFC;

        p = &this->offsets[index];
        
        d[0] = *p++;
        d[4] = *p++;
        d[8] = *p++;

        d += ALLCURV_PITCH / 2;
    }
}

#endif

static const u16 g_ymLevelBitmaps[16 * 3] =
{
    0x0000, 0,      0,
    0x0000, 0x0180, 0x0000,
    0x0000, 0x07E0, 0x0000,
    0x0000, 0x0FF0, 0x0000,
    0x0000, 0x3FFC, 0x0000,
    0x0000, 0x7FFE, 0x0000,
    0x0001, 0xFFFF, 0x8000,
    0x0003, 0xFFFF, 0xC000,
    0x000F, 0xFFFF, 0xF000,
    0x001F, 0xFFFF, 0xF800,
    0x007F, 0xFFFF, 0xFE00,
    0x00FF, 0xFFFF, 0xFF00,
    0x03FF, 0xFFFF, 0xFFC0,
    0x07FF, 0xFFFF, 0xFFE0,
    0x1FFF, 0xFFFF, 0xFFF8,  
    0x7FFF, 0xFFFF, 0xFFFE
};

#ifndef  __TOS__
static void allCurvePCDrawYM (void* _screen, s16 inc, u16 w0, u16 w1, u16 w2)
{
    u16* d = (u16*) _screen;
    u16  t;
    s16  acc = -inc;

    for (t = 0; t < ALLCURV_DISPLAY2_H; t++)
    {
        acc += inc;

        if (acc >= 0)
        {
            d[0] = w0;
            d[4] = w1;
            d[8] = w2;
        }
        else
        {
            d[0] = 0;
            d[4] = 0;
            d[8] = 0;
        }

        d += ALLCURV_PITCH / 2;
    }
}

static void allCurvePCDrawYMnoise (void* _screen, s16 bittotest, u16 w0, u16 w1, u16 w2)
{
    u16* d = (u16*) _screen;
    u16  t;

    for (t = 0; t < ALLCURV_DISPLAY2_H; t++)
    {
        if (STDmfrnd() & (1 << bittotest))
        {
            d[0] = w0;
            d[4] = w1;
            d[8] = w2;
        }
        else
        {
            d[0] = 0;
            d[4] = 0;
            d[8] = 0;
        }

        d += ALLCURV_PITCH / 2;
    }
}
#endif // ! __TOS__


static void allCurveDrawVymCurve (AllCurve* this, void* _screen, SmplCurveYMcurveState* state_, s16 inc_)
{
#   if ALLCURV_RASTERIZE()
    *HW_COLOR_LUT ^= 0x20;
#   endif

    if (state_->type == SmplCurveYMcurveType_NONE)
    {
#       ifdef __TOS__
        ((allCurveDrawVymCall)this->drawcurveroutine[1])((u8*)_screen + (128 * ALLCURV_PITCH), 0, 0UL, 0);
#       else
        allCurvePCDrawYM (_screen, 0,0,0,0);
#       endif
    }
    else
    {
        u16  w0, w1, w2;
        u8   level = state_->level;


        level = level + level + level;

        w0 = PCENDIANSWAP16(g_ymLevelBitmaps[level    ]);
        w1 = PCENDIANSWAP16(g_ymLevelBitmaps[level + 1]);
        w2 = PCENDIANSWAP16(g_ymLevelBitmaps[level + 2]);

        if (state_->type & SmplCurveYMcurveType_NOISE)  /* noise (if noise and square, display noise) */
        {
            u8 freqnoise = state_->freqnoise;

            freqnoise >>= 3;
            freqnoise += 5;
            if (freqnoise > 7)
                freqnoise = 7;

#           ifdef __TOS__
            ((allCurveDrawVymNoiseCall)this->drawcurveroutine[2])((u8*)_screen + (128 * ALLCURV_PITCH), HW_VIDEO_COUNT_L, freqnoise, (((u32)w0) << 16) | w1, w2);
#           else
            allCurvePCDrawYMnoise(_screen, 1, w0, w1, w2);
#           endif
        }
        else /* square */
        {
#           ifdef __TOS__
            ((allCurveDrawVymCall)this->drawcurveroutine[1])((u8*)_screen + (128 * ALLCURV_PITCH), inc_, (((u32)w0) << 16) | w1, w2);
#           else
            allCurvePCDrawYM(_screen, inc_, w0, w1, w2);
#           endif
        }
    }
}

static void allCurveInitPixels (AllCurve* this)
{
    u16* p = (u16*) this->offsets;
    s16 value;
    u32 w = 0;
    u16 count = 1;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "allCurveInitPixels", '\n');

    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;

    for (value = 4 ; value < 128 ; value += 4)
    {
        w |= 1UL << 23;

        *p++ = 0;
        *p++ = PCENDIANSWAP16((u16)(w >> 16));
        *p++ = PCENDIANSWAP16((u16) w);

        p++;

        if (++count & 3)
            w >>= 1;
    }

    count = 3;
    w = 0xFFFFFF00UL;

    for (value = -128 ; value < 0 ; value += 4)
    {
        *p++ = PCENDIANSWAP16((u16)(w >> 16));
        *p++ = PCENDIANSWAP16((u16) w);
        *p++ = 0;

        p++;

        if (++count & 3)
            w = ((w >> 1) & 0xFFFFFF00UL);
    }

#   ifndef __TOS__
    if (TRACisSelected(TRAC_LOG_SPECIFIC))
    {
        p = (u16*)this->offsets;

        printf("\n");

        for (value = 0; value < 256; value += 4)
        {
            u16 i;

            printf("%3d: ", value);

            for (i = 0; i < 3; i++)
            {
                u16 v = *p++;
                u16 j;

                v = PCENDIANSWAP16(v);

                for (j = 0; j < 16; j++)
                {
                    printf("%c", v & 0x8000 ? '1' : '0');
                    v <<= 1;
                }

                printf(" ");
            }

            printf("\n");

            p++;
        }
    }
#   endif
}


static u16* allCurveInitColors3Pgradient(u16* colors_, u16 offset_, u16 nbsteps_, u16 _start[3], u16 _end[3], u16 backcolor_)
{
    u16 pal[4];
    u16 dest[16*4];
    u16 g;


    ASSERT(nbsteps_ == 16); /* else do use COLcomputeGradientStep instead of COLcomputeGradient16Steps */

    for (g = 0 ; g < nbsteps_ ; g++)
    {
        u16* temp = dest;

        COLcomputeGradient16Steps(_start, _end, 3, g, pal);
        COLPinitColors3P(pal, backcolor_, &temp);
        
        STDmcpy2(colors_                               , dest     , 32);
        STDmcpy2(colors_ + offset_                     , dest + 16, 32);
        STDmcpy2(colors_ + offset_ + offset_           , dest + 32, 32);
        STDmcpy2(colors_ + offset_ + offset_ + offset_ , dest + 48, 32);

        colors_ += 16;
    }

    return colors_;
}

static void allCurveInitRasters(AllCurve* this)
{
    u8*  raster = this->rasters[0];
    u16  t, i;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "allCurveInitRasters", '\n');

    for (t = 0 ; t < 4 ; t++)
    {
        u16* p = this->coloranimation.colors + 1 + t * 32 * 16;

        this->rasters[t] = raster;

        /* 0 */
        {
            RASopVbl15* op = (RASopVbl15*)raster;

            STDmcpy2(op->colors, p, sizeof(op->colors));
            op->scanLinesTo1stInterupt = 8;
            op->nextRasterRoutine = (RASinterupt)RASmid15;
            p += 16;
            raster += sizeof(*op);
        }

        /* 1 -> 23 */
        for (i = 8; i < 192; i += 8)
        {
            RASopMid15* op = (RASopMid15*)raster;

            STDmcpy2(op->colors, p, sizeof(op->colors));
            p += 16;
            raster += sizeof(op->colors);
        }

        /* 24 */
        {
            RASopMid15* op = (RASopMid15*)raster;

            STDmcpy2(op->colors, p, sizeof(op->colors));
            op->scanLineToNextInterupt = 7;
            op->nextRasterRoutine = (RASinterupt)RASlowB15;
            op->colors[14] |= RASstopMask;
            p += 16;
            raster += sizeof(*op);
        }

        /* 25 */
        {
            RASopLowB15* op = (RASopLowB15*)raster;

            STDmcpy2(op->colors, p, sizeof(op->colors));
            op->scanLineToNextInterupt = 8;
            op->nextRasterRoutine = (RASinterupt)RASmid15;
            p += 16;
            raster += sizeof(*op);
        }

        /* 26 -> 29 */
        for (i = 208 ; i < 232 ; i += 8)
        {
            RASopMid15* op = (RASopMid15*)raster;

            STDmcpy2(op->colors, p, sizeof(op->colors));
            p += 16;
            raster += sizeof(op->colors);
        }

        /* 30 */
        {
            RASopMid15* op = (RASopMid15*)raster;

            STDmcpy2(op->colors, p, sizeof(op->colors));
            op->scanLineToNextInterupt = 200;
            op->nextRasterRoutine = NULL;
            op->colors[14] |= RASstopMask;
            p += 16;
            raster += sizeof(*op);
        }

        p += 16;
    }

    TRAClogNumber10(TRAC_LOG_SPECIFIC, "rastersize: ", (raster - this->rasters[0]), 8);
    ASSERT((raster - this->rasters[0]) <= ALLCURV_RASTERSIZE);
}


static void AllCurveManageCommands(AllCurve* this, bool allownavigation_)
{
    while (BLZ_COMMAND_AVAILABLE)
    {
        u8 cmd = BLZ_CURRENT_COMMAND;

        BLZ_ITERATE_COMMAND;

        switch (cmd - BLZ_CMD_Q)
        {
        case BLZ_CMD_Q - BLZ_CMD_Q: BLZ_TRAC_COMMAND("ALCrun");     this->feed = true;  break;
        case BLZ_CMD_W - BLZ_CMD_Q: BLZ_TRAC_COMMAND("ALCfreeze");  this->feed = false; break;
        case BLZ_CMD_E - BLZ_CMD_Q: BLZ_TRAC_COMMAND("ALCstep");    this->step = true;  break;
        
        default:
            if (allownavigation_)
                if (ScreensManageScreenChoice(BLZ_EP_ALL_CURVE, cmd))
                    return;
        }
    }
}


void AllCurveEnter (FSM* _fsm)
{
    AllCurve* this;


    STATIC_ASSERT(ALLCURV_DISPLAY2_H <= ARRAYSIZE(g_screens.allcurveStatic.codesampleoffset));

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "AllCurveEnter", '\n');

    this = g_screens.allcurve = MEM_ALLOC_STRUCT(&sys.allocatorMem, AllCurve);
    DEFAULT_CONSTRUCT(this);

    this->framebuffer = (u8*) MEM_ALLOC(&sys.allocatorMem, (u32)ALLCURV_PITCH * (u32)ALLCURV_DISPLAY2_H);

    STDfastmset(this->framebuffer, 0, (u32)ALLCURV_PITCH * (u32)ALLCURV_DISPLAY2_H);

    this->coloranimation.colors = g_screens.allcurveStatic.colors;
    this->coloranimation.colormode = COLP_RAIMBOW_MODE;

    this->rasters[0] = (u8*) MEM_ALLOC(&sys.allocatorMem, ALLCURV_RASTERSIZE);

    this->feed = true;    

    this->drawcurveroutine[0] = g_screens.allcurveStatic.drawcurveroutine[0];
    this->drawcurveroutine[1] = MEM_ALLOC(&sys.allocatorMem, ALLCURV_CODESIZE2);

#   ifdef __TOS__
    {
        u8* output;
       
        this->drawcurveroutine[2] = allCurveGenerateDrawVymCurves(this->drawcurveroutine[1]);
        output = allCurveGenerateDrawVymNoiseCurves(this->drawcurveroutine[2]);
    
        ASSERT((output - this->drawcurveroutine[1]) <= ALLCURV_CODESIZE2);
        TRAClogNumber10S(TRAC_LOG_SPECIFIC, "allcurvecodesize2:", (output - this->drawcurveroutine[1]), 5, '\n');
    }
#   endif

    allCurveInitRasters(this);
    allCurveInitPixels (this);
    SmplCurveInitSampleOffsets(this->drawcurveroutine[0], (u16*) g_screens.allcurveStatic.codesampleoffset, g_screens.allcurveStatic.nbcodesampleoffsets, 8, 8);

    BlitZsetVideoMode(HW_VIDEO_MODE_4P, 0, BLITZ_VIDEO_16XTRA_PIXELS);

    RASnextOpList     = this->rasters[0];
    SYSvblroutines[1] = (SYSinterupt)RASvbl15;

    AllCurveManageCommands(this, false); /* hack => do this here in sync to avoid complexifying to put it on main thread like it should be */

    TRACsetVideoMode (168);

    SYSwriteVideoBase ((u32) this->framebuffer);

#   ifndef __TOS__
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRAClog(TRAC_LOG_MEMORY, "AllCurve memallocator dump", '\n');
        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif

    FSMgotoNextState(_fsm);
    FSMgotoNextState(&g_stateMachine);
}


void AllCurveActivity(FSM* _fsm)
{
    AllCurve* this = g_screens.allcurve;
    u8* image = (u8*) this->framebuffer;
    /*RASopVbl15* op = (RASopVbl15*) RASnextOpList;
    

    STDmcpy2(HW_COLOR_LUT+1, op->colors, 30);*/

#   ifndef __TOS__
    EMULfbExStart (HW_VIDEO_MODE_4P, 80, 40, 80 + ALLCURV_PITCH * 2 - 1, 40 + ALLCURV_DISPLAY_H - 1, ALLCURV_PITCH, 0);
    {
        u16  colorindex = this->coloranimation.currentplane * 32 * 16;
        u16* colors     = &this->coloranimation.colors[colorindex];
        u32  cycles;
        u16  i;

        for (i = 0 ; i < 32 ; i++)
        {
            u16 t;

            cycles = EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, 40 + i * 7);

            for (t = 0 ; t < 16 ; t++)
            {
                EMULfbExSetColor(cycles, t, colors[t]);
            }

            colors += 16;
        }
    }
    EMULfbExEnd();
#   endif

    IGNORE_PARAM(_fsm);

    if (this->feed)
    {
        SmplCurveYMcurveState ymstates[3];
        s16 ymincs[3];


        SmplCurveYMgetStates (ymstates);

        if (ymstates[0].type == SmplCurveYMcurveType_SQUARE)
            ymincs[0] = (s16) STDdivu(ALLCURV_YM_FREQMULTIPLIER, ymstates[0].freqsquare); 
        
        if (ymstates[1].type == SmplCurveYMcurveType_SQUARE)
            ymincs[1] = (s16) STDdivu(ALLCURV_YM_FREQMULTIPLIER, ymstates[1].freqsquare);
        
        if (ymstates[2].type == SmplCurveYMcurveType_SQUARE)
            ymincs[2] = (s16) STDdivu(ALLCURV_YM_FREQMULTIPLIER, ymstates[2].freqsquare); 

        image += this->coloranimation.currentplane << 1;
        this->coloranimation.currentplane++;
        this->coloranimation.currentplane &= 3;

        RASnextOpList = this->rasters[this->coloranimation.currentplane];

#       if ALLCURV_RASTERIZE()
        *HW_COLOR_LUT = 0x4;
#       endif

        /* ----------------------------------
        Draw sample curves
        ----------------------------------*/

#       if ALLCURV_RASTERIZE()
        *HW_COLOR_LUT = 0x300;
#       endif

        allCurveDrawVCurve(this, (void*)(g_screens.player.dmabufstart + 0), ALLCURV_DISPLAY_H, 8, image); image += 24;
        allCurveDrawVCurve(this, (void*)(g_screens.player.dmabufstart + 1), ALLCURV_DISPLAY_H, 8, image); image += 24;
        allCurveDrawVCurve(this, (void*)(g_screens.player.dmabufstart + 2), ALLCURV_DISPLAY_H, 8, image); image += 24;
        allCurveDrawVCurve(this, (void*)(g_screens.player.dmabufstart + 3), ALLCURV_DISPLAY_H, 8, image); image += 24;

        allCurveDrawVymCurve(this, image, ymstates    , ymincs[0]); image += 24;
        allCurveDrawVymCurve(this, image, ymstates + 1, ymincs[1]); image += 24;
        allCurveDrawVymCurve(this, image, ymstates + 2, ymincs[2]);
    }
    else if (this->step)
    {
        u16* p;
        u16  t;

        this->step = false;

        image += this->coloranimation.currentplane << 1;
        this->coloranimation.currentplane++;
        this->coloranimation.currentplane &= 3;

        RASnextOpList = this->rasters[this->coloranimation.currentplane];

        p = (u16*) image;

        for (t = ALLCURV_DISPLAY_H; t > 0; t--)
        {
            p[0]  = 0;
            p[4]  = 0;
            p[8]  = 0;
            p[12] = 0;
            p[16] = 0;
            p[20] = 0;
            p[24] = 0;
            p[28] = 0;
            p[32] = 0;
            p[36] = 0;
            p[40] = 0;
            p[44] = 0;
            p[48] = 0;
            p[52] = 0;
            p[56] = 0;
            p[60] = 0;
            p[64] = 0;
            p[68] = 0;
            p[72] = 0;
            p[76] = 0;
            p[80] = 0;

            p += ALLCURV_PITCH / 2;
        }
    }

#   if ALLCURV_RASTERIZE()
    *HW_COLOR_LUT = 0x0;
#   endif

    AllCurveManageCommands(this, true);
}


void AllCurveBacktask(FSM* _fsm)
{
    IGNORE_PARAM(_fsm);
}

void AllCurveExit(FSM* _fsm)
{
    AllCurve* this = g_screens.allcurve;

    IGNORE_PARAM(_fsm);

    SYSvblroutines[1] = RASvbldonothing;
    BlitZturnOffDisplay();

    MEM_FREE(&sys.allocatorMem, this->drawcurveroutine[1]);
    MEM_FREE(&sys.allocatorMem, this->rasters[0]);
    MEM_FREE(&sys.allocatorMem, this->framebuffer);
    MEM_FREE(&sys.allocatorMem, g_screens.allcurve);

    this = g_screens.allcurve = NULL;

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


void AllCurveInitStatic (FSM* _fsm)
{    
    ScreensLogFreeArea("AllCurveInitStatic");

    g_screens.allcurveStatic.drawcurveroutine[0] = (u8*) MEM_ALLOC(&sys.allocatorCoreMem, ALLCURV_CODESIZE1);

#   ifdef __TOS__
    {
        u8* output = allCurveGenerateDrawVCurves(g_screens.allcurveStatic.codesampleoffset, &g_screens.allcurveStatic.nbcodesampleoffsets, g_screens.allcurveStatic.drawcurveroutine[0]);
        ASSERT((output - g_screens.allcurveStatic.drawcurveroutine[0]) <= ALLCURV_CODESIZE1);

        TRAClogNumber10S(TRAC_LOG_SPECIFIC, "allcurvecodesize1:", (output - g_screens.allcurveStatic.drawcurveroutine[0]), 5, '\n');
    }
#   endif

    {
        u16* colors;

        static u16 curve1[3] = {0x081, 0x023, 0x04E};
        static u16 curve2[3] = {0x888, 0x333, 0xEEE};
        static u16 curve3[3] = {0x180, 0x320, 0xE40};

        g_screens.allcurveStatic.colors = colors = (u16*) MEM_ALLOC(&sys.allocatorCoreMem, 32*4*16*2);

        colors = allCurveInitColors3Pgradient(colors, 32*16, 16, curve1, curve2, 0);
        allCurveInitColors3Pgradient(colors, 32*16, 16, curve2, curve3, 0);
    }

    FSMgotoNextState(_fsm);
}
