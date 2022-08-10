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


/*! @defgroup EMUL

  Very basic frame buffer emulation to test basic stuffs into demo code directly on PC.
*/

#ifndef EMUL_H
#define EMUL_H

#ifdef __TOS__

#define EMULinit(BUFFER,W,H,E,T)
#define EMULleftChannel()
#define EMULrightChannel()
#define EMULnewFrame()
#define EMULupdateRequested() true
#define EMULrender()
#define EMULbufferSize(SIZE)	SIZE
#define EMULalignBuffer(BUFFER)	BUFFER
#define EMULcreateSoundBuffer(LENGTH, STEREO, DMAFREQ)
#define EMULplaysound(DATA, BYTESLENGTH, OFFSET)
#define EMULblit()
#define EMULwait(ms)
#define EMULcls()
#define EMULdrawYMbuffer(_x,_y,_sync)
#define EMULsetVStretch(_vstretch)

#define EMULfbStdEnable()
#define EMULfbDisableAll()
#define EMULfbExStart(VIDEOMODE,STARTX,STARTY,ENDX,ENDY,PITCH,PIXOFFSET)
#define EMULfbExSetVideoMode(STARTCYCLE,VIDEOMODE)
#define EMULfbExSetColor(STARTCYCLE,INDEX,VALUE)
#define EMULfbExSetAdr(STARTCYCLE,ADR)
#define EMULfbExSetWidth(STARTCYCLE,STARTX,ENDX,PITCH)
#define EMULfbExEnd()
#define EMULfbExDisable()
#define EMULfbExComputeCycle(VIDEOSYNC,X,Y) 0

#define EMULgetWindow NULL

#define EMULlog(STR)

#define EMUL_BEGIN_ASYNC_REGION
#define EMUL_END_ASYNC_REGION
#define EMUL_REENTER_POINT
#define EMUL_EXIT_IF(COND)
#define EMUL_STATIC
#define EMUL_BACKUP_REENTER_POINT
#define EMUL_RESTORE_REENTER_POINT

#else /* ! __TOS__ */

#ifdef  __cplusplus 
#   define EXTERN_C extern "C"
#else
#   define EXTERN_C
#endif

EXTERN_C void    EMULinit              (void* _buffer, s16 _width, s16 _height, s16 _width_ext, char* _title);
EXTERN_C void    EMULnewFrame          (void);
EXTERN_C void    EMULrender            (void);
EXTERN_C bool    EMULupdateRequested   (void);
EXTERN_C void    EMULcls               (void);
EXTERN_C struct WINdow_* EMULgetWindow (void);
EXTERN_C void    EMULcreateSoundBuffer (u32 _length, bool _stereo, u32 _dmafreq);
EXTERN_C void    EMULplaysound         (void* _data, u32 _byteslength, u32 _offset);
EXTERN_C void    EMULleftChannel       (void);
EXTERN_C void    EMULrightChannel      (void);
EXTERN_C u32     EMULgetPlayOffset     (void);
EXTERN_C void    EMULblit              (void);
EXTERN_C void    EMULwait              (u32 _ms);
EXTERN_C void    EMULdrawYMbuffer      (s16 _x, s16 _y, bool _sync);
EXTERN_C void    EMULsetVStretch       (bool _vstretch);

EXTERN_C void    EMULfbStdEnable       (void);
EXTERN_C void    EMULfbDisableAll      (void);
EXTERN_C void    EMULfbExStart         (u8 _videomode, u16 _startcyclex, u16 _starty, u16 _endcyclex, u16 _endy, u16 _pitch, u16 _pixoffset);
EXTERN_C void    EMULfbExSetVideoMode  (u32 _execcycle, u8 _videomode);
EXTERN_C void    EMULfbExSetColor      (u32 _execcycle, u16 _index, u16 _value);
EXTERN_C void    EMULfbExSetAdr        (u32 _execcycle, u32 _adr);
EXTERN_C void    EMULfbExSetPixOffset  (u32 _execcycle, u8 _offsetx);
EXTERN_C void    EMULfbExSetWidth      (u32 _execcycle, u16 _startx, u16 _endx, u16 _pitch);
EXTERN_C void    EMULfbExEnd           (void);
EXTERN_C void    EMULfbExDisable       (void);
EXTERN_C u32     EMULfbExComputeCycle  (u8 _videosync, u16 _x, u16 _y);
EXTERN_C void    EMULlog               (char* str);

#define EMUL_WINDOW_WIDTH         768
#define EMUL_WINDOW_HEIGHT        576
#define EMUL_WINDOW_PCAREA_WIDTH  768

#define EMULbufferSize(SIZE)		((SIZE) | 0x1000000UL)
#define EMULalignBuffer(BUFFER)	(u8*)((0x1000000 + (u32)(BUFFER)) & 0xFF000000)	/* align on 16 mb for emulation features */

/* 68k port helpers */
struct EMULregisterByte_
{
    u8 reg;
    u8 pad[3];
};
typedef struct EMULregisterByte_ EMULregisterByte;

struct EMULregisterWord_
{
    u16 reg;
    u16 pad;
};
typedef struct EMULregisterWord_ EMULregisterWord;

union EMULregister_
{
    u32     l;
    u16     w;
    u8      b;
};
typedef union EMULregister_ EMULregister;

struct EMUL68k_
{
    EMULregister a0,a1,a2,a3,a4,a5,a6,a7;
    EMULregister d0,d1,d2,d3,d4,d5,d6,d7;
	bool carry;
};
typedef struct EMUL68k_ EMUL68k;

void EMUL_SWAP  (EMULregister* _reg);
u8*  EMUL_B_I   (EMULregister* _reg);
u16* EMUL_W_I   (EMULregister* _reg);
u32* EMUL_L_I   (EMULregister* _reg);
bool EMUL_ROR_W (EMULregister* _reg, u8 _shift);
void EMUL_ROL_L (EMULregister* _reg, u8 _shift);
void EMUL_LSL_L (EMULregister* _reg, u8 _shift);
void EMUL_SUB_B (EMULregister* _source, EMULregister* _dest, EMUL68k* _p);
bool EMUL_BTST  (u32 _data, u8 _bit);

#define EMUL_BEGIN_ASYNC_REGION     static int EMUL_reenter = -1; switch (EMUL_reenter) { case -1:
#define EMUL_END_ASYNC_REGION       } EMUL_reenter = -1;
#define EMUL_REENTER_POINT          case __LINE__: EMUL_reenter = __LINE__;
#define EMUL_EXIT_IF(COND)          if (COND) return;
#define EMUL_STATIC                 static
#define EMUL_BACKUP_REENTER_POINT   static int EMUL_reenterBackup; EMUL_reenterBackup = EMUL_reenter;
#define EMUL_RESTORE_REENTER_POINT  EMUL_reenter = EMUL_reenterBackup;
#endif


#if defined(__TOS__) || !EMUL_TRACED_VALUE_ENABLE

#define EMULtraceNewValue(_valueindex,_min,_max,_r,_g,_b)
#define EMULtracePushValue(_valueIndex,_val)
#define EMULtraceClearValue(_valueindex)

#else 

EXTERN_C void EMULtraceNewValue     (u16 _valueindex, s32 _min, s32 _max, u8 _r, u8 _g, u8 _b);
EXTERN_C void EMULtracePushValue    (u16 _valueIndex, s32 _val);
EXTERN_C void EMULtraceClearValue   (u16 _valueindex);

#endif


#endif /* EMUL_H */
