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

#ifndef HELPERS_H
#define HELPERS_H

#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT 1024

PREDECLARE_STRUCT(WINdow);
PREDECLARE_STRUCT(BITsurface);

u32  TOOLfileSize       (FILE* _file);
void TOOLreadcheck      (void* _buf, u32 _size, FILE* _file);
void TOOLwritecheck     (void* _buf, u32 _size, FILE* _file);
void TOOLtruncateFile   (const char* _source, const char* _dest, u32 _size);

void TOOLdisplayImage   (WINdow* _window, BITsurface* _surface);

void TOOLwriteStaticData(char* _filename, char* _sourcefile, char* _symbolName);

#endif
