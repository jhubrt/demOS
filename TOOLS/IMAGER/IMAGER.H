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

/*! @brief @ref IMAGER @file */

/*! @defgroup IMAGER

  IMAGER is a tool that:
  - creates the floppy disks image                                                          <br>
  - generates H headers containing entries enums for your data                              <br>
  - custom FAT sectors (2nd and 3rd sector of the disk) that maps exactly each entry        <br>
  - LOG files listing FAT data in a human readable format                                   <br>
  - strips .ARJ (mode 4) files (=> write shrinked .ARJX file)                               <br>
  - consider .BOT file as bootsector program => strips its gemdos descriptor
    and computes its checksum (delta to $1234) to make it executable                        <br>
*/


#ifndef IMAGER_CPP
#define IMAGER_CPP

#include "DEMOSDK\BASTYPES.H"

#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>

bool writeST (char* _filename, u16 _nbSectorsPerTrack, u16 _nbsides, u16 _nbtracks, char** _filesList, u16 _preferedUnit, bool _executableBootSector);

#endif
