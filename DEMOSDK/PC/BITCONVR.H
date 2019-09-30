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


/*! @brief @ref BIT @file */

#ifndef BITCONVR_H
#define BITCONVR_H

void BITfrom888Tox888       (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);
void BITfromx888To8b        (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);
void BITfrom8bTox888        (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);
void BITfrom8bTo888         (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);
void BITfrom8bTo4b          (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);

void BITfrom4bTo8b          (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);

void BITfrom4bToChunk1P     (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);
void BITfrom4bToChunk2P     (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);
void BITfrom4bToChunk3P     (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);
void BITfrom4bToChunk4P     (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);

void BITfromChunk1PTo4b     (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);
void BITfromChunk2PTo4b     (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);
void BITfromChunk3PTo4b     (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);
void BITfromChunk4PTo4b     (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);

void BITfromChunk1PTo8b     (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);
void BITfromChunk2PTo8b     (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);
void BITfromChunk3PTo8b     (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);
void BITfromChunk4PTo8b     (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);

void BITignorePlanes        (void *_source, bool* _planes, u16 _nbchunks, u16 _pitch, u16 _height, void* _dest);

#endif

