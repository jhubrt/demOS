/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2021 J.Hubert

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

#undef ARRAYSIZE
#undef MEM_FREE		/* collides with our MEM_FREE... */

#ifndef _CRT_SECURE_NO_WARNINGS
#	define _CRT_SECURE_NO_WARNINGS
#endif

#include <windows.h>
#include <windowsx.h>
#include <malloc.h>

#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\HARDWARE.H"

#define WINDOW_KEYBUFFER_SIZE 16
#define WINDOW_MAXTITLELEN    256

struct WINdow_
{
	HWND window;

	volatile bool isReady;
	volatile bool isDestroyed;
	bool isValid;

	HINSTANCE instance;

    HANDLE pixmap;
	HDC pixmapDC;
	HDC windowDC;

    COLORREF color;
	HBRUSH brush;
	HPEN   pen;

	HANDLE thread;
	DWORD threadId;

	u32 width;
	u32 height;

	volatile s32 mouseX;
	volatile s32 mouseY;
	volatile s32 mouseK;
	volatile s32 mouseZ;
	volatile u32 controlKeys;
	volatile u32 keyBuffer[WINDOW_KEYBUFFER_SIZE];
	volatile u32 keyBufferLen;
    volatile u32 keyState;
    volatile u32 keyExtraState;

	char title[WINDOW_MAXTITLELEN];
};

static DWORD WINAPI Window_eventThread (void* _param);

#define WINDOWCLASS "TEST_WINDOW"
#define TITLEBAR_H  20


WINdow* WINconstruct (WINinitParam* _param)
{
	WINdow* _m = (WINdow*) malloc(sizeof(WINdow));

	_m->instance    = (HINSTANCE) _param->hInstance;

    _m->isDestroyed = false;
	_m->mouseX      = 0;
	_m->mouseY      = 0;
	_m->mouseK      = 0;
	_m->mouseZ      = 0;
	_m->keyBufferLen = 0;
	_m->controlKeys  = 0;
	_m->isReady      = false;
	_m->isValid     = true;
	_m->window      = NULL;
	_m->windowDC    = NULL;
	_m->thread      = INVALID_HANDLE_VALUE;
	_m->pixmapDC    = NULL;
	_m->pixmap      = INVALID_HANDLE_VALUE;
	_m->color       = 0xFFFFFFFF;
	_m->pen         = 0;
	_m->brush       = 0;
    _m->keyState    = 0;
    _m->keyExtraState = 0;
	
	_m->width       = _param->w;
	_m->height      = _param->h;
	
	if (_param->title == NULL)
	{
		strcpy (_m->title, "");
	}
	else
	{
		strncpy (_m->title, _param->title, WINDOW_MAXTITLELEN);
		_m->title [WINDOW_MAXTITLELEN - 1] = 0;
	}

	if (_param->x == WINDOW_CENTER)
	{
		RECT rect;
		HWND root = GetDesktopWindow();

		GetWindowRect(root, &rect);

		_param->x = (rect.left + rect.right  - _m->width ) / 2;
		_param->y = (rect.top  + rect.bottom - _m->height) / 2;    
	}

	// Launches messages dispatching thread
    _m->thread = CreateThread (NULL, 32768, Window_eventThread, (void*)_m, 0, &_m->threadId );              

	// and wait for the window creation
	while (_m->isReady == false)
	{
		Sleep (1);
	}

	if ( _m->isValid )
	{
		s32   titleH, borderW;
		RECT  clientRect;
		RECT  windowRect;

		GetClientRect ( _m->window, &clientRect );
		GetWindowRect ( _m->window, &windowRect );

		borderW = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
		titleH  = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);

		SetWindowPos( _m->window, HWND_TOP, _param->x, _param->y, _param->w + borderW, _param->h + titleH, SWP_SHOWWINDOW);

		Sleep(100);
	}

	return _m;
}


