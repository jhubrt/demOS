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

#ifndef SCRIPT_H
#define SCRIPT_H

#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
#include <vector>
#include <string>

class FloppyImageScript
{
    std::vector<const char*> m_filelist;
    std::vector<std::string> m_strs;  

public:

    const char** getFilesList() { return &m_filelist[0]; }

    int         m_nbsectors;
    int         m_nbsides;
    int         m_nbtracks;
    int         m_preferedunit;
    bool        m_executablebootsector;


    bool loadScript(const char* _filename);
};

#endif
