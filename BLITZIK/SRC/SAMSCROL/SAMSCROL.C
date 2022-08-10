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

#include "FX\TEXT\TEXT.H"

#include "BLITZIK\SRC\SCREENS.H"

#include "DEMOSDK\PC\EMUL.H"


#define SAMSCROL_DEV() 0

#ifdef __TOS__
#define SAMSCROL_DISPLAYW   224
#else
#define SAMSCROL_DISPLAYW   224
#endif

#define SAMSCROL_RASTERIZE() 0

#define SAMSCROL_GENCODE_BUFFERSIZE 70000UL

#define SAMSCROL_PITCH      480
#define SAMSCROL_DISPLAYH   198
#define SAMSCROL_SPEED      6
#define SAMSCROL_CURVE1_Y   64
#define SAMSCROL_CURVE2_Y   134 /*(SAMSCROL_DISPLAYH - 64)*/

#define SAMSCROL_COLOR_Y            60
#define SAMSCROL_COLOR_H            5
#define SAMSCROL_CURVEBORDEROFFSET  120
#define SAMSCROL_CYCLINGOFFSET      240

#define SAMSCROL_OVERSCAN_1STRANGE_CYCLES   360
#define SAMSCROL_OVERSCAN_2NDRANGE_CYCLES   104

#define SAMSCROL_SAM1_VOFFSET (SAMSCROL_PITCH*(SAMSCROL_CURVE1_Y - SAMSCROL_DISPLAYH/2))
#define SAMSCROL_SAM2_VOFFSET (SAMSCROL_PITCH*(SAMSCROL_CURVE2_Y - SAMSCROL_DISPLAYH/2))

#define SAMSCROL_FONT_CODESIZE 5900

ASMIMPORT SamScrollASMimport BMscImportTable;

static u32 SamScroll_g_textOffsets[10] = 
{
    SAMSCROL_PITCH * 2UL,
    SAMSCROL_PITCH * 10UL,
    SAMSCROL_PITCH * 20UL,
    SAMSCROL_PITCH * 16UL,
    SAMSCROL_PITCH * 6UL,

    SAMSCROL_PITCH * 164UL,
    SAMSCROL_PITCH * 158UL,
    SAMSCROL_PITCH * 152UL,
    SAMSCROL_PITCH * 146UL,
    SAMSCROL_PITCH * 140UL
};

static u16 SamScroll_g_masks[]   = {0, 1, 3, 0x11, 0x33, 0xFF, 0x5555, 0xFFFF};
static u8  SamScroll_g_freqmax[] = {0, 3, 5, 7, 9};

static void samScrollPatchSampleAccess (SamScroll* this, void* _base, u16 off1, u16 off2, u16 inc1, u16 inc2)
{
    u16 sampleoffsets[6];
    u16 y;

    
    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "SamScrollPatchSampleAccess", '\n');

    sampleoffsets[0] = off1;
    sampleoffsets[1] = off2;
    sampleoffsets[2] = off1 + 4;
    sampleoffsets[3] = off2 + 4;
    sampleoffsets[4] = off1 + 8;
    sampleoffsets[5] = off2 + 8;

    for (y = 0 ; y < ARRAYSIZE(this->curveroutineoffsets) ; y++)
    {
        u8* p = (u8*) _base + this->curveroutineoffsets[y];


        /* *HW_COLOR_LUT ^= 7; */

        if (y & 1)
        {
            CGENpatchWords(p, this->sampleoffsetsroutine2, sampleoffsets + 3, ARRAYSIZE(this->sampleoffsetsroutine2));

            sampleoffsets[0] += inc1;
            sampleoffsets[2] += inc1;
            sampleoffsets[4] += inc1;
            sampleoffsets[1] += inc2;
            sampleoffsets[3] += inc2;
            sampleoffsets[5] += inc2;
        }
        else
        {
            CGENpatchWords(p, this->sampleoffsetsroutine1, sampleoffsets, ARRAYSIZE(this->sampleoffsetsroutine1));
        }
    }
}


