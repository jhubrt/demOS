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

/*! @brief @ref BIT @file */
   
#ifndef BMPLOADER_H
#define BMPLOADER_H

#include "DEMOSDK\PC\SURFACE.H"

ENUM(BITloadResult)
{
    BITloadResult_OK,    
    BITloadResult_READERROR,    
    BITloadResult_UNKNOWN_FORMAT
} ;

BITloadResult BITbmpLoad	(BITsurface* _surface, MEMallocator* _allocator, char* _filename);
BITloadResult BITbmpLoadLUT (BITlut* _lut, MEMallocator* _allocator, char* _filename);
bool          BITbmpSave    (BITsurface* _surface, char* _filename);

BITloadResult BITneoLoad    (BITsurface* _surface, MEMallocator* _allocator, char* _filename);

#endif
