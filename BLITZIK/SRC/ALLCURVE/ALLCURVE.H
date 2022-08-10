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

#ifndef ALLCURVE_H
#define ALLCURVE_H

#include "DEMOSDK\FSM.H"

#include "FX\COLPLANE\COLPLANE.H"

#define ALLCURV_DISPLAY2_H  246

void AllCurveInitStatic (FSM* _fsm);

void AllCurveEnter	   (FSM* _fsm);
void AllCurveBacktask  (FSM* _fsm);
void AllCurveExit	   (FSM* _fsm);

void AllCurveActivity  (FSM* _fsm);



enum AllCurveASMopcodes_
{   
    ASCrOp_DrawVBegin,
    ASCrOp_DrawV,
    ASCrOp_DrawVEnd,

    ASCrOp_DrawVymBegin,
    ASCrOp_DrawVym,
    ASCrOp_DrawVymNoise,
    ASCrOp_DrawVymEnd,

    ASCrOp_SIZE
};
typedef enum AllCurveASMopcodes_ AllCurveASMopcodes;


struct AllCurveASMimport_
{
    CGENdesc opcodes[ASCrOp_SIZE];
};
typedef struct AllCurveASMimport_ AllCurveASMimport;

struct AllCurveStatic_
{
    u8*     drawcurveroutine[1];
    u16     codesampleoffset[ALLCURV_DISPLAY2_H];
    u16     nbcodesampleoffsets;
    u16*    colors;
};
typedef struct AllCurveStatic_ AllCurveStatic;

struct AllCurve_
{
    COLPanimation coloranimation;

    u16*    colors2;
    u8* 	framebuffer;
    s16     offsets[256];
    u8*     drawcurveroutine[3];
    u8*     rasters[4];
    bool    feed;
    bool    step;
};
typedef struct AllCurve_ AllCurve;

#endif
