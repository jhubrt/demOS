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

/* IMAGER is a tool that:
    - creates the floppy disks image                                                          
    - generates H headers containing entries enums for your data                              
    - custom FAT sectors (2nd and 3rd sector of the disk) that maps exactly each entry        
    - LOG files listing FAT data in a human readable format                                   
    - strips .ARJ (mode 4) files (=> write shrinked .ARJX file)                               
    - consider .BOT file as bootsector program => strips its gemdos descriptor
      and computes its checksum (delta to $1234) to make it executable                        
*/

#ifndef IMAGER_H
#define IMAGER_H

bool writeST (const char* _sourcepath, const char* _imagepath, const char* _filename, u16 _nbSectorsPerTrack, u16 _nbsides, u16 _nbtracks, u16 _preferedUnit, bool _executableBootSector, const char** _filesList);

#endif
