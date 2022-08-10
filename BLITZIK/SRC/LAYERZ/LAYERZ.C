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
#include "DEMOSDK\BITMAP.H"

#include "DEMOSDK\PC\EMUL.H"

#include "DEMOSDK\SYNTHYMD.H"

#include "FX\SMPLCURV\SMPLCURV.H"

#include "BLITZIK\SRC\SCREENS.H"


void LAYZmid15(void) PCSTUB;

ASMIMPORT u16 LAYZcount;

#ifdef __TOS__
#define LAYERZ_RASTERIZE() 0
#else
#define LAYERZ_RASTERIZE() 0
#endif

#if LAYERZ_RASTERIZE()
#   define LAYERZ_RASTERIZE_COLOR(COLOR) *HW_COLOR_LUT=COLOR
#else
#   define LAYERZ_RASTERIZE_COLOR(COLOR)
#endif
                                   

#define LAYERZ_SCORE_COLOR_STE      0x0EF
#define LAYERZ_SCORE_COLOR_4B       0x0DF

#define LAYERZ_PITCH                168
#define LAYERZ_DISPLAY_H            245

#define LAYERZ_CIRCLE_W             292
                                    
#define LAYERZ_1CURVE_H             244
#define LAYERZ_2CURVES_H            90
#define LAYERZ_CURVE0_Y             ((LAYERZ_DISPLAY_H - LAYERZ_2CURVES_H) / 2 - 1)
#define LAYERZ_CURVE1_Y             ((LAYERZ_DISPLAY_H + LAYERZ_2CURVES_H) / 2 + 1)
                                    
#define LAYERZ_XOR_PASS1_H          17
#define LAYERZ_XOR_PASS3_H          43
                                    
#define LAYERZ_CURSORS_Y1           3
#define LAYERZ_CURSORS_H            34

#define LAYERZ_YM_FREQMULTIPLIER    397000UL /* (32768 / 333) * (15 * 256 + 192) */

#define LAYERZ_1STCHAR_VOL          1
#define LAYERZ_1STCHAR_MASK         18

#define LAYERZ_CHAR_MIX_SQR_NOISE   22
#define LAYERZ_CHAR_MIX_SQR         23
#define LAYERZ_CHAR_MIX_NOISE       24
#define LAYERZ_CHAR_MIX_NONE        25

#define LAYERZ_YM_FREQMULTIPLIER2 540000UL /* (32768 / DISPLAY_H) * (15 * 256 + 192) */



enum layerZType
{
    layerZ_TWINS,
    layerZ_SOLO,
    layerZ_SIAMESE
};


#define SPR_OPCODE_MOVE_DATA_X_A0   0x317C

static void layerZDrawHYM (SmplCurveYMcurveState* state, void* drawroutineym, void* display, u8 shift)
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
            (u8*) disp - LAYERZ_PITCH * level,
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
            (u8*) disp - LAYERZ_PITCH * level,
            -inc, inc, inc);
    }
}



static void layerZSetColors (LayerZ* this, bool inverttop_)
{
    u16* p = this->palette[this->flip][0];
    u16  color;
    u16  t;


    /*
    0000 0
    0001 1
    0010 2
    0011 3
    0100 4
    0101 5
    0110 6 
    0111 7
    1000 8
    1001 9 
    1010 10
    1011 11
    1100 12
    1101 13
    1110 14
    1111 15
    */

    for (t = 0 ; t < this->multinbcolors ; t++, p += 16)
    {
        color = this->multicolors[this->flip][t];

        p[0]  = 0;
        p[2]  = 0;
        p[4]  = 0;
        p[6]  = 0;
        p[8]  = 0;
        p[10] = 0;
        p[12] = 0;
        p[14] = 0;

        p[1]  = color;
        p[3]  = color;
        p[5]  = color;
        p[7]  = color;
        p[9]  = color;
        p[11] = color;
        p[13] = color;
        p[15] = color;

        color = this->layercolors[1];

        switch (this->layersop[1])
        {
        case LayerZop_TOP:

            if (inverttop_)
            {
                p[2]  = color;
                p[6]  = color;
                p[10] = color;
                p[14] = color;
            }
            else
            {
                p[2]  = color;
                p[3]  = color;
                p[6]  = color;
                p[7]  = color;
                p[10] = color;
                p[11] = color;
                p[14] = color;
                p[15] = color;
            }
            break;

        case LayerZop_OR:
            p[2]  |= color;
            p[3]  |= color;
            p[6]  |= color;
            p[7]  |= color;
            p[10] |= color;
            p[11] |= color;
            p[14] |= color;
            p[15] |= color;
            break;

        case LayerZop_XOR:
            p[2]  ^= color;
            p[3]  ^= color;
            p[6]  ^= color;
            p[7]  ^= color;
            p[10] ^= color;
            p[11] ^= color;
            p[14] ^= color;
            p[15] ^= color;
            break;

        case LayerZop_TRANPARENT:

            if (inverttop_)
            {
                p[0]  = p[2];
                p[1]  = p[3];
                p[4]  = p[6];
                p[5]  = p[7];
                p[8]  = p[10];
                p[9]  = p[11];
                p[12] = p[14];
                p[13] = p[15];
            }
            else
            {
                p[2]  = p[0];
                p[3]  = p[1];
                p[6]  = p[4];
                p[7]  = p[5];
                p[10] = p[8];
                p[11] = p[9];
                p[14] = p[12];
                p[15] = p[13];
            }
            break;
        }

        color = this->layercolors[2];

        switch (this->layersop[2])
        {
        case LayerZop_TOP:
            p[4]  = color;
            p[5]  = color;
            p[6]  = color;
            p[7]  = color;
            p[12] = color;
            p[13] = color;
            p[14] = color;
            p[15] = color;
            break;

        case LayerZop_OR:
            p[4]  |= color;
            p[5]  |= color;
            p[6]  |= color;
            p[7]  |= color;
            p[12] |= color;
            p[13] |= color;
            p[14] |= color;
            p[15] |= color;
            break;

        case LayerZop_XOR:
            p[4]  ^= color;
            p[5]  ^= color;
            p[6]  ^= color;
            p[7]  ^= color;
            p[12] ^= color;
            p[13] ^= color;
            p[14] ^= color;
            p[15] ^= color;
            break;

        case LayerZop_TRANPARENT:
            p[4]  = p[0];
            p[5]  = p[1];
            p[6]  = p[2];
            p[7]  = p[3];
            p[12] = p[8];
            p[13] = p[9];
            p[14] = p[10];
            p[15] = p[11];
            break;
        }

        color = this->layercolors[3];

        switch (this->layersop[3])
        {
        case LayerZop_TOP:
            p[8]  = color;
            p[9]  = color;
            p[10] = color;
            p[11] = color;
            p[12] = color;
            p[13] = color;
            p[14] = color;
            p[15] = color;
            break;

        case LayerZop_OR:
            p[8]  ^= color;
            p[9]  ^= color;
            p[10] ^= color;
            p[11] ^= color;
            p[12] ^= color;
            p[13] ^= color;
            p[14] ^= color;
            p[15] ^= color;
            break;

        case LayerZop_XOR:
            p[8]  |= color;
            p[9]  |= color;
            p[10] |= color;
            p[11] |= color;
            p[12] |= color;
            p[13] |= color;
            p[14] |= color;
            p[15] |= color;
            break;

        case LayerZop_TRANPARENT:
            p[8]  = p[0];
            p[9]  = p[1];
            p[10] = p[2];
            p[11] = p[3];
            p[12] = p[4];
            p[13] = p[5];
            p[14] = p[6];
            p[15] = p[7];
            break;
        }
    }
}



static void layerZInitRasters(LayerZ* this)
{
    u16 i;

    {
        RASopVbl* op = &this->rasters1.vbl;

        op->scanLinesTo1stInterupt = 199;
        op->nextRasterRoutine      = (RASinterupt) RASlowB;
    }
    
    {
        RASopLowB* op = &this->rasters1.lowb;

        op->scanLineToNextInterupt = 200;
        op->nextRasterRoutine = NULL;
    }

    /* ---------------------- */

    for (i = 0 ; i < 2 ; i++)
    {
        {
            RASopVbl* op = &this->rasters2[i].vbl;

            op->scanLinesTo1stInterupt = LAYERZ_CURSORS_H + LAYERZ_CURSORS_Y1;
            op->nextRasterRoutine = (RASinterupt)LAYZmid15;
        }

        {
            LAYZopMid15* op;
            u16 t;

            for (t = 0; t < 4; t++)
            {
                op = &this->rasters2[i].mid15[t];

                op->colors                 = &this->palette[i][t+1][1];
                op->scanLineToNextInterupt = LAYERZ_CURSORS_H;
                op->nextRasterRoutine      = LAYZmid15;
            }

            op = &this->rasters2[i].mid15[4];

            op->colors                 = &this->palette[i][5][1];
            op->scanLineToNextInterupt = 199 - ((LAYERZ_CURSORS_H * 5) + LAYERZ_CURSORS_Y1);
            op->nextRasterRoutine      = RASlowB;
        }

        {
            RASopLowB* op = &this->rasters2[i].lowb;

            op->scanLineToNextInterupt = ((LAYERZ_CURSORS_H * 6) + LAYERZ_CURSORS_Y1) - 199;
            op->nextRasterRoutine = LAYZmid15;
        }

        {
            LAYZopMid15* op = &this->rasters2[i].mid15_last;

            op->colors = &this->palette[i][6][1];
            op->scanLineToNextInterupt = 200;
            op->nextRasterRoutine = NULL;
        }
    }
}


