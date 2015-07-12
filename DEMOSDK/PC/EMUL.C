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
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\PC\WINDOW.H"

#include <string.h>

STRUCT(MiniEmul)
{
	WINdow* w;

	u32 image [640*400];
	u32 imagelut [16];
};

MiniEmul g_miniEmul;


WINdow* EMULgetWindow (void)
{
	return g_miniEmul.w;
}

void EMULinit (void)
{
	WINinitParam param;

	param.x = param.y = WINDOW_CENTER;
	param.w = 768;
	param.h = 576;
	param.title = NULL;
	param.hInstance = NULL;

	g_miniEmul.w = WINconstruct (&param);
}

static void ST2PCLut (u16* _STlut, u32* _PClut)
{
	u32 i;
	
	for ( i = 0 ; i < 16 ; i++ )
	{
		u16 color = PCENDIANSWAP16 (_STlut[i]);
		u8 R = (u8)(color >> 8);
		u8 G = (u8)(color >> 4);
		u8 B = (u8)(color);

		R <<= 1;
		G <<= 1;
		B <<= 1;

		R |= (R & 0x10) != 0;
		G |= (G & 0x10) != 0;
		B |= (B & 0x10) != 0;

		R &= 0xF;
		G &= 0xF;
		B &= 0xF;

		R <<= 4;
		G <<= 4;
		B <<= 4;

		_PClut[i] = ((u32)R << 16) | ((u32)G << 8) | (u32)B;
	}
}

void EMULrender ()
{
	u32 i = 0;

	if ( WINisKeyHit( g_miniEmul.w ) )
	{
		u32 key = WINgetKey( g_miniEmul.w );

		switch (key)
		{
		case ' ':
			*HW_KEYBOARD_DATA = HW_KEY_SPACEBAR | HW_KEYBOARD_KEYRELEASE;
			break;
		case '1':
			break;
		}
	}

	ST2PCLut(g_STHardware.colorLUT, g_miniEmul.imagelut);

	WINdrawImage (g_miniEmul.w, g_miniEmul.image, 640, 400, 32, NULL, (768 - 640) / 2, (576 - 400) / 2);
	WINrender (g_miniEmul.w, 20);
}
