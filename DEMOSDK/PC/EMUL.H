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

/*! @brief @ref EMUL @file */

/*! @defgroup EMUL
   
    Very basic frame buffer emulation to test basic stuffs into demo code directly on PC.
*/


#include "DEMOSDK\BASTYPES.H"

#ifdef __TOS__

#define EMULinit(BUFFER)
#define EMULrender()
#define EMULbufferSize(SIZE)	SIZE
#define EMULalignBuffer(BUFFER)	BUFFER
#define EMULcreateSoundBuffer(LENGTH, STEREO)
#define EMULplaysound(DATA, BYTESLENGTH, OFFSET)
#define EMULblit()

#else

#ifdef  __cplusplus 
#   define EXTERN_C extern "C"
#else
#   define EXTERN_C
#endif

PREDECLARE_STRUCT(WINdow);

EXTERN_C void    EMULinit              (void* _buffer);
EXTERN_C void    EMULrender            (void);
EXTERN_C WINdow* EMULgetWindow         (void);
EXTERN_C void    EMULcreateSoundBuffer (u32 _length, bool _stereo);
EXTERN_C void    EMULplaysound         (void* _data, u32 _byteslength, u32 _offset);
EXTERN_C u32     EMULgetPlayOffset     (void);
EXTERN_C void    EMULblit              (void);
EXTERN_C void    EMULgetSound          (void* _data, u32 _byteslength);

#define EMUL_WINDOW_WIDTH         768
#define EMUL_WINDOW_HEIGHT        576
#define EMUL_WINDOW_PCAREA_WIDTH  768

#define EMULbufferSize(SIZE)		((SIZE) | 0x1000000UL)
#define EMULalignBuffer(BUFFER)	(void*)((0x1000000 + (u32)(BUFFER)) & 0xFF000000)	/* align on 16 mb for emulation features */

#endif
