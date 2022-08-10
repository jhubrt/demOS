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

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\CODEGEN.H"

#include "FX\SMPLCURV\SMPLCURV.H"


enum SamCurveASMopcodes_
{
    BSCrOp_DrawHBegin,
    BSCrOp_DrawHStart,
    BSCrOp_DrawH,
    BSCrOp_DrawHEnd,

    BSCrOp_DrawHYMBegin,
    BSCrOp_DrawHYMStart,
    BSCrOp_DrawHYM,
    BSCrOp_DrawHYMEnd,

    BSCrOp_SIZE
};
typedef enum SamCurveASMopcodes_ SamCurveASMopcodes;


struct SamCurveASMimport_
{
    CGENdesc opcodes[BSCrOp_SIZE];
};
typedef struct SamCurveASMimport_ SamCurveASMimport;

ASMIMPORT SamCurveASMimport BSCrImportTable;

void SmplCurveInitOffset (s16 _halfHeight, s16* offsets_, s16 pitch_)
{
    s16  t;
    s16* pup = offsets_ + 128;
    s16* pdown = pup;
    s16  inc = 0;
    u16  acc = 0;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "SmplCurveInitOffset", '\n');

    pup--;
    *pdown++ = 0;                   

    for (t = 1 ; t < 128 ; t++)
    {
        acc += _halfHeight;
        if (acc >= 128)
        {
            acc -= 128;
            inc += pitch_;
        }

        *pup-- = -inc;
        *pdown++ = inc;
    }

    acc += _halfHeight;
    if (acc >= 128)
        inc += pitch_;

    *pup-- = -inc;
}

void SmplCurveYMgetStates (SmplCurveYMcurveState* states_)
{
    u8 mixer;
    u8 t;


    *HW_YM_REGSELECT = HW_YM_SEL_IO_AND_MIXER;
    mixer = HW_YM_GET_REG();

    for (t = 0; t < 3; t++)
    {
        states_->type = SmplCurveYMcurveType_NONE;

        if ((mixer & 9) != 9)   /* no square and no noise */
        {            
            *HW_YM_REGSELECT = HW_YM_SEL_LEVELCHA + t;
            states_->level = HW_YM_GET_REG();

            ASSERT(states_->level < 17);

            if ((mixer & 8) == 0)  /* noise */
            {
                states_->type = SmplCurveYMcurveType_NOISE;

                *HW_YM_REGSELECT = HW_YM_SEL_FREQNOISE;
                states_->freqnoise = HW_YM_GET_REG();
            }

            if ((mixer & 1) == 0) /* square */
            {
                u8 fsqr0, fsqr1;
                u8 voice2 = t << 1;

                states_->type |= SmplCurveYMcurveType_SQUARE;

                *HW_YM_REGSELECT = voice2;
                fsqr0 = HW_YM_GET_REG();

                *HW_YM_REGSELECT = voice2 + 1;
                fsqr1 = HW_YM_GET_REG();

                states_->freqsquare = ((u16)fsqr1 << 8) | (u16)fsqr0;
            }
        }

        mixer >>= 1;
        states_++;
    }
}


void SmplCurveInitSampleOffsets(u8*  basecode, u16* offsets, u16 nbcodesampleoffsets, u16 _inc1, u16 _inc2)
{
    u16  offset = 0;
    u16  i;

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "SmplCurveInitSampleOffsets", '\n');

    for (i = 0; i < nbcodesampleoffsets; i++)
    {
        *(u16*)(basecode + offsets[i]) = offset;
        offset += i & 1 ? _inc2 : _inc1;
    }
}


