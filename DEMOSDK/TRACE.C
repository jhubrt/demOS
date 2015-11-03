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
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\HARDWARE.H"


ASMIMPORT void* SYSfontbitmap;
void SYSfastPrint(char* _s, void* _screenprintadr, u16 _screenPitch, u16 _bitplanPitch, u32 _fontadr) PCSTUB;

static void tracLoadFont (LOADdisk* _disk, u16 _resourceId)
{
	u32 size = LOADresourceRoundedSize(_disk,_resourceId);
	u16 c;

	sys.font = (u8*) RINGallocatorAlloc ( &sys.coremem, size );
	ASSERT(sys.font != NULL);

	{
		LOADrequest* request;
        void* temp = (u8*) RINGallocatorAlloc ( &sys.mem, LOADresourceRoundedSize(_disk, _resourceId) );
		ASSERT_EARLY (temp != NULL, 0x770, 0x0);

		request = LOADrequestLoad (_disk, _resourceId, temp, LOAD_PRIORITY_INORDER);
		LOADwaitRequestCompleted ( request );

		STDmcpy (sys.font, temp, size);

		RINGallocatorFree ( &sys.mem, temp );
	}

	STDmset (sys.fontChars, 0xFFFFFFFFUL, sizeof(sys.fontChars));

	for (c = 0 ; c < 26 ; c++)
	{
		sys.fontChars['A' + c] = c * 8;
		sys.fontChars['a' + c] = (c + 54) * 8;
	} 

	for (c = 0 ; c < 10 ; c++)
	{
		sys.fontChars['0' + c] = (c + 26) * 8;
	} 

	sys.fontChars['-'] = 36 * 8;
	sys.fontChars['+'] = 37 * 8;
	sys.fontChars['.'] = 38 * 8;
	sys.fontChars['\''] = 39 * 8;
	sys.fontChars['/'] = 40 * 8;
	sys.fontChars['\\'] = 40 * 8;
	sys.fontChars['*'] = 41 * 8;
	sys.fontChars['<'] = 42 * 8;
	sys.fontChars['>'] = 43 * 8;
	sys.fontChars['='] = 44 * 8;
	sys.fontChars[':'] = 45 * 8;
	sys.fontChars[';'] = 46 * 8;
	sys.fontChars[','] = 47 * 8;
	sys.fontChars['?'] = 48 * 8;
	sys.fontChars['!'] = 49 * 8;
	sys.fontChars['['] = 50 * 8;
	sys.fontChars[']'] = 51 * 8;
	sys.fontChars['%'] = 52 * 8;
	sys.fontChars['|'] = 53 * 8;
	sys.fontChars[' '] = 80 * 8;

	SYSfontbitmap = sys.font;
}


#ifdef DEMOS_DEBUG

#define TRAC_NBMAXDISPLAYSERVICES 10
#define TRAC_ASSERT_COLOR 0xA00

STRUCT(trac_DisplayService)
{
    TRAC_DisplayCallback    m_callback;
    u16                     m_enableMask;
};

static u16                 g_displayServicesNb = 0;
static trac_DisplayService g_displayServices[TRAC_NBMAXDISPLAYSERVICES];

static u16	g_activatedTraces		= 0;
static bool g_verbose				= false;


STRUCT(trac_DisplayParam)
{
	u16  planePitch;
	u16  plane;
	u16  pitch;
	u16	 h;
	u8	 refresh;
};

static trac_DisplayParam trac_displayParam;
static trac_DisplayParam trac_displayParamLast;

void TRACinit (LOADdisk* _disk, u16 _resourceId)
{
	trac_displayParam.h			 = 0;
	trac_displayParam.pitch		 = 160;
	trac_displayParam.plane		 = 0;
	trac_displayParam.planePitch = SYS_4P_BITSHIFT;
	trac_displayParam.refresh	 = 3;

	trac_displayParamLast.refresh = 0;

    STDmset (g_displayServices, 0, sizeof(g_displayServices));

    tracLoadFont (_disk, _resourceId);
}

