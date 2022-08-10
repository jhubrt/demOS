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

#ifndef SAMSCROLL_H
#define SAMSCROLL_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"
#include "DEMOSDK\CODEGEN.H"


#define SAMSCROL_NBCOLORS_CHANGE 17
#define SAMSCROL_NBSAMPOINTS     249


void SamScrollInitStatic    (FSM* _fsm);
void samscrolCreateColors();
void SamScrollEnter	        (FSM* _fsm);
void SamScrollActivity	    (FSM* _fsm);
void SamScrollExit		    (FSM* _fsm);
void SamScrollBacktask      (FSM* _fsm);



struct SamScrollStatic_
{
    u16*    rasterscolors;
    u16     color[16][2][16];
};
typedef struct SamScrollStatic_ SamScrollStatic;

enum SamScrollASMopcodes_
{
    BSScOp_Begin,
    BSScOp_Clear,
    BSScOp_ReloadReg,
    BSScOp_LeftBorderRemoval,
    BSScOp_Sam1Sam2Sam1,
    BSScOp_Sam2Sam1Sam2,
    BSScOp_RightBorderColored,    
    BSScOp_RightBorderRemoval,
    BSScOp_FillCurvesOffsetInit,
    BSScOp_FillCurvesOffset,
    BSScOp_End,
    BSScOp_ClearCurve1,
    BSScOp_ClearCurve2,
    BSScOp_ClearCurve3,
    BSScOp_SIZE
};
typedef enum SamScrollASMopcodes_ SamScrollASMopcodes;


struct SamScrollASMimport_
{
    void*   playfieldClearAddress;
    u32     frontBuffer;
    u16     waitLoop;

    void*   samplebase;
    void*   clearoffsets;
    void*   displaybase;
    void*   colorstable;
    void*   yoffsettable2;
    void*   xoffsettable;
    void*   yoffsettable;

    u16     rollingpixel;
    u16     reserved3;
    u16     reserved4;

    CGENdesc opcodes[BSScOp_SIZE];
};
typedef struct SamScrollASMimport_ SamScrollASMimport;


struct SamScrollCurveControl_
{
    u8      channelAoff;
    u8      channelBoff;
    u8      channelAinc;
    u8      channelBinc;
};
typedef struct SamScrollCurveControl_ SamScrollCurveControl;


struct SamScroll_
{
    u8*     framebuf;
    u8*     empty;
    u8*     clearframes[2];
    u8*     fontcode[5];
    u16     scroll;
    u8      flip;
    u8      drawtext;
    u8      voice;
    u8      freqcount;
    u8      freqmax;
    bool    no0;
    u16     mask;
    u16     drawtextoffset;

    s16     colorindex;
    s16     colorindex2;
    s16     colorinc;
    s16     colorinc2;

    SamScrollCurveControl curveselect;
    SamScrollCurveControl curvedisplay;

    u16     waitloop;
    u8*     overscanroutine[2];
    void*   clearoffsets[2];
    u8      clearpixoff[2];
    void*   clearroutine;

    RASinterupt rasterBootFunc;
    RASopVbl1	rasterBootOp;

    u16     sampleoffsetsroutine1 [3];
    u16     sampleoffsetsroutine2 [3];
    u16     curveroutineoffsets   [SAMSCROL_NBSAMPOINTS * 2 / 3];
    s16     xoffsetstable         [SAMSCROL_NBSAMPOINTS + 16];
    s16     yoffsetstable         [128];
    s16     yoffsetstable2_h      [128];
    s16     yoffsetstable2_l      [128];
};
typedef struct SamScroll_ SamScroll;

#endif
