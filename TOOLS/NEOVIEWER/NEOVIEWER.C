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
