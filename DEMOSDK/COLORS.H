/*------------------------------------------------------------------------------  -----------------
  Copyright J.Hubert 2015

  This file is part of demOS

  demOS is free software: you can redistribute it and/or modify it under the terms of 
  the GNU Lesser General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  demOS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY ;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with demOS.  
  If not, see <http://www.gnu.org/licenses/>.
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
u16  COLcomputeGradientEx (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _nbSteps, u16* _destColors1, u16* _destColors2);

#endif
