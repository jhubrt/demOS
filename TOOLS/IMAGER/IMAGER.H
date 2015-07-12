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
  - generates C headers & sources to map data in your program                               <br>
  - strips .ARJ (mode 4) files (=> write shrinked .ARJX file)                               <br>
  - consider .BOT file as bootsector program => strips its gemdos descriptor
    and computes its checksum (delta to $1234) to make it executable                        <br>
 
   Warning: generating the sources of the disk map integrated into your program creates 
            a cyclic dependency for the floppy containing the program
            => when you recompile your program and write the image, 
            maybe its size has changed, so the following data may have moved
            => in this case you need to recompile it and rewrite the image a second time.   <br>

    => To avoid this problem, in the future it would be better to:                          <br>
    - only generate entries enum in .h file                                                 <br>
    - store the content of arrays (currently generated into .c files)
      into a specific sector of the floppy (2nd sector of the floppy for example) and load it
      at LOADinit
*/


#ifndef IMAGER_CPP
#define IMAGER_CPP

#include "DEMOSDK\BASTYPES.H"

#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>

bool writeST (char* _filename, u16 _nbSectorsPerTrack, u16 _nbsides, u16 _nbtracks, char** _filesList, u16 _preferedUnit, bool _executableBootSector);

#endif
