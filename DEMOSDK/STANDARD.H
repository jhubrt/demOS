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


/*! @brief @ref STD @file */

/*! @defgroup  STD

    - replaces stdlib for standard services. 
      STD system is designed to allow to compile without any provided standard libraries
      (it is the case when not in DEMOS_DEBUG mode)                                     <br>
    - provides access to some 68k specific instructions from C code                     <br>    
*/

#ifndef STANDARD_H
#define STANDARD_H

#define STD_SWAP(type,A,B) { type T = A; A = B; B = T; }

#define STD_MIN(A,B) ((A)<(B) ? (A):(B))
#define STD_MAX(A,B) ((A)>(B) ? (A):(B))

void  STDmset (void* _adr, u32 _value, u32 _length);
void  STDmcpy (void* _dest, void* _src, u32 _length);
void  STDmcpy2 (void* _dest, void* _src, u32 _length);
void  STDfastmset (void* _dest, u32 _value, u32 _count);
void  STDuxtoa(char* _text, u32 _value, s16 _nbchars);
void  STDstoa(char* _text, s32 _value, s16 _nbchars); 
void  STDutoa(char* _text, u32 _value, s16 _nbchars);
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
u16   STDstrLen(char* _str);
void  STDstop2300(void);

#ifdef DEMOS_UNITTEST
void STD_unitTest (void);
#endif

/* S should be a u8* pointer */
#define STD_READ_B(S,B) { STATIC_ASSERT(sizeof(B)==1); B=*S++; }

#ifdef __TOS__
#define STD_READ_W(S,W) { u8* pW=(u8*)&(W); STATIC_ASSERT(sizeof(W)==2); *pW++=*S++; *pW++=*S++;    }
#define STD_READ_L(S,L) { u8* pL=(u8*)&(L); STATIC_ASSERT(sizeof(L)==4); *pL++=*S++; *pL++=*S++; *pL++=*S++; *pL++=*S++;         }
#else
#define STD_READ_W(S,W) { STATIC_ASSERT(sizeof(W)==2); u8* pW=(u8*)&(W); pW[1]=S[0]; pW[0]=S[1]; S+=2; }
#define STD_READ_L(S,L) { STATIC_ASSERT(sizeof(L)==4); u8* pL=(u8*)&(L); pL[3]=S[0]; pL[2]=S[1]; pL[1]=S[2]; pL[0]=S[3]; S+=4; }
#endif

#ifdef DEMOS_LOAD_FROMHD
void STDwriteB (FILE* _file, u8  _b);
void STDwriteW (FILE* _file, u16 _w);
void STDwriteL (FILE* _file, u32 _l);
void STDwrite  (FILE* _file, void* _buf, u32 _size);

void* STDloadfile(MEMallocator* _allocator, char* _filename);
#endif

#endif