/* Static callbacks : declared into system interface but implemented at higher level into TRACE.C... */
void SYSdebugPrint(void* _screen, u16 _screenPitch, u16 _bitplanPitchShift, u16 _col, u16 _y, char* _s)
{
    u16 bitplanPitch = 1 << _bitplanPitchShift;
	u8* adr = (u8*)_screen;


	if  (_y != 0)	
	{
		adr += _y * _screenPitch;
	}

	adr += (_col & 0xFFFE) << (_bitplanPitchShift - 1);
	adr += _col & 1;

#	ifdef __TOS__
    SYSfastPrint (_s, adr, _screenPitch, bitplanPitch, (u32)sys.fontChars);
#	else

    bitplanPitch--;

	while (*_s)
	{
		u8 c = *_s++;
		u8* d = adr;

			if ( sys.fontChars[c] != 0xFFFF )
			{
				u8* bitmap = sys.font + sys.fontChars[c];

				*d = *bitmap++;	d += _screenPitch;
				*d = *bitmap++;	d += _screenPitch;
				*d = *bitmap++;	d += _screenPitch;
				*d = *bitmap++;	d += _screenPitch;
				*d = *bitmap++;	d += _screenPitch;
				*d = *bitmap++;	d += _screenPitch;
				*d = *bitmap++;	d += _screenPitch;
				*d = *bitmap++;	d += _screenPitch;
			}

		_col++;

		if (_col & 1)
		{
			adr++;
		}
		else
		{
			adr += bitplanPitch;
		}
	}
#	endif
}

static void trac_clear (void* _image, trac_DisplayParam* _displayParam)
{
    register u16* image		= (u16*) _image;
	register u16  p			= (1 << _displayParam->planePitch) >> 1;
    register s16  nbwords	= (_displayParam->planePitch == SYS_4P_BITSHIFT) ? _displayParam->pitch >> 2 : _displayParam->pitch >> 3;


	nbwords *= _displayParam->h;

	while (nbwords > 0)
	{
		*image = 0;
		image += p;

		*image = 0;
		image += p;

		*image = 0;
		image += p;

		*image = 0;
		image += p;

		nbwords -= 4;
	}
}

void TRACregisterDisplayService (TRAC_DisplayCallback _callback, u16 _enableMask)
{
    g_displayServices[g_displayServicesNb].m_callback   = _callback;
    g_displayServices[g_displayServicesNb].m_enableMask = _enableMask;
    g_displayServicesNb++;
}

void TRACdisplay (void* _image)
{	
	u8* image = (u8*) _image;
	u16 t;


	if ( g_activatedTraces & 256 ) /* F9 */
	{
		(*HW_COLOR_LUT) = 0x47;
	}

	if ( trac_displayParamLast.refresh > 0 )
	{
		trac_clear (image + trac_displayParamLast.plane, &trac_displayParamLast);
		trac_displayParamLast.refresh--;
	}
    else
    {
		u16	y = 0;

        for (t = 0 ; t < g_displayServicesNb ; t++)
        {
            if ( g_displayServices[t].m_enableMask & g_activatedTraces )
			{
				y += g_displayServices[t].m_callback (image + trac_displayParam.plane, trac_displayParam.pitch, trac_displayParam.planePitch, y);
                y += 4;
            }
        }

		trac_displayParam.h = y;    
	}

	if ( g_activatedTraces & 256 ) /* F9 */
	{
		(*HW_COLOR_LUT) = 0x7;
	}

    if ( g_activatedTraces & 512 ) /* F10 */
	{
        for (t = 1 ; t < 16 ; t++)
		{
      	    *(HW_COLOR_LUT+t) = 0x74;
		}
	}
}


void TRACsetVideoMode (u16 _pitch)
{
    trac_displayParamLast = trac_displayParam;
    trac_displayParam.pitch = _pitch;
    trac_displayParam.planePitch = ( *HW_VIDEO_MODE == 0 ) ? SYS_4P_BITSHIFT : SYS_2P_BITSHIFT;

    if ( trac_displayParam.plane >= trac_displayParam.planePitch )
    {
        trac_displayParam.plane = 0;
    }
}


void TRACmanage (u8 _key)
{
	if (( _key >= HW_KEY_F1 ) && ( _key <= HW_KEY_F10 ))
	{
		trac_displayParamLast = trac_displayParam;
		g_activatedTraces ^= 1 << (_key - HW_KEY_F1);
	}
	else
	{
		switch (_key)
		{
		case HW_KEY_BACKSPACE:
			{
				trac_displayParamLast = trac_displayParam;
				*HW_VIDEO_MODE ^= 1;	
				trac_displayParam.planePitch = ( *HW_VIDEO_MODE == 0 ) ? SYS_4P_BITSHIFT : SYS_2P_BITSHIFT;
			}
			break;

		case HW_KEY_TAB:
			{
				trac_displayParamLast = trac_displayParam;
				trac_displayParam.plane += 2;			
				if ( trac_displayParam.plane >= trac_displayParam.planePitch )
				{
					trac_displayParam.plane = 0;
				}
			}
			break;

		case HW_KEY_V:
			g_verbose = !g_verbose;
			break;

		case HW_KEY_S:
			*HW_VIDEO_SYNC ^= 2;
			break;
		}
	}
}


