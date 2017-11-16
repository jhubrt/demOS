/*------------------------------------------------------------------------------  -----------------
  The MIT License (MIT)

  Copyright (c) 2015-2016 J.Hubert

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
------------------------------------------------------------------------------------------------- */

/*! @brief @ref COLORS @file */

/*! @defgroup  COLORS

    COLORS implement optimized services to help you in managing colors in the STe LUT format <br>

    Provides:                                                                                <br>
    - arrays to convert to /from 4b to STe rotated format                                    <br>
    - routines to precompute gradients of color                                              <br>
*/

#ifndef COLORS_H
#define COLORS_H

#include "DEMOSDK\BASTYPES.H"

#ifndef COLORS_C
extern u8 COLST24b[16];
extern u8 COL4b2ST[16];
#endif

u16* COLcomputeGradient (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _nbSteps, u16* _destColors);
u16* COLcomputeGradientStep     (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _nbSteps, s16 _step, u16* _destColors);

u16  COLcomputeGradientEx (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _nbSteps, u16* _destColors1, u16* _destColors2);

#endif
