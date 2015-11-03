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

/*! @brief @ref BITMAP @file */

/*! @defgroup  BITMAP 
    
    BITMAP implement optimized routine to unpack a picture from 4b by pixel to ST chunked planar 
    4bits format.
    Using 4bits pixels format achieves better compression ratio for storage 
*/

#ifndef BITMAP_H
#define BITMAP_H

/* when _endpitch line is 0, you can use _src == _dst */
void BITpl2chunk(void* _src, u16 _h, u16 _nbchunks, u16 _endlinepitch, void* _dst);

#ifdef DEMOS_UNITTEST
void BITunitTest (void);
#endif

#endif

