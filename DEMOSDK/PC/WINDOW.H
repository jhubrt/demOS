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

/*! @brief @ref WINDOW @file */

/*! @defgroup WINDOW
   
    provides a very simple display window implementation for PC applications
*/

#ifndef WINWOW_H
#define WINDOW_H

#ifndef __TOS__

#include "DEMOSDK\BASTYPES.H"

#define WINDOW_CENTER -1

#define MOUSE_LBUT  0x01
#define MOUSE_RBUT  0x02
#define MOUSE_MBUT  0x04

#define CONTROLKEY_CTRL   0x01000
#define CONTROLKEY_ALT    0x02000
#define CONTROLKEY_ALTGR  0x03000
#define CONTROLKEY_SHIFT  0x08000


STRUCT(WINinitParam)
{
	s32 x;
	s32 y;
	u32 w;
	u32 h;
	void* hInstance;
	char* title;
};

PREDECLARE_STRUCT(WINdow);

WINdow* WINconstruct	 (WINinitParam* _initParam);
void WINdestroy		     (WINdow* _m);
void WINwaitLoop		 (WINdow* _m);
void WINclear			 (WINdow* _m);
void WINdrawImage		 (WINdow* _m, void* _image, u32 _width, u32 _height, u32 _nbBitsPerPixel, u8* _palette, u32 _x, u32 _y);
void WINfilledRectangle  (WINdow* _m,s32 _iX1, s32 _iY1, s32 _iX2, s32 _iY2);
void WINline			 (WINdow* _m,s32 _iX1, s32 _iY1, s32 _iX2, s32 _iY2);
void WInpoint			 (WINdow* _m,s32 _iX, s32 _iY);
void WINrectangle		 (WINdow* _m,s32 _iX1, s32 _iY1, s32 _iX2, s32 _iY2);
void WINsetColor		 (WINdow* _m,u8 _uiR, u8 _uiG, u8 _uiB);
void WINtext			 (WINdow* _m,s32 _iX, s32 _iY, char* _string);
void WINrender			 (WINdow* _m, u32 _waitms);
bool WINisClosed		 (WINdow* _m);
void WINgetMouse		 (WINdow* _m,s32* _piX, s32* _piY, s32* _piK, s32* _z);
bool WINisKeyHit		 (WINdow* _m);
u32  WINgetKey			 (WINdow* _m);
u32  WINgetKeyState      (WINdow* _m);
u32  WINgetControlKeys	 (WINdow* _m);
u32  WINgetKeyExtraState (WINdow* _m);
void* WINgetWindowHandle (WINdow* _m);

#endif // __TOS__

#endif
