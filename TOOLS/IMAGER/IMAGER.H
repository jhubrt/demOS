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