static void layerZInitColors(LayerZ* this)
{
    u16* p;
    u16 t;

    
    this->curvecolorcycling[0]  = 0xF00;
    this->curvecolorcycling[16] = 0xFE0;
    this->curvecolorcycling[32] = 0x725;
    this->curvecolorcycling[48] = 0x03F;
    this->curvecolorcycling[64] = 0x426;
    this->curvecolorcycling[80] = 0xF00;

    COLcomputeGradient(&this->curvecolorcycling[0 ], &this->curvecolorcycling[16], 1, 16, &this->curvecolorcycling[0 ]);
    COLcomputeGradient(&this->curvecolorcycling[16], &this->curvecolorcycling[32], 1, 16, &this->curvecolorcycling[16]);
    COLcomputeGradient(&this->curvecolorcycling[32], &this->curvecolorcycling[48], 1, 16, &this->curvecolorcycling[32]);
    COLcomputeGradient(&this->curvecolorcycling[48], &this->curvecolorcycling[64], 1, 16, &this->curvecolorcycling[48]);
    COLcomputeGradient(&this->curvecolorcycling[64], &this->curvecolorcycling[80], 1, 16, &this->curvecolorcycling[64]);

    this->scorecolor[0]  = LAYERZ_SCORE_COLOR_STE;
    this->scorecolor[16] = PCENDIANSWAP16(0xFFF);
    p = this->scorecolor + 1;

    for (t = 1 ; t < 16 ; t++)
        COLcomputeGradient16Steps(&this->scorecolor[0], &this->scorecolor[16], 1, t, p++);

    this->keyscolors[0] = 0xF00;  
    this->keyscolors[1] = 0xFF0;  
    this->keyscolors[2] = 0xF40;  
    this->keyscolors[3] = 0x04F;  
    this->keyscolors[4] = 0x0F0;  
    this->keyscolors[5] = 0x00F;  
    this->keyscolors[6] = 0xF0F;  
    this->keyscolors[7] = 0xF04;  
}



static void layerzSetSampleDisplay(LayerZ* this)
{
    SmplCurveInitSampleOffsets (
        g_screens.samcurveStatic.drawcurveroutine, 
        g_screens.samcurveStatic.codesampleoffset, 
        ARRAYSIZE(g_screens.samcurveStatic.codesampleoffset),
        this->samrequest.inc1, 
        this->samrequest.inc2);

    this->samdisplay = this->samrequest;
}

static void layerZprepareFont (LayerZ* this)
{
    u8* p;
    u16 t;
    u8  m, m2;
    u8  offset = SYSfont.size >> SYS_FNT_OFFSETSHIFT;


    this->font.size = SYSfont.size + 1000;
    this->font.data = (u8*) MEM_ALLOC(&sys.allocatorMem, this->font.size);
    STDmcpy(this->font.data, SYSfont.data, SYSfont.size);
    STDmcpy(this->font.charsmap, SYSfont.charsmap, ARRAYSIZE(SYSfont.charsmap));

    p = (this->font.charsmap['|'] << SYS_FNT_OFFSETSHIFT) + this->font.data;

    *p++ = 0x10;
    *p++ = 0x10;
    *p++ = 0x10;
    *p++ = 0x10;
    *p++ = 0x10;
    *p++ = 0x10;
    *p++ = 0x10;
    *p++ = 0x10;

    p = this->font.data + SYSfont.size;

    m  = 0;
    m2 = 0;

    this->font.charsmap[LAYERZ_1STCHAR_VOL] = offset;

    *p++ = m;
    *p++ = m;
    *p++ = m;
    *p++ = m;
    *p++ = m;
    *p++ = m;
    *p++ = m;
    *p++ = m;

    m >>= 1;
    m |= 0x80;

    offset += 8 >> SYS_FNT_OFFSETSHIFT;

    for (t = 0 ; t < 16 ; t += 2)
    {
        this->font.charsmap[t + 2] = offset;

        *p++ = m;
        *p++ = m2;
        *p++ = m;
        *p++ = m2;
        *p++ = m;
        *p++ = m2;
        *p++ = m;
        *p++ = m2;

        offset += 8 >> SYS_FNT_OFFSETSHIFT;

        this->font.charsmap[t + 3] = offset;

        *p++ = m;
        *p++ = m;
        *p++ = m;
        *p++ = m;
        *p++ = m;
        *p++ = m;
        *p++ = m;
        *p++ = m;

        m2 = m;
        m >>= 1;
        m |= 0x80;

        offset += 8 >> SYS_FNT_OFFSETSHIFT;
    }

    for (t = 0 ; t < 4 ; t ++)        
    {
        this->font.charsmap[LAYERZ_1STCHAR_MASK + t] = offset;

        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;

        offset += 8 >> SYS_FNT_OFFSETSHIFT;
    }

    this->font.charsmap[LAYERZ_CHAR_MIX_SQR_NOISE] = offset;

    *p++ = 0x8A;
    *p++ = 0x84;
    *p++ = 0x8A;
    *p++ = 0x84;
    *p++ = 0x2A;
    *p++ = 0x24;
    *p++ = 0x2A;
    *p++ = 0x24;
    offset += 8 >> SYS_FNT_OFFSETSHIFT;

    this->font.charsmap[LAYERZ_CHAR_MIX_SQR] = offset;

    *p++ = 0x80;
    *p++ = 0x80;
    *p++ = 0x80;
    *p++ = 0x80;
    *p++ = 0x20;
    *p++ = 0x20;
    *p++ = 0x20;
    *p++ = 0x20;
    offset += 8 >> SYS_FNT_OFFSETSHIFT;

    this->font.charsmap[LAYERZ_CHAR_MIX_NOISE] = offset;

    *p++ = 0xA;
    *p++ = 0x14;
    *p++ = 0xA;
    *p++ = 0x14;
    *p++ = 0xA;
    *p++ = 0x14;
    *p++ = 0xA;
    *p++ = 0x14;
    offset += 8 >> SYS_FNT_OFFSETSHIFT;

    ASSERT((p - this->font.data) <= this->font.size);
}


static void layerZActivityTwinsCurves(LayerZ* this, u8* image, s16 index_)
{   
    /*this->layersop[1] = LayerZop_OR;*/

    /* ---------------------------------------------------------------------------
    Clear plane                                                                
    --------------------------------------------------------------------------- */
    LAYERZ_RASTERIZE_COLOR(0x4);

    *HW_BLITTER_ADDR_DEST = (u32)image;
    *HW_BLITTER_ENDMASK1  = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1;
    *HW_BLITTER_XSIZE     = LAYERZ_PITCH / 8;
    *HW_BLITTER_YSIZE     = LAYERZ_DISPLAY_H;
    *HW_BLITTER_HOP       = HW_BLITTER_HOP_BIT1;
    *HW_BLITTER_OP        = HW_BLITTER_OP_BIT0;
    *HW_BLITTER_XINC_DEST = 8;
    *HW_BLITTER_YINC_DEST = 8;
    *HW_BLITTER_CTRL2     = 0;

    *HW_BLITTER_CTRL1     = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
    EMULblit();

    /*----------------------------------
    Draw sample curves
    ----------------------------------*/
    
    LAYERZ_RASTERIZE_COLOR(0x300);

    if (this->voice < 4)
    {
        SmplCurveDrawHCurve(
            g_screens.samcurveStatic.drawcurveroutine,
            (void*)(g_screens.player.dmabufstart + this->voice),
            333,
            this->samdisplay.inc1,
            image + LAYERZ_CURVE0_Y * LAYERZ_PITCH,
            this->offsets);
    }
    else
    {
        layerZDrawHYM (&this->ymstates[this->voice - 4], g_screens.samcurveStatic.drawcurveroutineym, image + (LAYERZ_CURVE0_Y - 1) * LAYERZ_PITCH, 2);
    }

    LAYERZ_RASTERIZE_COLOR(0x400);

    if (this->voice2 < 4)
    {
        SmplCurveDrawHCurve(
            g_screens.samcurveStatic.drawcurveroutine,
            (void*)(g_screens.player.dmabufstart + this->voice2),
            333,
            this->samdisplay.inc1,
            image + LAYERZ_CURVE1_Y * LAYERZ_PITCH,
            this->offsets);
    }
    else
    {
        layerZDrawHYM (&this->ymstates[this->voice2 - 4], g_screens.samcurveStatic.drawcurveroutineym, image + (LAYERZ_CURVE1_Y - 1) * LAYERZ_PITCH, 2);
    }

    /* --------------------------------------------------------------------------- */
    /* XOR PASS                                                                    */ 
    /* --------------------------------------------------------------------------- */

    LAYERZ_RASTERIZE_COLOR(0x30);

    /* --------------------------------------------------------------------------- */
    /* Common setup for xor pass */

    /* Already set by clear pass */
    /* *HW_BLITTER_ENDMASK1    = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1; */ 
    /* *HW_BLITTER_XINC_DEST   = 8; */                                                
    /* *HW_BLITTER_YINC_DEST   = 8; */
    /* *HW_BLITTER_XSIZE       = LAYERZ_PITCH / 8; */

    *HW_BLITTER_HOP         = HW_BLITTER_HOP_SOURCE;
    *HW_BLITTER_OP          = HW_BLITTER_OP_S_XOR_D;

    /* --------------------------------------------------------------------------- */
    /* Curve 1 pass                                                                */

    *HW_BLITTER_ADDR_SOURCE =  (u32) image + (LAYERZ_CURVE0_Y - (LAYERZ_2CURVES_H / 2)) * LAYERZ_PITCH;
    *HW_BLITTER_ADDR_DEST   = *HW_BLITTER_ADDR_SOURCE + LAYERZ_PITCH;
    *HW_BLITTER_YSIZE       = (LAYERZ_2CURVES_H / 2) - 1;
    *HW_BLITTER_XINC_SOURCE = 8;
    *HW_BLITTER_YINC_SOURCE = 8;

    *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
    EMULblit();

    LAYERZ_RASTERIZE_COLOR(0x40);

    *HW_BLITTER_ADDR_SOURCE = ((u32) image) + (LAYERZ_CURVE0_Y + (LAYERZ_2CURVES_H / 2 - 1)) * LAYERZ_PITCH;
    *HW_BLITTER_ADDR_DEST   = *HW_BLITTER_ADDR_SOURCE - LAYERZ_PITCH;
    *HW_BLITTER_YSIZE       = (LAYERZ_2CURVES_H / 2) - 1;
    *HW_BLITTER_YINC_SOURCE = -LAYERZ_PITCH*2 + 8;
    *HW_BLITTER_YINC_DEST   = -LAYERZ_PITCH*2 + 8;

    *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
    EMULblit();

    LAYERZ_RASTERIZE_COLOR(0x40);

    /* --------------------------------------------------------------------------- */
    /* Curve 2 pass                                                                */

    image += (LAYERZ_CURVE1_Y - (LAYERZ_2CURVES_H / 2)) * LAYERZ_PITCH;

    *HW_BLITTER_ADDR_SOURCE = ((u32) image);
    *HW_BLITTER_ADDR_DEST   = ((u32) image) + LAYERZ_PITCH;
    *HW_BLITTER_YSIZE       = (LAYERZ_2CURVES_H / 2) - 1;
    *HW_BLITTER_YINC_SOURCE = 8;
    *HW_BLITTER_YINC_DEST   = 8;
    
    *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
    EMULblit();

    LAYERZ_RASTERIZE_COLOR(0x30);

    *HW_BLITTER_ADDR_SOURCE = ((u32) image) + (LAYERZ_2CURVES_H - 1) * LAYERZ_PITCH;
    *HW_BLITTER_ADDR_DEST   = ((u32) image) + (LAYERZ_2CURVES_H - 2) * LAYERZ_PITCH;
    *HW_BLITTER_YSIZE       = (LAYERZ_2CURVES_H / 2) - 1;
    *HW_BLITTER_YINC_SOURCE = -LAYERZ_PITCH*2 + 8;
    *HW_BLITTER_YINC_DEST   = -LAYERZ_PITCH*2 + 8;
    
    *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;       /* run */
    EMULblit();

    LAYERZ_RASTERIZE_COLOR(0x7);

    this->multicolors[this->flip][0] = this->curvecolorcycling[this->colorindex >> 4];

    this->colorindex += this->colorinc;
    if (this->colorindex >= (ARRAYSIZE(this->curvecolorcycling) << 4))
        this->colorindex = 0;

    layerZSetColors(this, this->inverttop);

    LAYERZ_RASTERIZE_COLOR(0);
}


