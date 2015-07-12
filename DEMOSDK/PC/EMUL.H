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

#define EMULinit()
#define EMULrender()
#define EMULbufferSize(SIZE)	SIZE
#define EMULalignBuffer(BUFFER)	BUFFER

#else

PREDECLARE_STRUCT(WINdow);

void EMULinit (void);
void EMULrender (void);
WINdow* EMULgetWindow (void);

#define EMULbufferSize(SIZE)		((SIZE) | 0x1000000UL)
#define EMULalignBuffer(BUFFER)	(void*)((0x1000000 + (u32)(BUFFER)) & 0xFF000000)	/* align on 16 mb for emulation features */

#endif
