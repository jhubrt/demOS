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

/*! @brief @ref STD @file */

/*! @defgroup  STD

    - replaces stdlib for standard services. 
      STD system is designed to allow to compile without any provided standard libraries
      (it is the case when not in DEMOS_DEBUG mode)                                     <br>
    - provides access to some 68k specific instructions from C code                     <br>    
*/

#ifndef STANDARD_H
#define STANDARD_H

#include "DEMOSDK\BASTYPES.H"

#define STD_SWAP(type,A,B) { type T = A; A = B; B = T; }

#define STD_MIN(A,B) ((A)<(B) ? (A):(B))
#define STD_MAX(A,B) ((A)>(B) ? (A):(B))

void  STDmset (void* _adr, u32 _value, u32 _length);
void  STDmcpy (void* _dest, void* _src, u32 _length);
void  STDuxtoa(char* _text, u32 _value, s16 _nbchars);
u16   STDmfrnd (void);
u16   STDifrnd (void);
u16   STDswap16(u16 _v);
u32   STDswap32(u32 _v);
u32   STDmulu (u16 _x, u16 _y);
s32   STDmuls (s16 _x, s16 _y);
u32   STDdivu (u32 _x, u16 _y);
u32   STDdivs (s32 _x, s16 _y);
void  STDcpuSetSR (u16 _status);
void* STDgetSP(void);
void* STDgetUSP(void);
void  STDsetUSP(void* _adr);
 
#ifdef DEMOS_UNITTEST
void STD_unitTest (void);
#endif

#endif

