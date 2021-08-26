/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2021 J.Hubert

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


/*! \defgroup RINGallocator

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

struct RINGallocator_
{
	u8*	buffer;
	u32	size;

	u8*	bufferEnd;

	u8*	head;
	u8*	tail;
	u8* last;
};
typedef struct RINGallocator_ RINGallocator;

struct RINGallocatorFreeArea_
{
    u16 nbareas;
    u32 areasizes[2];
    u32 size;
};
typedef struct RINGallocatorFreeArea_ RINGallocatorFreeArea;

void  RINGallocatorInit      (RINGallocator* _m, void* _buffer, u32 _size);
void* RINGallocatorAlloc     (RINGallocator* _m, u32 _size);
void* RINGallocatorAllocTemp (RINGallocator* _m, u32 _size);
void  RINGallocatorFree      (RINGallocator* _m, void* _address);
void  RINGallocatorReset     (RINGallocator* _m);
bool  RINGallocatorIsEmpty   (RINGallocator* _m);
void  RINGallocatorFreeSize  (RINGallocator* _m, RINGallocatorFreeArea* _info);

#ifdef DEMOS_DEBUG

void  RINGallocatorDump      (RINGallocator* _m, FILE* _output);
void  RINGallocatorDumpAlloc(RINGallocator* _m, void* _address, FILE* _output);
u32   RINGallocatorList      (RINGallocator* _m, void** _list, u32 _maxSize);
u32   RINGallocatorCheck     (RINGallocator* _m);

#else

#	define RINGallocatorDump(A, B)
#	define RINGallocatorDumpAlloc(A, B, C)
#	define RINGallocatorList(A, B, C)
#   define RINGallocatorCheck(A)

#endif


#ifdef DEMOS_UNITTEST

void RINGallocatorUnitTest (void);

#else

#	define RINGallocatorUnitTest()

#endif


#endif
