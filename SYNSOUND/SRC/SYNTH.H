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


/*! @defgroup SYNTH

    SYNTH prototypes use blitter to mix PCM voices on 2 voices
*/
    
#ifndef SYNTH_H
#define SYNTH_H

#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\ALLOC.H"
#include "DEMOSDK\LOAD.H"

#define SND_SCREEN_FREQ         50UL

#define SND_FREQ 1

#if SND_FREQ == 1
#   define SND_DMA_FREQ            50000UL
#   define SND_FRAME_NBSAMPLES     1000 /*(SND_DMA_FREQ / SND_SCREEN_FREQ)*/
#else
#   define SND_DMA_FREQ            25000UL
#   define SND_FRAME_NBSAMPLES     500 /*(SND_DMA_FREQ / SND_SCREEN_FREQ)*/
#endif

extern volatile u8		SNDleftVolume;
extern volatile u8		SNDrightVolume;
extern volatile s8		SNDfademasterVolume;
extern volatile u8		SNDspeedfade;
extern volatile u16		SNDdmaLoopCount;
extern void*			SNDsourceTransfer;
extern volatile void*	SNDendSourceTransfer;
extern void*			SNDdestTransfer;

extern volatile void*	SNDlastDMAposition;	    /* read only */
extern volatile s8		SNDmasterVolume;		/* ready only - debug purpose */


struct SNDsynPlayerInitParam_
{
	int dummy;
};
typedef struct SNDsynPlayerInitParam_ SNDsynPlayerInitParam;

struct SNDsynSample_
{
    s8* data;
    u32 length;
    u32 periodlength;
    u16 nbperiods;

    float fundamentalfreq;
    float correctedfreq;

    u16 loopsperframe;

    float* tmpdata;
};
typedef struct SNDsynSample_ SNDsynSample;

struct SNDsynSound_
{
    SNDsynSample*   samples;
    u8              nbsamples;
    u8              transposerange;

    u8              sampleindexes[16];
    u8              nbsampleindexes;

    u8              sustainindexes[16];
    u8              nbsustainindexes;

    u8              sustainindex;
};
typedef struct SNDsynSound_ SNDsynSound;

typedef enum SNDsynNote_
{
    SNDsynC,
    SNDsynCsharp,
    SNDsynD,
    SNDsynDsharp,
    SNDsynE,
    SNDsynF,
    SNDsynFsharp,
    SNDsynG,
    SNDsynGsharp,
    SNDsynH,
    SNDsynHsharp,
    SNDsynI,

} SNDsynNote;

typedef enum SNDsynNoteFrench_
{
    SNDsynDO,
    SNDsynDOdiese,
    SNDsynRE,
    SNDsynREdiese,
    SNDsynMI,
    SNDsynFA,
    SNDsynFAdiese,
    SNDsynSOL,
    SNDsynSOLdiese,
    SNDsynLA,
    SNDsynLAdiese,
    SNDsynSI

} SNDsynNoteFrench;


struct SNDsynSoundSet_
{
    SNDsynSound* sounds[12];
};
typedef struct SNDsynSoundSet_ SNDsynSoundSet;

struct SNDsynSoundCtrl_
{
    SNDsynSound* currentsound;
    u16          currentsample;
    u16          currentoffset;
};
typedef struct SNDsynSoundCtrl_ SNDsynSoundCtrl;

struct SNDsynSampleConfig_
{
    u8 nbperiods;
};
typedef struct SNDsynSampleConfig_ SNDsynSampleConfig;

struct SNDsynConfig_
{
    float   basefreq;
    u8      transposerange;
    u8      nbsamples;
    SNDsynSampleConfig* sampleconfigs;
};
typedef struct SNDsynConfig_ SNDsynConfig;

/* frames are the static data part of the voices */
/* when replaying music, this part will be full precomputed */
struct SNDsynSoundFrame_
{
    SNDsynSample*   sample;      /* if NULL => sound mute */
    u16             nbloops;
    u16             bytesoffset;
    u16             bytetotransfer;
    u16*            mask;
    u8              volume;
};
typedef struct SNDsynSoundFrame_ SNDsynSoundFrame;

struct SNDsynFrame_
{
    SNDsynSoundFrame*   soundFrames;
    u8                  nbsoundFrames;
    u8                  transpose;
    u8                  volume;    
    u16*                mask;

    SNDsynSound*        lastSound;
    u16                 lastSoundSrcOffset;
    u16                 lastSampleIndexIndex;
    u16                 lastSampleIndex;
    u8                  lastvolume;
};
typedef struct SNDsynFrame_ SNDsynFrame;

/*
struct SNDsynModulator_
{
    u8  framescount;
    u8  stepcount;
    u8  speed;
    u8  length;
    u8* steps;
};

typedef enum SNDsynModulatorType_
{
    SNDsynModulatorType_VOLUME,
    SNDsynModulatorType_MASK, 
    SNDsynModulatorType_WAVE,
    SNDsynModulatorType_NBMODULATORS
} SNDsynModulatorType;

struct SNDsynLFO_
{
    u8  frames;
    u8  nbframes;

    SNDsynModulator     modulators[SNDsynModulatorType_NBMODULATORS];
};*/

struct SNDsynVoice_
{
    SNDsynFrame     frame;

    u16             sampledisplay;

/*    SNDsynLFO       lfo;

    SNDsynSoundCtrl currentsound;
    u16             chaneloffset;
    u16*            copyMask;*/
/*    s8              volume; */
/*    u8              transpose; */
/*    bool            mute;*/
};
typedef struct SNDsynVoice_ SNDsynVoice;

struct SNDsynPlayer_
{
    SNDsynVoice         voices[2];
  
    SNDsynSoundFrame    soundFrames[2][256];

    s8*                 buffer;
    s8*                 dmabuffers[2];
    u16                 dmabufferlen;
    u16                 backbuffer;
};
typedef struct SNDsynPlayer_ SNDsynPlayer;

#ifndef SYNTH_C

extern SNDsynPlayer synth;

#endif

void SNDsynUpdate             (u8 _keyb1, u8 _keyb2);
void SNDsynPlayerInit         (RINGallocator* _allocator, SNDsynPlayerInitParam* _init);
void SNDsynPlayerShutdown     (RINGallocator* _allocator);
void SNDsynSound_drawCurve    (SNDsynSample* _sample, void* _screen);
void SNDsynSample_drawCurve   (void* _sample, u16 _nbsamples, u16 _incx, void* _screen);
void SNDsynSample_drawXorPass (u16 _nbsamples, void* _screen);

SNDsynSoundSet*     SNDsynthLoad   (char* _filename);

void SNDtest (SNDsynSoundSet* _soundSet);

#ifdef DEMOS_DEBUG
u16  SNDsynPlayerTrace        (void* _image, u16 _pitch, u16 _planePitch, u16 _y);
#endif

#endif
