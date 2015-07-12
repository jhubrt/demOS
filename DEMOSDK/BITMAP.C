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
