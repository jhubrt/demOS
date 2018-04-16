/*------------------------------------------------------------------------------  -----------------
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
------------------------------------------------------------------------------------------------- */

/*! @brief @ref WINDOW @file */

/*! @defgroup WINDOW
   
    provides a very simple display window implementation for PC applications
*/

#ifndef WINDOW_H
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
void WINdrawImage		 (WINdow* _m, void* _image, u32 _width, u32 _height, u32 _nbBitsPerPixel, void* _palette, u32 _x, u32 _y);
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

void WINwaitForGUI 		 (WINdow* _m);

#endif // __TOS__

#endif
