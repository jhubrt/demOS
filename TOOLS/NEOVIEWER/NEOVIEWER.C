/*------------------------------------------------------------------------------  -----------------
  The MIT License (MIT)

  Copyright (c) 2015-2017 J.Hubert

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

#define _CRT_SECURE_NO_WARNINGS
#include <conio.h>

#include "DEMOSDK\BASTYPES.H"
#include "DEMOSDK\STANDARD.H"

#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\SURFACE.H"
#include "DEMOSDK\PC\BMPLOADER.H"


static void* stdAlloc(void* _alloc, u32 _size)
{
	IGNORE_PARAM(_alloc);
	return malloc(_size);
}

static void stdFree(void* _alloc, void* _adr)
{
	IGNORE_PARAM(_alloc);
	free(_adr);
}

MEMallocator stdAllocator = { NULL, stdAlloc, stdFree };


int main(int argc, char** argv)
{
	if ( argc == 2 )
	{
		WINinitParam init;
		char drive[256];
		char subdir[256];
		char filename[256];
		char ext[256];
        char title[512];
        BITsurface surface, surface2;
        BITloadResult result;


        BITsurfaceConstruct(&surface);
        BITsurfaceConstruct(&surface2);

        _splitpath(argv[1], drive, subdir, filename, ext);

		if (_strcmpi(ext, ".NEO") == 0)
		{
            result = BITneoLoad(&surface, &stdAllocator, argv[1]);
		}
		else 
		{
            result = BITdegasLoad(&surface, &stdAllocator, argv[1]);
		}

        if ( result != BITloadResult_OK )
        {
            printf ("ERROR: cannot load file %s\n", argv[1]);
            _getch();
        }
        else
        {
            sprintf(title, "NeoViewer - %s [press S to save bmp]", argv[1]);

		init.x = WINDOW_CENTER;
		init.y = WINDOW_CENTER;
		init.w = 384;
		init.h = 288;
		init.title = argv[1];
		init.hInstance = NULL;

		{
			WINdow* window = WINconstruct (&init);

            BITsurfaceConvert(&stdAllocator, &surface, &surface2, BITformat_8bits);

			WINdrawImage (window, surface2.buffer, 320, 200, 8, surface2.lut.data.p, (384 - 320) / 2, (288- 200) / 2);
			WINrender (window, 0);
			{
				s32 k = 0;

				do
				{
                        if (WINisKeyHit(window))
                        {
                            switch (WINgetKey(window))                        
                            {
                            case 'S':
                            case 's':
                                {
                                    char path[512];

                                    sprintf (path, "%s%s%s_neoview.BMP", drive, subdir, filename);
                                    if ( BITbmpSave(&surface2, path) )
                                    {
                                        WINsetColor(window, 0, 255, 0);
                                        WINtext(window, 0, 0, "bmp file saved");
                                    }
                                    else
                                    {
                                        WINsetColor(window, 255, 0, 0);
                                        WINtext(window, 0, 0, "cannot save file");
                                    }
                                    WINrender (window, 0);
                                }
                            }
                        }

					WINgetMouse (window, NULL, NULL, &k, NULL);
				}
				while (!WINisKeyHit(window) && !WINisClosed(window) && !k);
			}

            BITsurfaceDestroy(&surface2);
            BITsurfaceDestroy(&surface);
		}
	}
    }

	return 0;
}
