/*------------------------------------------------------------------------------  --------------
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
-----------------------------------------------------------------------------------------------*/

/*! @brief @ref RING @file */

/*! @defgroup  RING 

    RINGallocator implements a ring heap where you can allocate buffers of various sizes   <br>
    
    You can create as many rings as you want (object oriented implementation)              <br> 
    - new buffers are allocated at the head if enough room available                       <br> 
    - allocated buffers can be freed                                                       <br>
        - if corresponding buffers are at the tail or head they are effectively freed      <br>
        - if not, they are marked as free. They will be effectively freed when the tail 
          or head will reach them (because of other free calls)                            <br>

    In debug mode guards are added in order to check that borders of buffers are not corrupted
    In debug mode you also have additional services to dump current structure of the heap */


#ifndef ALLOC_H
#define ALLOC_H

#include "DEMOSDK\BASTYPES.H"

STRUCT(RINGallocator)
{
	u8*	buffer;
	u32	size;

	u8*	bufferEnd;

	u8*	head;
	u8*	tail;
	u8* last;
};

void  RINGallocatorInit    (RINGallocator* _m, void* _buffer, u32 _size);
void* RINGallocatorAlloc   (RINGallocator* _m, u32 _size);
void  RINGallocatorFree    (RINGallocator* _m, void* _address);
void  RINGallocatorReset   (RINGallocator* _m);
bool  RINGallocatorIsEmpty (RINGallocator* _m);

#ifdef DEMOS_DEBUG

void  RINGallocatorDump  (RINGallocator* _m, FILE* _output);
u32   RINGallocatorList  (RINGallocator* _m, void** _list, u32 _maxSize);
u32   RINGallocatorCheck (RINGallocator* _m);

#else

#	define RINGallocatorDump(A, B)
#	define RINGallocatorList(A, B, C)
#   define RINGallocatorCheck(A)

#endif


#ifdef DEMOS_UNITTEST

void RINGallocatorUnitTest (void);

#else

#	define RINGallocatorUnitTest()

#endif


#endif