static void samScrollGenerateCodeOverscan(SamScroll* this)
{
    CGENdesc* code = BMscImportTable.opcodes;
    u8*       output;
    u32       cycles = 0;
    u16       y, x;
    u16       nblines = 0;
    u16       result;
              
    u16       pixeloffsetsroutine1  [9];
    u16       pixeloffsetsroutine2  [9];
    u16       colorsoffsets         [4];
              
    u16*      colorsroutines;
    u16       colorsroutinesindex = 0;

    static u16 colorsregs1[]  = {0x8242, 0x8244, 0x8248, 0x824C};
    static u16 colorsregs2[]  = {0x8242, 0x8244, 0x8250, 0x8254};
  /*static u16 colorsregs1[]  = {0x8240, 0x8240, 0x8240, 0x8240};
    static u16 colorsregs2[]  = {0x8240, 0x8240, 0x8240, 0x8240};*/
    static s16 pixeloffsets[] =  
    { 
        SAMSCROL_SAM1_VOFFSET, SAMSCROL_SAM1_VOFFSET-4, SAMSCROL_SAM1_VOFFSET-SAMSCROL_CYCLINGOFFSET-4,
        SAMSCROL_SAM2_VOFFSET, SAMSCROL_SAM2_VOFFSET-4, SAMSCROL_SAM2_VOFFSET-SAMSCROL_CYCLINGOFFSET-4,
        SAMSCROL_SAM1_VOFFSET, SAMSCROL_SAM1_VOFFSET-4, SAMSCROL_SAM1_VOFFSET-SAMSCROL_CYCLINGOFFSET-4,
        SAMSCROL_SAM2_VOFFSET, SAMSCROL_SAM2_VOFFSET-4, SAMSCROL_SAM2_VOFFSET-SAMSCROL_CYCLINGOFFSET-4
    };

    /*static s16 pixeloffsets[] =  
    { 
    SAMSCROL_SAM1_VOFFSET, SAMSCROL_SAM1_VOFFSET, SAMSCROL_SAM1_VOFFSET,
    SAMSCROL_SAM2_VOFFSET, SAMSCROL_SAM2_VOFFSET, SAMSCROL_SAM2_VOFFSET,
    SAMSCROL_SAM1_VOFFSET, SAMSCROL_SAM1_VOFFSET, SAMSCROL_SAM1_VOFFSET,
    SAMSCROL_SAM2_VOFFSET, SAMSCROL_SAM2_VOFFSET, SAMSCROL_SAM2_VOFFSET,
    };*/

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "SamScrollGenerateCodeOverscan", '\n');

    colorsroutines = MEM_ALLOCTEMP(&sys.allocatorMem, SAMSCROL_NBCOLORS_CHANGE * sizeof(u16));

    output = this->overscanroutine[0];

    result = CGENfindWords(&code[BSScOp_Sam1Sam2Sam1], this->sampleoffsetsroutine1, 0xBBA);
    ASSERT(result == ARRAYSIZE(this->sampleoffsetsroutine1));
    
    result = CGENfindWords(&code[BSScOp_Sam2Sam1Sam2], this->sampleoffsetsroutine2, 0xBBA);
    ASSERT(result == ARRAYSIZE(this->sampleoffsetsroutine2));
    
    result = CGENfindWords(&code[BSScOp_Sam1Sam2Sam1], pixeloffsetsroutine1, 0xABA);
    ASSERT(result == ARRAYSIZE(pixeloffsetsroutine1));
    
    result = CGENfindWords(&code[BSScOp_Sam2Sam1Sam2], pixeloffsetsroutine2, 0xABA);
    ASSERT(result == ARRAYSIZE(pixeloffsetsroutine2));
    
    result = CGENfindWords(&code[BSScOp_RightBorderColored], colorsoffsets, 0x8242);
    ASSERT(result == ARRAYSIZE(colorsoffsets));

    IGNORE_PARAM(result);

    CGENgenerateSimple(&code[BSScOp_Begin], output);

    TRAClogFrameNum(TRAC_LOG_FLOW);

    /* clear lines */
    for (y = 0 ; y < (SAMSCROL_DISPLAYH - 12) ; y += 12)
    {
        CGENgenerateSimple(&code[BSScOp_LeftBorderRemoval], output);
        cycles = 0;
        for (x = 10 ; x > 0 ; x--)
        {
            CGENgenerate(&code[BSScOp_Clear], cycles, output);
        }
        CGENaddNops(cycles, SAMSCROL_OVERSCAN_1STRANGE_CYCLES, output);

        CGENgenerateSimple(&code[BSScOp_RightBorderRemoval], output);
        cycles = 0;
        CGENgenerate(&code[BSScOp_Clear], cycles, output);
        CGENgenerate(&code[BSScOp_Clear], cycles, output);
        CGENaddNops(cycles, SAMSCROL_OVERSCAN_2NDRANGE_CYCLES, output);   

        ASSERT (nblines < SAMSCROL_COLOR_Y);    /* else needs color change instead of clear */
        nblines++;
    }

    /* remaining clear lines */
    CGENgenerateSimple(&code[BSScOp_LeftBorderRemoval], output);
    y = SAMSCROL_DISPLAYH - y;
    ASSERT(y < 10);
    cycles = 0;

    TRAClogNumber10(TRAC_LOG_SPECIFIC, "clear: ", y, 4);

    for (; y > 0 ; y--)
    {        
        CGENgenerate(&code[BSScOp_Clear], cycles, output);   
    }
    CGENaddNops(cycles, SAMSCROL_OVERSCAN_1STRANGE_CYCLES, output);
    CGENgenerateSimple(&code[BSScOp_RightBorderRemoval], output);
    CGENaddNops(0, SAMSCROL_OVERSCAN_2NDRANGE_CYCLES, output);
    nblines++;

    TRAClogFrameNum(TRAC_LOG_FLOW);

    /* load regs */
    CGENgenerateSimple(&code[BSScOp_LeftBorderRemoval], output);
    cycles = 0;
    CGENgenerate(&code[BSScOp_ReloadReg], cycles, output);   
    CGENaddNops(cycles, SAMSCROL_OVERSCAN_1STRANGE_CYCLES, output);

    CGENgenerateSimple(&code[BSScOp_RightBorderRemoval], output);
    CGENaddNops(0, SAMSCROL_OVERSCAN_2NDRANGE_CYCLES, output);
    nblines++;

    TRAClogFrameNum(TRAC_LOG_FLOW);
    
    /* curves display */
    x = 0;

    {
        u8* cachecode[2];
        u16 sizecode[2][2];

        for (y = 0; y < 2; y++)
        {
            u8* p0 = output;
            u8* p1;
            
            cachecode[y] = output;

            CGENgenerateSimple(&code[BSScOp_LeftBorderRemoval], output);

            sizecode[0][y] = output - p0;
            p1 = output;

            this->curveroutineoffsets[y] = (u16)((u8*)output - this->overscanroutine[0]);

            cycles = 0;
            if (y & 1)
            {
                void* patch = output;
                CGENgenerate(&code[BSScOp_Sam2Sam1Sam2], cycles, output);
                CGENpatchWords(patch, pixeloffsetsroutine2, &pixeloffsets[3], ARRAYSIZE(pixeloffsetsroutine2));
            }
            else
            {
                void* patch = output;
                CGENgenerate(&code[BSScOp_Sam1Sam2Sam1], cycles, output);
                CGENpatchWords(patch, pixeloffsetsroutine1, pixeloffsets, ARRAYSIZE(pixeloffsetsroutine1));
            }
            CGENaddNops(cycles, SAMSCROL_OVERSCAN_1STRANGE_CYCLES, output);

            CGENgenerateSimple(&code[BSScOp_RightBorderRemoval], output);

            sizecode[1][y] = output - p1;

            CGENaddNops(0, SAMSCROL_OVERSCAN_2NDRANGE_CYCLES, output);

            x++;
            if (x >= SAMSCROL_COLOR_H)
            {
                x = 0;
            }

            nblines++;
        }

        TRAClogFrameNum(TRAC_LOG_FLOW);

        {
            for (y = 2; y < ARRAYSIZE(this->curveroutineoffsets); y++)
            {
                u16 select = y & 1;

                STDmcpy2(output, cachecode[select], sizecode[0][select] + sizecode[1][select]);
                output += sizecode[0][select];

                this->curveroutineoffsets[y] = (u16)((u8*)output - this->overscanroutine[0]);
                output += sizecode[1][select];

                cycles = 0;
                if ((nblines >= SAMSCROL_COLOR_Y) && (x == 0) && (colorsroutinesindex < SAMSCROL_NBCOLORS_CHANGE))
                {
                    u8* patch = output;

                    colorsroutines[colorsroutinesindex++] = (u16)((u8*)output - this->overscanroutine[0]);
                    ASSERT(colorsroutinesindex <= SAMSCROL_NBCOLORS_CHANGE);

                    CGENgenerate(&code[BSScOp_RightBorderColored], cycles, output);
                    CGENpatchWords(patch, colorsoffsets, colorsregs1, ARRAYSIZE(colorsoffsets));
                }
                CGENaddNops(cycles, SAMSCROL_OVERSCAN_2NDRANGE_CYCLES, output);

                x++;
                if (x >= SAMSCROL_COLOR_H)
                {
                    x = 0;
                }

                nblines++;
            }
        }
    }

    TRAClogFrameNum(TRAC_LOG_FLOW);

    /* end curves clear offsets */
    CGENgenerateSimple (&code[BSScOp_LeftBorderRemoval], output);
    
    cycles = 0;
    CGENgenerate (&code[BSScOp_FillCurvesOffsetInit], cycles, output);
    for (x = 0 ; x < 32; x++)
    {
        CGENgenerate (&code[BSScOp_FillCurvesOffset], cycles, output);
    }
    CGENaddNops  (cycles, SAMSCROL_OVERSCAN_1STRANGE_CYCLES,   output);
    
    CGENgenerateSimple (&code[BSScOp_RightBorderRemoval], output);
    CGENaddNops  (0, SAMSCROL_OVERSCAN_2NDRANGE_CYCLES     ,   output);
    nblines++;    

    ASSERT(nblines <= SAMSCROL_DISPLAYH);
    TRAClogNumber10(TRAC_LOG_SPECIFIC, "overscan lastline", nblines, 4);

    TRAClogFrameNum(TRAC_LOG_FLOW);

    /* empty lines */
    for (y = nblines ; y < SAMSCROL_DISPLAYH ; y++)
    {
        CGENgenerateSimple (&code[BSScOp_LeftBorderRemoval] , output);
        CGENaddNops  (0, SAMSCROL_OVERSCAN_1STRANGE_CYCLES  , output);
        CGENgenerateSimple (&code[BSScOp_RightBorderRemoval], output);
        CGENaddNops  (0, SAMSCROL_OVERSCAN_2NDRANGE_CYCLES  , output);
    }

    CGENgenerateSimple(&code[BSScOp_End], output);

    this->overscanroutine[1] = (u8*)output;

    {
        u32 size = this->overscanroutine[1] - this->overscanroutine[0];

        u8 channelAoff = this->curveselect.channelAoff;
        u8 channelBoff = this->curveselect.channelBoff;


        TRAClogNumber(TRAC_LOG_SPECIFIC, "gencodelendiv2", size, 6);
        ASSERT(size <= (SAMSCROL_GENCODE_BUFFERSIZE/2UL));

        STDmcpy2(this->overscanroutine[1], this->overscanroutine[0], size);
        size <<= 1;

        TRAClogFrameNum(TRAC_LOG_FLOW);

        for (x = 0 ; x < colorsroutinesindex ; x++)
        {
            u8* p = colorsroutines[x] + this->overscanroutine[1];
            CGENpatchWords(p, colorsoffsets, colorsregs2, ARRAYSIZE(colorsoffsets));
        }

        samScrollPatchSampleAccess(this, this->overscanroutine[0], channelAoff, channelBoff, 12, 12);
        samScrollPatchSampleAccess(this, this->overscanroutine[1], channelAoff, channelBoff, 12, 12);

        STDmcpy2(&this->curvedisplay, &this->curveselect, sizeof(this->curveselect));

        MEM_FREE(&sys.allocatorMem, colorsroutines);
    }
}


