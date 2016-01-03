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

/*! @brief @ref BIT @file */

#ifndef BITCONVR_H
#define BITCONVR_H

#include "DEMOSDK\BASTYPES.H"

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

