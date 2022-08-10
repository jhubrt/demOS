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

#ifndef SMPLCURV_H
#define SMPLCURV_H

#define SMPLCURV_CODESIZE    5360
#define SMPLCURV_CODESIZE_YM 4690

enum SmplCurveYMcurveType_
{
    SmplCurveYMcurveType_NONE,
    SmplCurveYMcurveType_NOISE,
    SmplCurveYMcurveType_SQUARE,
    SmplCurveYMcurveType_NOISE_AND_SQUARE
};
typedef enum SmplCurveYMcurveType_ SmplCurveYMcurveType;

struct SmplCurveYMcurveState_
{
    u8  type;
    u8  level;
    u8  freqnoise;
    u16 freqsquare;
};
typedef struct SmplCurveYMcurveState_ SmplCurveYMcurveState;

void SmplCurveInitSampleOffsets     (u8*  basecode, u16* offsets, u16 nbcodesampleoffsets, u16 _inc1, u16 _inc2);
void SmplCurveYMgetStates           (SmplCurveYMcurveState* states_);
void SmplCurveGenerateDrawHCurves   (u8* outputpcm_, u8* outputym_, u16* codesampleoffset_, u16 nbcodesampleoffset_);
void SmplCurveInitOffset            (s16 _halfHeight, s16* offsets_, s16 pitch_);

#ifdef __TOS__

typedef void (*SmplCurveDrawHCall)(void* sample, void* display, u32 offsetytable);

#define SmplCurveDrawHCurve(_routine, _sample, _nbsamples, _incx, _screen, offsets_) ((SmplCurveDrawHCall)_routine)(_sample, ((u8*)(_screen))+128, (u32)(offsets_+128))

typedef void (*SmplCurveDrawHYMCall)(void* display1, void* display2, u16 _acc, u16 _inc1, u16 _inc2);

#define SmplCurveDrawHYMCurve(_routine, _nbsamples, display1, display2, _acc, _inc1, _inc2) ((SmplCurveDrawHYMCall)_routine)(display1, display2, _acc, _inc1, _inc2)

#else

void SmplCurveDrawHCurve   (void* routine_, void* _sample, u16 _nbsamples, u16 _incx, void* _screen, s16* offsets_);
void SmplCurveDrawHYMCurve (void* routine_, u16 _nbsamples, void* display1, void* display2, s16 _acc, s16 _inc1, s16 _inc2);

#endif

#endif