static void samScrollGenerateCodeClearCurves(SamScroll* this)
{
    CGENdesc* code = BMscImportTable.opcodes;
    u8*       output;
    s16       t;
    u16       offsets[2];
    u16       result;
 

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "SamScrollGenerateCodeClearCurves", '\n');

    output = this->clearroutine;

    CGENgenerateSimple(&code[BSScOp_ClearCurve1], output);

    result = CGENfindWords(&code[BSScOp_ClearCurve2], offsets, 0x007A);
    ASSERT(result == ARRAYSIZE(offsets));

    for (t = -16 ; t < SAMSCROL_NBSAMPOINTS ; t++)
    {
        s16  offset = t >> 1;
        u8*  p = (u8*) output;


        CGENgenerateSimple(&code[BSScOp_ClearCurve2], output);
        
        ASSERT(offset >= -128);
        ASSERT(offset <   128);

        offset &= 0x00F8;   /* new opcode */

        *(u16*)(p + offsets[0]) = offset;
        *(u16*)(p + offsets[1]) = offset;
    }

    CGENgenerateSimple(&code[BSScOp_ClearCurve3], output);
}


static u16* samScrollGenFontCode(u16* code_, u16 offset_, u16 opcode_)
{
    *code_++ = opcode_;
    *code_++ = offset_;

    *code_++ = opcode_;
    *code_++ = offset_ - SAMSCROL_CYCLINGOFFSET;

    *code_++ = opcode_;
    *code_++ = offset_ + SAMSCROL_PITCH;

    *code_++ = opcode_;
    *code_++ = offset_ + SAMSCROL_PITCH - SAMSCROL_CYCLINGOFFSET;

    *code_++ = opcode_;
    *code_++ = offset_ + SAMSCROL_PITCH * 2;

    *code_++ = opcode_;
    *code_++ = offset_ + SAMSCROL_PITCH * 2 - SAMSCROL_CYCLINGOFFSET;

    return code_;
}