void WINdestroy (WINdow* _m)
{
	if ( _m->pen != 0 )
	{
		DeleteObject (_m->pen);
	}

	if ( _m->brush != 0 )
	{
		DeleteObject (_m->brush);
	}

	if (_m->thread != INVALID_HANDLE_VALUE)
	{
		_m->isDestroyed = true; //Thread will finish alone
		WaitForSingleObject (_m->thread, 2000); //2 secondes should be sufficient to destroy a window        }
	}
}


void* WINgetWindowHandle (WINdow* _m)
{
    return _m->window;
}



static void WINinsertKey (WINdow* _m, u32 _key)
{
	if ( _m->keyBufferLen < WINDOW_KEYBUFFER_SIZE )
	{
		memmove ((void*)(_m->keyBuffer + 1), (void*)_m->keyBuffer, (WINDOW_KEYBUFFER_SIZE - 1) * sizeof(u32));

		*_m->keyBuffer = _key;
		_m->keyBufferLen++;        
	}
}

void WINwaitLoop (WINdow* _m)
{
	if (_m->thread != INVALID_HANDLE_VALUE)
	{
		//The message loop is done by the other thread
		WaitForSingleObject (_m->thread, INFINITE); //Passive wait
	}
}


void WINclear (WINdow* _m)
{
	WINfilledRectangle (_m, 0,0,_m->width, _m->height);
}

void WINdrawImage (WINdow* _m, void* _image, u32 _width, u32 _height, u32 _bitsPerPixel, void* _palette, u32 _x, u32 _y)
{
	u8 buffer [ sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD) ];
	BITMAPINFO * bmpi = (BITMAPINFO *) buffer;


	bmpi->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
	bmpi->bmiHeader.biWidth         = _width;
	bmpi->bmiHeader.biHeight        = -(s32)_height;    // negative => top-down
	bmpi->bmiHeader.biPlanes        = 1;
	bmpi->bmiHeader.biBitCount      = (u16)_bitsPerPixel;
	bmpi->bmiHeader.biCompression   = BI_RGB;
    switch (_bitsPerPixel)
    {
    case 32:
        bmpi->bmiHeader.biSizeImage = _width * _height * 4;
        break;
    case 1:
        bmpi->bmiHeader.biSizeImage = _width * _height / 8;
        break;
    default:
        bmpi->bmiHeader.biSizeImage = _width * _height;
        break;
    }
	bmpi->bmiHeader.biXPelsPerMeter = 0;
	bmpi->bmiHeader.biYPelsPerMeter = 0;
	bmpi->bmiHeader.biClrUsed       = 0;
	bmpi->bmiHeader.biClrImportant  = 0;

	if ((_palette != NULL) && (bmpi->bmiHeader.biBitCount == 8))
	{
		memcpy (bmpi->bmiColors, _palette, 256*sizeof (RGBQUAD));
	}

	SetDIBitsToDevice(  
		_m->pixmapDC, 
		_x, _y, _width, _height, 
		0, 0, 
		0, _height, 
		_image, 
		bmpi, 
		DIB_RGB_COLORS);
}

void WINfilledRectangle (WINdow* _m, s32 _x1, s32 _y1, s32 _x2, s32 _y2)
{
	RECT rect;


	rect.left   = _x1;
	rect.top    = _y1;
	rect.right  = _x2;
	rect.bottom = _y2;

	SelectObject (_m->pixmapDC, _m->brush);

	FillRect(_m->pixmapDC, &rect, _m->brush);
}

void WINgetMouse (WINdow* _m, s32* _x, s32* _y, s32* _k, s32* _z)
{
	if ( _x != NULL )
	{
		(*_x) = _m->mouseX;
	}

	if ( _y != NULL )
	{
		(*_y) = _m->mouseY;
	}

	if ( _k != NULL )
	{
		(*_k) = _m->mouseK | _m->controlKeys;
	}

	if ( _z != NULL )
	{
		(*_z) = _m->mouseZ;
	}

}

bool WINisKeyHit (WINdow* _m)
{
	return _m->keyBufferLen > 0;
}

