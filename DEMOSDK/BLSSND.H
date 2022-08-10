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

#ifndef BLITSND_H
#define BLITSND_H

#include "DEMOSDK\SYNTHYM.H"
#include "DEMOSDK\SYSTEM.H"

#ifdef __TOS__
#   if blsUSEASM
#       define BLZupdate        aBLZupdate /* comment this define to test with independent ASM routines. In this case 'exportall' should be set to 1 in BLITSNDP.S file */
#       define BLZ2update       aBLZ2update
#       define BLZupdAsync      aBLZupdAsync
#   endif

#   define BLSsetSecondaryBufferFreq(_freq)   
#else
    void BLSsetSecondaryBufferFreq(u32 _freq);
#endif

#if blsUSEASM & defined(BLZupdate)
#   define BLS_SCOREMODE_ENABLE 0
#else
#   ifndef BLS_SCOREMODE_ENABLE
#       define BLS_SCOREMODE_ENABLE 1
#   endif
#endif
   
#ifndef BLS_ENABLE_OPTIMIZED_DUMP
#   define BLS_ENABLE_OPTIMIZED_DUMP 0
#endif

#ifndef blsBENCHMARK
#   define blsBENCHMARK 1
#endif

#define blsLOGDMA 0

#if blsBENCHMARK
#   define blsRASTER(COLOR) *HW_COLOR_LUT = COLOR;
#else
#   define blsRASTER(COLOR)
#endif

#define BLS_FORMAT              0xB50D
#define BLS_FORMAT_BLITZ        0xB712
#define BLS_FORMAT_REVISION     18

#define BLS_NBVOICES            4
#define BLS_DMA_FREQ            50000UL
#define BLS_NB_BUFFERS          3
#define BLS_NBSAMPLES_PERFRAME  1000
#define BLS_NBBYTES_PERFRAME    (BLS_NBSAMPLES_PERFRAME << 1)   /* stereo => * 2 */
#define BLS_NBBYTES_OVERHEAD    4
#define BLS_NBBYTES_CLEARFLAGS  4
#define BLS_STEP_PERFRAME       (BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD + BLS_NBBYTES_CLEARFLAGS)

#define BLS_STORAGE_WORD        0x0000
#define BLS_STORAGE_BYTE_0      0x4000
#define BLS_STORAGE_BYTE_1      0x8000
#define BLS_STORAGE_ORDER_MASK  0x3FFF

#define BLS_SAMPLE_INTERPOLATE       2
#define BLS_SAMPLE_STORAGE_SHIFT     4
#define BLS_SAMPLE_STORAGE_INTERLACE 8

#define BLSisBlitzSndtrack(SNDTRACK) (((SNDTRACK)->framesdata) != NULL)

struct BLSsample_
{
    s8*  sample;
    u32  sampleLen;
    u32  sampleLoopStart;
    u16  sampleLoopLength;
    u16  flags;
};
typedef struct BLSsample_ BLSsample;


struct BLSprecomputedKey_
{
    u8   sampleIndex;
    s8   blitterTranspose;
    u16  freqmul;
    u16  freqdiv;
    u8   freqmulshift;
    u8   freqdivshift;
};
typedef struct BLSprecomputedKey_ BLSprecomputedKey;


#define BLS_BASEKEY_FLAG 1
#define BLS_IS_BASEKEY(KEY) (((KEY)->blitterTranspose & BLS_BASEKEY_FLAG) != 0)

enum BLSfxType_
{
    BLSfx_NONE              ,
    BLSfx_VOLUME            ,
    BLSfx_SPEED             ,
    BLSfx_JUMP              , 
    BLSfx_ARPEGGIO          , 
    BLSfx_SETBALANCE        , 
    BLSfx_PATTERN_BREAK     , 
    BLSfx_BITMASK           , 
    BLSfx_SETSAMPLEOFFSET   , 
    BLSfx_RETRIGGER_SAMPLE  , 
    BLSfx_AVAILABLE         , /* free code available for something else (BLSfx_CLIENT_EVENT) */
    BLSfx_PORTAMENTO_TICKS  ,
    BLSfx_LOOP_START_SET    ,
    BLSfx_LOOP              ,
    BLSfx_DELAY_PATTERN     ,
    BLSfx_DELAY_SAMPLE      ,
    BLSfx_PITCHBEND_TICKS   ,
    BLSfx_PITCHBEND_RANGE   ,
    BLSfx_SET_FINETUNE      
};
typedef enum BLSfxType_ BLSfxType;