static void SamScrollManageCommands(SamScroll* this, bool allownavigation_)
{
    while (BLZ_COMMAND_AVAILABLE)
    {
        u8 cmd = BLZ_CURRENT_COMMAND;
        u8 category = cmd & BLZ_CMD_CATEGORY_MASK;


        BLZ_ITERATE_COMMAND;

        switch (category >> 4)
        {
        case BLZ_CMD_VOICE1_CATEGORY >> 4:

            cmd -= BLZ_CMD_VOICE1_1;

            if (cmd < 4)
            {
                BLZ_TRAC_COMMAND_NUM("SCRvoice1_", cmd);
                this->curveselect.channelAoff = cmd;
            }
            else if (cmd == 4)
            {
                BLZ_TRAC_COMMAND("SCRwithLevel0");
                this->no0 = true;
            }
            else if (cmd == 5)
            {
                BLZ_TRAC_COMMAND("SCRNoLevel0");
                this->no0 = false;
            }

            break;

        case BLZ_CMD_VOICE2_CATEGORY >> 4:
            
            cmd -= BLZ_CMD_VOICE2_1;

            if (cmd < 4)
            {
                BLZ_TRAC_COMMAND_NUM("SCRvoice2_", cmd);
                this->curveselect.channelBoff = cmd;
            }
            
            break;

        case BLZ_CMD_LINE1_CATEGORY >> 4:

            if (cmd <= BLZ_CMD_P)
            {
                BLZ_TRAC_COMMAND_NUM("SCRdrawText", cmd - BLZ_CMD_Q);
                this->drawtext = cmd - BLZ_CMD_Q + 1;
            }

            break;

        case BLZ_CMD_LINE2_CATEGORY >> 4:

            cmd &= BLZ_CMD_COMMAND_MASK;

            if (cmd < ARRAYSIZE(SamScroll_g_freqmax))
            {
                BLZ_TRAC_COMMAND_NUM("SCRfreq", cmd);
                this->freqmax = SamScroll_g_freqmax[cmd];
            }
            else 
            {
                static u16 colorindexes[] = { 0,0,  7,7 * 7 * SAMSCROL_NBCOLORS_CHANGE,  15,15 * 7 * SAMSCROL_NBCOLORS_CHANGE };

                cmd -= ARRAYSIZE(SamScroll_g_freqmax);               

                if (cmd < (ARRAYSIZE(colorindexes) >> 1))
                {
                    BLZ_TRAC_COMMAND_NUM("SCRcolorCycleStep", cmd);
                    cmd <<= 1;
                    this->colorindex  = colorindexes[cmd];
                    this->colorindex2 = colorindexes[cmd+1];
                }
                else if (cmd == 3)
                {
                    BLZ_TRAC_COMMAND("SCRcolorCycleStop");
                    this->colorinc  = 0;
                    this->colorinc2 = 0;
                }
                else if (cmd == 4)
                {
                    BLZ_TRAC_COMMAND("SCRcolorCycleRun");
                    this->colorinc  = 1;
                    this->colorinc2 = 7 * SAMSCROL_NBCOLORS_CHANGE;
                }

                if (((this->colorindex >= 15) && (this->colorinc ==  1)) ||
                    ((this->colorindex <= 0 ) && (this->colorinc == -1)))
                {
                    this->colorinc  = -this->colorinc;
                    this->colorinc2 = -this->colorinc2;
                }
            }

            break;

        case BLZ_CMD_LINE3_CATEGORY >> 4:
        
            cmd &= BLZ_CMD_COMMAND_MASK;

            if (cmd < ARRAYSIZE(SamScroll_g_masks))
            {
                BLZ_TRAC_COMMAND_NUM("SCRcurvesFX", cmd);
                this->mask = SamScroll_g_masks[cmd];
            }

#           if defined(__TOS__) && SAMSCROL_DEV()
            else if (cmd == ARRAYSIZE(SamScroll_g_masks)
            {
                if (this->waitloop > 0)
                    this->waitloop--;
                TRAClogNumber10("waitLoop", this->waitloop, 4);
            }
            else if (cmd == (ARRAYSIZE(SamScroll_g_masks) + 1))
            {
                if (this->waitloop < 300)
                    this->waitloop++;
                TRAClogNumber10("waitLoop", this->waitloop, 4);
            }
#           endif

            break;

        default:
            if (allownavigation_)
                if (ScreensManageScreenChoice(BLZ_EP_SAM_SCROLL, cmd))
                    return;
        }
    }
}


static void samScrollPrecomputeTables(SamScroll* this)
{
    u16 t;
    s16 off;
    s16* p;

#       ifdef __TOS__
    /* test overscan */
    /*        u8* p   = this->framebuf;

    for (t = 0; t < SAMSCROL_DISPLAYH; t++)
    {
    STDmset (p, 0xAAAAAAAAUL, 64);
    p += SAMSCROL_PITCH;
    }*/
#       endif

    p = this->xoffsetstable;

    for (t = 0; t < ARRAYSIZE(this->xoffsetstable) ; t += 16)
    {
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;

        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;

        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;

        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 8;
    }

    this->yoffsetstable   [0] = 0;
    this->yoffsetstable2_h[0] = (s16) (-65 * SAMSCROL_PITCH);
    this->yoffsetstable2_l[0] = (s16) (+65 * SAMSCROL_PITCH);

    off = 0;
    for (t = 1; t < 64; t++)
    {
        this->yoffsetstable2_h [t] = off;
        this->yoffsetstable2_l [t] = off;
        off += SAMSCROL_PITCH;
        this->yoffsetstable [t] = off;
    }

    off = (s16)(-64 * SAMSCROL_PITCH);
    for (t = 64; t < 128; t++)
    {
        this->yoffsetstable    [t] = off;
        this->yoffsetstable2_l [t] = off;
        this->yoffsetstable2_h [t] = off;
        off += SAMSCROL_PITCH;
    }
}



static void samscrolAssignColors(SamScrollStatic* _init, u16 _index, u16 _c1, u16 _c2, u16 _cc)
{
    u16* c0 = _init->color[_index][0];
    u16* c1 = _init->color[_index][1];


    *c0++ = 0;              /* 0000 */
    *c0++ = _c2;            /* 0001 */
    *c0++ = _c2;            /* 0010 */
    *c0++ = _c1;            /* 0011 */
    *c0++ = 0;              /* 0100 */
    *c0++ = _c2;            /* 0101 */
    *c0++ = _c2;            /* 0110 */
    *c0++ = _c1;            /* 0111 */
    *c0++ = _cc;            /* 1000 */ 
    *c0++ = _cc;            /* 1001 */
    *c0++ = _cc;            /* 1010 */
    *c0++ = _cc;            /* 1011 */
    *c0++ = _cc;            /* 1100 */
    *c0++ = _cc;            /* 1101 */
    *c0++ = _cc;            /* 1110 */
    *c0++ = _cc;            /* 1111 */

    *c1++ = 0;              /* 0000 */
    *c1++ = _c2;            /* 0001 */
    *c1++ = _c2;            /* 0010 */
    *c1++ = _c1;            /* 0011 */
    *c1++ = _cc;            /* 0100 */
    *c1++ = _cc;            /* 0101 */
    *c1++ = _cc;            /* 0110 */
    *c1++ = _cc;            /* 0111 */
    *c1++ = 0;              /* 1000 */ 
    *c1++ = _c2;            /* 1001 */
    *c1++ = _c2;            /* 1010 */
    *c1++ = _c1;            /* 1011 */
    *c1++ = _cc;            /* 1100 */
    *c1++ = _cc;            /* 1101 */
    *c1++ = _cc;            /* 1110 */
    *c1++ = _cc;            /* 1111 */
}



static void samscrolCreateColors(SamScrollStatic* _init)
{
    u16  c1_1[2] = { 0x55 , 0xAA };
    u16  c1_2[2] = { 0x606, 0xA8A };
    
    u16  c2_1[2] = { 0x050 , 0x0A0 };
    u16  c2_2[2] = { 0x660 , 0xAA0 };

    /*static u16 curve1[3] = {0x081, 0x023, 0x04E};
    static u16 curve2[3] = {0x888, 0x333, 0xEEE};
    static u16 curve3[3] = {0x180, 0x320, 0xE40};*/

    u16  c1[2], c2[2], c[2];
    u8   i, g;
    u16* p = _init->rasterscolors;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "samscrolCreateColors", '\n');

    for (g = 0; g < 16; g++)
    {
        COLcomputeGradient16Steps(c1_1, c2_1, 2, g, c1);
        COLcomputeGradient16Steps(c1_2, c2_2, 2, g, c2);

        samscrolAssignColors(_init, g, PCENDIANSWAP16(c1[0]), PCENDIANSWAP16(c1[1]), PCENDIANSWAP16(0xFFF));

        for (i = 0; i < SAMSCROL_NBCOLORS_CHANGE; i++)
        {
            STATIC_ASSERT((SAMSCROL_NBCOLORS_CHANGE - 1) == 16); /* else use COLcomputeGradientStep instead of COLcomputeGradient16Steps */
            COLcomputeGradient16Steps(c1, c2, 2, i, c);

#           ifndef __TOS__
            c[0] = PCENDIANSWAP16(c[0]);
            c[1] = PCENDIANSWAP16(c[1]);
#           endif

            /* bit 0 or 1 => c[1] / bit 0 and 1 => c[0] */
            *p++ = c[1];            /* 001 */
            *p++ = c[1];            /* 010 */
            *p++ = c[0];            /* 011 */
            *p++ = 0;               /* 100 */
            *p++ = c[1];            /* 101 */
            *p++ = c[1];            /* 110 */
            *p++ = c[0];            /* 111 */
        }
    }
}


