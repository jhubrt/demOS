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

#ifndef COLPLANE_H
#define COLPLANE_H

#define COLP_RAIMBOW_MODE  0
#define COLP_GRADIENT_MODE 1

struct COLPanimation_
{
    u16*    colors;
    u16     currentpal;
    u8      currentplane;
    u8      colormode;
};

typedef struct COLPanimation_ COLPanimation;

u16* COLPinitColors4Praimbow    (MEMallocator* _allocator_, u16 _colors[4], bool _combinemax);
u16* COLPinitColors4Pgradient   (MEMallocator* _allocator_, u16 _start[4], u16 _end[4], bool _combinemax);

void COLPinitColors3Praimbow    (u16 _colors[3], u16 backcolor_, u16 colorsbuffer[16*4]);
u16* COLPinitColors3Pgradient   (MEMallocator* _allocator_, u16 _start[3], u16 _end[3], u16 backcolor_);

void COLPinitColors3P(u16 _colors[3], u16 backcolor_, u16** colors_);

u16 COLPanimate4P (COLPanimation* animation_);
u16 COLPanimate3P (COLPanimation* animation_);

#endif