u32 WINgetKey (WINdow* _m)
{
	u32 uiKey;

	if ( _m->keyBufferLen == 0 )
	{
		do
		{
		}
		while (_m->keyBufferLen == 0);
	}

	_m->keyBufferLen--;

	uiKey = _m->keyBuffer[_m->keyBufferLen];

	memmove ((void*) &_m->keyBuffer[0], (void*)&_m->keyBuffer[1], WINDOW_KEYBUFFER_SIZE * sizeof(u32));

	return uiKey;
}

u32 WINgetKeyState (WINdow* _m)
{
    return _m->keyState;
}

u32 WINgetKeyExtraState (WINdow* _m)
{
    return _m->keyExtraState;
}

void WINresetKeyExtraState (WINdow* _m)
{
	_m->keyExtraState = 0;
}


u32 WINgetControlKeys (WINdow* _m)
{
	return _m->controlKeys;
}

void WINline (WINdow* _m, s32 _x1, s32 _y1, s32 _x2, s32 _y2)
{
	// Optimize trivial cases
	if (( _x1 < 0 ) && ( _x2 < 0 ))
		return;

	if (( _y1 < 0 ) && ( _y2 < 0 ))
		return;

	if (( _x1 > (int)_m->width ) && ( _x2 > (int)_m->width ))
		return;

	if (( _y1 > (int)_m->height ) && ( _y2 > (int)_m->height ))
		return;

	SelectObject  (_m->pixmapDC, _m->pen);

	MoveToEx (_m->pixmapDC, _x1, _y1, NULL);
	LineTo   (_m->pixmapDC, _x2, _y2);   
}

void WINpoint (WINdow* _m, s32 _iX, s32 _iY)
{
	SetPixel(_m->pixmapDC, _iX, _iY, _m->color);
}

void WINrectangle (WINdow* _m, s32 _x1, s32 _y1, s32 _x2, s32 _y2)
{
	SelectObject  (_m->pixmapDC, _m->pen);
	SelectObject  (_m->pixmapDC, GetStockObject(NULL_BRUSH)); 

	Rectangle(_m->pixmapDC, _x1, _y1, _x2, _y2);
}

void WINsetColor (WINdow* _m, u8 _r, u8 _g, u8 _b)
{
	DWORD color = RGB(_r,_g,_b);

	if ( _m->color != color )
	{
		_m->color = color;

		if ( _m->pen != 0 )
		{
			DeleteObject (_m->pen);
		}

		_m->pen = CreatePen (PS_SOLID, 1, _m->color);

		if ( _m->brush != 0 )
		{
			DeleteObject (_m->brush);
		}

		_m->brush = CreateSolidBrush ( _m->color);
	}
}

void WINtext (WINdow* _m, s32 _x, s32 _y, char* _string)
{
	SetTextColor ( _m->pixmapDC, _m->color);
	SetTextAlign ( _m->pixmapDC, TA_LEFT | TA_TOP);
	SetBkMode    ( _m->pixmapDC, TRANSPARENT );
	TextOut      ( _m->pixmapDC, _x, _y, _string, strlen (_string) ); 

}

bool WINisClosed (WINdow* _m)
{
	return !_m->isReady;
}


void WINrender(WINdow* _m)
{
	BitBlt (_m->windowDC, 0, 0, _m->width, _m->height, _m->pixmapDC, 0, 0, SRCCOPY);       
}