static void layerZActivitySoloCurve(LayerZ* this, u8* image, s16 index_)
{
    LAYERZ_RASTERIZE_COLOR(0x4);

    /* ----------------------------------
    Clear plane
    ----------------------------------*/
    *HW_BLITTER_ADDR_DEST = (u32)image;
    *HW_BLITTER_ENDMASK1  = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1;
    *HW_BLITTER_XSIZE     = LAYERZ_PITCH / 8;
    *HW_BLITTER_YSIZE     = LAYERZ_DISPLAY_H;
    *HW_BLITTER_HOP       = HW_BLITTER_HOP_BIT1;
    *HW_BLITTER_OP        = HW_BLITTER_OP_BIT0;
    *HW_BLITTER_XINC_DEST = 8;
    *HW_BLITTER_YINC_DEST = 8;
    *HW_BLITTER_CTRL2     = 0;
 
    *HW_BLITTER_CTRL1     = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
    EMULblit();

    /* ----------------------------------
    Draw sample curves
    ----------------------------------*/

    LAYERZ_RASTERIZE_COLOR(0x300);

    if (this->voice < 4)
    {
        SmplCurveDrawHCurve(g_screens.samcurveStatic.drawcurveroutine,
            (void*)(g_screens.player.dmabufstart + this->voice),
            333,
            this->samdisplay.inc1,
            image + (LAYERZ_1CURVE_H / 2) * LAYERZ_PITCH,
            this->offsets2);
    }
    else
    {
        layerZDrawHYM(&this->ymstates[this->voice - 4], g_screens.samcurveStatic.drawcurveroutineym, image + ((LAYERZ_1CURVE_H / 2) - 1) * LAYERZ_PITCH, 3);
    }

    /* ----------------------------------
    Xor pass
    ----------------------------------*/

    if (this->xorfreeareah != 255)
    {
        LAYERZ_RASTERIZE_COLOR(0x30);

        /* Common setup for xor pass */

        /* *HW_BLITTER_ENDMASK1    = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1; */ /* Already set by clear pass */
                                                                                          /* *HW_BLITTER_XSIZE       = LAYERZ_PITCH / 8; */ /* Already set by clear pass */
        *HW_BLITTER_HOP         = HW_BLITTER_HOP_SOURCE;
        *HW_BLITTER_OP          = HW_BLITTER_OP_S_XOR_D;
        *HW_BLITTER_XINC_SOURCE = 8;
        /* *HW_BLITTER_XINC_DEST = 8; */                                                  /* Already set by clear pass */
                                                                                          /* *HW_BLITTER_CTRL2       = 0; */                /* Already set by clear pass */
                                                                                                                                            /* draw curve 1 */
        *HW_BLITTER_ADDR_SOURCE = (u32)image;
        *HW_BLITTER_ADDR_DEST   = ((u32)image) + LAYERZ_PITCH;
        *HW_BLITTER_YSIZE       = (LAYERZ_1CURVE_H / 2) - 1 - this->xorfreeareah;
        *HW_BLITTER_YINC_SOURCE = 8;
        /**HW_BLITTER_YINC_DEST = 8;*/                                                    /* Already set by clear pass */
        *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;  /* run */

        EMULblit();

        LAYERZ_RASTERIZE_COLOR(0x20);

        *HW_BLITTER_ADDR_SOURCE = ((u32)image) + (u32)(LAYERZ_1CURVE_H - 1) * (u32)LAYERZ_PITCH;
        *HW_BLITTER_ADDR_DEST   = ((u32)image) + (u32)(LAYERZ_1CURVE_H - 2) * (u32)LAYERZ_PITCH;
        *HW_BLITTER_YSIZE       = (LAYERZ_1CURVE_H / 2) - 1 - this->xorfreeareah;
        *HW_BLITTER_YINC_SOURCE = -LAYERZ_PITCH * 2 + 8;
        *HW_BLITTER_YINC_DEST   = -LAYERZ_PITCH * 2 + 8;
        *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */

        EMULblit();
    }

    LAYERZ_RASTERIZE_COLOR(0x7);

    this->multicolors[this->flip][0] = this->curvecolorcycling[this->colorindex >> 4];

    this->colorindex += this->colorinc;
    if (this->colorindex >= (ARRAYSIZE(this->curvecolorcycling) << 4))
        this->colorindex = 0;

    layerZSetColors(this, this->inverttop);

    LAYERZ_RASTERIZE_COLOR(0x0);
}


static void layerZActivitySiameseCurves(LayerZ* this, u8* image, s16 index_)
{
    LAYERZ_RASTERIZE_COLOR(0x4);

    /* ----------------------------------
    Clear plane
    ----------------------------------*/
    *HW_BLITTER_ADDR_DEST = (u32)image;
    *HW_BLITTER_ENDMASK1  = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1;
    *HW_BLITTER_XSIZE     = LAYERZ_PITCH / 8;
    *HW_BLITTER_YSIZE     = LAYERZ_DISPLAY_H;
    *HW_BLITTER_HOP       = HW_BLITTER_HOP_BIT1;
    *HW_BLITTER_OP        = HW_BLITTER_OP_BIT0;
    *HW_BLITTER_XINC_DEST = 8;
    *HW_BLITTER_YINC_DEST = 8;
    *HW_BLITTER_CTRL2     = 0;

    *HW_BLITTER_CTRL1     = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
    EMULblit();

    /* ----------------------------------
    Draw sample curves
    ----------------------------------*/

    LAYERZ_RASTERIZE_COLOR(0x300);

    if (this->voice < 4)
    {
        SmplCurveDrawHCurve(g_screens.samcurveStatic.drawcurveroutine,
            (void*)(g_screens.player.dmabufstart + this->voice),
            333, this->samdisplay.inc1,
            image + LAYERZ_CURVE0_Y * LAYERZ_PITCH,
            this->offsets);
    }
    else
    {
        layerZDrawHYM(&this->ymstates[this->voice - 4], g_screens.samcurveStatic.drawcurveroutineym, image + (LAYERZ_CURVE0_Y - 1) * LAYERZ_PITCH, 2);
    }

    LAYERZ_RASTERIZE_COLOR(0x400);

    if (this->voice2 < 4)
    {
        SmplCurveDrawHCurve(g_screens.samcurveStatic.drawcurveroutine,
            (void*)(g_screens.player.dmabufstart + this->voice2),
            333,
            this->samdisplay.inc1,
            image + LAYERZ_CURVE1_Y * LAYERZ_PITCH,
            this->offsets);
    }
    else
    {
        layerZDrawHYM(&this->ymstates[this->voice2 - 4], g_screens.samcurveStatic.drawcurveroutineym, image + (LAYERZ_CURVE1_Y - 1) * LAYERZ_PITCH, 2);
    }

    /* ----------------------------------
    Xor pass
    ----------------------------------*/
    if (this->xorfreeareah != 255)
    {
        LAYERZ_RASTERIZE_COLOR(0x30);

        /* Common setup for xor pass */

        /* *HW_BLITTER_ENDMASK1    = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1; */ /* Already set by clear pass */
                                                                                          /* *HW_BLITTER_XSIZE       = LAYERZ_PITCH / 8; */                                /* Already set by clear pass */
        *HW_BLITTER_HOP = HW_BLITTER_HOP_SOURCE;
        *HW_BLITTER_OP = HW_BLITTER_OP_S_XOR_D;
        *HW_BLITTER_XINC_SOURCE = 8;
        *HW_BLITTER_YINC_SOURCE = 8;
                                                                                          /* *HW_BLITTER_CTRL2       = 0; */                                                /* Already set by clear pass */
                                                                                                                                                                            /* draw curve 1 */
        
        *HW_BLITTER_ADDR_SOURCE = (u32)image + (LAYERZ_CURVE0_Y - LAYERZ_2CURVES_H / 2) * LAYERZ_PITCH;
        *HW_BLITTER_ADDR_DEST = *HW_BLITTER_ADDR_SOURCE + LAYERZ_PITCH;
        *HW_BLITTER_YSIZE = LAYERZ_2CURVES_H * 2;
        /**HW_BLITTER_XINC_DEST = 8;*/                                                    /* Already set by clear pass */
        /**HW_BLITTER_YINC_DEST = 8;*/                                                    /* Already set by clear pass */
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */

        EMULblit();
    }

    LAYERZ_RASTERIZE_COLOR(0x7);
   
    this->multicolors[this->flip][0] = this->curvecolorcycling[this->colorindex >> 4];

    this->colorindex += this->colorinc;
    if (this->colorindex >= (ARRAYSIZE(this->curvecolorcycling) << 4))
        this->colorindex = 0;

    layerZSetColors(this, this->inverttop);

    LAYERZ_RASTERIZE_COLOR(0);
}


