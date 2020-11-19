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

#ifndef HELPERS_H
#define HELPERS_H

#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT 1024

STRUCT(TOOLerror)
{
    char m_message[256];

    TOOLerror(const char* _message) 
    {
        strcpy(m_message, _message);
    }
};

u32  TOOLfileSize       (FILE* _file);
void TOOLreadcheck      (void* _buf, u32 _size, FILE* _file);
void TOOLwritecheck     (const void* _buf, u32 _size, FILE* _file);

void TOOLwritecheck     (const u32& _val, FILE* _file);
void TOOLwritecheck     (const u16& _val, FILE* _file);
void TOOLwritecheck     (const u8&  _val, FILE* _file);

void TOOLtruncateFile   (const char* _source, const char* _dest, u32 _size);

void TOOLdisplayImage   (struct WINdow_* _window, struct BITsurface_* _surface);

void TOOLwriteStaticData(char* _filename, char* _sourcefile, char* _symbolName);

#endif
