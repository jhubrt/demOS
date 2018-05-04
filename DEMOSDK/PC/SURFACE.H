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