typedef void (*LayerZSpriteCall)(void* adr_);

#ifdef __TOS__

static void layerZdrawSprite(LayerZ* this, void* adr_, u16 x_)
{  
    u16  offset = (x_ >> 4) << 3;
    u16* base   = (u16*) adr_;


    base += offset >> 1;

    ((LayerZSpriteCall) this->spritecode[x_ & 15])(base);
}

void LAYZcomputeVolumes (s8*p, u16* vol);
void LAYZym             (void* _screen, s16 inc, u8 w0, u16 h_);
void LAYZymnoise        (void* _screen, s16 bittotest, u8 w0, u16 h_);

#else

static void layerZdrawSprite(LayerZ* this, void* adr_, u16 x_)
{  
    u16* code   = (u16*) this->spritecode[x_ & 15];
    u16  offset = (x_ >> 4) << 3;
    u16* base   = (u16*) adr_;


    base += offset >> 1;

    while (*code != CGEN_OPCODE_RTS)
    {
        base[ code[2] >> 1 ] = code[1];
        code += 3;
    }    
}

static void LAYZcomputeVolumes (s8*p, u16* vol)
{
    u16 t;


    vol[0] = vol[1] = vol[2] = vol[3] = 0;

    for (t = 0 ; t < 250 ; t++)
    {
        s8 s;

        s = *p++;   
        vol[0] += s > 0 ? s : -s;

        s = *p++;   
        vol[1] += s > 0 ? s : -s;

        s = *p++;   
        vol[3] += s > 0 ? s : -s;

        s = *p++;   
        vol[2] += s > 0 ? s : -s;

        p += 4;
    }
}


static void LAYZym (void* _screen, s16 inc, u8 w0, u16 h_)
{
    u8*  d = (u8*) _screen;
    u16  t;
    s16  acc = -inc;


    for (t = 0 ; t < h_ ; t++)
    {
        acc += inc;

        if (acc >= 0)
        {
            *d = w0;
        }
        else
        {
            *d = 0;
        }
        
        d += LAYERZ_PITCH;
    }
}


static void LAYZymnoise (void* _screen, s16 bittotest, u8 w0, u16 h_)
{
    u8* d = (u8*) _screen;
    u16 t;


    for (t = 0; t < h_ ; t++)
    {
        if (STDmfrnd() & (1 << bittotest))
        {
            *d = w0;
        }
        else
        {
            *d = 0;
        }

        d += LAYERZ_PITCH;
    }
}

#endif // ! __TOS__




static void layzDrawVymCurve (LayerZ* this, void* _screen, SmplCurveYMcurveState* state_, s16 inc_)
{
#   if LAYERZ_RASTERIZE()
    *HW_COLOR_LUT ^= 0x20;
#   endif

    if (state_->type == SmplCurveYMcurveType_NONE)
    {
        LAYZym (_screen, 0,0, LAYERZ_DISPLAY_H);
    }
    else
    {
        static const u8 ymLevelBitmaps[16] = { 0x00, 0x01, 0x02, 0x03,   0x07, 0x0F, 0x0F, 0x1F,   0x3F, 0x7F, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0xFF };

        u8   level = state_->level;
        u8   w0    = ymLevelBitmaps[level];

        if (state_->type & SmplCurveYMcurveType_NOISE)  /* noise (if noise and square, display noise) */
        {
            u8 freqnoise = state_->freqnoise;

            freqnoise >>= 3;
            freqnoise += 5;
            if (freqnoise > 7)
                freqnoise = 7;

            LAYZymnoise(_screen, 1, w0, LAYERZ_DISPLAY_H);
        }
        else /* square */
        {
            LAYZym(_screen, inc_, w0, LAYERZ_DISPLAY_H);
        }
    }
}


static void layerzDrawYM(LayerZ* this, u8* image)
{
    s16 ymincs[3];


    if (this->ymstates[0].type == SmplCurveYMcurveType_SQUARE)
        ymincs[0] = (s16)STDdivu(LAYERZ_YM_FREQMULTIPLIER2, this->ymstates[0].freqsquare);

    if (this->ymstates[1].type == SmplCurveYMcurveType_SQUARE)
        ymincs[1] = (s16)STDdivu(LAYERZ_YM_FREQMULTIPLIER2, this->ymstates[1].freqsquare);

    if (this->ymstates[2].type == SmplCurveYMcurveType_SQUARE)
        ymincs[2] = (s16)STDdivu(LAYERZ_YM_FREQMULTIPLIER2, this->ymstates[2].freqsquare);

#   if LAYERZ_RASTERIZE()
    *HW_COLOR_LUT = 0x4;
#   endif

    layzDrawVymCurve (this, image + 105, this->ymstates    , ymincs[0]);
    layzDrawVymCurve (this, image + 128, this->ymstates + 1, ymincs[1]);
    layzDrawVymCurve (this, image + 145, this->ymstates + 2, ymincs[2]);

#   if LAYERZ_RASTERIZE()
    *HW_COLOR_LUT = 0x300;
#   endif
}



#define LAYERZ_NBKEYS 4096

static void layerZFillYMtable (u8* ymlinear_)
{
    u16  t;
    u8   last;
    u8*  p = ymlinear_;
    u16* k = SNDYM_g_keys.w;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "layerZFillYMtable", '\n');

    STDfastmset(ymlinear_, -1, LAYERZ_NBKEYS);

    for (t = 0; t < SNDYM_NBKEYS; t++)
    {
        u16 index = *k++;
        p[PCENDIANSWAP16(index)] = (u8) t; 
    }

    last = SNDYM_NBKEYS - 1;

    p = ymlinear_;

    for (t = 0 ; t < LAYERZ_NBKEYS ; t++)
    {
        if (*p != 0xFF)
            last = *p++;
        else
            *p++ = last; 
    }
}





static void layerZActivityColorDots(LayerZ* this, u8* image, s16 index_)
{
    u16 currentcolor;
    u16 vol[4];
    
    LAYERZ_RASTERIZE_COLOR(0x40);

    /* ----------------------------------
    Clear plane
    ----------------------------------*/
    *HW_BLITTER_ADDR_DEST = (u32)image;
    *HW_BLITTER_ENDMASK1  = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1;
    *HW_BLITTER_XSIZE     = LAYERZ_PITCH / 8;
    *HW_BLITTER_YSIZE     = LAYERZ_DISPLAY_H;
    *HW_BLITTER_HOP       = HW_BLITTER_HOP_BIT1;
    *HW_BLITTER_OP        = HW_BLITTER_OP_BIT0;
    *HW_BLITTER_XINC_DEST = 8;
    *HW_BLITTER_YINC_DEST = 8;
    *HW_BLITTER_CTRL2     = 0;

    *HW_BLITTER_CTRL1     = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
    EMULblit();

    /* ----------------------------------
    Draw sample curves
    ----------------------------------*/
    LAYERZ_RASTERIZE_COLOR(0x4);

    LAYZcomputeVolumes ((s8*) g_screens.player.dmabufstart, vol);

    {
        u16 t;
        BLSvoice* voice = g_screens.player.voices;
        u16 mul;
        u16 colorzero = 0;
        s16 xpos [7] = {-1, -1, -1, -1, -1, -1, -1};
        u16* multicolors = this->multicolors[this->flip];
        

        for (t = 0; t < 4; t++)
        {
            BLSprecomputedKey* key0 = voice->keys[0];

            if (key0 != NULL)
            {
                u16 index = ((u8*)key0 - (u8*)g_screens.player.sndtrack->keys) / sizeof(BLSprecomputedKey);
                u8 sampleIndex = key0->sampleIndex;

                if (BLS_IS_BASEKEY(key0) == false)
                {
                    ASSERT(key0->sampleIndex <= g_screens.player.sndtrack->nbKeys);
                    sampleIndex = g_screens.player.sndtrack->keys[sampleIndex].sampleIndex;
                }

                currentcolor = this->keyscolors[g_screens.sampleToSourceSample[sampleIndex] & 7];

                mul = ((u16)vol[t]) >> 9;
                if (mul > 16)
                    mul = 16;

                COLcomputeGradient16Steps4bSTe(&colorzero, &currentcolor, 1, mul, multicolors);

                index = g_screens.player.sndtrack->keysnoteinfo[index];
                index = ((index >> 4) * 12 + (index & 0xF));

                index -= g_screens.sndtrackKeyMin;
                index = LAYERZ_CIRCLE_W * index / (g_screens.sndtrackKeyMax - g_screens.sndtrackKeyMin);
                index += 8;

                if (*multicolors > 0)
                    xpos[t] = index;
            }
            else
            {
                *multicolors = 0;
            }

            voice++;
            multicolors++;
        }

        {
            SmplCurveYMcurveState* ymstate = this->ymstates;

            for (t = 0; t < 3; t++, ymstate++)
            {
                u8 level;


                if (ymstate->type & SmplCurveYMcurveType_SQUARE)
                {
                    level = ymstate->level;

                    currentcolor = this->keyscolors[t + ((t == 2) ? 2 : 0)]; /* 0, 1, 4*/

                    if (level > 0)
                    {
                        u16 x = g_screens.layerzStatic.ymlinear[ymstate->freqsquare];
                        xpos[t + 4] = x + x + x + 6;
                    }
                }
                else if (ymstate->type & SmplCurveYMcurveType_NOISE)
                {
                    level = ymstate->level;

                    currentcolor = SYSvblcount & 1 ? 0xFFF : 0x888;

                    if (level > 0)
                        xpos[t + 4] = ((LAYERZ_PITCH * 2) - (LAYERZ_PITCH - 128)) - (ymstate->freqnoise << 3);
                }
                else
                {
                    level = 0;
                }

                mul = level << 1;
                if (mul > 16)
                    mul = 16;

                COLcomputeGradient16Steps4bSTe(&colorzero, &currentcolor, 1, mul, multicolors);

                multicolors++;
            }
        }

        /* Display sprites */
        {
            u16* p = (u16*)(image + LAYERZ_PITCH * LAYERZ_CURSORS_Y1);

            for (t = 0; t < 7; t++)
            {
                LAYERZ_RASTERIZE_COLOR(1 + t);

                if (xpos[t] >= 0)
                    layerZdrawSprite (this, p, xpos[t]);

                p += (LAYERZ_PITCH / 2) * LAYERZ_CURSORS_H;
            }
        }
    }

    LAYERZ_RASTERIZE_COLOR(0x7);

    layerZSetColors(this, this->inverttop);

    RASnextOpList = &this->rasters2[this->flip];

    LAYERZ_RASTERIZE_COLOR(0x0);
}



