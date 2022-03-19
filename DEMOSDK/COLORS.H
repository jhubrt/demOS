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


/*  COLORS implement optimized services to help you in managing colors in the STe LUT format

    Provides:                                                                               
    - arrays to convert to /from 4b to STe rotated format                                   
    - routines to precompute gradients of color                                             
*/

#ifndef COLORS_H
#define COLORS_H

#ifndef COLORS_C
extern u8 COLST24b[16];
extern u8 COL4b2ST[17];
#endif

#define COL_4B_TO_STE(COLOR) ((COL4b2ST[((COLOR) >> 8) & 0xF] << 8) | (COL4b2ST[((COLOR) >> 4) & 0xF] << 4) | COL4b2ST[(COLOR) & 0xF])
#define COL_STE_TO_4B(COLOR) ((COLST24b[((COLOR) >> 8) & 0xF] << 8) | (COLST24b[((COLOR) >> 4) & 0xF] << 4) | COLST24b[(COLOR) & 0xF])

#ifdef __TOS__

#define COLswapEndianPC(_startColors,_nbColors)

#define COLcomputeGradient16Steps(_startColors, _endcolors, _nbColors, _step, _destColors)       COL16STST(_startColors, _endcolors, _nbColors, _step, (u32)(_destColors))
#define COLcomputeGradient16Steps4b4b(_startColors, _endcolors, _nbColors, _step, _destColors)   COL164b4b(_startColors, _endcolors, _nbColors, _step, (u32)(_destColors))
#define COLcomputeGradient16Steps4bSTe(_startColors, _endcolors, _nbColors, _step, _destColors)  COL164bST(_startColors, _endcolors, _nbColors, _step, (u32)(_destColors))

u16* COL16STST (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _step, u32 _destColors);
u16* COL164bST (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _step, u32 _destColors);
u16* COL164b4b (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _step, u32 _destColors);

#else

void COLswapEndianPC                 (u16* _colors, u16 _nbColors);

u16* COLcomputeGradient16Steps       (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _step, u16* _destColors);
u16* COLcomputeGradient16Steps4b4b   (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _step, u16* _destColors);
u16* COLcomputeGradient16Steps4bSTe  (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _step, u16* _destColors);

#endif

u16* COLcomputeGradientStep          (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _nbSteps, s16 _step, u16* _destColors);
u16* COLcomputeGradient              (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _nbSteps, u16* _destColors);
u16* COLcomputeGradient4b4b          (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _nbSteps, u16* _destColors);

u16  COLcomputeGradientEx            (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _nbSteps, u16* _destColors1, u16* _destColors2);

#ifdef DEMOS_UNITTEST
void COLrunUnitTests(void);
#endif

#endif
