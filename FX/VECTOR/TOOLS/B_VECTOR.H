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

#ifndef B_VECTOR_H
#define B_VECTOR_H

struct VECpoint
{
    s16 x,y;
};

typedef std::vector< std::vector<VECpoint> > VECscene;

void VECwriteScene (const VECscene& _scene, u16 _centerx, u16 _centery, float _scalex, float _scaley, const char* _outputFile);
void VECdisplayScene (WINdow* _window, BITsurface& _surface, const VECscene& polygons, s16 _centerx, s16 _centery);
void VECbinarizeScene (WINdow* _window, char* _inputImage, u16 _maxcolorindex, u16 _x1, u16 _y1, u16 _x2, u16 _y2, u16 _centerx, u16 _centery, float _scalex, float _scaley, char* _outputFile);

#endif