static void layerZActivityRawSamples(LayerZ* this, u8* image_, s16 index_)
{
    u16 t;

    LAYERZ_RASTERIZE_COLOR(0x4);

    /* ----------------------------------
    Clear plane
    ----------------------------------*/
    *HW_BLITTER_ADDR_DEST = (u32)image_;
    *HW_BLITTER_ENDMASK1  = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1;
    *HW_BLITTER_XSIZE     = LAYERZ_PITCH / 8;
    *HW_BLITTER_YSIZE     = LAYERZ_DISPLAY_H;
    *HW_BLITTER_HOP       = HW_BLITTER_HOP_BIT1;
    *HW_BLITTER_OP        = HW_BLITTER_OP_BIT0;
    *HW_BLITTER_XINC_DEST = 8;
    *HW_BLITTER_YINC_DEST = 8;
    *HW_BLITTER_CTRL2     = 0;

    *HW_BLITTER_CTRL1     = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
    EMULblit();

    /* ----------------------------------
    Draw sample curves
    ----------------------------------*/

    LAYERZ_RASTERIZE_COLOR(0x300);

    {
        u32 dmabuf = (u32)g_screens.player.dmabufstart;
        u32 image  = (u32)image_;


        /*image += 24 * LAYERZ_PITCH;*/

        *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1;

        *HW_BLITTER_XSIZE       = LAYERZ_DISPLAY_H;
        *HW_BLITTER_OP          = HW_BLITTER_OP_S;
        *HW_BLITTER_HOP         = HW_BLITTER_HOP_SOURCE_AND_HTONE;
        *HW_BLITTER_XINC_SOURCE = 4;
        *HW_BLITTER_YINC_SOURCE = 4;
        *HW_BLITTER_XINC_DEST   = LAYERZ_PITCH;
        *HW_BLITTER_YINC_DEST   = LAYERZ_PITCH;

        /* voice 0 */
        *HW_BLITTER_CTRL2 = 8;
        *HW_BLITTER_ENDMASK1 = PCENDIANSWAP16(0xFF);

        for (t = 0; t < 16; t++)
            HW_BLITTER_HTONE[t] = PCENDIANSWAP16(0xFF);

        *HW_BLITTER_ADDR_SOURCE = dmabuf;

        *HW_BLITTER_YSIZE = 1;
        *HW_BLITTER_ADDR_DEST   = image + 8;
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();

        *HW_BLITTER_YSIZE = 1;
        *HW_BLITTER_ADDR_DEST   = image + 16;
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();

        /* voice 2 */
        *HW_BLITTER_CTRL2 = 0;
        *HW_BLITTER_ENDMASK1 = PCENDIANSWAP16(0xFFFF);

        *HW_BLITTER_ADDR_SOURCE = dmabuf + 2;

        *HW_BLITTER_YSIZE = 1;
        *HW_BLITTER_ADDR_DEST   = image + 48;
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();

        *HW_BLITTER_YSIZE = 1;
        *HW_BLITTER_ADDR_DEST   = image + 56;
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();

        /* voice 1 */
        *HW_BLITTER_CTRL2 = 8;
        *HW_BLITTER_ENDMASK1 = PCENDIANSWAP16(0xFF);

        for (t = 0; t < 16; t++)
            HW_BLITTER_HTONE[t] = PCENDIANSWAP16(0xFF00);

        *HW_BLITTER_ADDR_SOURCE = dmabuf;

        *HW_BLITTER_YSIZE = 1;
        *HW_BLITTER_ADDR_DEST   = image + 32;
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();

        *HW_BLITTER_YSIZE = 1;
        *HW_BLITTER_ADDR_DEST   = image + 40;
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();

        /* voice 3 */
        *HW_BLITTER_CTRL2 = 0;
        *HW_BLITTER_ENDMASK1 = PCENDIANSWAP16(0xFFFF);

        *HW_BLITTER_ADDR_SOURCE = dmabuf + 2;

        *HW_BLITTER_YSIZE = 1;
        *HW_BLITTER_ADDR_DEST   = image + 72;
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();

        *HW_BLITTER_YSIZE = 1;
        *HW_BLITTER_ADDR_DEST   = image + 80;
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();
    }

    LAYERZ_RASTERIZE_COLOR(0x7);

    layerzDrawYM(this, image_);

    this->multicolors[this->flip][0] = this->curvecolorcycling[this->colorindex >> 4];

    this->colorindex += this->colorinc;
    if (this->colorindex >= (ARRAYSIZE(this->curvecolorcycling) << 4))
        this->colorindex = 0;

    layerZSetColors(this, this->inverttop);

    LAYERZ_RASTERIZE_COLOR(0x0);
}




/* 
|PCM                 YM                    |
|1    2    3    4           1    2    3    |
|KKmV KKmV KKmV KKmV SEE NN MFFFLMFFFLMFFFL| 
*/

#define PCM1_K         0
#define PCM1_MASK      2
#define PCM1_VOL       3

#define PCM2_K         5
#define PCM2_MASK      7
#define PCM2_VOL       8

#define PCM3_K         10
#define PCM3_MASK      12
#define PCM3_VOL       13

#define PCM4_K         15
#define PCM4_MASK      17
#define PCM4_VOL       18

#define YM_ENVSHAPE    20
#define YM_ENVFREQ     21
#define YM_NOISEFREQ   24

#define YM1_MIXER      26
#define YM1_FREQ       27
#define YM1_LEVEL      30

#define YM2_MIXER      31
#define YM2_FREQ       32
#define YM2_LEVEL      35

#define YM3_MIXER      36
#define YM3_FREQ       37
#define YM3_LEVEL      40


static char g_hexa[]   = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 
                          'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V'};

static char g_mixer[]  = {' ', LAYERZ_CHAR_MIX_NOISE, LAYERZ_CHAR_MIX_SQR, LAYERZ_CHAR_MIX_SQR_NOISE};

static char g_shape[]  = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'Z', ' ', '<', ' ', 'Z', ' ', '>', ' '};