void TRACdrawScanlinesScale (void* _image, u16 _screenPitch, u16 _bitplanPitchShift, u16 _h)
{
    char line[4] = "   ";
    u16* p = (u16*) _image;
    u16 y, t;
    u16 planePitchW  = (1 << _bitplanPitchShift) >> 1;
    u16 screenPitchW = _screenPitch >> 1;


    for (y = 0 ; y < _h ; y += 5)
    {
        u16 mask = -1;

        if ( y & 1 )
        {
            mask = 0x5555;
        }

        p += planePitchW * 2;
        t = planePitchW * 2;

        if (y & 1)
        {
            p += planePitchW;
            t += planePitchW;
        }

        for ( ; t < screenPitchW ; t += planePitchW)
        {
            *p = mask;
            p += planePitchW;
        }

        if ( (y + 8) < _h )
        {
            if ( (y & 1) == 0 )
            {
                STDuxtoa(line, y, 3);
                SYSdebugPrint (_image, _screenPitch, _bitplanPitchShift, 0, y, line);
            }
        }

        p += (screenPitchW << 2);
    }
}



#ifdef DEMOS_UNITTEST
void TRACunitTest (void* _screen)
{
	SYSdebugPrint(_screen, 160, SYS_4P_BITSHIFT, 0, 0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-+.'");
	SYSdebugPrint(_screen, 160, SYS_4P_BITSHIFT, 0, 8, "/*<>=:;,?![]%|abcdefghijklmnopqrstuvwxyz");
}
#endif

#else

void TRACinit (LOADdisk* _disk, u16 _resourceId)
{
    tracLoadFont (_disk, _resourceId);
}

#endif /* DEMOS_DEBUG */


#ifdef DEMOS_ASSERT

void SYSassert(char* _message, char* _file, int _line)
{
    static char line[16] = "line=0x       ";


    STDcpuSetSR(0x2700);

    _message[79] = 0;
    _file   [79] = 0;

    STDuxtoa (&line[7], _line, 6);

    if ( sys.mem.buffer != NULL )
    {
        SYSwriteVideoBase((u32) sys.mem.buffer);
        STDmset(sys.mem.buffer, 0, 32000);
    }

    *HW_COLOR_LUT = TRAC_ASSERT_COLOR;
    *HW_VIDEO_OFFSET = 0;
    *HW_VIDEO_PIXOFFSET = 0;
    *HW_VIDEO_MODE = HW_VIDEO_MODE_2P;

    STDmset (HW_COLOR_LUT + 1, 0xFFFFFFFFUL, 30);

    if (sys.font != NULL)
    {
        SYSdebugPrint(sys.mem.buffer, 160, SYS_2P_BITSHIFT, 0,  0, "Assertion failed:");
        SYSdebugPrint(sys.mem.buffer, 160, SYS_2P_BITSHIFT, 0,  8, _message);
        SYSdebugPrint(sys.mem.buffer, 160, SYS_2P_BITSHIFT, 0, 16, _file);
        SYSdebugPrint(sys.mem.buffer, 160, SYS_2P_BITSHIFT, 0, 24, line);
        while(1);
    }
    else
    {   /* early asserts management */
        while(1)
        {
            (*HW_COLOR_LUT) = 0x700;  /* consider to use ASSERT_EARLY instead for a use specified color code */
            (*HW_COLOR_LUT) = 0x600;
        }
    }
}

void SYSassertEarly(u16 _c1, u16 _c2)
{
    /* to display assertion before system initialization */
    STDcpuSetSR(0x2700);

    if ( sys.mem.buffer != NULL )
    {
        SYSwriteVideoBase((u32) sys.mem.buffer);
        STDmset(sys.mem.buffer, 0, 32000);
    }

    STDmset (HW_COLOR_LUT + 1, 0xFFFFFFFFUL, 30);

    while(1)
    {
        (*HW_COLOR_LUT) = _c1; 
        (*HW_COLOR_LUT) = _c2;
    }
}

#endif /* DEMOS_ASSERT */
