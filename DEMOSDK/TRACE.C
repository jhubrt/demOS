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

#define TRACE_C

#include "DEMOSDK\BASTYPES.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\HARDWARE.H"

#ifdef DEMOS_DEBUG

#define TRAC_KEEPLASTLOG 1   /* define 1 to keep last log (looping log) else 0 */

#define TRAC_NBMAXDISPLAYSERVICES 10

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

STRUCT(TRACloggerState)
{
    u32     current;
    bool    haslooped;
};

TRAClogger              tracLogger      = {NULL, 0};
static TRACloggerState  tracLoggerState = {0, false};

#ifndef __TOS__
u16 TRAmaxraster (u16 maxraster) { return 0; }
#endif

void TRACinit (void)
{
	trac_displayParam.h			 = 0;
	trac_displayParam.pitch		 = 160;
	trac_displayParam.plane		 = 0;
	trac_displayParam.planePitch = SYS_4P_BITSHIFT;
	trac_displayParam.refresh	 = 3;

	trac_displayParamLast.refresh = 0;

    STDmset (g_displayServices, 0, sizeof(g_displayServices));

    STDmset(tracLogger.logbase, 0UL, tracLogger.logSize);
}

void TRAClog (char* _str, char _separator)
{
    u8* p = tracLogger.logbase + tracLoggerState.current;

#   if TRAC_KEEPLASTLOG
    {
        while (*_str != 0)
        {
            *p++ = *_str++;

            tracLoggerState.current++;

            if (tracLoggerState.current >= tracLogger.logSize)
            {
                tracLoggerState.current = 0;
                tracLoggerState.haslooped = true;
            }
        }

        if (_separator)
        {
            *p++ = _separator;
            tracLoggerState.current++;
        }

        if (tracLoggerState.current >= tracLogger.logSize)
        {
            tracLoggerState.current = 0;
            tracLoggerState.haslooped = true;
        }
    }
#   else
    {
        while (*_str != 0)
        {
            if (trac_logger.m_current < trac_logger.m_logSize)
            {
                trac_logger.m_current++;
                *p++ = *_str++;
            }
            else
            {
                break;
            }
        }

        if (trac_logger.m_current < trac_logger.m_logSize)
        {
            if (_separator)
            {
                *p++ = _separator;
                trac_logger.m_current++;
            }
        }
    }
#   endif
}


void TRAClogClear()
{
    tracLoggerState.current = 0,
    tracLoggerState.haslooped = false;
    
    STDmset(tracLogger.logbase, 0UL, tracLogger.logSize);
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

#endif /* DEMOS_DEBUG */



