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

#ifndef BLITSND_H
#define BLITSND_H

#include "DEMOSDK\BASTYPES.H"

#define BLS_FORMAT              0xB50D
#define BLS_FORMAT_REVISION     6

#define BLS_NBVOICES            4
#define BLS_NBPATTERNROWS       64
#define BLS_DMA_FREQ            50000UL
#define BLS_NBSAMPLES_PERFRAME  1000
#define BLS_NBBYTES_PERFRAME    (BLS_NBSAMPLES_PERFRAME << 1)   /* stereo => * 2 */
#define BLS_NBBYTES_OVERHEAD    4
#define BLS_NBBYTES_CLEARFLAGS  4

#define BLS_STORAGE_WORD        0x0000
#define BLS_STORAGE_BYTE_0      0x4000
#define BLS_STORAGE_BYTE_1      0x8000
#define BLS_STORAGE_ORDER_MASK  0x3FFF

#define BLS_SAMPLE_INTERPOLATE       2
#define BLS_SAMPLE_STORAGE_SHIFT     4
#define BLS_SAMPLE_STORAGE_INTERLACE 8

STRUCT(BLSsample)
{
    s8*  sample;
    u32  sampleLen;
    u32  sampleLoopStart;
    u16  sampleLoopLength;
    u16  flags;
};

STRUCT(BLSprecomputedKey)
{
    u8   sampleIndex;
    s8   blitterTranspose;
    u16  freqmul;
    u16  freqdiv;
    u8   freqmulshift;
    u8   freqdivshift;
};

#define BLS_BASEKEY_FLAG 1
#define BLS_IS_BASEKEY(KEY) (((KEY)->blitterTranspose & BLS_BASEKEY_FLAG) != 0)

ENUM(BLSfxType)
{
    BLSfx_NONE,
    BLSfx_VOLUME,
    BLSfx_SPEED,
    BLSfx_JUMP,
    BLSfx_ARPEGGIO,
    BLSfx_SETBALANCE,
    BLSfx_PATTERN_BREAK,
    BLSfx_BITMASK,
    BLSfx_SETSAMPLEOFFSET,
    BLSfx_RETRIGGER_SAMPLE,
    BLSfx_CLIENT_EVENT,
    BLSfx_VOICE_OFF,
    BLSfx_LOOP_START_SET,
    BLSfx_LOOP,
    BLSfx_DELAY_PATTERN
};

STRUCT(BLScell)
{
    u8  precomputedKey;
    u8  fx;
    u8  value;
    u8  value2;
};
    

STRUCT(BLSrow)
{
    BLScell cells[BLS_NBVOICES];
};

STRUCT(BLSpattern)
{
    BLSrow rows[BLS_NBPATTERNROWS];
};

STRUCT(BLSsoundTrack)
{
    BLSsample*          sourceSamples;
    BLSsample*          samples;                
    u16*                sampleAllocOrder;
    BLSprecomputedKey*  keys;
    s8*                 sampleHeap;
    u32                 sampleHeapSize;
    BLSpattern*         patterns;
    u8*                 track;
   
    u16                 nbSamples;    
    u16                 nbKeys;
    u8                  nbSourceSamples;
    u8                  nbPatterns;
    u8                  trackLen;
};

ENUM(ArpeggioState)
{
    ArpeggioState_STOPPED  = 0,     /* pre shift value for ASM optims */
    ArpeggioState_STARTING = 4,
    ArpeggioState_RUNNING  = 8,
    ArpeggioState_STOPPING = 12
};


STRUCT(BLSvoice)
{
    BLSsample*          samples[3];
    BLSprecomputedKey*  keys[3];
    
    u32   current;
    u32   currentsource;

    u16   mask;

    bool  mute;

    u8    volume;
    u8    volumeoffset;
    u8    storageshift;
    u8    retrigger;
    u8    retriggercount;
    u8    arpeggioState;
    u8    arpeggioOffset; /* = index * 4 */   
};

STRUCT(BLSplayer)
{
    BLSvoice voices[BLS_NBVOICES];

    BLSsoundTrack* sndtrack;
    
    s8* buffer;
    s8* dmabuffers[2];
    u16 dummy;
    u16 backbuffer;

    u8  speed;
    u8  speedcount;
    u8  trackindex;
    u8  pattern;
    u8  row;
    u8  loopstart;
    s8  loopcount;

    u8  volumeLeft;
    u8  volumeRight;

    u8  clientEvent;
};

typedef void (*BLSinitCallback)(u16 _index, u16 _nb);

#ifdef __TOS__
#   define blsUSEASM 1

#   if blsUSEASM
#       define BLSupdate  aBLSupdate /* comment this define to test with independent ASM routines. In this case 'exportall' should be set to 1 in BLITSNDP.S file */
#   endif
#endif

#define blsDUMP 1 /* if you want to call BLStestPlay, you need to activate this flag and in this case stdlib should be included */

BLSsoundTrack*  BLSread       (MEMallocator* _allocator, void* _buffer);
void            BLSinit       (MEMallocator* _allocator, BLSsoundTrack* _sndtrack, BLSinitCallback _statCallback);
void            BLSplayerInit (MEMallocator* _allocator, BLSplayer* _player, BLSsoundTrack* _sndtrack, bool _initaudio);
void            BLSfree       (MEMallocator* _allocator, BLSsoundTrack* _sndtrack);
void            BLSplayerFree (MEMallocator* _allocator, BLSplayer* _player);
void            BLSupdate     (BLSplayer* _player);
u32             BLSasyncPlay  (BLSplayer* _player, u8 _trackindex, void* _buffer, u32 _buffersize);

#if blsDUMP
void            BLSdumpHeap   (BLSsoundTrack* _sndtrack, char* _filename);
void            BLStestPlay   (BLSplayer* _player, u8 _trackindex, char* _filesamplename, char* _filetracename);
#endif

#ifdef DEMOS_LOAD_FROMHD /* if you have loaded data from floppy you can use BLSread directly */
BLSsoundTrack*  BLSload       (MEMallocator* _allocator, char* _filename);
void            BLSwrite      (BLSsoundTrack* _sndtrack, char* _filename);
#endif

#endif