#define BLS_FX_VOICENUM_SHIFT 5
#define BLS_FX_MASK		      ((1 << BLS_FX_VOICENUM_SHIFT) - 1)

#define BLS_PRECOMPUTED_KEY_OFF 255

struct BLScell_
{
    u8  precomputedKey;
    u8  fx;
    u8  value;
    u8  value2;
};
typedef struct BLScell_ BLScell;


struct BLSpattern_
{
    u8       nbrows;
	u16*	 rowtocellindex;
	BLScell* cells;
};
typedef struct BLSpattern_ BLSpattern;


struct BLSsoundTrack_
{
    BLSsample*          samples;               
    BLSprecomputedKey*  keys;                  

    u8*                 framesdata;            /* Blitz mode */
    u32*                patternsdata;          /* Blitz mode */
    u16*                patternslength;        /* Blitz mode */
    u8*                 track;                 
    u8                  trackLen;   
    u8                  nbPatterns;

    u8                  nbSourceSamples;       

    s8*                 sampleHeap;            
    u32                 sampleHeapSize;

    u16                 nbSamples;             
    u16                 nbKeys;                
    BLSsample*          sourceSamples;         
    u16*                sampleAllocOrder;      

    u8*                 keysnoteinfo;

#   if BLS_SCOREMODE_ENABLE
    BLSpattern*         patterns;
    u8*                 nbPatternRows;  /* nb rows for each pattern */ 
    u32                 nbcells;
    BLScell*            cells;

    SNDYMsoundSet       YMsoundSet;
#   endif
};
typedef struct BLSsoundTrack_ BLSsoundTrack;


enum ArpeggioState_
{
    ArpeggioState_STOPPED  = 0,     /* pre shift value for ASM optims */
    ArpeggioState_STARTING = 4,
    ArpeggioState_RUNNING  = 8,
    ArpeggioState_STOPPING = 12
};
typedef enum ArpeggioState_ ArpeggioState;


struct BLSvoice_
{
    BLSprecomputedKey*  keys[3];

    u32   current;
    u32   currentsource;

    u16   mask;

    u8    sampledelay;      /* should be contiguous to retrigger, volume, retrigger  and arpeggioState and aligned on 2 (optimization) */
    u8    volume;           
    u8    retrigger;
    u8    arpeggioState;

    bool  mute;             

    u8    retriggercount;   
    u8    arpeggioOffset;   /* = index * 4 */   
    u8    volumeoffset;

    u8    dummy;            /* keep it aligned on 32 for perf reason (avoid muls) */

#   if BLS_SCOREMODE_ENABLE
    bool  mirroractivation;
#   endif

#   if BLS_ENABLE_OPTIMIZED_DUMP
    u8    voicenum;
#   endif
};
typedef struct BLSvoice_ BLSvoice;


#if BLS_SCOREMODE_ENABLE && BLS_ENABLE_OPTIMIZED_DUMP

#define BLS_SEQ_FRAMENUM_UNDEFINED 0xFFFF

struct BLSextSequenceRow_
{
    u8  trackindex;
    u8  row;
    u8  nbcommands;
    u8  commands[15];

    u16 framenum;
};
typedef struct BLSextSequenceRow_ BLSextSequenceRow;

#endif

struct BLSplayer_
{
    BLSvoice voices[BLS_NBVOICES];

    BLSsoundTrack* sndtrack;

    s8* buffer;
    s8* buffertoupdate;
    u32 dmabufstart;
    u32 dmabufend;
    u16 bufferstate;

    u8* blizcurrent;
    u8* patternend;

    u16 volumeLeft;
    u16 volumeRight;
    u16 volumeLeft2;
    u16 volumeRight2;

    u8   trackindex;
    bool tracklooped;