static bool layerzGetPlayerDesc(LayerZ* this)
{
    CONST BLSvoice* voice = g_screens.player.voices;
    CONST u8* blitzdata = this->lastplayerframe;
    char* str_ = this->str;
    bool print = false;
    u8 freqL;
    u8 freqH;
    u16 t, i;
    u8 voicedata;
    u8 ymvoicedata;
    

    if (blitzdata == NULL)
        return false;

    ymvoicedata = *blitzdata++;

    if (ymvoicedata != 0)
    {
        print = true;

        if (ymvoicedata & 0x80)
        { 
            blitzdata++;    /* mixer has changed */
        }
        ymvoicedata <<= 1;

        /* FREQ */
        if (ymvoicedata & 0x80)
        {
            freqL = *blitzdata++;
            freqH = *blitzdata++;
            str_[YM1_FREQ    ] = g_hexa[freqH & 15];
            str_[YM1_FREQ + 1] = g_hexa[freqL >> 4];
            str_[YM1_FREQ + 2] = g_hexa[freqL & 15];
        }
        ymvoicedata <<= 1;

        if (ymvoicedata & 0x80)
        {
            freqL = *blitzdata++;
            freqH = *blitzdata++;
            str_[YM2_FREQ    ] = g_hexa[freqH & 15];
            str_[YM2_FREQ + 1] = g_hexa[freqL >> 4];
            str_[YM2_FREQ + 2] = g_hexa[freqL & 15];
        }
        ymvoicedata <<= 1;

        if (ymvoicedata & 0x80)
        {
            freqL = *blitzdata++;
            freqH = *blitzdata++;
            str_[YM3_FREQ    ] = g_hexa[freqH & 15];
            str_[YM3_FREQ + 1] = g_hexa[freqL >> 4];
            str_[YM3_FREQ + 2] = g_hexa[freqL & 15];
        }
        ymvoicedata <<= 1;

        /* Env freq */
        if (ymvoicedata & 0x80)
        {
            u8 freqH;

            blitzdata++;
            freqH = *blitzdata++;

            str_[YM_ENVFREQ    ] = g_hexa[freqH >> 4];
            str_[YM_ENVFREQ + 1] = g_hexa[freqH & 15];
        }
        ymvoicedata <<= 1;

        /* LEVEL */
        if (ymvoicedata & 0x80)
        {
            s8 levelA = *blitzdata++;
            if (levelA < 0)
            {
                u8 noisefreq = *blitzdata++;
                ASSERT(noisefreq < 32)
                str_[YM_NOISEFREQ] = g_hexa[noisefreq];
            }
        }
        ymvoicedata <<= 1;

        if (ymvoicedata & 0x80)
        {
            s8 levelB = *blitzdata++;
            if (levelB < 0)
            {
                u8 envshape = *blitzdata++;
                str_[YM_ENVSHAPE] = g_shape[envshape];
            }
        }
        
        ymvoicedata <<= 1;

        if (ymvoicedata & 0x80)
            blitzdata++;
        /*ymvoicedata <<= 1;*/
    }
     
    {
        u8 levelA;
        u8 levelB;
        u8 levelC;
        u8 type;
        bool noise = false;


        levelA = this->ymstates[0].level;
        levelB = this->ymstates[1].level;
        levelC = this->ymstates[2].level;

        type = this->ymstates[0].type;
        str_[YM1_MIXER] = g_mixer[type]; 
        if (type == SmplCurveYMcurveType_NONE)
            levelA = 0;
        else if (type & SmplCurveYMcurveType_NOISE)
            noise = true;

        type = this->ymstates[1].type;
        str_[YM2_MIXER] = g_mixer[type]; 
        if (type == SmplCurveYMcurveType_NONE)
            levelB = 0;
        else if (type & SmplCurveYMcurveType_NOISE)
            noise = true;

        type = this->ymstates[2].type;
        str_[YM3_MIXER] = g_mixer[type]; 
        if (type == SmplCurveYMcurveType_NONE)
            levelC = 0;
        else if (type & SmplCurveYMcurveType_NOISE)
            noise = true;

        if (levelA == 0)
            str_[YM1_FREQ] = str_[YM1_FREQ + 1] = str_[YM1_FREQ + 2] = str_[YM1_LEVEL] = ' ';
        else if (levelA == 16)
            str_[YM1_LEVEL] = '>';
        else
            str_[YM1_LEVEL] = LAYERZ_1STCHAR_VOL + levelA;

        if (levelB == 0)
            str_[YM2_FREQ] = str_[YM2_FREQ + 1] = str_[YM2_FREQ + 2] = str_[YM2_LEVEL] = ' ';
        else if (levelB == 16)
            str_[YM2_LEVEL] = '>';
        else
            str_[YM2_LEVEL] = LAYERZ_1STCHAR_VOL + levelB;

        if (levelC == 0)
            str_[YM3_FREQ] = str_[YM3_FREQ + 1] = str_[YM3_FREQ + 2] = str_[YM3_LEVEL] = ' ';
        else if (levelC == 16)
            str_[YM3_LEVEL] = '>';
        else
            str_[YM3_LEVEL] = LAYERZ_1STCHAR_VOL + levelC;

        if ((levelA != 16) && (levelB != 16) && (levelC != 16))
            str_[YM_ENVFREQ] = str_[YM_ENVFREQ + 1] = str_[YM_ENVSHAPE] = ' ';

        if (noise == false)
        {
            str_[YM_NOISEFREQ    ] = ' ';
            str_[YM_NOISEFREQ + 1] = ' ';
        }
    }

    voicedata = *blitzdata++;

    for (t = 0, i = PCM1_K; t < BLS_NBVOICES; t++, i += 5, voice++)
    {
        bool hasvoicedesc = (voicedata & 0x80) != 0;
        bool voiceactive  = (voicedata & 8) != 0;
        

        /*voice->mute = !voiceon;*/
        if (hasvoicedesc)
        {
            u8 voicedesc = *blitzdata++;
           

            /* implemented the same way than in ASM (by shifting) */
            if ((voicedesc & 0x80) != 0) /* haskeynum */
            {
                u8 keynum = *blitzdata++;
                if (keynum == 0)
                {
                    str_[i] = str_[i + 1] = ' ';
                }
                else
                {
                    str_[i]     = g_hexa[keynum >> 4];
                    str_[i + 1] = g_hexa[keynum & 15];
                    print = true;
                }
            }
            voicedesc <<= 1;

            if ((voicedesc & 0x80) != 0) /* arpegiostart */
            {
                blitzdata += 2;               
                print = true;
            }
            voicedesc <<= 2;

            if (voicedesc & 0x80) /* mask */
            {
                blitzdata += 2;
                print = true;
            }
            voicedesc <<= 1;

            if ((voicedesc & 0x80) != 0)
            {
                print = true;
            }
            voicedesc <<= 1;

            if ((voicedesc & 0x80) != 0) /* volume set */
            {
                blitzdata++;
                print = true;
            }
            voicedesc <<= 1;

            if (voicedesc & 0x80)
            {
                blitzdata += 3;               
            }
            voicedesc <<= 1;

            if (voice->arpeggioState == ArpeggioState_RUNNING)
            {
                str_[i + PCM1_K    ] = '*';
                str_[i + PCM1_K + 1] = '*';
            }
        }

        {
            u8* p = this->font.data + (this->font.charsmap[LAYERZ_1STCHAR_MASK + t] << SYS_FNT_OFFSETSHIFT);
            u8 mask;

            if (voiceactive)
            {
                u8  volume = voice->volume;

                mask = (u8)voice->mask;
                str_[i + PCM1_VOL] = ((8 - volume) << 1) + LAYERZ_1STCHAR_VOL;
            }
            else
            {
                mask = 0xFF;
                str_[i + PCM1_VOL] = ' ';
            }

            if (mask == 0xFF)
            {
                *p++ = 0;
                *p++ = 0;
                *p++ = 0;
                *p++ = 0;
            }
            else
            {
                *p++ = mask;
                *p++ = mask;
                *p++ = mask;
                *p++ = mask;
            }
        }

        voicedata <<= 1;
    }

    return print;
}



static void layerZActivityScoreMatrix(LayerZ* this, u8* image, s16 index_)
{  
    IGNORE_PARAM(image);

#   ifdef __TOS__
    LAYERZ_RASTERIZE_COLOR(0);

    while (*HW_VECTOR_TIMERB != 0);

    LAYERZ_RASTERIZE_COLOR(0x330);
#   endif

    if (layerzGetPlayerDesc(this))
    {
        /*static u16 count = 0;*/
        u8* p = this->framebuffer[0] + this->offsety + 2;
        u8* p2 = this->framebuffer[1] + this->offsety + 2;
        u8* ps = p + (u32)LAYERZ_PITCH * 248UL;

        /*STDutoa(currentstr, count++, 5);*/
        LAYERZ_RASTERIZE_COLOR(0x770);

        this->str[ARRAYSIZE(this->str) - 1] = 0;
        SYSfastPrint(this->str, ps+1, LAYERZ_PITCH, 8, (u32)&this->font);

        LAYERZ_RASTERIZE_COLOR(0x330);

        *HW_BLITTER_ENDMASK1 = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1;
        *HW_BLITTER_XSIZE = (LAYERZ_PITCH / 8) * 8;
        *HW_BLITTER_HOP = HW_BLITTER_HOP_SOURCE;
        *HW_BLITTER_OP = HW_BLITTER_OP_S;
        *HW_BLITTER_XINC_SOURCE = 8;
        *HW_BLITTER_XINC_DEST   = 8;
        *HW_BLITTER_YINC_SOURCE = 8;
        *HW_BLITTER_YINC_DEST   = 8;
        *HW_BLITTER_CTRL2       = 0;

        *HW_BLITTER_ADDR_SOURCE = (u32)ps;
        *HW_BLITTER_ADDR_DEST   = (u32)(p - LAYERZ_PITCH * 8);
        *HW_BLITTER_YSIZE       = 1;
        *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();

        LAYERZ_RASTERIZE_COLOR(0x440);

        *HW_BLITTER_ADDR_SOURCE = (u32)ps;
        *HW_BLITTER_ADDR_DEST   = (u32)(p2 + (u32)LAYERZ_PITCH * 248UL);
        *HW_BLITTER_YSIZE       = 1;
        *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();

        LAYERZ_RASTERIZE_COLOR(0x330);

        *HW_BLITTER_ADDR_SOURCE = (u32)ps;
        *HW_BLITTER_ADDR_DEST   = (u32)(p2 - LAYERZ_PITCH * 8);
        *HW_BLITTER_YSIZE       = 1;
        *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();

        LAYERZ_RASTERIZE_COLOR(0x2);

        if (this->str[PCM1_K] != ' ')
        {
            this->str[PCM1_K] = '|';
            this->str[PCM1_K + 1] = ' ';
        }

        if (this->str[PCM2_K] != ' ')
        {
            this->str[PCM2_K] = '|';
            this->str[PCM2_K + 1] = ' ';
        }

        if (this->str[PCM3_K] != ' ')
        {
            this->str[PCM3_K] = '|';
            this->str[PCM3_K + 1] = ' ';
        }

        if (this->str[PCM4_K] != ' ')
        {
            this->str[PCM4_K] = '|';
            this->str[PCM4_K + 1] = ' ';
        }

        if (this->str[YM1_FREQ] != ' ')
        {
            this->str[YM1_FREQ + 1] = '|';
            this->str[YM1_FREQ] = this->str[YM1_FREQ + 2] = ' ';
        }

        if (this->str[YM2_FREQ] != ' ')
        {
            this->str[YM2_FREQ + 1] = '|';
            this->str[YM2_FREQ] = this->str[YM2_FREQ + 2] = ' ';
        }

        if (this->str[YM3_FREQ] != ' ')
        {
            this->str[YM3_FREQ + 1] = '|';
            this->str[YM3_FREQ] = this->str[YM3_FREQ + 2] = ' ';
        }

        if (this->str[YM_NOISEFREQ] != ' ')
        {
            this->str[YM_NOISEFREQ] = '|';
            this->str[YM_NOISEFREQ + 1] = ' ';
        }

        this->offsety += LAYERZ_PITCH * 8;

        if (this->offsety >= (u32)LAYERZ_PITCH * (u32)(248 + 8))
        {
            this->offsety = LAYERZ_PITCH * 8;
        }

        LAYERZ_RASTERIZE_COLOR(0x0);
    }
}


static void layerZActivityClearScreen(LayerZ* this, u8* image, s16 index_)
{     
    *HW_BLITTER_ADDR_SOURCE = (u32)image; /* no used */

    *HW_BLITTER_ADDR_DEST = (u32)(image + 4 + (index_ << 1));
    *HW_BLITTER_ENDMASK1  = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1;
    *HW_BLITTER_XSIZE     = LAYERZ_PITCH / 8;
    *HW_BLITTER_YSIZE     = LAYERZ_DISPLAY_H;
    *HW_BLITTER_HOP       = HW_BLITTER_HOP_BIT1; 
    *HW_BLITTER_OP        = HW_BLITTER_OP_BIT0; 
    *HW_BLITTER_XINC_DEST = 8;
    *HW_BLITTER_YINC_DEST = 8;
    *HW_BLITTER_CTRL2     = 0;

    *HW_BLITTER_CTRL1     = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
    EMULblit();

    this->clearcount[index_]--;
    if (this->clearcount[index_] == 0) 
    {
        /*printf("%d %d %30s %s\n", index_, SYSvblLcount, "NULL", __FUNCTION__);*/
        this->activities[index_] = NULL;
    }
}


