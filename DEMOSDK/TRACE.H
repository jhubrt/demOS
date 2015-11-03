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

/*! @brief @ref TRAC @file */

/*! @defgroup  TRAC
    
    TRAC provides services to display traces.                                           <br>

    It provides:                                                                        <br>
    - a fast print routine to display messages on the screen. 
      The corresponding lightweight font data is provided on floppy.                    <br>
    - a list of trace routines you can register. Then the user can 
      activate / deactivate traces by pressing functions key.                           <br>
*/

#ifndef TRACE_H
#define TRACE_H

#include "DEMOSDK\BASTYPES.H"
#include "DEMOSDK\LOAD.H"

void TRACinit (LOADdisk* _disk, u16 _resourceId);

#ifdef DEMOS_DEBUG
    typedef u16 (*TRAC_DisplayCallback) (void* _image, u16 _pitch, u16 _planePitch, u16 _y);

    void TRACsetVideoMode (u16 _pitch);
    void TRACregisterDisplayService (TRAC_DisplayCallback _callback, u16 _enableMask); 
	void TRACdisplay (void* _image);
	void TRACmanage (u8 _key);
    void TRACdrawScanlinesScale (void* _image, u16 _screenPitch, u16 _bitplanPitchShift, u16 _h);
#	ifdef DEMOS_UNITTEST
	void TRACunitTest (void* _screen);
#	endif
#else
#   define TRACsetVideoMode(_pitch)
#	define TRACdisplay(_image)
#	define TRACmanage(KEY) false
#endif

#endif