static s32 WINconvertMapping (WPARAM _wparam)
{
    s32 key = -1;

    switch (_wparam)	// implemented for AZERTY keyboard here : would need proper convertion accorfing to current keyboard layout
    {
    case VK_DOWN:           key = HW_KEY_DOWN;              break;
    case VK_UP:             key = HW_KEY_UP;                break;
    case VK_RIGHT:          key = HW_KEY_RIGHT;             break;
    case VK_LEFT:           key = HW_KEY_LEFT;              break;
    case VK_HOME:           key = HW_KEY_HOME;              break;
    case VK_INSERT:         key = HW_KEY_INSERT;            break;
    case VK_DELETE:         key = HW_KEY_DELETE;            break;
    case VK_PRIOR:          key = HW_KEY_HELP;              break;
    case VK_NEXT:           key = HW_KEY_UNDO;              break;
    case VK_NUMPAD0:        key = HW_KEY_NUMPAD_0;          break;
    case VK_NUMPAD1:        key = HW_KEY_NUMPAD_1;          break;
    case VK_NUMPAD2:        key = HW_KEY_NUMPAD_2;          break;
    case VK_NUMPAD3:        key = HW_KEY_NUMPAD_3;          break;
    case VK_NUMPAD4:        key = HW_KEY_NUMPAD_4;          break;
    case VK_NUMPAD5:        key = HW_KEY_NUMPAD_5;          break;
    case VK_NUMPAD6:        key = HW_KEY_NUMPAD_6;          break;
    case VK_NUMPAD7:        key = HW_KEY_NUMPAD_7;          break;
    case VK_NUMPAD8:        key = HW_KEY_NUMPAD_8;          break;
    case VK_NUMPAD9:        key = HW_KEY_NUMPAD_9;          break;
    case VK_BACK:           key = HW_KEY_BACKSPACE;         break;
    case VK_TAB:            key = HW_KEY_TAB;               break;
    case VK_RETURN:         key = HW_KEY_RETURN;            break;
    case VK_MULTIPLY:       key = HW_KEY_NUMPAD_MULTIPLY;   break;
    case VK_DIVIDE:         key = HW_KEY_NUMPAD_DIVIDE;     break;
    case VK_SUBTRACT:       key = HW_KEY_NUMPAD_MINUS;      break;
    case VK_ADD:            key = HW_KEY_NUMPAD_PLUS;       break;
    case VK_SEPARATOR:      key = HW_KEY_NUMPAD_ENTER;      break;
    case VK_SPACE:          key = HW_KEY_SPACEBAR;          break;
    case VK_ESCAPE:         key = HW_KEY_ESC;               break;

    case VK_F1:             key = HW_KEY_F1 ;               break;
    case VK_F2:             key = HW_KEY_F2 ;               break;
    case VK_F3:             key = HW_KEY_F3 ;               break;
    case VK_F4:             key = HW_KEY_F4 ;               break;
    case VK_F5:             key = HW_KEY_F5 ;               break;
    case VK_F6:             key = HW_KEY_F6 ;               break;
    case VK_F7:             key = HW_KEY_F7 ;               break;
    case VK_F8:             key = HW_KEY_F8 ;               break;
    case VK_F9:             key = HW_KEY_F9 ;               break;
    case VK_F10:            key = HW_KEY_F10;               break;

    case 'A':               key = HW_KEY_Q;                 break;
    case 'Z':               key = HW_KEY_W;                 break;
    case 'E':               key = HW_KEY_E;                 break;
    case 'R':               key = HW_KEY_R;                 break;
    case 'T':               key = HW_KEY_T;                 break;
    case 'Y':               key = HW_KEY_Y;                 break;
    case 'U':               key = HW_KEY_U;                 break;
    case 'I':               key = HW_KEY_I;                 break;
    case 'O':               key = HW_KEY_O;                 break;
    case 'P':               key = HW_KEY_P;                 break;

    case 'Q':               key = HW_KEY_A;                 break;
    case 'S':               key = HW_KEY_S;                 break;
    case 'D':               key = HW_KEY_D;                 break;
    case 'F':               key = HW_KEY_F;                 break;
    case 'G':               key = HW_KEY_G;                 break;
    case 'H':               key = HW_KEY_H;                 break;
    case 'J':               key = HW_KEY_J;                 break;
    case 'K':               key = HW_KEY_K;                 break;
    case 'L':               key = HW_KEY_L;                 break;
    case 'M':               key = HW_KEY_SEMICOLON;         break;
	case VK_OEM_3:          key = HW_KEY_AT;		        break;	
	case VK_OEM_5:			key = HW_KEY_SHARP;				break;

	case VK_OEM_102:		key = HW_KEY_ANTI_SLASH;		break;
    case 'W':               key = HW_KEY_Z;                 break;
    case 'X':               key = HW_KEY_X;                 break;
    case 'C':               key = HW_KEY_C;                 break;
    case 'V':               key = HW_KEY_V;                 break;
    case 'B':               key = HW_KEY_B;                 break;
    case 'N':               key = HW_KEY_N;                 break;
    case VK_OEM_COMMA:      key = HW_KEY_M;                 break;
    case VK_OEM_PERIOD:     key = HW_KEY_COMMA;             break;
    case VK_OEM_2:          key = HW_KEY_DOT;               break;
    case VK_OEM_8:          key = HW_KEY_SLASH;             break;

    case '0':               key = HW_KEY_0;                 break;
    case '1':               key = HW_KEY_1;                 break;
    case '2':               key = HW_KEY_2;                 break;
    case '3':               key = HW_KEY_3;                 break;
    case '4':               key = HW_KEY_4;                 break;
    case '5':               key = HW_KEY_5;                 break;
    case '6':               key = HW_KEY_6;                 break;
    case '7':               key = HW_KEY_7;                 break;
    case '8':               key = HW_KEY_8;                 break;
    case '9':               key = HW_KEY_9;                 break;

	case VK_OEM_4:			key = HW_KEY_MINUS;				break;
	case VK_OEM_MINUS:      key = HW_KEY_MINUS;             break;
    case VK_OEM_PLUS:       key = HW_KEY_EQUAL;             break;

    case VK_OEM_6:          key = HW_KEY_BRACKET_LEFT;      break;
    case VK_OEM_1:          key = HW_KEY_BRACKET_RIGHT;     break;
    }
    
    return key;
}


