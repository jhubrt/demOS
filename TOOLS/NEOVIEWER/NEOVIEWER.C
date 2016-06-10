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

#include "DEMOSDK\BASTYPES.H"
#include "DEMOSDK\STANDARD.H"

#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\SURFACE.H"


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
		void* buffer  = malloc(32000);
		u32*  image   = (u32*) malloc(640*480*4);
		u16 lut[16];
		char ext[16];

		FILE* file = fopen (argv[1], "rb");

		_splitpath(argv[1], NULL, NULL, NULL, ext);

		if (_strcmpi(ext, ".NEO") == 0)
		{
			fseek (file, 4, SEEK_SET);
			fread (lut, 32, 1, file);

			fseek (file, 128, SEEK_SET);
		}
		else 
		{
			fseek (file, 2, SEEK_SET);
			fread (lut, 32, 1, file);
		}

		fread (buffer, 32000, 1, file);

		fclose(file);

		init.x = WINDOW_CENTER;
		init.y = WINDOW_CENTER;
		init.w = 384;
		init.h = 288;
		init.title = argv[1];
		init.hInstance = NULL;

		{
            BITsurface surface, surface2;
			WINdow* window = WINconstruct (&init);


            BITsurfaceConstruct(&surface);
            BITsurfaceConstruct(&surface2);

            BITsurfaceSetExternal(&surface, buffer, BITformat_Chunk4P, 320, 200, 160);
            BITlutSetExternal(&surface.lut, BITlutFormat_STe, lut, 16);

            BITsurfaceConvert(&stdAllocator, &surface, &surface2, BITformat_8bits);

			WINdrawImage (window, surface2.buffer, 320, 200, 8, surface2.lut.data.p, (384 - 320) / 2, (288- 200) / 2);
			WINrender (window, 0);
			{
				s32 k = 0;

				do
				{
					WINgetMouse (window, NULL, NULL, &k, NULL);
				}
				while (!WINisKeyHit(window) && !WINisClosed(window) && !k);
			}

            BITsurfaceDestroy(&surface2);
            BITsurfaceDestroy(&surface);
		}
	}

	return 0;
}