static void layerZActivityFadeColors(LayerZ* this, u8* image, s16 index_)
{
    u16 start = 0xFFF;
    u16 end   = 0;
    u16 color;
    u8  count = this->colorscount[index_ - 2];


    COLcomputeGradient16Steps4bSTe(&end, &start, 1, count, &color);

    this->layercolors[index_] = color;

    count--;
    this->colorscount[index_ - 2] = count;

    if (count == 0)
    {
        /*printf("%d %d %30s %s\n", index_, SYSvblLcount, "NULL", __FUNCTION__);
        printf("%d %d %30s %s\n", index_ - 2 , SYSvblLcount, "layerZActivityClearScreen", __FUNCTION__);*/

        this->activities[index_]     = NULL;
        this->activities[index_ - 2] = layerZActivityClearScreen;
    }
}


static void layerZActivityFadeScoreMatrixColor(LayerZ* this, u8* image, s16 index_)
{
    u16 start = 0;
    u16 end = LAYERZ_SCORE_COLOR_4B;
    u16 color;
    u16 count = this->scorematrixcolorscount++;


    if (count < 16) 
        COLcomputeGradient16Steps4bSTe(&end, &start, 1, count, &color);    
    else if (count < 32)
        color = 0;
    else 
        COLcomputeGradient16Steps4bSTe(&start, &end, 1, count - 32, &color);    

    this->layercolors[1] = color;

    if (this->scorematrixcolorscount >= 48)
    {
        /*printf("%d %d %30s %s\n", index_, SYSvblLcount, "NULL", __FUNCTION__);*/
        this->activities[index_] = NULL;
    }
}


static void layerZActivityDecompressImage(LayerZ* this, u8* image, s16 index_)
{
    u8* bmp    = g_screens.layerzStatic.bmps[this->bmpnum].blitzbmp;
    u16 offset = g_screens.layerzStatic.bmps[this->bmpnum].blitzbmpoffset;


    LAYERZ_RASTERIZE_COLOR(0x77);
    LAYERZ_RASTERIZE_COLOR(0x77);

    image += LAYERZ_PITCH * ((LAYERZ_DISPLAY_H - 200) / 2);

    this->layercolors[index_ + 2] = PCENDIANSWAP16(0xFFF);

    BIT1pUncompress(bmp, bmp + offset, (u32)(image + 4 + (index_ << 1)));
    
    this->decompresscount[index_]--;
    if (this->decompresscount[index_] == 0) 
    {
        this->clearcount[index_] = 2;
        if (this->colorscount[index_] == 0)
        {
            /*printf("%d %d %30s %s\n", index_, SYSvblLcount, "layerZActivityClearScreen", __FUNCTION__);*/
            this->activities[index_] = layerZActivityClearScreen;
        }
        else
        {
            /*printf("%d %d %30s %s\n", index_    , SYSvblLcount, "NULL", __FUNCTION__);
            printf("%d %d %30s %s\n", index_ + 2, SYSvblLcount, "layerZActivityFadeColors", __FUNCTION__);*/

            this->activities[index_]    = NULL;
            this->activities[index_+ 2] = layerZActivityFadeColors;
        }
    }
}


extern u8 samcurve_g_interlaceincs[];
extern u8 samcurve_g_xorfreeareah [];

static void layerZActivitySetDecompressImage(LayerZ* this, u8* image, s16 index_)
{
    this->decompresscount[index_ - 2]--;
    if (this->decompresscount[index_ - 2] == 2)
    {
        this->activities[index_ - 2] = layerZActivityDecompressImage;
        this->activities[index_]     = NULL;

        if (this->activities[LayerZactivities_SCORECOLORS] == NULL)
        {
            /*printf("%d %d %30s %s\n", LayerZactivities_SCORECOLORS, SYSvblLcount, "layerZActivityFadeScoreMatrixColor", __FUNCTION__);*/
            this->activities[LayerZactivities_SCORECOLORS] = layerZActivityFadeScoreMatrixColor;
            this->scorematrixcolorscount = 0;
        }
        else if (this->scorematrixcolorscount > 24)
        {
            this->scorematrixcolorscount = 48 - this->scorematrixcolorscount;
        }
    }
}


static void LayerZManageCommands(LayerZ* this, bool allownavigation_)
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
                BLZ_TRAC_COMMAND_NUM("LAYvoice1_", cmd);
                this->voice = cmd;
            }
            else if (cmd >= 8)
            {
                cmd -= 8;

                BLZ_TRAC_COMMAND_NUM("LAYinterlace", cmd);

                cmd <<= 1;
                this->samrequest.inc1 = samcurve_g_interlaceincs[cmd];
                this->samrequest.inc2 = samcurve_g_interlaceincs[cmd + 1];
            }
            break;

        case BLZ_CMD_VOICE2_CATEGORY >> 4:

            cmd &= BLZ_CMD_COMMAND_MASK;
            if (cmd < 7)
            {
                BLZ_TRAC_COMMAND_NUM("LAYvoice2_", cmd);
                this->voice2 = cmd;
            }
            break;

        case BLZ_CMD_LINE1_CATEGORY >> 4:

            cmd &= BLZ_CMD_COMMAND_MASK;

            RASnextOpList = &this->rasters1;

            switch (cmd)
            {
            case 0:
                BLZ_TRAC_COMMAND("LAYcurveTwins");
                this->activities[LayerZactivities_CURVE] = layerZActivityTwinsCurves;
                this->multinbcolors = 1;
                break;

            case 1:
                BLZ_TRAC_COMMAND("LAYcurveSolo");
                this->activities[LayerZactivities_CURVE] = layerZActivitySoloCurve;
                this->multinbcolors = 1;
                break;

            case 2:
                BLZ_TRAC_COMMAND("LAYcurveSiamese");
                this->activities[LayerZactivities_CURVE] = layerZActivitySiameseCurves;
                this->multinbcolors = 1;
                break;

            case 3:
                BLZ_TRAC_COMMAND("LAYcurveKeys");
                this->activities[LayerZactivities_CURVE] = layerZActivityColorDots;
                this->multinbcolors = 7;
                RASnextOpList = &this->rasters2[this->flip];
                break;

            case 4:
                BLZ_TRAC_COMMAND("LAYcurveRawSamples");
                this->activities[LayerZactivities_CURVE] = layerZActivityRawSamples;
                this->multinbcolors = 1;
                break;

            default:

                cmd -= 5;

                if (cmd < 5)
                {
                    BLZ_TRAC_COMMAND_NUM("LAYcurveSoloFill", cmd);
                    this->xorfreeareah = samcurve_g_xorfreeareah [cmd];
                }
                else if (cmd == 5)
                {
                    BLZ_TRAC_COMMAND("LAYflash");
                    this->backgroundcolor = 0xFFF;
                    this->backgroundinc1  = 0x111;
                    this->backgroundinc2  = 0x888;
                }

                break;
            }

            break;

        case BLZ_CMD_LINE2_CATEGORY >> 4:
        {
            static u8 layerz_ops[] = 
            {
                LayerZop_XOR,        false, 
                LayerZop_OR,         false, 
                LayerZop_TOP,        false,
                LayerZop_TOP,        true,
                LayerZop_TRANPARENT, false
            };

            cmd &= BLZ_CMD_COMMAND_MASK;

            if (cmd < (ARRAYSIZE(layerz_ops) >> 1))
            {
                BLZ_TRAC_COMMAND_NUM("LAYbmpOp", cmd);

                cmd <<= 1;

                this->layersop[1] = layerz_ops[cmd];
                this->inverttop   = layerz_ops[cmd + 1];
            }
            else
            {
                static u16 colorsindexes[] = {0, 16 << 4, 32 << 4, (48 << 4) + 8, 64 << 4};

                cmd -= ARRAYSIZE(layerz_ops) >> 1;

                if (cmd < ARRAYSIZE(colorsindexes))
                {
                    BLZ_TRAC_COMMAND_NUM("LAYcolorCycleStep", cmd);
                    this->colorindex = colorsindexes[cmd];
                }
                else
                {
                    cmd -= ARRAYSIZE(colorsindexes);

                    if (cmd == 0)
                    {
                        BLZ_TRAC_COMMAND("LAYcolorCycleOnOff");  
                        this->colorinc ^= 1;
                    }
                }
            }
        }
            break;
        
        case BLZ_CMD_LINE3_CATEGORY >> 4:

            cmd -= BLZ_CMD_ANTISLASH;

            if (cmd < 8)
            {
                s16 bmpplane = this->bmpplane;

                BLZ_TRAC_COMMAND_NUM("LAYbitmap", cmd);

                /*printf("%d %d %30s %s\n", LayerZactivities_BMP_EXCLUSIVE_P3 + bmpplane, SYSvblLcount, "layerZActivityDecompressImage", __FUNCTION__);*/
                this->activities[LayerZactivities_BMP_CONCURRENT_P3 + bmpplane] = layerZActivitySetDecompressImage;

                this->bmpnum = cmd;
                this->decompresscount[bmpplane] = 4;
                this->colorscount[bmpplane] = 15;

                this->bmpplane ^= 1;
            }
            else 
            {
                static u8 bitmapops [] = {LayerZop_XOR, LayerZop_OR, LayerZop_TOP};

                cmd -= 8;

                if (cmd < ARRAYSIZE(bitmapops))
                {
                    BLZ_TRAC_COMMAND_NUM("LAYbitmapOp", cmd);
                    this->layersop[2] = this->layersop[3] = bitmapops[cmd];
                }
            }

            break;

        default:
            if (allownavigation_)
                if (ScreensManageScreenChoice(BLZ_EP_LAYERZ, cmd))
                    return;
        }
    }
}

#define LAYERZ_SPRITE_CODESIZE 9000

