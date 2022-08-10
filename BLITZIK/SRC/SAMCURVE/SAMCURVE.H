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

#ifndef SAMCURVE_H
#define SAMCURVE_H

#include "DEMOSDK\FSM.H"

#include "FX\COLPLANE\COLPLANE.H"
#include "FX\SMPLCURV\SMPLCURV.H"

void SamCurveInitStatic (FSM* _fsm);

void SamCurveEnter	   (FSM* _fsm);
void SamCurveBacktask  (FSM* _fsm);
void SamCurveExit	   (FSM* _fsm);

void SamCurveActivity  (FSM* _fsm);

struct SamCurveStatic_
{
    u8* drawcurveroutine;
    u8* drawcurveroutineym;
    u16 codesampleoffset[41*8+5];
};
typedef struct SamCurveStatic_ SamCurveStatic;


struct SamDisplay_
{
    u8      inc1;
    u8      inc2;
};
typedef struct SamDisplay_ SamDisplay;

struct SamCurveRaster1_
{
    RASopVbl1 vbl1;
    RASopLowB lowb;
};
typedef struct SamCurveRaster1_ SamCurveRaster1;

struct SamCurveRaster2_
{
    RASopVbl1  vbl1;
    RASopMid15 mid15;
    RASopLowB  lowb;
};
typedef struct SamCurveRaster2_ SamCurveRaster2;

struct SamCurve_
{
    COLPanimation coloranimation;

    u8* 	framebuffer;
    u8*     lastimage[2];
    s16     offsets[256];
    s16     offsets2[256];
    u16     colors [2][4][16*4];

    SamCurveRaster1 rasters1;
    SamCurveRaster2 rasters2;

    SmplCurveYMcurveState ymstates[3];
    s16     ymincs[3];

    SamDisplay samrequest;
    SamDisplay samdisplay;

    u8      colorchoice;
    u8      voice;
    u8      voice2;
    u8      xorfreeareah;
    s8      ypos;
    u8      curvetype;
    u8      oldcurvetype;
};
typedef struct SamCurve_ SamCurve;

#endif
