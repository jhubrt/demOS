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

#define RASTERS_C

#include "DEMOSDK\RASTERS.H"

#ifndef __TOS__
void RASsetColReg (u16 _shortregaddress) {}
void RASvbl1	  (void) {}
void RASvbl2	  (void) {}
void RAStop1	  (void) {}
void RASmid1	  (void) {}
void RASbot1	  (void) {}

void* RASnextOpList = NULL;
#endif
