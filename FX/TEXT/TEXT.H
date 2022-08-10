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

#ifndef TEXT_H
#define TEXT_H

typedef u16*(*TEXfont44GenerateCodeCallback)(u16* code_, u16 offset_, u16 opcode_);
typedef void(*TEXTfont44Display)(void*);

u16 TEXfont44GenerateCode(u8* _code, char* _str, u16 pitch_, TEXfont44GenerateCodeCallback generatecallback_);

#ifdef __TOS__
#   define TEXfont44Display(_code, _adr, codeoffset1_, codeoffset2_) ((TEXTfont44Display) ( ((u8*)_code) + 16) )(_adr)
#else
    void TEXfont44DisplayPC(void* _code, u8* _adr, u16 codeoffset1_, u16 codeoffset2_);
#   define TEXfont44Display TEXfont44DisplayPC
#endif

#endif
