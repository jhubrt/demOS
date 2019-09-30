/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2018 J.Hubert

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


#include "DEMOSDK\BASTYPES.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\BITMAP.H"


void BITpl2chunk(void* _src, u16 _h, u16 _nbchunks, u16 _endlinepitch, void* _dst) PCSTUB;

#ifdef DEMOS_UNITTEST
void BITunitTest (void)
{
	FILE* file;
	u16* dest = (u16*)(((u32)*HW_VIDEO_BASE_H << 16) | ((u32)*HW_VIDEO_BASE_M << 8) | ((u32)*HW_VIDEO_BASE_L));

	void* image = malloc(32000);

	*HW_VIDEO_MODE = HW_VIDEO_MODE_4P;

	file = fopen ("d:\\MANGA.PLN", "rb");
	fread(image, 32, 1, file);
	memcpy (HW_COLOR_LUT, image, 32);
	fread(image, 32000, 1, file);
	fclose (file);

	BITpl2chunk(image, 200, 20, 0, dest);

	free(image);

	while(1);
}
#endif