void SmplCurveGenerateDrawHCurves(u8* outputpcm_, u8* outputym_, u16* codesampleoffset_, u16 nbcodesampleoffset_)
{
    CGENdesc* code = BSCrImportTable.opcodes;
    u8        *output, *outputym;
    s16       t,i;
    u16       offsetsdisplay1[5];
    u16       offsetsdisplay2[8];
    u16       offsetsdisplayym1[10];
    u16       offsetsdisplayym2[16];
    u16       offsetssample1[5];
    u16       offsetssample2[8];
    s16       offset = -128;
    s16       offsetym = 0;
    u16       result;
    u8        *p, *p2;
    u16       nbsamoffs = 0;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "SmplCurveGenerateDrawHCurves", '\n');

    output   = outputpcm_;

    CGENgenerateSimple(&code[BSCrOp_DrawHBegin]  , output);

    result = CGENfindWords(&code[BSCrOp_DrawHStart], offsetsdisplay1, 0x0077);
    ASSERT(result == ARRAYSIZE(offsetsdisplay1));

    result = CGENfindWords(&code[BSCrOp_DrawHStart], offsetssample1, 0xABA);
    ASSERT(result == ARRAYSIZE(offsetssample1));

    result = CGENfindWords(&code[BSCrOp_DrawH], offsetsdisplay2, 0x0077);
    ASSERT(result == ARRAYSIZE(offsetsdisplay2));

    result = CGENfindWords(&code[BSCrOp_DrawH], offsetssample2, 0xABA);
    ASSERT(result == ARRAYSIZE(offsetssample2));

    outputym = outputym_;

    CGENgenerateSimple(&code[BSCrOp_DrawHYMBegin], outputym);

    result = CGENfindWords(&code[BSCrOp_DrawHYMStart], offsetsdisplayym1, 0x0077);
    ASSERT(result == ARRAYSIZE(offsetsdisplayym1));

    result = CGENfindWords(&code[BSCrOp_DrawHYM], offsetsdisplayym2, 0x0077);
    ASSERT(result == ARRAYSIZE(offsetsdisplayym2));

    p  = (u8*) output;
    p2 = (u8*) outputym;

    CGENgenerateSimple(&code[BSCrOp_DrawHStart]  , output);
    CGENgenerateSimple(&code[BSCrOp_DrawHYMStart], outputym);

    {
        u16 o = offset & 0xFF;

        for (i = 0 ; i < ARRAYSIZE(offsetsdisplay1) ; i++)
        {
            *(u16*)(p + offsetsdisplay1[i]) = o;
            codesampleoffset_[nbsamoffs++] = (u16)(p - outputpcm_) + offsetssample1[i];
        }

        for (i = 0 ; i < ARRAYSIZE(offsetsdisplayym1) ; i += 2)
        {
            *(u16*)(p2 + offsetsdisplayym1[i    ]) = offsetym;
            *(u16*)(p2 + offsetsdisplayym1[i + 1]) = offsetym;
        }
    }

    offset++;
    offsetym++;

    for (t = 0 ; t < 41 ; t++)
    {
        u16 o = offset & 0xFF;

        p  = (u8*) output;
        p2 = (u8*) outputym;

        CGENgenerateSimple(&code[BSCrOp_DrawH]  , output);
        CGENgenerateSimple(&code[BSCrOp_DrawHYM], outputym);

        for (i = 0 ; i < ARRAYSIZE(offsetsdisplay2) ; i++)
        {
            *(u16*)(p + offsetsdisplay2[i]) = o;
            codesampleoffset_[nbsamoffs++] = (u16)(p - outputpcm_) + offsetssample2[i];
        }

        for (i = 0 ; i < ARRAYSIZE(offsetsdisplayym2) ; i += 2)
        {
            *(u16*)(p2 + offsetsdisplayym2[i    ]) = offsetym;
            *(u16*)(p2 + offsetsdisplayym2[i + 1]) = offsetym;
        }

        if (offsetym & 1)
        {
            offset   += 7;
            offsetym += 7;
        }
        else
        {
            offset  ++;
            offsetym++;
        }
    }

    CGENgenerateSimple(&code[BSCrOp_DrawHEnd  ], output);
    CGENgenerateSimple(&code[BSCrOp_DrawHYMEnd], outputym);

    TRAClogNumber10(TRAC_LOG_SPECIFIC, "Codesize: "  , output   - outputpcm_  , 6);
    TRAClogNumber10(TRAC_LOG_SPECIFIC, "CodeYMsize: ", outputym - outputym_   , 6);

    ASSERT(nbsamoffs == nbcodesampleoffset_);
    ASSERT((output   - outputpcm_) <= SMPLCURV_CODESIZE);
    ASSERT((outputym - outputym_ ) <= SMPLCURV_CODESIZE_YM);
}

#ifndef __TOS__

void SmplCurveDrawHCurve (void* routine_, void* _sample, u16 _nbsamples, u16 _incx, void* _screen, s16* offsets_)
{
    u16* disp = (u16*) _screen;
    s8* sample = (s8*)_sample;
    u16 i = 0;


    while(1)
    {
        u16 p1 = 0x8000;

        do
        {
            s16 s = offsets_[128+*sample];
            sample += _incx;

            disp[s >> 1] |= PCENDIANSWAP16(p1);

            p1 >>= 1;

            if (++i >= _nbsamples)
                return;
        }
        while (p1 != 0);

        disp += 4;
    }
}

void SmplCurveDrawHYMCurve (void* routine_, u16 _nbsamples, void* display1, void* display2, s16 _acc, s16 _inc1, s16 _inc2)
{
    u16* disp1 = (u16*) display1;
    u16* disp2 = (u16*) display2;
    u16  i     = 0;


    while(1)
    {
        u16 p1 = 0x8000;

        do
        {
            _acc += _inc1;

            if (_acc >= 0)
                *disp2 |= PCENDIANSWAP16(p1);
            else
                *disp1 |= PCENDIANSWAP16(p1);

            p1 >>= 1;

            if (++i >= _nbsamples)
                return;

            _acc += _inc2;

            if (_acc >= 0)
                *disp2 |= PCENDIANSWAP16(p1);
            else
                *disp1 |= PCENDIANSWAP16(p1);

            p1 >>= 1;

            if (++i >= _nbsamples)
                return;
        }
        while (p1 != 0);

        disp1 += 4;
        disp2 += 4;
    }
}


#endif
