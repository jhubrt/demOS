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

extern "C"
{
#   include "DEMOSDK\BASTYPES.H"

#   include "DEMOSDK\STANDARD.H"

#   include "DEMOSDK\PC\SURFACE.H"
#   include "DEMOSDK\PC\WINDOW.H"
}

#include "TOOLS\BINARIZE\HELPERSW.H"

extern MEMallocator stdAllocator;

void TOOLdisplayImage (WINdow* _window, BITsurface* _surface)
{
    if ( _window != NULL )
    {
        BITsurface temp;
        BITsurface* surface = _surface;


        if (( _surface->format != BITformat_8bits ) && ( _surface->format != BITformat_x888 ))
        {
            surface = &temp; 
            
            BITsurfaceConstruct (surface);
            BITsurfaceConvert (&stdAllocator, _surface, surface, BITformat_8bits);
        }

        WINdrawImage (
            _window, 
            surface->buffer, 
            surface->width, surface->height, 
            surface->format == BITformat_8bits ? 8 : 32, 
            surface->format == BITformat_8bits ? surface->lut.data.p : NULL,
            (WINDOW_WIDTH - surface->width) / 2, (WINDOW_HEIGHT - surface->height) / 2);

        WINrender (_window);
        WINwaitForGUI(_window);

        if ( surface == &temp )
        {
            BITsurfaceDestroy(surface);
        }
    }
}