void SamScrollEnter (FSM* _fsm)
{
    SamScroll* this;

    IGNORE_PARAM(_fsm);


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "SamScrollEnter", '\n');

    this = g_screens.samscroll = MEM_ALLOC_STRUCT (&sys.allocatorMem, SamScroll);
    DEFAULT_CONSTRUCT(this);

    STDmset (HW_COLOR_LUT, 0x0UL, 32);

    this->framebuf  = (u8*) MEM_ALLOC(&sys.allocatorMem, (u32)SAMSCROL_PITCH * (u32)(SAMSCROL_DISPLAYH + 4));
    
    this->clearroutine = MEM_ALLOC(&sys.allocatorMem, 
        BMscImportTable.opcodes[BSScOp_ClearCurve1].opcodelen_div2 * 2 +
        BMscImportTable.opcodes[BSScOp_ClearCurve2].opcodelen_div2 * 2 * (SAMSCROL_NBSAMPOINTS + 16) +
        BMscImportTable.opcodes[BSScOp_ClearCurve3].opcodelen_div2 * 2
    );

    this->empty = MEM_ALLOC(&sys.allocatorMem, SAMSCROL_PITCH*2);

    this->clearoffsets[0] = (u8*) MEM_ALLOC(&sys.allocatorMem, (SAMSCROL_NBSAMPOINTS + 16) * 2 * sizeof(s16) * 2);
    this->clearoffsets[1] = (u8*)this->clearoffsets[0] + ((SAMSCROL_NBSAMPOINTS + 16) * 2 * sizeof(s16));

    STDfastmset (this->clearoffsets[0], 0UL, (SAMSCROL_NBSAMPOINTS + 16) * 2 * sizeof(s16) * 2);
    STDfastmset (this->empty, 0, SAMSCROL_PITCH*2);
    STDfastmset (this->framebuf, 0, (u32)SAMSCROL_PITCH * (u32)(SAMSCROL_DISPLAYH + 4));

    this->curveselect.channelAoff = 0;
    this->curveselect.channelBoff = 2;

    BlitZsetVideoMode(HW_VIDEO_MODE_4P, (SAMSCROL_PITCH - SAMSCROL_DISPLAYW) >> 1, BLITZ_VIDEO_NOXTRA_PIXEL);

    this->overscanroutine[0] = MEM_ALLOC(&sys.allocatorMem, SAMSCROL_GENCODE_BUFFERSIZE);

    this->fontcode[0] = (u8*) MEM_ALLOC(&sys.allocatorMem, SAMSCROL_FONT_CODESIZE);

#   ifdef __TOS__
    samScrollGenerateCodeOverscan(this);
    samScrollGenerateCodeClearCurves(this);
