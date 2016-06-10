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

