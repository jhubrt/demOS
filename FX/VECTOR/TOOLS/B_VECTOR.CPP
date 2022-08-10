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
#   include "DEMOSDK\SYSTEM.H"
#   include "DEMOSDK\STANDARD.H"
#   include "DEMOSDK\COLORS.H"
#   include "DEMOSDK\BITMAP.H"

#   include "DEMOSDK\PC\WINDOW.H"
#   include "DEMOSDK\PC\BMPLOADER.H"
#   include "DEMOSDK\PC\BITCONVR.H"
}

#include <vector>

#include "TOOLS\BINARIZE\HELPERS.H"
#include "FX\VECTOR\TOOLS\B_VECTOR.H"

extern MEMallocator stdAllocator;

void VECwriteScene (const VECscene& _scene, u16 _centerx, u16 _centery, float _scalex, float _scaley, const char* _outputFile)
{
    FILE* file = fopen(_outputFile, "wb");
    assert(file != NULL);

    u16 v;

    v = (u16) _scene.size();
    v = PCENDIANSWAP16(v);
    fwrite(&v, sizeof(v), 1, file);

    for (u16 poly = 0 ; poly < _scene.size() ; poly++)
    {
        const std::vector<VECpoint>& polygon = _scene[poly];
        u16 edge;
        v = (u16) polygon.size();

        v = PCENDIANSWAP16(v);
        fwrite(&v, sizeof(v), 1, file);

        for (edge = 0 ; edge < polygon.size() ; edge++)        
        {
            v = polygon[edge].x;
            v -= _centerx;
            v  = (s16)(0.5f + ((float)v) * _scalex);
            v = PCENDIANSWAP16(v);
            fwrite(&v, sizeof(v), 1, file);

            v = polygon[edge].y;
            v -= _centery;
            v  = (s16)(0.5f + ((float)v) * _scaley);
            v = PCENDIANSWAP16(v);
            fwrite(&v, sizeof(v), 1, file);
        }
    }

    fclose(file);
}

void VECdisplayScene (WINdow* _window, BITsurface& _surface, const VECscene& polygons, s16 _centerx, s16 _centery)
{
    if (_window != NULL)
    {
        WINsetColor(_window, 0,0,0);

        WINclear(_window);

        WINdrawImage (_window, 
            _surface.buffer, 
            _surface.width, _surface.height, 
            BITformat_8bits, _surface.lut.data.p,
            0, 0);

        WINsetColor(_window, 0,0,255);

        WINline(_window, 0, _centery, 1024, _centery);
        WINline(_window, _centerx, 0, _centerx, 1024);

        WINsetColor(_window, 255,0,0);

        for (u16 poly = 0 ; poly < polygons.size() ; poly++)
        {
            const std::vector<VECpoint>& polygon = polygons[poly];
            u16 edge;

            WINsetColor(_window, 128, 128, 128);

            for (edge = 0 ; edge < polygon.size() ; edge++)        
            {
                char temp[10];

                sprintf(temp, "%d", edge);
                WINtext(_window, polygon[edge].x, polygon[edge].y, temp);
            }

            WINsetColor(_window, 255, (poly * 64) & 255,0);

            for (edge = 1 ; edge < polygon.size() ; edge++)        
            {
                WINline(_window, polygon[edge-1].x, polygon[edge-1].y, polygon[edge].x, polygon[edge].y);
            }

            edge--;

            WINline(_window, polygon[0].x, polygon[0].y, polygon[edge].x, polygon[edge].y);
        }

        WINrender(_window);

        WINwaitForGUI(_window);
    }
}

void VECbinarizeScene (WINdow* _window, char* _inputImage, u16 _maxcolorindex, u16 _x1, u16 _y1, u16 _x2, u16 _y2, u16 _centerx, u16 _centery, float _scalex, float _scaley, char* _outputFile)
{
    BITsurface surface;

    BITsurfaceConstruct (&surface);

    BITbmpLoad (&surface, &stdAllocator, _inputImage);
    ASSERT(surface.format == BITformat_8bits);
    ASSERT(surface.height > _y2);
    ASSERT(_y1 < _y2);
    ASSERT(surface.width > _x2);
    ASSERT(_x1 < _x2);

    VECscene polygons;

    polygons.resize(1);

    u16 totaledges = 0;

    for (u16 c = 2 ; c <= _maxcolorindex ; c++)
    {
        bool found = false;
        VECpoint point;
        u32 line = _y1 * surface.pitch;


        for (u16 y = _y1 ; y <= _y2 ; y++)
        {
            for (u16 x = _x1 ; x < _x2 ; x++)
            {
                if (surface.buffer[line + x] == c)
                {
                    if (found)
                    {
                        printf("double defined : c = %d x = %d ; y = %d previous x = %d ; y = %d\n", c, x, y, point.x, point.y);

                        WINdrawImage(_window, surface.buffer, surface.width, surface.height, 8, surface.lut.data.p, 0, 0);
                        WINsetColor(_window,255,0,255);
                        WINfilledRectangle (_window, x - 5, y - 5, x + 5, y + 5);
                        WINfilledRectangle (_window, point.x - 5, point.y - 5, point.x + 5, point.y + 5);
                        WINrender(_window);
                        WINwaitForGUI(_window);
                    }
                    found = true;
                    point.x = x;
                    point.y = y;
                }
            }

            line += surface.pitch;
        }

        if (found)
        {
            polygons.back().push_back(point);
        }
        else if (polygons.back().size() > 0)
        {
            if (polygons.back().size() <= 2)
            {
                WINsetColor(_window,0,255,255);
                for (const auto& it : polygons.back())
                {
                    WINfilledRectangle (_window, it.x - 5, it.y - 5, it.x + 5, it.y + 5);
                }
            }
            else
            {
                totaledges += (u16) polygons.back().size();
                polygons.push_back(std::vector<VECpoint>());
            }
        }
    }

    if (polygons.back().size() == 0)
    {
        polygons.pop_back();
    }

    printf ("found %d polygons for %d edges\n", polygons.size(), totaledges);

    VECwriteScene (polygons, _centerx, _centery, _scalex, _scaley, _outputFile);

    VECdisplayScene (_window, surface, polygons, _centerx, _centery);

    BITsurfaceDestroy (&surface);
}