#   endif

    {
        char* texts[] = { "Boing", "Boom", "Tschak", "Peng", "Zong"};
        u8* p = this->fontcode[0];
        u8  t;


        switch (g_screens.persistent.menu.currentmodule + RSC_BLITZWAV_ZIKS_LOADER_ARJX)
        {
        case RSC_BLITZWAV_ZIKS_NUTEK_ARJX:
            texts[0] = "Grafx";
            texts[1] = "Sound";
            break;
        }

        for (t = 0 ; t < ARRAYSIZE(texts) ; t++)
        {
            this->fontcode[t] = p;
            p += TEXfont44GenerateCode(p, texts[t],  SAMSCROL_PITCH, samScrollGenFontCode);
        }

        ASSERT((p - this->fontcode[0]) <= SAMSCROL_FONT_CODESIZE);
    }

    this->rasterBootFunc                      = RASvbl1;
    this->rasterBootOp.scanLinesTo1stInterupt = 1;
    this->rasterBootOp.backgroundColor        = 0;
    this->rasterBootOp.nextRasterRoutine      = (RASinterupt) this->overscanroutine[0];
    this->waitloop = 8;
    this->mask = 1;
    this->colorinc  = 1;
    this->colorinc2 = SAMSCROL_NBCOLORS_CHANGE * 7;

    BMscImportTable.colorstable = g_screens.samscrollStatic.rasterscolors + this->colorindex2;

    samScrollPrecomputeTables(this);

    SYSwriteVideoBase((u32) this->empty);    

#   if DEMOS_MEMDEBUG
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif

    SYSvsync;
    RASnextOpList     = &this->rasterBootOp;
    SYSvblroutines[1] = this->rasterBootFunc;

    SamScrollManageCommands(this, false);

#   ifndef __TOS__
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRAClog(TRAC_LOG_MEMORY, "SamScroll memallocator dump", '\n');
        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif

    FSMgotoNextState (_fsm);
    FSMgotoNextState (&g_stateMachine);
}

#ifdef __TOS__

static void samScrollDisplay(SamScroll* this, u8* framebase, u8 pixoff, u8* nextframe, u8* nextframebase, u8 nextpixoff, u8 flip, u8* clearframe, u16* clearoffsets, u8 clearpixoff, u16* _nextclearoffsets)
{    
    typedef void (*ClearCall)(void* _list, void* _adr1, u32 _adr2, u16 _value, u16 _pixoffset);
    u16  mask = this->mask;

    IGNORE_PARAM(framebase);

    if (this->freqcount != 0)
        mask = 0;

    *HW_VIDEO_PIXOFFSET = pixoff;
    if (pixoff)
    {
        *HW_VIDEO_OFFSET = ((SAMSCROL_PITCH - SAMSCROL_DISPLAYW) >> 1) - 4;
        BMscImportTable.waitLoop = this->waitloop + 8;
    }
    else
    {
        *HW_VIDEO_OFFSET = (SAMSCROL_PITCH - SAMSCROL_DISPLAYW) >> 1;
        BMscImportTable.waitLoop = this->waitloop;
    }

    BMscImportTable.frontBuffer           = (u32) framebase << 8;
    BMscImportTable.samplebase            = (void*) g_screens.player.dmabufstart;
    BMscImportTable.displaybase           = nextframe + (u32)SAMSCROL_DISPLAYH / 2UL * (u32)SAMSCROL_PITCH + (u32)(SAMSCROL_DISPLAYW - SAMSCROL_CURVEBORDEROFFSET);
    BMscImportTable.playfieldClearAddress = nextframebase + SAMSCROL_DISPLAYW;
    BMscImportTable.xoffsettable          = this->xoffsetstable + nextpixoff;
    BMscImportTable.rollingpixel          = mask << (15 - nextpixoff);
    BMscImportTable.rollingpixel         |= mask >> (1 + nextpixoff);
    BMscImportTable.clearoffsets          = _nextclearoffsets;
    BMscImportTable.colorstable           = g_screens.samscrollStatic.rasterscolors + this->colorindex2;

    if (this->no0)
    {
        BMscImportTable.yoffsettable  = this->yoffsetstable2_h;
        BMscImportTable.yoffsettable2 = this->yoffsetstable2_l;
    }
    else
    {
        BMscImportTable.yoffsettable  = this->yoffsetstable;
        BMscImportTable.yoffsettable2 = this->yoffsetstable;
    }

    this->rasterBootOp.nextRasterRoutine = (void*) this->overscanroutine[!flip];

#   if SAMSCROL_RASTERIZE()
    *HW_COLOR_LUT = 0x70;
#   endif

    if (this->drawtext != 0)
    {
        u8 codeindex = this->drawtext - 1;

        if (codeindex > 4)
            codeindex -= 5;

        TEXfont44Display (this->fontcode[codeindex], (nextframebase + SamScroll_g_textOffsets[this->drawtext - 1] + 120 + this->drawtextoffset), 5, 2);
        this->drawtext = 0;
        this->drawtextoffset ^= 2;
    }

#   if SAMSCROL_RASTERIZE()
    *HW_COLOR_LUT = 0x770;
#   endif

    if (clearframe)
    {
        while(*HW_MFP_TIMER_B_CONTROL); /* sync on low border */

        ((ClearCall)this->clearroutine)(
            clearoffsets,
            clearframe  + (u32)(SAMSCROL_DISPLAYW - SAMSCROL_CURVEBORDEROFFSET) + ((u32)SAMSCROL_CURVE1_Y*(u32)SAMSCROL_PITCH) + 8UL,
            (u32)(clearframe + (u32)(SAMSCROL_DISPLAYW - SAMSCROL_CURVEBORDEROFFSET) + ((u32)SAMSCROL_CURVE2_Y*(u32)SAMSCROL_PITCH) + 8UL),
            0,
            clearpixoff);
    }

#   if SAMSCROL_RASTERIZE()
    *HW_COLOR_LUT = 0x0;
#   endif
}

#else