    u16  framenum;
    u16  maxframenum;

#   if BLS_SCOREMODE_ENABLE
    u8  row;
    u8  pattern;
    u8  speed;
    u8  speedcount;
    u8  loopstart;
    s8  loopcount;
    u8  padding;

    SNDYMplayer ymplayer;

    SNDYMcommand ymcommands[SND_YM_NB_CHANNELS];

#   if BLS_ENABLE_OPTIMIZED_DUMP
    struct BLSoptDump_* blitzDumper;

    BLSextSequenceRow* sequence;
    u16                sequencesize;
    u16*               sequenceframenummap;
#   endif
#   endif
};
typedef struct BLSplayer_ BLSplayer;

#if blsUSEASM || !defined(__TOS__)
    void aBLZvbl(void);
    void aBLZ2vbl(void);
    extern BLSplayer* aBLZplayer;
    extern u16        aBLZbackground;
#endif

typedef void (*BLSinitCallback)(bool _deltadecode, u16 _index, u16 _nb);

enum BLZdmaMode_
{
    BLZ_DMAMODE_PLAYONCE,
    BLZ_DMAMODE_LOOP,
    BLZ_DMAMODE_NOAUDIO
};
typedef enum BLZdmaMode_ BLZdmaMode;

void            BLSinit          (MEMallocator* _allocator, MEMallocator* _allocatorTemp, BLSsoundTrack* _sndtrack, BLSinitCallback _statCallback);

/* Base functions can play blitz mode soundtracks */
void*           BLZread          (MEMallocator* _allocator, MEMallocator* _allocatorTemp, void* _buffer, BLSsoundTrack** _sndtrack);
void            BLZplayerInit    (MEMallocator* _allocator, BLSplayer* _player, BLSsoundTrack* _sndtrack, BLZdmaMode _dmamode);
void            BLZplayerFree    (MEMallocator* _allocator, BLSplayer* _player);
void            BLZupdate        (BLSplayer* _player);
void            BLZ2update       (BLSplayer* _player);
void            BLZupdAsync      (BLSplayer* _player);
void            BLZgoto          (BLSplayer* _player, u8 _trackIndex);
void            BLZfree          (MEMallocator* _allocator, BLSsoundTrack* _sndtrack);
void            BLZresetYM       (void);

#ifdef DEMOS_LOAD_FROMHD
void            BLZtestPlay      (BLSplayer* _player, char* _filesamplename, char* _filetracename, u8 _mode);
#endif

#if BLS_SCOREMODE_ENABLE
/* Advanced functions can play score mode / blitz mode and also capture blitz when BLS_ENABLE_OPTIMIZED_DUMP defined */
void*           BLSread          (MEMallocator* _allocator, MEMallocator* _allocatorTemp, void* _buffer, BLSsoundTrack** _sndtrack);
void            BLSplayerInit    (MEMallocator* _allocator, BLSplayer* _player, BLSsoundTrack* _sndtrack, BLZdmaMode _dmamode);
void            BLSplayerFree    (MEMallocator* _allocator, BLSplayer* _player);
void            BLSupdate        (BLSplayer* _player);
void            BLSupdAsync      (BLSplayer* _player);
u32             BLZasyncPlay     (BLSplayer* _player, u8 _trackindex, void* _buffer, u32 _buffersize);
void            BLSgoto          (BLSplayer* _player, u8 _trackIndex);
void            BLSfree          (MEMallocator* _allocator, BLSsoundTrack* _sndtrack);
#ifdef DEMOS_LOAD_FROMHD
void            BLStestPlay      (BLSplayer* _player, char* _filesamplename, char* _filetracename, u8 _mode);
#endif
#endif

#ifdef DEMOS_LOAD_FROMHD
void            BLSsave         (BLSsoundTrack* _sndtrack, char* _filename);
void            BLSwriteHeader  (bool _blitzmode, FILE* _file);
void            BLSwriteSounds  (BLSsoundTrack* _sndtrack, FILE* _file);
void            BLSwriteScore   (BLSsoundTrack* _sndtrack, FILE* _file);
void            BLSdumpHeap     (BLSsoundTrack* _sndtrack, char* _filename);
#endif

#endif