typedef enum WINmappingTypeEnum_
{
    WINmappingType_ASCII,
    WINmappingType_KEY,
    WINmappingType_EXTRAKEY,
    WINmappingType_CTRLKEY
} WINmappingTypeEnum;


static WINmappingTypeEnum WINgetMappingType (WPARAM _wparam)
{
    switch (_wparam)
    {
    case VK_DOWN:   
    case VK_UP:
    case VK_RIGHT:
    case VK_LEFT:
    case VK_HOME:
    case VK_INSERT:
    case VK_DELETE:
    case VK_PRIOR:
    case VK_NEXT:
        return WINmappingType_KEY;

    case VK_END:
    case VK_PAUSE:
    case VK_SCROLL:
    case VK_OEM_7:
	case VK_F11:
		return WINmappingType_EXTRAKEY;

    case VK_MENU:
    case VK_CONTROL:
    case VK_SHIFT:
        return WINmappingType_CTRLKEY;

    default:
        return WINmappingType_ASCII;
    }
}


LRESULT CALLBACK Window_wndProc (HWND _wnd, UINT _message, WPARAM _wparam, LPARAM _lparam)
{   
	WINdow *thisWindow = (WINdow*)(GetWindowLongPtr(_wnd, GWLP_USERDATA));
	bool  processDefault = false;


	switch (_message) 
	{
	case WM_CREATE: 
		{
			LPCREATESTRUCT param = (LPCREATESTRUCT) _lparam;

			thisWindow           = (WINdow*) param->lpCreateParams;            
			thisWindow->window   = _wnd;
			thisWindow->windowDC = GetDC (_wnd);
			thisWindow->pixmapDC = CreateCompatibleDC ( thisWindow->windowDC );
			thisWindow->pixmap   = CreateCompatibleBitmap (thisWindow->windowDC, thisWindow->width, thisWindow->height);

			SelectObject (thisWindow->pixmapDC, thisWindow->pixmap);
			SetWindowLongPtr (_wnd, GWLP_USERDATA, (LONG)thisWindow);

			processDefault = true;

			thisWindow->isReady = true;
		}
		break;

	case WM_DESTROY:
		{
			if ( thisWindow->pixmap != INVALID_HANDLE_VALUE )
			{
				HANDLE pixmap = thisWindow->pixmap;
				thisWindow->pixmap = INVALID_HANDLE_VALUE;
				DeleteObject (pixmap);
			}

			if ( thisWindow->pixmapDC != NULL )
			{
				HDC pixmapDC = thisWindow->pixmapDC;
				thisWindow->pixmapDC = NULL;
				DeleteDC (pixmapDC);
			}

			if ( thisWindow->windowDC != NULL )
			{
				HDC winDC = thisWindow->windowDC;
				thisWindow->windowDC = NULL;
				ReleaseDC (thisWindow->window, winDC);
			}

			PostQuitMessage (0);
		}
		break;

	case WM_PAINT:
		{
			if ( thisWindow != NULL)
			{
				PAINTSTRUCT ps;
				HDC hdc;


				hdc = BeginPaint(_wnd, &ps);

				BitBlt (hdc, 0, 0, thisWindow->width, thisWindow->height, thisWindow->pixmapDC, 0, 0, SRCCOPY);       

				EndPaint(_wnd, &ps);
			}
		}
		break;

	case WM_MOUSEMOVE:
		{
			processDefault = true;

			if ( thisWindow != NULL)
			{
                s32 mouseK = 0;   // also manage buttons here to detect when buttons has been released outside of the window

                thisWindow->mouseX  = GET_X_LPARAM ( _lparam ); 
                thisWindow->mouseY  = GET_Y_LPARAM ( _lparam ); 

                if (_wparam & MK_LBUTTON)
                    mouseK |= MOUSE_LBUT;

                if (_wparam & MK_MBUTTON)
                    mouseK |= MOUSE_MBUT;

                if (_wparam & MK_RBUTTON)
                    mouseK |= MOUSE_RBUT;

                thisWindow->mouseK = mouseK; 

                processDefault = false;
			}
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		{
			processDefault = true;

			if ( thisWindow != NULL)
			{
                thisWindow->mouseK |=  _message == WM_LBUTTONDOWN; 
                thisWindow->mouseK |= (_message == WM_RBUTTONDOWN) << 1; 
                thisWindow->mouseK |= (_message == WM_MBUTTONDOWN) << 2; 

                processDefault = false;
			}
		}
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		{
			processDefault = true;

			if ( thisWindow != NULL)
			{
                thisWindow->mouseK &= ~(s32) (_message == WM_LBUTTONUP); 
                thisWindow->mouseK &= ~(s32)((_message == WM_RBUTTONUP) << 1); 
                thisWindow->mouseK &= ~(s32)((_message == WM_MBUTTONUP) << 2); 

                processDefault = false;
			}
		}
		break;

	case WM_MOUSEWHEEL:
		{
			processDefault = true;

			if ( thisWindow != NULL)
			{
                s16 value = (s16)(_wparam >> 16);

                thisWindow->mouseZ += (s32) value;

                processDefault = false;
			}
		}
		break;

    case WM_KEYDOWN:

        processDefault = true;

        if ( thisWindow != NULL)
        {
            if ((_lparam & (1 << 30)) == 0) // do not key auto key repeat signal
            {
                WINmappingTypeEnum mappingType = WINgetMappingType (_wparam);
                s32 key = WINconvertMapping (_wparam);

                if ( key != -1 )
                {
                    thisWindow->keyState = key;
                }

                switch ( mappingType )
                {
                case WINmappingType_EXTRAKEY:
                    thisWindow->keyExtraState = _wparam;
                    break;

                case WINmappingType_KEY:
                    {
                        s32 i;

                        for (i = 0 ; i < (_lparam & 15) ; i++)
                        {
                            WINinsertKey(thisWindow, key);
                        }

                        processDefault = false;
                    }
                    break;

                case WINmappingType_CTRLKEY:
                    {
                        processDefault = false;

                        switch (_wparam)
                        {
                        case VK_MENU:       thisWindow->controlKeys |= CONTROLKEY_ALT;      break;
                        case VK_CONTROL:    thisWindow->controlKeys |= CONTROLKEY_CTRL;     break;
                        case VK_SHIFT:      thisWindow->controlKeys |= CONTROLKEY_SHIFT;    break;
                        }
                    }
                    break;
                }
            }
        }

		break;

	case WM_KEYUP:

		processDefault = true;

		if ( thisWindow != NULL)
        {
            WINmappingTypeEnum mappingType = WINgetMappingType (_wparam);
            s32 key = WINconvertMapping (_wparam) | HW_KEYBOARD_KEYRELEASE;

            if ( key != -1 )
            {
                thisWindow->keyState = key | HW_KEYBOARD_KEYRELEASE;
            }
            
            thisWindow->keyExtraState = 0;

            switch ( mappingType )
            {
            case WINmappingType_CTRLKEY:
                {
                    processDefault = false;

                    switch (_wparam)
                    {
                    case VK_MENU:       thisWindow->controlKeys &= ~CONTROLKEY_ALT;     break;
                    case VK_CONTROL:    thisWindow->controlKeys &= ~CONTROLKEY_CTRL;    break;
                    case VK_SHIFT:      thisWindow->controlKeys &= ~CONTROLKEY_SHIFT;   break;
                    }
                }
                break;

            case WINmappingType_EXTRAKEY:
                thisWindow->keyExtraState = _wparam | 0x100;
                break;
            }
		}
		break;

	case WM_CHAR:
		{
			processDefault = true;

			if ( thisWindow != NULL)
			{
                s32 i;


                for (i = 0 ; i < (_lparam & 15) ; i++)
                {
                    WINinsertKey(thisWindow, (u32)_wparam);
                }

                processDefault = false;
			}
		}
		break;

	case WM_CLOSE:
		thisWindow->isReady = false;

		processDefault = true;
		break;

	default:
		processDefault = true;
		break;
	}

	if ( processDefault )
	{
		return DefWindowProc(_wnd, _message, _wparam, _lparam);
	}
	else
	{
		return 0;
	}

}

static DWORD WINAPI Window_eventThread (void* _param)
{
	WINdow *thisWindow = (WINdow*) _param;
	MSG msg;


	// Window must be created by the thread which will read message queue 
	// (there is one message queue per thread)
	WNDCLASSEX wcex;

	wcex.cbSize		= sizeof(wcex); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC) Window_wndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= thisWindow->instance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH) GetStockObject (BLACK_BRUSH);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= WINDOWCLASS;
	wcex.hIconSm		= NULL;

	if (!RegisterClassEx(&wcex))
	{
		thisWindow->isValid = false;
		thisWindow->isReady = true;
		return 0;
 	}

	HWND wnd = CreateWindow( WINDOWCLASS, thisWindow->title, WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, thisWindow->instance, thisWindow);

	if (!wnd)
	{
		thisWindow->isValid = false;
		thisWindow->isReady = true;
		return 0;
	}

	ShowWindow(wnd, SW_SHOWNORMAL);
	UpdateWindow(wnd);

    // message loop
    bool stop = false;

    while (!stop)
    {
        if (thisWindow->isDestroyed)
        {
            thisWindow->thread = INVALID_HANDLE_VALUE;
            DestroyWindow(thisWindow->window);
            stop = true;
        }
        else
        {
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    thisWindow->thread = INVALID_HANDLE_VALUE;
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                    stop = true;
                }
                else
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }

            Sleep(1);
        }
    }

	return 0;
}


void WINwaitForGUI (WINdow* _m)
{
    s32 k = 0;

    do
    {
        WINgetMouse (_m, NULL, NULL, &k, NULL);
    }
    while (!WINisKeyHit(_m) && !WINisClosed(_m) && !k);

    if ( !WINisClosed(_m) )
    {
        while ( WINisKeyHit(_m) )
        {
            WINgetKey(_m);
        }

        do
        {
            WINgetMouse (_m, NULL, NULL, &k, NULL);
        }
        while (k);
    }
}


void WINwait(u32 _ms)
{
	Sleep(_ms);
}
