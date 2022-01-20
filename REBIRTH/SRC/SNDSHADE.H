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

#ifndef SNDSHADE_H

#define VIS_WIDTH 48

#ifndef SNDSHADE_C

extern u16 SNScolor;
extern u32 SNStrace;

extern u16 SNStunX1;
extern u16 SNStunX2;
extern u16 SNStunX3;
extern u16 SNStunX4;

extern u16 SNSireg;
extern u16 SNSload;
extern u32 SNSstore;

extern void* SNSlines;

#endif

void SNSinit     (void* _tcbuf, void* _lines, u16 _tcWidth, u16 _tcHeight);
void SNStc       (void);
void SNSfade     (void* _source, void* _table, u32 _dest, s32 _pitch, u32 _count);
void SNSfade3	 (void* _source, void* _table, u32 _dest, s32 _pitch, u32 _count);
void SNScopy     (void* _source, void* _dest, u16 _nbwords);
u16  SNSfilsampl (void* _sample, void* _startcolors, u32 _dest, u16 _width, s16 _sampleoffset);

#endif

