/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2022 J.Hubert

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


/*  TRAC provides services for traces. It provides:
    - basic logging features
    - improved "bomb" crash screen
    - advanced memory logging feature
    - a list of trace routines you can register. Then the user can by pressing keys.                           
*/

#ifndef TRACE_H
#define TRACE_H

#include "DEMOSDK\SYSTEM.H"

#define TRAC_LOG_ALL        0xFFFFFFFFUL

#define TRAC_LOG_MEMORY     0x00000001UL
#define TRAC_LOG_FSM        0x00000002UL
#define TRAC_LOG_FLOW       0x00000010UL
#define TRAC_LOG_DISK       0x00000020UL
#define TRAC_LOG_COMMANDS   0x20000000UL
#define TRAC_LOG_SPECIFIC   0x40000000UL
#define TRAC_LOG_TEMP       0x80000000UL



#ifdef DEMOS_DEBUG
    
    struct TRAClogger_
    {
        u8* logbase;
        u32 logSize;
        u32 filtermask;
    };
    typedef struct TRAClogger_ TRAClogger;

#   ifndef TRACE_C
    extern TRAClogger tracLogger;
#   endif

    typedef u16 (*TRAC_DisplayCallback) (void* _image, u16 _pitch, u16 _planePitch, u16 _y);

    void TRACinit (char*);
    void TRAClogOut (char* _str, char _separator);
    void TRAClogClear (void);
    void TRACsetVideoMode (u16 _pitch);
    void TRACregisterDisplayService (TRAC_DisplayCallback _callback, u16 _enableMask); 
	void TRACdisplay (void* _image);
	void TRACmanage (u8 _key);
    void TRACdrawScanlinesScale (void* _image, u16 _screenPitch, u16 _bitplanPitchShift, u16 _h);
    u16  TRACmaxraster (u16 maxraster);

#   define TRACselect(FILTER)                                   tracLogger.filtermask = (FILTER)
#   define TRACisSelected(FILTER)                               ((FILTER) & tracLogger.filtermask)
#   define TRAClog(FILTER, STR, SEPARATOR)                      while (TRACisSelected(FILTER)) { TRAClogOut(STR,SEPARATOR); break; }
#   define TRAClogNumber(FILTER,STR,NUM,NBCHARS)                while (TRACisSelected(FILTER)) { static char trace[] = STR ":         "; STDuxtoa(&trace[sizeof(trace) - 9], (NUM), (NBCHARS)); TRAClogOut(trace, ' '      ); break; }
#   define TRAClogNumberS(FILTER, STR,NUM,NBCHARS,SEPARATOR)    while (TRACisSelected(FILTER)) { static char trace[] = STR ":         "; STDuxtoa(&trace[sizeof(trace) - 9], (NUM), (NBCHARS)); TRAClogOut(trace, SEPARATOR); break; }

#   define TRAClogNumber10(FILTER,STR,NUM,NBCHARS)              while (TRACisSelected(FILTER)) { static char trace[] = STR ":         "; STDstoa (&trace[sizeof(trace) - 9], (NUM), (NBCHARS)); TRAClogOut(trace, ' '      ); break; }
#   define TRAClogNumber10S(FILTER,STR,NUM,NBCHARS,SEPARATOR)   while (TRACisSelected(FILTER)) { static char trace[] = STR ":         "; STDstoa (&trace[sizeof(trace) - 9], (NUM), (NBCHARS)); TRAClogOut(trace, SEPARATOR); break; }

#   define TRAClogSourceLine(FILTER)                            while (TRACisSelected(FILTER)) { static char trace[] = "\nExec line:        "; STDutoa(&trace[sizeof(trace) - 8], __LINE__, 6); TRAClogOut(trace, ' '); TRAClogOut(__FILE__, '\n'); break; }

#   define TRAClogFrameNum(FILTER)                              TRAClogNumber(FILTER,"frame",SYSvblcount,5)

#   ifndef __TOS__
    void TRACsaveLog(char* _filename);
#   endif

#else

#   define TRACinit(FILENAME)
#   define TRAClogClear()
#   define TRACsetVideoMode(_pitch)
#   define TRACregisterDisplayService(_callback, _enableMask)
#	define TRACdisplay(_image)
#	define TRACmanage(KEY) false

#   define TRACselect(FILTER)
#   define TRACisSelected(FILTER) false
#   define TRAClog(FILTER,STR,SEPARATOR)
#   define TRAClogNumber(FILTER,STR,NUM,NBCHARS)
#   define TRAClogNumberS(FILTER,STR,NUM,NBCHARS,SEPARATOR)
#   define TRAClogNumber10(FILTER,STR,NUM,NBCHARS)
#   define TRAClogNumber10S(FILTER,STR,NUM,NBCHARS,SEPARATOR)
#   define TRAClogSourceLine(FILTER)
#   define TRAClogFrameNum(FILTER)

#endif

#endif