static u16* samScrollDrawCurvePC (SamScroll* this, void* _sample, u16 _nbsamples, u16 _incx1, u16 _incx2, u8* _screen, u8 _offset, u16* _yoffsetstable, u16* clean)
{
    u16* disp   = (u16*)_screen;
    s8*  sample = (s8*)_sample;
    u16  i = _nbsamples;
    u16  p1;
    s16  xoffset = 0;
    s16* hoffset = this->xoffsetstable + _offset;
    u16  mask = this->mask;


    if (this->freqcount != 0)
        mask = 0;

    p1  = mask << (15 - _offset);
    p1 |= mask >> (1 + _offset);

    while (1)
    {
        u8 s = *sample;

        s16 offset = _yoffsetstable[s >> 1] + xoffset;

        sample += _incx1;

        //if (s)
            disp[offset >> 1] |= PCENDIANSWAP16(p1);

        if (clean != NULL)
            *clean++ = offset;

        p1 = (p1 >> 1) | (p1 << 15);

        xoffset += *hoffset++;

        if (--i == 0)
            return clean;

        /* -------------------*/
        s = *sample;

        offset = _yoffsetstable[s >> 1] + xoffset;

        sample += _incx2;

        //if (s)
            disp[offset >> 1] |= PCENDIANSWAP16(p1);

        if (clean != NULL)
            *clean++ = offset;

        p1 = (p1 >> 1) | (p1 << 15);

        xoffset += *hoffset++;

        if (--i == 0)
            return clean;
    }
}


static void samScrollDisplay(SamScroll* this, u8* framebase, u8 pixoff, u8* nextframe, u8* nextframebase, u8 nextpixoff, u8 flip, u8* clearframe, u16* clearoffsets, u8 clearpixoff, u16* _nextclearoffsets)
{
    s8* sampled = (s8*) g_screens.player.dmabufstart;
    u8  incx1  = 1 << (1 + 1 /*+ g_screens.interlace*/);
    u8  incx2  = 8 - incx1;
    u16 t;
      

    {
        u8* p = nextframebase + SAMSCROL_DISPLAYW;

        for (t = 0; t < SAMSCROL_DISPLAYH; t++)
        {
            STDmset(p, 0, 4);
            STDmset(p - SAMSCROL_CYCLINGOFFSET, 0, 4);
            p += SAMSCROL_PITCH;
        }
    }

    {
        u16* nextclearroutine = _nextclearoffsets;
        u8   channelAoff = this->curveselect.channelAoff;
        u8   channelBoff = this->curveselect.channelBoff;
        u16  *yoffsettable, *yoffsettable2;

        if (this->no0)
        {
            yoffsettable   = this->yoffsetstable2_h;
            yoffsettable2  = this->yoffsetstable2_l;
        }
        else
        {
            yoffsettable  = this->yoffsetstable;
            yoffsettable2 = this->yoffsetstable;
        }

        nextframe += SAMSCROL_DISPLAYW - SAMSCROL_CURVEBORDEROFFSET;

        /* white */                                                                                           
        nextclearroutine = samScrollDrawCurvePC(this, sampled + channelAoff, SAMSCROL_NBSAMPOINTS, incx1, incx2 , nextframe           + SAMSCROL_PITCH * SAMSCROL_CURVE1_Y, nextpixoff, yoffsettable , nextclearroutine);
        nextclearroutine = samScrollDrawCurvePC(this, sampled + channelBoff, SAMSCROL_NBSAMPOINTS, incx1, incx2 , nextframe           + SAMSCROL_PITCH * SAMSCROL_CURVE2_Y, nextpixoff, yoffsettable2, nextclearroutine);
        ASSERT((nextclearroutine - _nextclearoffsets) < (4 * SAMSCROL_NBSAMPOINTS + 1));

        /* green */                                                                                           
        samScrollDrawCurvePC(this, sampled + channelAoff, SAMSCROL_NBSAMPOINTS, incx1, incx2 , nextframe - 4                          + SAMSCROL_PITCH * SAMSCROL_CURVE1_Y, nextpixoff, yoffsettable , NULL);
        samScrollDrawCurvePC(this, sampled + channelBoff, SAMSCROL_NBSAMPOINTS, incx1, incx2 , nextframe - 4                          + SAMSCROL_PITCH * SAMSCROL_CURVE2_Y, nextpixoff, yoffsettable2, NULL);
 
        samScrollDrawCurvePC(this, sampled + channelAoff, SAMSCROL_NBSAMPOINTS, incx1, incx2 , nextframe - 4 - SAMSCROL_CYCLINGOFFSET + SAMSCROL_PITCH * SAMSCROL_CURVE1_Y, nextpixoff, yoffsettable , NULL);
        samScrollDrawCurvePC(this, sampled + channelBoff, SAMSCROL_NBSAMPOINTS, incx1, incx2 , nextframe - 4 - SAMSCROL_CYCLINGOFFSET + SAMSCROL_PITCH * SAMSCROL_CURVE2_Y, nextpixoff, yoffsettable2, NULL);
    }

    {
        static u8* delayedclearframe = NULL;        /* no sync display on pc... */
        static u16 delayedclearoffsets[SAMSCROL_NBSAMPOINTS*2];

        if (delayedclearframe)
        {
            u16* p = delayedclearoffsets;

            delayedclearframe += SAMSCROL_DISPLAYW - SAMSCROL_CURVEBORDEROFFSET;
            for (t = 0; t < SAMSCROL_NBSAMPOINTS; t++)
            {
                s16 offset = *p++;
                *(u16*)(delayedclearframe + (s32)offset + (s32)(SAMSCROL_PITCH * SAMSCROL_CURVE1_Y)) = 0;
            }

            for (t = 0; t < SAMSCROL_NBSAMPOINTS; t++)
            {
                s16 offset = *p++;
                *(u16*)(delayedclearframe + (s32)offset + (s32)(SAMSCROL_PITCH * SAMSCROL_CURVE2_Y)) = 0;
            }
        }

        delayedclearframe = clearframe;

        if (clearframe)
            memcpy (delayedclearoffsets, clearoffsets, SAMSCROL_NBSAMPOINTS*2*sizeof(u16));
    }

    if (this->drawtext != 0)
    {
        u8 codeindex = this->drawtext - 1;

        if (codeindex > 4)
            codeindex -= 5;

        nextframebase += SAMSCROL_DISPLAYW - SAMSCROL_CURVEBORDEROFFSET;

        TEXfont44DisplayPC(this->fontcode[codeindex], nextframebase + SamScroll_g_textOffsets[this->drawtext-1] + this->drawtextoffset, 5, 2);
        this->drawtext = 0;
        this->drawtextoffset ^= 2;
    }

    EMULfbExStart(HW_VIDEO_MODE_4P, 0, 63, (SAMSCROL_DISPLAYW * 2) - 1, 63 + SAMSCROL_DISPLAYH - 1, SAMSCROL_PITCH, pixoff);
    EMULfbExSetAdr(EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, 0), (u32) framebase);

    {
        u16* p = g_screens.samscrollStatic.rasterscolors + this->colorindex2;

        for (t = 0; t < SAMSCROL_NBCOLORS_CHANGE; t++)
        {
            u32 cycle = EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, t * 5 + 60 + 63);

            if (flip)
            {
                EMULfbExSetColor(cycle, 1,  p[0]);
                EMULfbExSetColor(cycle, 2,  p[1]);
                EMULfbExSetColor(cycle, 3,  p[2]);
                EMULfbExSetColor(cycle, 8,  p[3]);
                EMULfbExSetColor(cycle, 9,  p[4]);
                EMULfbExSetColor(cycle, 10, p[5]);
                EMULfbExSetColor(cycle, 11, p[6]);
            }
            else
            {
                EMULfbExSetColor(cycle, 1,  p[0]);
                EMULfbExSetColor(cycle, 2,  p[1]);
                EMULfbExSetColor(cycle, 3,  p[2]);
                EMULfbExSetColor(cycle, 4,  p[3]);
                EMULfbExSetColor(cycle, 5,  p[4]);
                EMULfbExSetColor(cycle, 6,  p[5]);
                EMULfbExSetColor(cycle, 7,  p[6]);
            }

            p += 7;
        }
    }

    EMULfbExEnd();
}
#endif

