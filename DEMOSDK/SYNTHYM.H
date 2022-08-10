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

#ifndef SYNTHYM_H
#define SYNTHYM_H

#define SND_YM_NBMAX_CURVES 8
#define SND_YM_NB_CHANNELS 3 

#define SND_YM_FORMAT      0xBEE9500DUL
#define SND_YM_FORMAT_REV  1

#define SND_YM_PLAYER_NBLINES_INSTR 48

#ifndef __TOS__
#   define SNDYM_REGS_MIRRORING 1
#endif


enum SNDYMcurveType_
{
    SND_YM_AMPLITUDE,
    SND_YM_MIX,
    SND_YM_FREQ,
    SND_YM_FINE_FREQ,
    SND_YM_FIX_FREQ,
    SND_YM_ENVFREQ,
    SND_YM_FINE_ENVFREQ,
    SND_YM_ENVSHAPE,
    SND_YM_NOISEFREQ,
    SND_YM_SQUARESYNC
};
typedef enum SNDYMcurveType_ SNDYMcurveType;


struct SNDYMcurve_
{
    u8     curveType;
    u8     attacklen;
    u8     sustainlen;
    u8     decaylen;
    u16    tick;
    u16    len;
    s8*    levels;
    s8     min;      /* can be optimized */
    s8     max;      /* can be optimized */
};
typedef struct SNDYMcurve_ SNDYMcurve;


struct SNDYMsound_
{
    SNDYMcurve  curves[SND_YM_NBMAX_CURVES];
    u8          nbcurves;
};
typedef struct SNDYMsound_ SNDYMsound;

typedef char SNDYMsoundName[24];


struct SNDYMsoundSet_
{
    SNDYMsound*      sounds;
    u16              nbSounds;
    SNDYMsoundName*  names;
};
typedef struct SNDYMsoundSet_ SNDYMsoundSet;


struct SNDYMchannelCurve_
{
    u16     currenttick;
    u8      current;
    bool    running;
};
typedef struct SNDYMchannelCurve_ SNDYMchannelCurve;


struct SNDYMchannel_
{
    SNDYMchannelCurve curvestate[SND_YM_NBMAX_CURVES];

    bool square;
    bool noise;
    bool env;
    s8   finefreq;
    u16  freq;
    u16  envfreq;
    s8   fineenvfreq;
    bool freqsync;
    u8   amplitude;
    s8   pitchbend;

    s16  portaoffset;
    s16  portainc;
    s16  portfreqoffset;
    u8   portkey;
};
typedef struct SNDYMchannel_ SNDYMchannel;


struct SNDYMcommand_
{
    u8      key;
    bool    pressed;
    bool    justpressed;
    s8      finetune;
    u8      portamientoticks;   
	s8      soundindex;
    u8      scorevolume;
    bool    scorevolumeset;
    u8      pitchbendticks;
    s8      pitchbendrange;
    bool    forcesqrsync;

    bool	mute;
};
typedef struct SNDYMcommand_ SNDYMcommand;


struct YMregs_
{
    u8  regscopy[16];

    bool            envshapeReselected;
    bool            squareRestarted[SND_YM_NB_CHANNELS];
};
typedef struct YMregs_ YMregs;


struct SNDYMplayer_
{
    SNDYMsoundSet*  soundSet;
    SNDYMchannel    channels[SND_YM_NB_CHANNELS];
    SNDYMcommand    commands[SND_YM_NB_CHANNELS];
    u8*             volumes;

#   if SNDYM_REGS_MIRRORING
    YMregs          YMregs;
#   endif
};
typedef struct SNDYMplayer_ SNDYMplayer;


#ifdef DEMOS_LOAD_FROMHD
bool  SNDYMloadSounds  (MEMallocator* _allocator, char* _filename, SNDYMsoundSet* _soundset);
void  SNDYMwriteSounds (SNDYMsoundSet* _soundsSet, FILE* _file);
char* SNDYMgetError    (void);
#endif

u8*  SNDYMreadSounds   (MEMallocator* _allocator, void* _buffer, SNDYMsoundSet* _soundsSet);
void SNDYMinitPlayer   (MEMallocator* _allocator, SNDYMplayer* _player, SNDYMsoundSet* _soundSet);
void SNDYMfreePlayer   (MEMallocator* _allocator, SNDYMplayer* _player);
bool SNDYMupdate       (SNDYMplayer* _player);
void SNDYMstop         (SNDYMplayer* _player);
void SNDYMfreeSounds   (MEMallocator* _allocator, SNDYMsoundSet* _soundSet);

u8*  SNDYMdrawSoundCurves (SNDYMplayer* _player, bool _active[SND_YM_NB_CHANNELS], u8* pinstr);

#endif
