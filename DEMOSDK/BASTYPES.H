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

/*! @brief @ref BASTYPES @file */

/*! @defgroup BASTYPES 

   BASTYPES defines configuration and basic C language construct:                       <br>
    - Configuration flags for whole compilation (DEBUG mode, ASSERT activation...)      <br>
    - Some basic C language constructs (STRUCT, ENUM, MEMallocator, endian swap, STATIC_ASSERT...)
*/

#ifndef BASTYPES_H
#define BASTYPES_H

/* #define DEMOS_OPTIMIZED */ /* enforce optimization */

/* Configuration */
#ifndef __TOS__
#	define DEMOS_LOAD_FROMHD
#endif

#ifdef DEMOS_OPTIMIZED

#	define IGNORE_PARAM(NAME)
#   define STATIC_ASSERT(COND)

#else

#	define DEMOS_DEBUG
#	define DEMOS_ASSERT
/*#   define DEMOS_ALLOW_DEBUGGER */
/*#   define DEMOS_LOAD_FROMHD */

#	if defined(DEMOS_ASSERT) && defined(DEMOS_DEBUG)
#		define DEMOS_UNITTEST
#	endif

#	define IGNORE_PARAM(NAME)	NAME = NAME;
#   define STATIC_ASSERT(COND) typedef char static_assertion##__LINE__[(COND)?1:-1]

#endif

#ifndef DEMOS_INVERT_DRIVE
#   define DEMOS_INVERT_DRIVE 0
#endif

/* OS standards */
#if !defined(DEMOS_OPTIMIZED) || !defined(__TOS__)
#	ifndef __TOS__
#		define _CRT_SECURE_NO_WARNINGS
#       include <assert.h>
#	endif
#	include <stdio.h>
#	include <stdlib.h>
#	include <string.h>
#endif

/* constructs */
#define ENUM(NAME)			                            typedef enum NAME##_ NAME; enum NAME##_
#define PREDECLARE_STRUCT(NAME)							typedef struct NAME##_ NAME
#define STRUCT(NAME)									PREDECLARE_STRUCT(NAME); struct NAME##_ 
#define UNION(NAME)										typedef union NAME##_ NAME; union NAME##_ 

#define DEFAULT_CONSTRUCT(INSTANCE_PTR)                 STDmset(INSTANCE_PTR,0,sizeof(*INSTANCE_PTR));

/* basic types */
typedef signed char		s8;
typedef unsigned char	u8;
typedef short        	s16;
typedef unsigned short  u16;
typedef long            s32;
typedef unsigned long   u32;

#ifndef __cplusplus
typedef u8 bool;
#   define false 0
#   define true  1
#endif

#ifndef ARRAYSIZE
#	define ARRAYSIZE(TAB)	(sizeof(TAB) / sizeof(*TAB))
#endif

#ifndef NULL
#	define NULL ((void*)0)
#endif

#define U16_SIZEOF_SHIFT 1
#define U32_SIZEOF_SHIFT 2

/* endian swap */
#ifdef __TOS__
#	define PCENDIANSWAP16(V)	V
#	define PCENDIANSWAP32(V)	V
#	define PCSTUB
#	define PCSTUBRET
#	define ASMIMPORT			extern
#   define ASMIMPORTDEFAULT(A)
#else
#	define PCENDIANSWAP16(V)	STDswap16(V)
#	define PCENDIANSWAP32(V)	STDswap32(V)
#	define PCSTUB               {}
#	define PCSTUBRET            { return 0; }
#	define ASMIMPORT
#   define ASMIMPORTDEFAULT(A)   = A
#endif

/* memory allocator interface */
typedef void* (*MEMallocFunc)(void* _allocator, u32 _size);
typedef void  (*MEMfreeFunc)(void* _allocator, void* _adr);

STRUCT(MEMallocator)
{
	void* allocator;
	MEMallocFunc alloc;
	MEMfreeFunc  free;
};

#define MEM_ALLOC(ALLOCATOR,SIZE) (ALLOCATOR)->alloc((ALLOCATOR)->allocator,(SIZE))
#define MEM_FREE(ALLOCATOR,ADR)   (ALLOCATOR)->free((ALLOCATOR)->allocator,(ADR))

#define MEM_ALLOC_STRUCT(ALLOCATOR,NAME) (NAME*) (ALLOCATOR)->alloc((ALLOCATOR)->allocator, sizeof(NAME))

/* assertion */
#ifdef __TOS__
#   ifdef DEMOS_ASSERT
	    void SYSassert(char* _message, char* _file, int _line);
	    void SYSassertColor(u16 _c1, u16 _c2);
#	    define ASSERT(A) if (!(A)) { SYSassert(#A, __FILE__, __LINE__); }
#	    define ASSERT_COLOR(A,COLOR1,COLOR2) if (!(A)) { SYSassertColor(COLOR1,COLOR2); }
#	else
#	    define ASSERT(A)
#	    define ASSERT_COLOR(A,COLOR1,COLOR2)
#   endif
#	define ASSERT_ST(A) ASSERT(A)
#else
#   define _CRT_SECURE_NO_WARNINGS
#   define ASSERT(A) assert(A)
#	define ASSERT_COLOR(A,COLOR1,COLOR2) assert(A)
#	define ASSERT_ST(A) if (!(A)) {}        /* just check compilation */
#endif

#endif
