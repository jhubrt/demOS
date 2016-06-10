/*------------------------------------------------------------------------------  -----------------
  The MIT License (MIT)

  Copyright (c) 2015-2016 J.Hubert

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
------------------------------------------------------------------------------------------------- */

/*! @brief @ref EMUL @file */

/*! @defgroup EMUL
   
    Very basic frame buffer emulation to test basic stuffs into demo code directly on PC.
*/


#include "DEMOSDK\BASTYPES.H"

#ifdef __TOS__

#define EMULinit(BUFFER)
#define EMULrender()
#define EMULbufferSize(SIZE)	SIZE
#define EMULalignBuffer(BUFFER)	BUFFER
#define EMULcreateSoundBuffer(LENGTH, STEREO)
#define EMULplaysound(DATA, BYTESLENGTH, OFFSET)
#define EMULblit()
#define EMULwait(ms)

#else

#ifdef  __cplusplus 
#   define EXTERN_C extern "C"
#else
#   define EXTERN_C
#endif

PREDECLARE_STRUCT(WINdow);

EXTERN_C void    EMULinit              (void* _buffer);
EXTERN_C void    EMULrender            (void);
EXTERN_C WINdow* EMULgetWindow         (void);
EXTERN_C void    EMULcreateSoundBuffer (u32 _length, bool _stereo);
EXTERN_C void    EMULplaysound         (void* _data, u32 _byteslength, u32 _offset);
EXTERN_C u32     EMULgetPlayOffset     (void);
EXTERN_C void    EMULblit              (void);
EXTERN_C void    EMULgetSound          (void* _data, u32 _byteslength);
EXTERN_C void    EMULwait              (u32 _ms);

#define EMUL_WINDOW_WIDTH         768
#define EMUL_WINDOW_HEIGHT        576
#define EMUL_WINDOW_PCAREA_WIDTH  768

#define EMULbufferSize(SIZE)		((SIZE) | 0x1000000UL)
#define EMULalignBuffer(BUFFER)	(void*)((0x1000000 + (u32)(BUFFER)) & 0xFF000000)	/* align on 16 mb for emulation features */

/* 68k port helpers */

STRUCT(EMULregisterByte)
{
    u8 reg;
    u8 pad[3];
};

STRUCT(EMULregisterWord)
{
    u16 reg;
    u16 pad;
};

UNION(EMULregister)
{
    u32     l;
    u16     w;
    u8      b;
};

STRUCT(EMUL68k)
{
    EMULregister a0,a1,a2,a3,a4,a5,a6,a7;
    EMULregister d0,d1,d2,d3,d4,d5,d6,d7;
	bool carry;
};

u8*  EMUL_B_I   (EMULregister* _reg);
u16* EMUL_W_I   (EMULregister* _reg);
u32* EMUL_L_I   (EMULregister* _reg);
void EMUL_ROR_W (EMULregister* _reg, u8 _shift);
void EMUL_ROL_L (EMULregister* _reg, u8 _shift);
void EMUL_LSL_L (EMULregister* _reg, u8 _shift);
void EMUL_SUB_B (EMULregister* _source, EMULregister* _dest, EMUL68k* _p);
bool EMUL_BTST  (u32 _data, u8 _bit);

#endif
