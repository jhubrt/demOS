/*------------------------------------------------------------------------------  -----------------
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
------------------------------------------------------------------------------------------------- */

#ifndef MORPH_H
#define MORPH_H 

#include "DEMOSDK\BASTYPES.H"

u32  SLIgetMorphCodeLen (u16 _nbPoints);
void SLIgenerateMorphCode (u8* _codeBuffer, u16 _nbPoints);
void SLIinitMorph    (void* _verttable, void* _horitable, u16 _h, u16 _pitch, u16 _w);
void SLIstartMorph   (void* _startPos, void* _endPos, u32 _morphCode, u32 _pos, u16 _nbpoints);
void SLIdisplayMorph (void* _screenbase, void* _pos, u32 _voffsettable, u32 _htable, u16 _nbPoints);
void SLImorphFunc    (void* _screenbase, void* _pos, u32 _voffsettable, u32 _htable, u32 _morphcode);

#ifndef MORPH_C
extern u8 SLIcodepattern;
extern u8 SLIendcodepattern;
#endif

#define SLI_MORPHPOINTS 950
#define SLI_MORPHFRAMES 64

#endif
