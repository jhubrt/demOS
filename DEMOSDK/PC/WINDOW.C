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

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <windowsx.h>
#include <malloc.h>

#undef MEM_FREE /* collides with our MEM_FREE... */

#include "DEMOSDK/PC/WINDOW.H"
#include "DEMOSDK/HARDWARE.H"

#define WINDOW_KEYBUFFER_SIZE 16
#define WINDOW_MAXTITLELEN    256

STRUCT(WINdow)
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

	_m->instance    = _param->hInstance;

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
		s32   titleH;
		RECT  clientRect;
		RECT  windowRect;

		GetClientRect ( _m->window, &clientRect );
		GetWindowRect ( _m->window, &windowRect );

		titleH = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);

		SetWindowPos( _m->window, HWND_TOP, _param->x, _param->y, _param->w, _param->h + titleH, SWP_SHOWWINDOW);

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

void WINdrawImage (WINdow* _m, void* _image, u32 _width, u32 _height, u32 _bitsPerPixel, u8* _palette, u32 _x, u32 _y)
{
	u8 buffer [ sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD) ];
	BITMAPINFO * bmpi = (BITMAPINFO *) buffer;


	bmpi->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
	bmpi->bmiHeader.biWidth         = _width;
	bmpi->bmiHeader.biHeight        = -(s32)_height;    // negative => top-down
	bmpi->bmiHeader.biPlanes        = 1;
	bmpi->bmiHeader.biBitCount      = (u16)_bitsPerPixel;
	bmpi->bmiHeader.biCompression   = BI_RGB;
	bmpi->bmiHeader.biSizeImage     = _width * _height * (_bitsPerPixel == 32 ? 4 : 1);
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

void WInpoint (WINdow* _m, s32 _iX, s32 _iY)
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


void WINrender(WINdow* _m, u32 _waitms)
{
	BitBlt (_m->windowDC, 0, 0, _m->width, _m->height, _m->pixmapDC, 0, 0, SRCCOPY);       
    Sleep (_waitms);
}

static s32 WINconvertMapping (WPARAM _wparam)
{
    s32 key = -1;

    switch (_wparam)
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

    case 'W':               key = HW_KEY_Z;                 break;
    case 'X':               key = HW_KEY_X;                 break;
    case 'C':               key = HW_KEY_C;                 break;
    case 'V':               key = HW_KEY_V;                 break;
    case 'B':               key = HW_KEY_B;                 break;
    case 'N':               key = HW_KEY_N;                 break;
    case '?':
    case ',':               key = HW_KEY_M;                 break;

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
	WINdow *thisWindow = (WINdow*)(GetWindowLongPtr(_wnd, 0));
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
			SetWindowLongPtr (_wnd, 0, (LONG)thisWindow);

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
                thisWindow->keyExtraState = _wparam | HW_KEYBOARD_KEYRELEASE;
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

BOOL Window_initInstance (WINdow* _m, HINSTANCE _instance, int _cmdShow)
{
	HWND wnd;


	wnd = CreateWindow( WINDOWCLASS, _m->title, WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, _instance, _m);

	if (!wnd)
	{
		return FALSE;
	}

	ShowWindow(wnd, _cmdShow);
	UpdateWindow(wnd);

	return TRUE;
}

ATOM Window_registerClass (WINdow* _m, HINSTANCE _hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC) Window_wndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= sizeof(void*) * 2;
	wcex.hInstance		= _hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= WINDOWCLASS;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

static DWORD WINAPI Window_eventThread (void* _param)
{
	WINdow *thisWindow = (WINdow*) _param;
	MSG msg;


	// Window must be created by the thread which will read message queue 
	// (there is one message queue per thread)
	Window_registerClass(thisWindow, thisWindow->instance);

	// Perform application initialization
	if ( !Window_initInstance (thisWindow, thisWindow->instance, SW_SHOWNORMAL) ) 
	{
		thisWindow->isValid = false;
		thisWindow->isReady = true;
	}
	else
	{      
		// message loop
		bool stop = false;

		while (!stop)
		{
			if (thisWindow->isDestroyed)
			{
				thisWindow->thread = INVALID_HANDLE_VALUE;
				DestroyWindow (thisWindow->window);
				stop = true;
			}
			else
			{
				while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
					{
						thisWindow->thread = INVALID_HANDLE_VALUE;
						TranslateMessage (&msg);
						DispatchMessage (&msg);
						stop = true;
					}
					else
					{
						TranslateMessage (&msg);
						DispatchMessage (&msg);
					}
				}

				Sleep (1);
			}
		}
	}

	return 0;
}