/* color 0 : bitplane 3 visible */
/* color 1 : bitplane 2 visible */

void SamScrollActivity (FSM* _fsm)
{
    SamScroll* this = g_screens.samscroll;
    u8* frame;
    u8* framebase;
    u8* nextframe;
    u8* nextframebase;
    u8  flip   = this->flip;
    u16 nextscroll = this->scroll;
    u8  pixoff = nextscroll & 15;
    u8  nextpixoff;


    IGNORE_PARAM(_fsm);

    framebase = this->framebuf + SAMSCROL_PITCH * 2;
    nextframe = framebase;
    framebase += (nextscroll >> 4) << 3;

    nextscroll += SAMSCROL_SPEED;
    if (nextscroll >= 480)
        nextscroll -= 480;

    nextframe += (nextscroll >> 4) << 3;
    nextframebase = nextframe;
    nextframe += 4;
    nextpixoff = nextscroll & 15;

    frame = framebase + 4;
    
    STDmcpy2 (HW_COLOR_LUT, g_screens.samscrollStatic.color[this->colorindex][flip], 32);

    if (flip)
    {
        frame += 2;
        nextframe += 2;
    }

    {
        s16 lastflip = !flip;

        samScrollDisplay(this,
            framebase, pixoff, 
            nextframe, nextframebase, nextpixoff, 
            flip, 
                   this->clearframes  [lastflip],
            (u16*) this->clearoffsets [lastflip],
                   this->clearpixoff  [lastflip],
            (u16*) this->clearoffsets [flip] );

        this->clearpixoff[flip] = nextpixoff;
        this->clearframes[flip] = nextframe;
    }

    this->scroll = nextscroll;
    this->flip ^= 1;      
    
    this->freqcount++;

    if (this->freqcount >= this->freqmax)
        this->freqcount = 0;


    {
        static int count = 0;

        if ((count++ & 7) == 0)
        {
            this->colorindex  += this->colorinc;
            this->colorindex2 += this->colorinc2;

            if ((this->colorindex >= 15) || (this->colorindex <= 0))
            {
                this->colorinc = -this->colorinc;
                this->colorinc2 = -this->colorinc2;
            }
        }
    }

    SamScrollManageCommands(this, true);
}


void SamScrollExit (FSM* _fsm)
{
    SamScroll* this = g_screens.samscroll;
        
    IGNORE_PARAM(_fsm);

    SYSvblroutines[1] = RASvbldonothing;
    BlitZturnOffDisplay();

    RINGallocatorCheck(sys.allocatorMem.allocator);

    MEM_FREE(&sys.allocatorMem, this->fontcode[0]);
    MEM_FREE(&sys.allocatorMem, this->empty);
    MEM_FREE(&sys.allocatorMem, this->clearroutine);
    MEM_FREE(&sys.allocatorMem, this->clearoffsets[0]);
    MEM_FREE(&sys.allocatorMem, this->overscanroutine[0]);
    MEM_FREE(&sys.allocatorMem, this->framebuf);

    MEM_FREE(&sys.allocatorMem, g_screens.samscroll);
    this = g_screens.samscroll = NULL;

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    ScreensGotoScreen();
}


void SamScrollBacktask (FSM* _fsm)
{
    SamScroll* this = g_screens.samscroll;

    IGNORE_PARAM(_fsm);

    if ((this->curvedisplay.channelAoff != this->curveselect.channelAoff) ||
        (this->curvedisplay.channelBoff != this->curveselect.channelBoff))
    {
        samScrollPatchSampleAccess(
            this, 
            this->overscanroutine[0], 
            this->curveselect.channelAoff,
            this->curveselect.channelBoff,
            12, 12);

        samScrollPatchSampleAccess(
            this,
            this->overscanroutine[1], 
            this->curveselect.channelAoff,
            this->curveselect.channelBoff,
            12, 12);

        STDmcpy(&this->curvedisplay, &this->curveselect, sizeof(this->curveselect));
    }

}

void SamScrollInitStatic (FSM* _fsm)
{
    g_screens.samscrollStatic.rasterscolors = (u16*) MEM_ALLOC(&sys.allocatorCoreMem,  SAMSCROL_NBCOLORS_CHANGE * 7 * 16 * sizeof(u16));

    samscrolCreateColors(&g_screens.samscrollStatic);

    FSMgotoNextState(_fsm);
}
