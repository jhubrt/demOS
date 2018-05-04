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


/*! @brief @ref TRAC @file */

/*! @defgroup  TRAC
    
    TRAC provides services to display traces.                                           <br>

    It provides:                                                                        <br>
    - a list of trace routines you can register. Then the user can 
      activate / deactivate traces by pressing functions key.                           <br>
*/

#ifndef TRACE_H
#define TRACE_H

#include "DEMOSDK\BASTYPES.H"


#ifdef DEMOS_DEBUG
    typedef u16 (*TRAC_DisplayCallback) (void* _image, u16 _pitch, u16 _planePitch, u16 _y);

    void TRACinit (void* _logmem, u32 _logmemsize);
    void TRAClog (char* _str);
    void TRAClogClear (void);
    void TRACsetVideoMode (u16 _pitch);
    void TRACregisterDisplayService (TRAC_DisplayCallback _callback, u16 _enableMask); 
	void TRACdisplay (void* _image);
	void TRACmanage (u8 _key);
    void TRACdrawScanlinesScale (void* _image, u16 _screenPitch, u16 _bitplanPitchShift, u16 _h);
    u16  TRAmaxraster (u16 maxraster);

#   define TRAClogNumber(STR,NUM,NBCHARS) { static char trace[] = STR ":         "; STDuxtoa(&trace[sizeof(trace) - 9], (NUM), (NBCHARS)); TRAClog(trace); }

#	ifdef DEMOS_UNITTEST
	void TRACunitTest (void* _screen);
#	endif
#else
#   define TRACinit(_logmem, _logmemsize)
#   define TRAClog(_str)
#   define TRAClogClear()
#   define TRACsetVideoMode(_pitch)
#	define TRACdisplay(_image)
#	define TRACmanage(KEY) false

#   define TRAClogNumber(STR,NUM,NBCHARS)

#endif

#endif
