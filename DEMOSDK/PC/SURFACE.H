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

/*! @defgroup BIT
   
    provides services for bitmap management / conversion for data binarization tools      <br>

    SURFACE implements the common structures representing a bitmap and its 
    associated color LUT in memory. Then surfaces can be used with load / save and convert 
    routines.
*/


#ifndef SURFACE_H
#define SURFACE_H 

#include "DEMOSDK\BASTYPES.H"

#define BIT_DEFAULT_PITCH 0xFFFF

ENUM(BITlutFormat)
{
    BITlutFormat_BnW,
	BITlutFormat_STe,
	BITlutFormat_444,
	BITlutFormat_x888
};

UNION(BITlutPtr)
{
	void*   p;
	u16*	p16;
	u32*	p32;
};

STRUCT(BITlut)
{
    MEMallocator*   allocator;
	BITlutFormat format;
	BITlutPtr	 data;
	u16		     size;
    bool         bufferowner;
};

#define BITlutConstruct(INSTANCE_PTR) DEFAULT_CONSTRUCT(INSTANCE_PTR)

void BITlutDestroy      (BITlut* _lut);
void BITlutInit         (MEMallocator* _allocator, BITlut* _lut, BITlutFormat _format, u16 _lutSize);
void BITlutSet          (MEMallocator* _allocator, BITlut* _lut, BITlutFormat _format, void* _lutData, u16 _lutSize);
void BITlutSetExternal  (BITlut* _lut, BITlutFormat _format, void* _lutData, u16 _lutSize);
void BITlutConvert      (MEMallocator* _allocator, BITlut* _source, BITlut* _dest, BITlutFormat _destFormat);

ENUM(BITformat)
{
	BITformat_Chunk4P,
	BITformat_Chunk3P,
	BITformat_Chunk2P,
	BITformat_Plane1P,
	BITformat_Plane2P,
	BITformat_Plane3P,
	BITformat_Plane4P,
	BITformat_4bits,
	BITformat_8bits,
	BITformat_888,
    BITformat_x888,
    
    BITformat_NBMAX
};

STRUCT(BITsurface)
{
	u8*         buffer;
	u16         width;
	u16         height;
	u16         pitch;
	u32         size;
	u16         nbPlanes;
	BITformat   format;
	BITlut	    lut;
    bool        bufferowner;

    MEMallocator*  allocator;
};

#define BITsurfaceConstruct(INSTANCE_PTR) DEFAULT_CONSTRUCT(INSTANCE_PTR)

void BITsurfaceDestroy        (BITsurface* _surface);
void BITsurfaceInit           (MEMallocator* _allocator, BITsurface* _surface, BITformat _format, u16 _w, u16 _h, u16 _pitch);
void BITsurfaceSetExternal    (BITsurface* _surface, void* _buffer, BITformat _format, u16 _w, u16 _h, u16 _pitch);
void BITsurfaceConvert        (MEMallocator* _allocator, BITsurface* _source, BITsurface* _dest, BITformat _destFormat);
void BITsurfaceClear          (BITsurface* _surface);
void BITsurfaceFSErrorDiffuse (MEMallocator* _allocator, BITsurface* _surface, u16 _rbits, u16 _gbits, u16 _bbits);

#endif