u8* LAZgenSprite(u16* data_, u16 nblines_, u8**sprite, u32 temp)
#ifdef __TOS__
;
#else
{
    u16 shift;
    u16* d = (u16*) sprite[0];


    for (shift = 0; shift < 16; shift++)
    {
        u16  y, offset = 0, line = 0;
        u16* s = data_;

        sprite[shift] = (u8*) d;

        for (y = 0; y < nblines_; y++)
        {
            u16 w0, w1, w2;

            offset = line;

            w0 = PCENDIANSWAP16(s[0]) >> shift;
            w1 = PCENDIANSWAP16(s[1]) >> shift;

            w1 |= PCENDIANSWAP16(s[0]) << (16 - shift);
            w2 = PCENDIANSWAP16(s[1]) << (16 - shift);

            if (w0 != 0)
            {
                *d++ = SPR_OPCODE_MOVE_DATA_X_A0;
                *d++ = PCENDIANSWAP16(w0);
                *d++ = offset;
            }

            offset += 8;

            if (w1 != 0)
            {
                *d++ = SPR_OPCODE_MOVE_DATA_X_A0;
                *d++ = PCENDIANSWAP16(w1);
                *d++ = offset;
            }

            offset += 8;

            if (w2 != 0)
            {
                *d++ = SPR_OPCODE_MOVE_DATA_X_A0;
                *d++ = PCENDIANSWAP16(w2);
                *d++ = offset;
            }

            line += LAYERZ_PITCH;
            s += 2;
        }

        *d++ = CGEN_OPCODE_RTS;
    }

    return (u8*) d;
}
#endif

static void layerZgenSpriteCode (LayerZ* this)
{
    void* temp;
    u8* d;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "layerZgenSpriteCode", '\n');

    temp = MEM_ALLOCTEMP(&sys.allocatorMem, sizeof(u16)*3*32);
    this->spritecode[0] = d = (u8*) MEM_ALLOC(&sys.allocatorMem, LAYERZ_SPRITE_CODESIZE);

    d = LAZgenSprite(g_screens.layerzStatic.blitzsprite, 32, this->spritecode, (u32) temp);
    ASSERT((d - this->spritecode[0]) <= LAYERZ_SPRITE_CODESIZE);

    MEM_FREE(&sys.allocatorMem, temp);
}



void LayerZEnter (FSM* _fsm)
{
    LayerZ* this;
    u32 framebuffersize;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "LayerZEnter", '\n');

    this = g_screens.layerz = MEM_ALLOC_STRUCT(&sys.allocatorMem, LayerZ);
    DEFAULT_CONSTRUCT(this);

    framebuffersize = (u32)LAYERZ_PITCH * (u32)(264 * 2);

    this->framebuffer[0] = (u8*) MEM_ALLOC(&sys.allocatorMem, framebuffersize * 2UL);
    STDfastmset(this->framebuffer[0], 0, framebuffersize * 2UL);
    this->framebuffer[1] = this->framebuffer[0] + framebuffersize;

#   ifdef __TOS__
    layerZInitRasters(this);
#   endif
    
    layerZInitColors(this);

    SmplCurveInitOffset(LAYERZ_2CURVES_H / 2, this->offsets , LAYERZ_PITCH);
    SmplCurveInitOffset(LAYERZ_1CURVE_H  / 2, this->offsets2, LAYERZ_PITCH);

    this->voice2 = 1;
    this->samrequest.inc1 = this->samrequest.inc2 = 6;

    layerzSetSampleDisplay(this);    

    this->activities[LayerZactivities_CURVE] = layerZActivitySoloCurve;
    this->activities[LayerZactivities_SCORE] = layerZActivityScoreMatrix;

    this->layercolors[0] = 0;
    this->layersop[0]    = LayerZop_TOP;

    this->layercolors[1] = LAYERZ_SCORE_COLOR_STE;
    this->layersop[1]    = LayerZop_OR;

    this->layersop[2]    = LayerZop_OR;
    this->layersop[3]    = LayerZop_OR;

    this->offsety = 8 * LAYERZ_PITCH;

    STDmset(this->str, 0x20202020UL, ARRAYSIZE(this->str) - 1);
    this->str[ARRAYSIZE(this->str) - 1] = 0;

    this->str[PCM1_VOL ] = 16;
    this->str[PCM1_MASK] = LAYERZ_1STCHAR_MASK;

    this->str[PCM2_VOL ] = 16;
    this->str[PCM2_MASK] = LAYERZ_1STCHAR_MASK + 1;

    this->str[PCM3_VOL ] = 16;
    this->str[PCM3_MASK] = LAYERZ_1STCHAR_MASK + 2;

    this->str[PCM4_VOL ] = 16;
    this->str[PCM4_MASK] = LAYERZ_1STCHAR_MASK + 3;

    this->multinbcolors = 1;
    this->colorinc      = 1;

    layerZprepareFont (this);

    layerZgenSpriteCode (this);

    BlitZsetVideoMode(HW_VIDEO_MODE_4P, 0, BLITZ_VIDEO_16XTRA_PIXELS);

    RASnextOpList     = &this->rasters1;
    SYSvblroutines[1] = (SYSinterupt)RASvbl;

    LayerZManageCommands(this, false);  /* hack => do this here in sync to avoid complexifying to put it on main thread like it should be */

    TRACsetVideoMode (168);

    SYSwriteVideoBase ((u32) this->framebuffer);

#   ifndef __TOS__
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRAClog(TRAC_LOG_MEMORY,"LayerZ memallocator dump", '\n');
        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif

    FSMgotoNextState(_fsm);
    FSMgotoNextState(&g_stateMachine);
}


void LayerZActivity(FSM* _fsm)
{
    LayerZ* this = g_screens.layerz;
    u8* image = (u8*)this->framebuffer[this->flip];


    IGNORE_PARAM(_fsm);

    STDmcpy2(HW_COLOR_LUT + 1, &this->palette[!this->flip][0][1], 30);

    LAYERZ_RASTERIZE_COLOR(0x440);

    SmplCurveYMgetStates(this->ymstates);

    image += this->offsety;

    {
        layerZActivitiesCall call1, call2;

        
        if (this->activities[LayerZactivities_SCORECOLORS] != NULL)
        {
            this->activities[LayerZactivities_SCORECOLORS](this, image, LayerZactivities_SCORECOLORS);
        }
        else
        {
            u16 color = this->scorecolor[this->scorematrixcolorsindex >> 1];
            this->layercolors[1] = color;

            if (this->scorematrixcolorsindex > 0)
                this->scorematrixcolorsindex--;
        }
            
        call1 = this->activities[LayerZactivities_BMP_EXCLUSIVE_P3];
        call2 = this->activities[LayerZactivities_BMP_EXCLUSIVE_P4];

        if (call1 != NULL)
        {
            call1 (this, image, LayerZactivities_BMP_EXCLUSIVE_P3);

            if (call2 != NULL)
                call2 (this, image, LayerZactivities_BMP_EXCLUSIVE_P4);
        }
        else if (call2 != NULL)
        {
            call2 (this, image, LayerZactivities_BMP_EXCLUSIVE_P4);
        }
        else
        {
            call1 = this->activities[LayerZactivities_CURVE];

            if (call1 != NULL)
                call1 (this, image, LayerZactivities_CURVE);

            call1 = this->activities[LayerZactivities_BMP_CONCURRENT_P3];
            call2 = this->activities[LayerZactivities_BMP_CONCURRENT_P4];

            if (call1 != NULL)
                call1 (this, image, LayerZactivities_BMP_CONCURRENT_P3);

            if (call2 != NULL)
                call2 (this, image, LayerZactivities_BMP_CONCURRENT_P4);

            if ((call1 == NULL) && (call2 == NULL))
            {
                this->activities[LayerZactivities_SCORE] (this, image, LayerZactivities_SCORE);
            }
        }
    }

    /* Background color */
    {
        u16 color = this->backgroundcolor;

        if (color != 0)
        {
            if (color & 0x888)
            {
                color &= 0x777;
            }
            else
            {
                color -= this->backgroundinc1;
                color |= this->backgroundinc2;
            }
        }

        this->backgroundcolor = color;
        aBLZbackground = PCENDIANSWAP16(color);
    }

    LAYERZ_RASTERIZE_COLOR(0x1);

    LayerZManageCommands(this, true);   

    SYSwriteVideoBase((u32) image);

    this->lastplayerframe = g_screens.player.blizcurrent;

    this->flip ^= 1;

#   ifndef __TOS__
    {
        const int BASEY = 40;

        EMULfbExStart(HW_VIDEO_MODE_4P, 80, 40, 80 + LAYERZ_PITCH * 2 - 1, 40 + LAYERZ_DISPLAY_H - 1, LAYERZ_PITCH, 0);

        {
            u16 y = BASEY;
            u16 t;
            u32 cycles = EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, y);
            u16 i;

            for (i = 1 ; i < 16 ; i++)
                EMULfbExSetColor(cycles, i, PCENDIANSWAP16(this->palette[this->flip][0][i]));

            y += LAYERZ_CURSORS_Y1;

            for (t = 1 ; t < this->multinbcolors ; t++)
            {
                y += LAYERZ_CURSORS_H;

                cycles = EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, y);

                for (i = 1 ; i < 16 ; i++)
                    EMULfbExSetColor(cycles, i, PCENDIANSWAP16(this->palette[this->flip][t][i]));               
            }
        }

        EMULfbExEnd();
    }
#   endif

    LAYERZ_RASTERIZE_COLOR(0x0);
}


void LayerZBacktask(FSM* _fsm)
{
    LayerZ* this = g_screens.layerz;

    IGNORE_PARAM(_fsm);

    if ((this->samdisplay.inc1  != this->samrequest.inc1) ||
        (this->samdisplay.inc2  != this->samrequest.inc2))
    {
        layerzSetSampleDisplay(this);
    }

    /*STDstop2300();*/
}


void LayerZExit(FSM* _fsm)
{
    LayerZ* this = g_screens.layerz;

    IGNORE_PARAM(_fsm);

    SYSvblroutines[1] = RASvbldonothing;
    BlitZturnOffDisplay();

    MEM_FREE(&sys.allocatorMem, this->font.data);
    MEM_FREE(&sys.allocatorMem, this->framebuffer[0]);
    MEM_FREE(&sys.allocatorMem, this->spritecode[0]);
    MEM_FREE(&sys.allocatorMem, this);

    this = g_screens.layerz = NULL;

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


void LayerZStaticInit(FSM* _fsm)
{
    g_screens.layerzStatic.ymlinear = (u8*) MEM_ALLOC(&sys.allocatorCoreMem, LAYERZ_NBKEYS);

    layerZFillYMtable (g_screens.layerzStatic.ymlinear);

    /* sprite data is loaded in sshade static init */

    FSMgotoNextState(_fsm);
}
