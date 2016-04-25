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

#define SYNTH_C

#include "DEMOSDK\SYNTH.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\STANDARD.H"

#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"

#include <math.h>

#ifndef __TOS__
#include <windowsx.h>
volatile u8		SNDleftVolume;
volatile u8		SNDrightVolume;
volatile s8		SNDmasterVolume;	/* ready only - debug purpose */

void SNDsynMonitor (void) {}
#endif

void SNDsynMonitor (void);

/*
20   0db = 1.0
19  -2db = 0.8
18  -4db = 0.63
17  -6db = 0.5
16  -8db = 0.4
15 -10db = 0.32
14 -12db = 0.25
13 -14db = 0.2
12 -16db = 0.16
11 -18db = 0.125
10 -20db = 0.1
9  -22db = 0.08
8  -24db = 0.063
7  -26db = 0.05
6  -28db = 0.04
5  -30db = 0.032
4  -32db = 0.025
3  -34db = 0.02
2  -36db = 0.016
1  -38db = 0.0125
0  -40db = 0.01
*/

/* Sums of these channels achieve a quite constant global volume 
u8 SNDchannelVolume[] = { 20, 19, 18, 17, 16, 13, 0 };*/

static void SND_waitMicrowire (void)
{
	while (*HW_MICROWIRE_MASK != 0x7FF);
}

SNDsynPlayer synth;

/*-----------------------------------------------------------------------------
   Constants
------------------------------------------------------------------------------*/

#ifdef __TOS__
#define sinf sin
#endif

/* Synth constants */

#define FC2PI     6.28318530717958647692528676655901f
#define FSEMITONE 1.059463094359295f                    /* 2^1/12 */

/*     volume       | max sample value | */
/* 2 ^ 7        -1 = 127                 */
/* 2 ^ 6.5      -1 = 90                  */
/* 2 ^ 6        -1 = 63                  */
/* 2 ^ 5.5      -1 = 44                  */
/* 2 ^ 5        -1 = 31                  */
/* 2 ^ 4.5      -1 = 22                  */
/* 2 ^ 4        -1 = 15                  */
/* 2 ^ 3.5      -1 = 10                  */
/* 2 ^ 3        -1 = 7                   */
/* 2 ^ 2.5      -1 = 5                   */
/* 2 ^ 2        -1 = 3                   */
/* 2 ^ 1.5      -1 = 2                   */
/* 2 ^ 1        -1 = 1                   */

#define SND_SYN_NBSEMITONES 12

#define SND_SYN_ALLOC_MAIN(A)   malloc(A)
#define SND_SYN_ALLOC_TEMP(A)   malloc(A)
#define SND_SYN_FREE_TEMP(A)    free(A)
#define SND_SYN_ALLOC_SAMPLE(A) malloc (A)





static char* SNDsyndelims = "\n\r";

SNDsynSoundSet* soundSet1 = NULL;
SNDsynSoundSet* soundSet2 = NULL;

/*
SNDsynSampleConfig  configTest1Samples[3] = {40, 1, 40};
SNDsynConfig        configTest1 = {220.0f, 2, ARRAYSIZE(configTest1Samples), configTest1Samples};

SNDsynSampleConfig  configTest2Samples[1] = {1};
SNDsynConfig        configTest2 = {220.0f, 1, ARRAYSIZE(configTest2Samples), configTest2Samples};



SNDsynSampleConfig  configTest3Samples[5] = {10, 1, 1, 1, 5};
SNDsynConfig        configTest3 = {220.0f, 2, ARRAYSIZE(configTest3Samples), configTest3Samples};

SNDsynSampleConfig  configTest4Samples[1] = {2};
SNDsynConfig        configTest4 = {220.0f, 2, ARRAYSIZE(configTest4Samples), configTest4Samples};
*/


float SNDsynSound_allocatePeriods(SNDsynSound* _sound, u16 _sampleindex, float _fundamentalfreq, u16 _nbPeriods)
{
    u32 nbsamples = (u32)((((float)(SND_DMA_FREQ * 2)) / _fundamentalfreq) + 0.5f);
    u32 roundmultiple = (1 << _sound->transposerange);
    u32 transposerangemask = ~(roundmultiple - 1);
    SNDsynSample* sample = &_sound->samples[_sampleindex];
    u32 nbsamples1, nbsamples2;
    float freq1, freq2;
    float error1, error2, error;


    ASSERT (_sampleindex < _sound->nbsamples);

    nbsamples++;
    nbsamples >>= 1;

    nbsamples1 = nbsamples2 = nbsamples;

    nbsamples1 &= transposerangemask;

    nbsamples2 += roundmultiple >> 1;
    nbsamples2 &= transposerangemask;

    ASSERT(nbsamples1 < 65536UL);
    ASSERT(nbsamples2 < 65536UL);

    freq1 = ((float)SND_DMA_FREQ) / (float) nbsamples1;
    freq2 = ((float)SND_DMA_FREQ) / (float) nbsamples2;

    error1 = (freq1 - _fundamentalfreq);
    error2 = (_fundamentalfreq - freq2);

    if ( error1 < 0 ) 
        error1 = -error1;

    if ( error2 < 0 ) 
        error2 = -error2;

    if ( error1 < error2 )
    {
        error = error1;
        nbsamples = nbsamples1;
        sample->correctedfreq = freq1;
    }
    else
    {
        error = error2;
        nbsamples = nbsamples2;
        sample->correctedfreq = freq2;
    }

    sample->fundamentalfreq = _fundamentalfreq;
    sample->periodlength    = nbsamples;
    nbsamples *= _nbPeriods;
    sample->length          = nbsamples;
    sample->loopsperframe   = (u16) STDdivu(SND_DMA_FREQ / SND_SCREEN_FREQ, (u16) nbsamples);

    sample->tmpdata = (float*) SND_SYN_ALLOC_TEMP (nbsamples * sizeof(float));
    sample->data    = (s8*) SND_SYN_ALLOC_SAMPLE(nbsamples << 1);

    STDmset (sample->tmpdata, 0, nbsamples * sizeof(float));
    
    return error / (_fundamentalfreq * (FSEMITONE - 1.0f));
}


void SNDsynSample_finalize(SNDsynSample* _sample)
{
    u16 length = (u16) _sample->length;
    u16    t;
    float* src  = _sample->tmpdata;
    s8*    dest = _sample->data;


    for (t = 0 ; t < length ; t++)
    {
        float s = *src++;
        int   v = (int)(s * 127.0f);


        if ( v >= 0 )
        {
            *dest++ = 0;
        }
        else
        {
            *dest++ = 0xFF;
        }

        *dest++ = (s8)v;
    }

    SND_SYN_FREE_TEMP(_sample->tmpdata);
}

void SNDsynSound_finalize(SNDsynSound* _sound)
{
    u16 t;


    for (t = 0 ; t < _sound->nbsamples ; t++)
    {
        SNDsynSample_finalize ( &_sound->samples[t] );
    }
}


void SNDsynSample_dump (SNDsynSample* _sample, char* _filename, u16 _loops)
{
    u16 t;
    FILE* file = fopen (_filename, "wb");

    for (t = 0 ; t < _loops ; t++)
    {
        u32 i, nb = _sample->length << 1;

        for (i = 1 ; i < nb ; i += 2)
        {
            fwrite (&_sample->data[i], 1, 1, file);
        }
    }

    fclose (file);
}

void SNDsynSound_dump (SNDsynSound* _sound, char* _filename)
{
    char temp[256];
    u16 t;
        
        
    for (t = 0 ; t < _sound->nbsamples; t++)
    {
        sprintf (temp, "%s_%d.RAW", _filename, t);
        SNDsynSample_dump (&_sound->samples[t], temp, 1);
    }
}



void SNDsynSound_allocateSound(SNDsynSound* _sound, u8 _nbSamples, u8 _transposerange)
{
    _sound->samples = (SNDsynSample*) SND_SYN_ALLOC_MAIN (_nbSamples * sizeof(SNDsynSample));
    _sound->nbsamples = _nbSamples;
    _sound->transposerange = _transposerange;

    STDmset (_sound->samples, 0, _nbSamples * sizeof(SNDsynSample));
}


void SNDsynSound_addTriangle(SNDsynSample* _sample, float _freq, float _gain, float _point)
{
    u32 nbsamples  = _sample->length;
    float* destsample = _sample->tmpdata;
    float inc = _freq / (float) SND_DMA_FREQ;
    u16 i;
    float t = 0.0f;
    float a = _point;
    float b = 0.5f - a;


    for (i = 0 ; i < nbsamples ; i++, t += inc)
    {
        float v;

        if (t < 0.5f)
        {
            if (t > a)
            {
                v = 1.0f - (t - a) / b;
            }
            else
            {
                v = t / a;
            }
        }
        else
        {
            float t2 = t - 0.5f;
            
            if (t2 < b)
            {
                v = -t2 / b;
            }
            else
            {
                v = -1.0f + (t2 - b) / a;
            }
        }
    
        *destsample++ += v * _gain;
    }
}


void SNDsynSound_addSineWave(SNDsynSample* _sample, float _sinefreq, float _gain)
{
    u32 nbsamples  = _sample->length;
    float* destsample = _sample->tmpdata;
    float inc = FC2PI * (_sinefreq / (float) SND_DMA_FREQ);
    u16 i;
    float t = 0.0f;


    for (i = 0 ; i < nbsamples ; i++, t += inc)
    {
        *destsample++ += (float)sinf(t) * _gain;
    }
}


void SNDsynSample_ramp (SNDsynSample* _sample, float _gain1, float _gain2)
{
    u32 nbsamples  = _sample->length;
    float* destsample = _sample->tmpdata;
    float inc = 1.0f / (float) (nbsamples - 1);
    u16 i;
    float t1 = 0.0f;
    float t2 = 1.0f;


    for (i = 0 ; i < nbsamples ; i++, t1 += inc, t2 -= inc)
    {
        float fv = *destsample;
        
        *destsample++ = fv * _gain2 * t1 + fv * _gain1 * t2;
    }   
}


void SNDsynSample_drawFloatCurve (SNDsynSample* _sample, void* _screen)
{
    u16* line = (u16*) _screen;
    u16* disp = line + (34*80);
    float* sample = _sample->tmpdata;
    u16 i,b = 0;


    for (i = 0 ; i < 40 ; i++)
    {
        u16 p1 = 0x8000;

        do
        {
            float v = (*sample++) * 127.0f / 4.0f;
            s16 s = (s16) v;
            
            s *= 80;

            disp[s] |= PCENDIANSWAP16(p1);

            p1 >>= 1;

            if ( ++b >= _sample->length )
        {
                goto endsample;
        }
        }
        while (p1 != 0);

        disp += 2;
    }   
endsample: ;
}


void SNDsynSample_drawCurve (void* _sample, u16 _nbsamples, u16 _incx, void* _screen)
{
    u16* line = (u16*) _screen;
    u16* disp = line + (34*80);
    s8* sample = (s8*)_sample + 1;
    u16 i,b = 0;


    for (i = 0 ; i < 40 ; i++)
    {
        u16 p1 = 0x8000;

        do
        {
            s16 s = *sample;
            sample += _incx;
            s >>= 2;
            s *= 80;

            disp[s] |= PCENDIANSWAP16(p1);

            p1 >>= 1;

            if ( ++b >= _nbsamples )
            {
                goto endsample;
            }
        }
        while (p1 != 0);

        disp += 2;
    }

endsample: 
    ;

#   ifndef __TOS__
    if (0)
    {
        u32 width = EMUL_WINDOW_WIDTH - 20;

        WINsetColor  ( EMULgetWindow(), 0, 0, 0);
        WINclear     ( EMULgetWindow() );

        sample = (s8*)_sample + 3;

        WINsetColor  ( EMULgetWindow(), 128, 128, 128);
        WINline ( EMULgetWindow(), EMUL_WINDOW_WIDTH, 130, EMUL_WINDOW_WIDTH + _nbsamples, 130);
        WINline ( EMULgetWindow(), EMUL_WINDOW_WIDTH, 390, EMUL_WINDOW_WIDTH + width, 390);

        for (i = 1 ; i < _nbsamples ; i++)
        {
            WINsetColor  ( EMULgetWindow(), 255, 0, 0);
            WINline ( EMULgetWindow(), EMUL_WINDOW_WIDTH + i-1, sample[-2] + 130, EMUL_WINDOW_WIDTH + i, *sample + 130);

            WINsetColor  ( EMULgetWindow(), 0, 255, 0);        
            WINline ( EMULgetWindow(), EMUL_WINDOW_WIDTH + (i-1) * width / _nbsamples, sample[-2] + 390 , EMUL_WINDOW_WIDTH + i * width / _nbsamples, *sample + 390);

            sample += 2;
        }

        WINsetColor  ( EMULgetWindow(), 128, 128, 128);
        WINline ( EMULgetWindow(), EMUL_WINDOW_WIDTH + _nbsamples-1, sample[-2] + 130, EMUL_WINDOW_WIDTH + i, ((s8*)_sample)[1] + 130);
        WINline ( EMULgetWindow(), EMUL_WINDOW_WIDTH + (i-1) * width / _nbsamples, sample[-2] + 390 , EMUL_WINDOW_WIDTH + i * width / _nbsamples, ((s8*)_sample)[1] + 390);
    }
#   endif
}

void SNDsynSample_drawXorPass (u16 _nbsamples, void* _screen)
{
    u16* line = (u16*) _screen;
    u16* disp = line + (34*80);
    u16 i,t;
    u16  nbwords = (_nbsamples + 15) >> 4;

    u16* l = line + 80; 

    for (i = 0 ; i < 34 ; i++)
    {
        disp = l;

        for (t = 0 ; t < nbwords ; t += 4)
        {
            *disp ^= disp[-80]; disp += 2;
            *disp ^= disp[-80]; disp += 2;
            *disp ^= disp[-80]; disp += 2;
            *disp ^= disp[-80]; disp += 2;
        }

        l += 80;
    }

    l = line + (68 * 80);

    for (i = 0 ; i < 34 ; i++)
    {
        disp = l;

        for (t = 0 ; t < nbwords ; t += 4)
        {
            *disp ^= disp[80]; disp += 2;
            *disp ^= disp[80]; disp += 2;
            *disp ^= disp[80]; disp += 2;
            *disp ^= disp[80]; disp += 2;        
        }

        l -= 80;
    }
}

void SNDsynSound_drawCurve (SNDsynSample* _sample, void* _screen)
{
    SNDsynSample_drawCurve (_sample->data, (u16) _sample->length, 2, _screen);
}


void SNDsynPlayerInit(RINGallocator* _allocator, SNDsynPlayerInitParam* _init)
{
    SYSsoundtrackUpdate = (void*) SNDsynMonitor;   /* setup synth routine into VBL */

    DEFAULT_CONSTRUCT(&synth);

	synth.dmabufferlen = (SND_DMA_FREQ / SND_SCREEN_FREQ * 2UL);    /* stereo sample */ 
	synth.buffer       = (s8*) RINGallocatorAlloc (_allocator, synth.dmabufferlen * 2);

    STDmset (synth.buffer, 0, synth.dmabufferlen * 2);

    synth.dmabuffers[0] = synth.buffer;
    synth.dmabuffers[1] = synth.buffer + synth.dmabufferlen;
    	
    *HW_MICROWIRE_MASK = HW_MICROWIRE_MASK_SOUND;
    
    *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME | 40;
    while (*HW_MICROWIRE_MASK != HW_MICROWIRE_MASK_SOUND);

    /**HW_MICROWIRE_DATA = HW_MICROWIRE_MIXER_YM;
    while (*HW_MICROWIRE_MASK != HW_MICROWIRE_MASK_SOUND);*/

#   if SND_FREQ == 1
    *HW_DMASOUND_MODE = HW_DMASOUND_MODE_50066HZ | HW_DMASOUND_MODE_STEREO;
#   else
    *HW_DMASOUND_MODE = HW_DMASOUND_MODE_25033HZ | HW_DMASOUND_MODE_STEREO;
#   endif

    EMULcreateSoundBuffer (SND_FRAME_NBSAMPLES * 2, true);
}


void SNDsynUpdateVoiceFrame (SNDsynVoice* _voice, u32 _backbuffer, u16 _blitmask)
{
    static void*        lastmaskadr    = (void*) 0x1UL;
    u8                  transpose      = _voice->frame.transpose;
    SNDsynSoundFrame*   snd            = _voice->frame.soundFrames;
    u16                 t;


    *HW_BLITTER_ADDR_DEST   = _backbuffer;
    *HW_BLITTER_XINC_SOURCE = 2 << transpose;               /* can be set to do octave transpose */

    for (t = _voice->frame.nbsoundFrames ; t > 0 ; t--, snd++)
    {
        u16 bytetotransfer = snd->bytetotransfer;


        if ( snd->sample == NULL )
        {
            *HW_BLITTER_ADDR_SOURCE = _backbuffer;          /* dummy valid adr */
            *HW_BLITTER_HOP = HW_BLITTER_HOP_BIT1;
            *HW_BLITTER_OP  = HW_BLITTER_OP_BIT0; 
        }
        else
        {
            *HW_BLITTER_OP  = HW_BLITTER_OP_S; 
            *HW_BLITTER_ADDR_SOURCE = (u32) (snd->sample->data + snd->bytesoffset);

            if ( snd->mask != NULL )
            {
                if ( lastmaskadr != snd->mask )
                {
                STDmcpy (HW_BLITTER_HTONE, snd->mask, 32);
                    lastmaskadr = snd->mask;
                }

                *HW_BLITTER_HOP = HW_BLITTER_HOP_SOURCE_AND_HTONE;          /* try combination with htone for special effects (noisy) */
            }
            else
            {
                *HW_BLITTER_HOP = HW_BLITTER_HOP_SOURCE;
            }
        }

        *HW_BLITTER_YINC_SOURCE = *HW_BLITTER_XINC_SOURCE - (bytetotransfer << transpose);      /* Y inc should integrate X inc value when looping */
        *HW_BLITTER_XSIZE       = bytetotransfer >> 1;
        *HW_BLITTER_YSIZE       = snd->nbloops;                                                 /* need to compute the right number of loops */
        *HW_BLITTER_CTRL2       = _blitmask + snd->volume;
        *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */

        EMULblit();
    }

    ASSERT (*HW_BLITTER_ADDR_DEST == (_backbuffer + synth.dmabufferlen));
}


extern u16 mask[8][16];

/*
void SNDsynUpdateLFO (SNDsynVoice* _voice)
{
    SNDsynModulator* modulator;
    u16 t;

    for (t = 0 ; t < ARRAYSIZE(_voice->lfo.modulators) ; t++)
    {
        modulator = &_voice->lfo.modulators[t];

        if (modulator->length > 0)
        {
            modulator->framescount++;

            if (modulator->framescount >= modulator->speed)
            {
                modulator->framescount = 0;
                modulator->stepcount++;

                if (modulator->stepcount >= modulator->length)
                {
                    modulator->stepcount = 0;
                }
            }
        }
    }

    modulator = &_voice->lfo.modulators[SNDsynModulatorType_VOLUME];
    
    if ( modulator->length > 0)
    {
        _voice->frame.volume = modulator->steps[ modulator->stepcount ];
    }
    else
    {
        _voice->frame.volume = 0;
    }

    modulator = &_voice->lfo.modulators[SNDsynModulatorType_MASK];

    if ( modulator->length > 0)
    {
        _voice->frame.mask = &mask[ modulator->steps[ modulator->stepcount ] ][0];
    }
    else
    {
        _voice->frame.mask = NULL;
    }

    if (_voice->frame.lastSound != NULL) // TEMPORARY : TODO LINK TO SOUND / UPDATE ELSEWHERE 
    {
        modulator = &_voice->lfo.modulators[SNDsynModulatorType_WAVE];

        if ( modulator->length > 0)
        {
            _voice->frame.lastSound->sampleindexes[ _voice->frame.lastSound->sustainindex ] = modulator->steps[ modulator->stepcount ];
        }
    }
}
*/


void SNDsynUpdateFrames (SNDsynVoice* _voice, bool _pressed, SNDsynSound* _sound, SNDsynSoundFrame* _freeframes)
{
    /* convertions between src bytes and dest bytes should take transpose in account */
    SNDsynSoundFrame* p = _freeframes;
    u8  framescount = 0;

    u8  transpose = _voice->frame.transpose;
    u8   volume          = _voice->frame.volume;
    u16* mask            = _voice->frame.mask;

    u16 startindex = 0;
    u16 destbytesremaining  = synth.dmabufferlen;
    u16 sourceByteslength;
    
    SNDsynSound* currentsound; 
    
    
    _voice->frame.soundFrames = _freeframes;
    _voice->sampledisplay = 0;

    if (_pressed)
        currentsound = _sound;
    else
        currentsound = _voice->frame.lastSound;

    if (currentsound == NULL)
        goto finishsound;
    
    ASSERT(transpose <= currentsound->transposerange);

    if (currentsound == _voice->frame.lastSound)
    {
        SNDsynSample* sample;
        startindex = _voice->frame.lastSampleIndexIndex;
        sample = &_voice->frame.lastSound->samples[_voice->frame.lastSampleIndex];

        sourceByteslength = (u16) (sample->length << 1);

        if ( _voice->frame.lastSoundSrcOffset > 0 )
        {
            u16 destbytestotransfer = (sourceByteslength -_voice->frame.lastSoundSrcOffset) >> transpose;

            if (destbytestotransfer > destbytesremaining)
            {
                destbytestotransfer = destbytesremaining;
            }
            else
            {
                _voice->sampledisplay = destbytestotransfer;
            }

            p->bytesoffset    = _voice->frame.lastSoundSrcOffset;
            p->bytetotransfer = destbytestotransfer;
            p->sample         = sample;
            p->nbloops        = 1;
            p->mask           = mask;
            p->volume         = _voice->frame.lastvolume;

            p++;
            framescount++;

            destbytesremaining -= destbytestotransfer;

            _voice->frame.lastSoundSrcOffset += destbytestotransfer << transpose;
            if ( _voice->frame.lastSoundSrcOffset == sourceByteslength )
            {
                _voice->frame.lastSoundSrcOffset = 0;
            }
        }

        if ( destbytesremaining > 0 )
        {
            if ( startindex != currentsound->sustainindex )
            {
                startindex++;
            }
        }
    }

    if ( destbytesremaining > 0 )
    {
        u16 t;

        _voice->frame.lastSoundSrcOffset = 0;

        for (t = startindex ; t < currentsound->nbsampleindexes ; t++)
        {
            SNDsynSample* currentsample = &currentsound->samples[ currentsound->sampleindexes[t] ];
            sourceByteslength = (u16) (currentsample->length << 1);


            if ( t == currentsound->sustainindex )
            {
                if ( _pressed )
                {
                    u32 result  = STDdivu (destbytesremaining, sourceByteslength >> transpose);
                    u16 nbloops = (u16) result;

                    if (nbloops > 0)
                    {
                        p->bytesoffset    = 0;
                        p->bytetotransfer = sourceByteslength >> transpose;
                        p->sample         = currentsample;
                        p->nbloops        = nbloops;
                        p->mask           = mask;
                        p->volume         = volume;

                        p++;
                        framescount++;
                    }

                    destbytesremaining = (u16) (result >> 16);

                    if ( destbytesremaining > 0 )
                    {
                        p->bytesoffset    = 0;
                        p->bytetotransfer = destbytesremaining;
                        p->sample         = currentsample;
                        p->nbloops        = 1;
                        p->mask           = mask;
                        p->volume         = volume;

                        p++;
                        framescount++;

                        _voice->frame.lastSoundSrcOffset = destbytesremaining << transpose;
                        destbytesremaining = 0;
                    }
                }
                /*else skip sustain when key not pressed */
            }
            else
            {
                u16 destbytestotransfer = sourceByteslength >> transpose;
                
                if ( destbytestotransfer > destbytesremaining )
                {
                    destbytestotransfer = destbytesremaining;
                    _voice->frame.lastSoundSrcOffset = destbytestotransfer << transpose;
                }
                    
                p->bytesoffset    = 0;
                p->bytetotransfer = destbytestotransfer;
                p->sample         = currentsample;
                p->nbloops        = 1;
                p->mask           = mask;
                p->volume         = volume;

                destbytesremaining -= destbytestotransfer;

                p++;
                framescount++;
            }

            if (destbytesremaining == 0)
            {
                break;
            }
        }

        if (t >= currentsound->nbsampleindexes)
        {
            _voice->frame.lastSound = NULL;
            _voice->frame.lastSoundSrcOffset = 0;
            _voice->frame.lastSampleIndexIndex = 0;
            _voice->frame.lastSampleIndex = 0;
        }
        else
        {
            _voice->frame.lastSound = currentsound;
            _voice->frame.lastSampleIndexIndex = t;
            _voice->frame.lastSampleIndex = currentsound->sampleindexes[t];
            _voice->frame.lastvolume = _voice->frame.volume;
        }
    }

finishsound:

    if ( destbytesremaining > 0 )
    {
        /* no sound */
        p->bytetotransfer = destbytesremaining; 
        p->bytesoffset = 0; 
        p->nbloops = 1; 
        p->sample = NULL; 
        p->mask = mask;

        p++;
        framescount++;

        _voice->frame.lastSound = NULL;
        _voice->frame.lastSoundSrcOffset = 0;
    }

    _voice->frame.nbsoundFrames = framescount;
}


void SNDsynUpdate (u8 _keyb1, u8 _keyb2)
{
    u32 frontbuf = (u32) synth.dmabuffers[synth.backbuffer];
    u32 endbuf = frontbuf + synth.dmabufferlen;

#   ifndef __TOS__
    u8 playBuffer = EMULgetPlayOffset () >= (SND_FRAME_NBSAMPLES * 2);
    {
        static u8 oldBuffer = 0;

        if (oldBuffer == playBuffer)
        {
            return;
        }
        oldBuffer = playBuffer;
    }
#   endif

    synth.backbuffer ^= 1;

    (*HW_DMASOUND_STARTADR_H) = (u8)(frontbuf >> 16);
    (*HW_DMASOUND_STARTADR_M) = (u8)(frontbuf >> 8);
    (*HW_DMASOUND_STARTADR_L) = (u8) frontbuf;

    (*HW_DMASOUND_ENDADR_H) = (u8)(endbuf >> 16);
    (*HW_DMASOUND_ENDADR_M) = (u8)(endbuf >> 8);
    (*HW_DMASOUND_ENDADR_L) = (u8) endbuf;

    {
#       ifdef __TOS__
        u8 p = *HW_VIDEO_COUNT_L;
        while (p == *HW_VIDEO_COUNT_L);
#       endif
    }

    {
        *HW_DMASOUND_CONTROL = HW_DMASOUND_CONTROL_PLAYONCE;
    }

    /*SNDsynUpdateLFO (&synth.voices[0]);
    SNDsynUpdateLFO (&synth.voices[1]);*/

    {
        SNDsynSound* currentSound1 =  _keyb1 != 0 ? soundSet1->sounds[_keyb1 - 1] : NULL;
        SNDsynSound* currentSound2 =  _keyb2 != 0 ? soundSet2->sounds[_keyb2 - 1] : NULL;

        if (currentSound1 !=  NULL)
        {
            if (synth.voices[0].frame.transpose > currentSound1->transposerange)
            {
                synth.voices[0].frame.transpose = currentSound1->transposerange;
            }
        }

        if (currentSound2 !=  NULL)
        {
            if (synth.voices[1].frame.transpose > currentSound2->transposerange)
            {
                synth.voices[1].frame.transpose = currentSound2->transposerange;
            }
        }
              
        *HW_COLOR_LUT = 0x70;
        SNDsynUpdateFrames (&synth.voices[0], _keyb1 != 0, currentSound1, synth.soundFrames[0]);

        *HW_COLOR_LUT = 0x50;
        SNDsynUpdateFrames (&synth.voices[1], _keyb2 != 0, currentSound2, synth.soundFrames[1]);
    }

    {
        u32 backbuf = (u32) synth.dmabuffers[synth.backbuffer];

        *HW_BLITTER_XINC_DEST       = 2;
        *HW_BLITTER_YINC_DEST       = 2;

        /* voice 0 */
        *HW_BLITTER_ENDMASK1        = 0xFFFF;                   /* -1 for left channel, 00FF for right channel in the 2nd pass */
        *HW_BLITTER_ENDMASK2        = 0xFFFF;
        *HW_BLITTER_ENDMASK3        = 0xFFFF;

        *HW_COLOR_LUT = 0x550;
        SNDsynUpdateVoiceFrame (&synth.voices[0], backbuf, 0);

        /* voice 1 */
        *HW_BLITTER_ENDMASK1        = PCENDIANSWAP16(0xFF00);   /* -1 for left channel, FF00 for right channel in the 2nd pass */
        *HW_BLITTER_ENDMASK2        = PCENDIANSWAP16(0xFF00);
        *HW_BLITTER_ENDMASK3        = PCENDIANSWAP16(0xFF00);

        *HW_COLOR_LUT = 0x770;
        SNDsynUpdateVoiceFrame (&synth.voices[1], backbuf, 8 | HW_BLITTER_CTRL2_FORCE_XTRA_SRC | HW_BLITTER_CTRL2_NO_FINAL_SRC_READ);

        *HW_COLOR_LUT = 0;

        EMULplaysound ((void*)backbuf, SND_FRAME_NBSAMPLES * 2, playBuffer != 0 ? 0 : SND_FRAME_NBSAMPLES * 2);
    }

   /* STDmcpy (synth.dmabuffers[0], sndcontroller->currentsound->samples[0].data, sndcontroller->currentsound->samples[0].length * 2 ); */
}

void SNDtestMask (s8* _sample, u16 _length, u8* _mask)
{
	u16 t;
	s8* s = _sample + 1;


	for (t = 0 ; t < _length ; t++)
	{
		*s &= _mask[t & 15];
		s += 2;
 	}
}

#define SND_SYN_GENERATE_ALLTONES   NULL


typedef void(*SNDsynSoundGenerator)(SNDsynSound*);

void SNDsynSoundSet_generate(SNDsynSoundSet* _soundset, SNDsynConfig* _config, bool* _generationFilter, SNDsynSoundGenerator _generationCallback)
{
    float freq = _config->basefreq;
    u16 t;


    for (t = 0 ; t < SND_SYN_NBSEMITONES ; t++)
    {
        if (( _generationFilter == NULL ) || _generationFilter[t] )
        {
            u16 s;
            SNDsynSound* sound = (SNDsynSound*) SND_SYN_ALLOC_MAIN( sizeof(SNDsynSound) );

            _soundset->sounds[t] = sound;

            DEFAULT_CONSTRUCT(sound);
        
            SNDsynSound_allocateSound (sound, _config->nbsamples, _config->transposerange);

            for (s = 0 ; s < _config->nbsamples ; s++)
            {
                SNDsynSampleConfig* sampleConfig = &_config->sampleconfigs[s];
               
                float error = SNDsynSound_allocatePeriods (sound, s, freq, sampleConfig->nbperiods);

                printf ("%d%%\n", (s32)(error * 100.0f));
            }

            _generationCallback (sound);

            SNDsynSound_finalize (sound);

            /*for (s = 0 ; s < _config->nbsamples ; s++)
            {
                SNDsynSample_dump (&sound.samples[1], "E:\\temp\\dumpsine.raw", 1);
            }*/
        }
        else
        {
            _soundset->sounds[t] = NULL;
        }

        freq *= FSEMITONE;
    }
}


static char* SNDsynToken (char* _init, char* _p)
{
    do
    {
        _p = strtok(_init, SNDsyndelims);
        _init = NULL;
        if (_p == NULL)
        {
            return NULL;
        }
        
        if ( *_p != ';' )
        {
            while ((*_p == ' ') || (*_p == '\t'))
            {
                _p++;
            }

            if (*_p != 0)
            {
                return _p;
            }
        }
    }
    while (1);

    return NULL;
}

static char* SNDsynFirsttoken (char* _init)
{
    return SNDsynToken(_init, _init);
}

static char* SNDsynNexttoken (char* _p)
{
    return SNDsynToken(NULL, _p);
}


void SNDsynTranspose (s8* _sourcesample, s8* _dest, u16 _destlen, u32 sourceinc)
#ifdef __TOS__
;
#else
{
    u16 t;
    u32 acc = 0;
        
    
/*    sourceinc = STDdivu(_filesize, _destlen);
    sourceinc <<= 16;
    div        = (u16) STDdivu(_filesize << 16, _destlen);
    sourceinc |= div;*/

    for (t = 0 ; t < _destlen ; t++)
    {
        u16 index = acc >> 16;
        u16 error = acc & 0xFFFF;

        s16 s0 = _sourcesample[index]     + 128;
        s16 s1 = _sourcesample[index + 1] + 128;

        u32 sm0 = STDmulu (s0, 0xFFFF - error);
        u32 sm1 = STDmulu (s1, error);

        u32 dsm = (sm0 + sm1) >> 16;

        s8 sample = (s8)(dsm - 128);
        
        if ( sample >= 0 )
        {
            *_dest++ = 0;
        }
        else
        {
            *_dest++ = 0xFF;
        }

        *_dest++ = sample;

        acc += sourceinc;
    }
}
#endif


void SNDsynTestTranspose (s8* _sourcesample, u32 _filesize, SNDsynSample* _sample)
{
    u16 t, i = 0;
    u16 destlen = (u16) _sample->length;
    s8* dest = _sample->data;
        
    
    ASSERT(_sample->length < 65536UL);

    for (t = 0 ; t < destlen ; t++)
    {
        dest[t] = _sourcesample[i++];

        if (i >= _filesize)
        {
            i = 0;
        }
    }
}



static u32 getFileSize (FILE* _file)
{
    u32 filesize;

    fseek (_file, 0, SEEK_END);
    filesize = ftell(_file);
    fseek (_file, 0, SEEK_SET);

    return filesize;
}


char* SNDsynSoundSet_load(SNDsynSoundSet* _soundset, char* _p)
{
    float freq;
    u16 t, refsemitone;
    u8 s;
    u8 transposerange, nbsamples;
    u8 samplescaps[16];
    

    _p = SNDsynNexttoken (_p);
    freq = (float) atof(_p);

    _p = SNDsynNexttoken (_p);
    refsemitone = atoi(_p);

    _p = SNDsynNexttoken (_p);
    transposerange = atoi(_p);

    _p = SNDsynNexttoken (_p);
    nbsamples = atoi(_p);    

    while ( refsemitone > 0 )
    {
        freq /= FSEMITONE;
        refsemitone--;
    }

    for (t = 0 ; t < SND_SYN_NBSEMITONES ; t++)
    {
        SNDsynSound* sound = (SNDsynSound*) SND_SYN_ALLOC_MAIN( sizeof(SNDsynSound) );
        DEFAULT_CONSTRUCT(sound);
        _soundset->sounds[t] = sound;

        SNDsynSound_allocateSound (sound, nbsamples, transposerange);
    }
    
    for (s = 0 ; s < nbsamples ; s++)
    {
        float currentfreq = freq;
        u32   filesize;
        s8*   sourcesample;
        u16   nbperiods;

        _p = SNDsynNexttoken (_p);

        {
            u32 result;
            FILE* samplefile = fopen (_p, "rb");
            ASSERT(samplefile != NULL);
            filesize = getFileSize(samplefile);
            sourcesample = (s8*) SND_SYN_ALLOC_TEMP (filesize+1);
            result = fread(sourcesample, 1, filesize, samplefile); 
            sourcesample[filesize] = sourcesample[0];
            fclose (samplefile);
            ASSERT(result == filesize);
        }

        _p = SNDsynNexttoken (_p);
        nbperiods = (u16) atoi(_p);

        _p = SNDsynNexttoken (_p);
        samplescaps[s] = *_p;

        /* compute the 12 semi tones */
        for (t = 0 ; t < SND_SYN_NBSEMITONES ; t++)
        {
            SNDsynSound* sound = _soundset->sounds[t];
            float error;
            
            error = SNDsynSound_allocatePeriods (sound, s, currentfreq, nbperiods);
            printf ("sampl %2d tone %2d error = %3ld%% [%d]\n", (u16) s, t, (s32)(error * 100.0f), (u16) sound->samples[s].length);
            SND_SYN_FREE_TEMP(sound->samples[s].tmpdata);
            sound->samples[s].tmpdata = NULL;

            ASSERT(sound->samples[s].length < 65536UL);
            {
                u32 inc = (filesize << 16) / sound->samples[s].length;

                SNDsynTranspose (sourcesample, sound->samples[s].data, (u16) sound->samples[s].length, inc);
            }

            currentfreq *= FSEMITONE;
        }

        SND_SYN_FREE_TEMP(sourcesample);
    }

    for (t = 0 ; t < SND_SYN_NBSEMITONES ; t++)
    {
        u8 nbsamplesindexes = 0;
        u8 nbsustainindexes = 0;
        bool firstsustain = true;
        SNDsynSound* sound = _soundset->sounds[t];


        for (s = 0 ; s < nbsamples ; s++)
        {
            if (samplescaps[s] != 's')
            {
                sound->sampleindexes[nbsamplesindexes++] = s;
            }
            else 
            {
                if (firstsustain)
                {
                    firstsustain = false;
                    sound->sustainindex = nbsamplesindexes;
                    sound->sampleindexes[nbsamplesindexes++] = s;
                }
                
                sound->sustainindexes[nbsustainindexes++] = s;
            }
        }
        
        ASSERT(nbsamplesindexes > 0);
        sound->nbsampleindexes = nbsamplesindexes;

        ASSERT(nbsustainindexes > 0);
        sound->nbsustainindexes = nbsustainindexes;
    }
            
    return _p;
}



SNDsynSoundSet* SNDsynthLoad (char* _filename)
{
    FILE*   file = fopen(_filename, "rb");
    u32     filesize;
    char*   buffer;
    char*   p;
    SNDsynSoundSet* soundSet;
    u16     nbsoundsets, t;


    fseek (file, 0, SEEK_END);
    filesize = ftell(file);
    fseek (file, 0, SEEK_SET);
    buffer = (char*) SND_SYN_ALLOC_MAIN (filesize + 1);
    fread (buffer, 1, filesize, file);
    fclose (file);
    buffer[filesize] = 0;

    p = SNDsynFirsttoken(buffer);

    nbsoundsets = atoi(p);
    printf ("nb sound sets: %d\n", nbsoundsets);

    soundSet = (SNDsynSoundSet*) SND_SYN_ALLOC_MAIN (sizeof(SNDsynSoundSet) * nbsoundsets);
    
    for (t = 0 ; t < nbsoundsets ; t++)
    {
        DEFAULT_CONSTRUCT (&soundSet[t]);
        p = SNDsynSoundSet_load (&soundSet[t], p);
    }

    return soundSet;
}



void SNDsynPlayerShutdown(RINGallocator* _allocator)
{
	(*HW_DMASOUND_CONTROL) = HW_DMASOUND_CONTROL_OFF;

    RINGallocatorFree (_allocator, synth.buffer);
}


#ifdef DEMOS_DEBUG
u16 SNDsynPlayerTrace (void* _image, u16 _pitch, u16 _planePitch, u16 _y)
{
    IGNORE_PARAM(_image);
    IGNORE_PARAM(_pitch);
    IGNORE_PARAM(_planePitch);
    IGNORE_PARAM(_y);

    return 8;
}
#endif


void SNDtest (SNDsynSoundSet* _soundSet)
{
    soundSet1 = _soundSet;
    soundSet2 = soundSet1 + 1;
}


/*

void gentest1 (SNDsynSound* _sound)
{
    SNDsynSound_addSineWave (&_sound->samples[0], _sound->samples[0].correctedfreq, 90.0f / 128.0f);
    SNDsynSample_ramp (&_sound->samples[0], 0.0f, 1.0f);

    SNDsynSound_addSineWave (&_sound->samples[1], _sound->samples[1].correctedfreq, 90.0f / 128.0f);

    SNDsynSound_addSineWave (&_sound->samples[2], _sound->samples[2].correctedfreq, 90.0f / 128.0f);
    SNDsynSample_ramp (&_sound->samples[2], 1.0f, 0.0f);

    _sound->sampleindexes[0] = 0;
    _sound->sampleindexes[1] = 1;
    _sound->sampleindexes[2] = 2;
    _sound->nbsampleindexes = 3;
    _sound->sustainindex = 1;

}

void gentest2 (SNDsynSound* _sound)
{
    SNDsynSound_addSineWave (&_sound->samples[0], _sound->samples[0].correctedfreq, 96.0f / 128.0f);

    _sound->sampleindexes[0] = 0;
    _sound->nbsampleindexes = 1;
    _sound->sustainindex = 0;
}




void gentest3 (SNDsynSound* _sound)
{
    SNDsynSound_addSineWave (&_sound->samples[0], _sound->samples[0].correctedfreq, 90.0f / 128.0f);
    SNDsynSample_ramp (&_sound->samples[0], 0.0f, 1.0f);

    SNDsynSound_addSineWave (&_sound->samples[1], _sound->samples[1].correctedfreq, 90.0f / 128.0f);
    SNDsynSound_addSineWave (&_sound->samples[1], _sound->samples[1].correctedfreq * 2.0f, 32.0f / 128.0f);

    SNDsynSound_addSineWave (&_sound->samples[2], _sound->samples[2].correctedfreq, 90.0f / 128.0f);

    SNDsynSound_addSineWave (&_sound->samples[2], _sound->samples[2].correctedfreq * 2.0f, 32.0f / 128.0f);
    SNDsynSound_addSineWave (&_sound->samples[2], _sound->samples[2].correctedfreq * 3.0f, 8.0f / 128.0f);

    SNDsynSound_addSineWave (&_sound->samples[3], _sound->samples[3].correctedfreq, 90.0f / 128.0f);
    SNDsynSound_addSineWave (&_sound->samples[3], _sound->samples[3].correctedfreq * 2.0f, 32.0f / 128.0f);
    SNDsynSound_addSineWave (&_sound->samples[3], _sound->samples[3].correctedfreq * 3.0f, 8.0f / 128.0f);
    SNDsynSound_addSineWave (&_sound->samples[2], _sound->samples[3].correctedfreq * 4.0f, 4.0f / 128.0f);

    SNDsynSound_addSineWave (&_sound->samples[4], _sound->samples[4].correctedfreq, 90.0f / 128.0f);
    SNDsynSample_ramp (&_sound->samples[4], 1.0f, 0.0f);

    _sound->sampleindexes[0] = 0;
    _sound->sampleindexes[1] = 1;
    _sound->sampleindexes[2] = 4;
    _sound->nbsampleindexes = 3;
    _sound->sustainindex = 1;
}


void gentest4 (SNDsynSound* _sound)
{
    SNDsynSound_addSineWave (&_sound->samples[0], _sound->samples[0].correctedfreq, 96.0f / 128.0f);

//    SNDsynSound_addTriangle(&_sound->samples[0], _sound->samples[0].correctedfreq, 0.9f, 0.25f);

    _sound->sampleindexes[0] = 0;
    _sound->nbsampleindexes = 1;
    _sound->sustainindex = 0;
}


static u8 volumestest[] =
{
    0,
    1,
    2,
    1,
    0
};

static u8 maskstest[] =
{
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    6,
    5,
    4,
    3,
    2,
    1,
    0,
    0
};

static u8 wavestest[] = 
{
    1,
    1,
    2,
    3,
    3,
    3,
    3,
    2,
    1,
    1
};


void lfoTest (void)
{
    synth.voices[0].lfo.nbframes = 5;
    synth.voices[0].lfo.frames = 0;    

    synth.voices[1].lfo.nbframes = 0;
    synth.voices[1].lfo.frames = 0;    

    synth.voices[1].lfo.modulators[ SNDsynModulatorType_VOLUME ].length = ARRAYSIZE(volumestest); 
    synth.voices[1].lfo.modulators[ SNDsynModulatorType_VOLUME ].steps  = volumestest;
    synth.voices[1].lfo.modulators[ SNDsynModulatorType_VOLUME ].speed  = 4;
    
    synth.voices[1].lfo.modulators[ SNDsynModulatorType_MASK   ].length = ARRAYSIZE(maskstest); 
    synth.voices[1].lfo.modulators[ SNDsynModulatorType_MASK   ].steps  = maskstest;
    synth.voices[1].lfo.modulators[ SNDsynModulatorType_MASK   ].speed  = 3;

    synth.voices[0].lfo.modulators[ SNDsynModulatorType_WAVE   ].length = ARRAYSIZE(wavestest); 
    synth.voices[0].lfo.modulators[ SNDsynModulatorType_WAVE   ].steps  = wavestest;
    synth.voices[0].lfo.modulators[ SNDsynModulatorType_WAVE   ].speed  = 5;
}


void SNDtest2 (void)
{
    (*HW_VIDEO_MODE) = HW_VIDEO_MODE_2P;

    soundSet1 = (SNDsynSoundSet*) SND_SYN_ALLOC_MAIN( sizeof(SNDsynSoundSet) );
    soundSet2 = (SNDsynSoundSet*) SND_SYN_ALLOC_MAIN( sizeof(SNDsynSoundSet) );

    DEFAULT_CONSTRUCT(soundSet1);
    DEFAULT_CONSTRUCT(soundSet2);

    SNDsynSoundSet_generate (soundSet2, &configTest1, NULL, gentest1); 
    SNDsynSoundSet_generate (soundSet1, &configTest3, NULL, gentest3);
    lfoTest ();

    SNDsynSound_dump (soundSet1->sounds[0], "PCM\\SND1");
    SNDsynSound_dump (soundSet2->sounds[0], "PCM\\SND2");

//    SNDsynSample_dump (soundSet1.sounds[0].samples[1], "E:\\temp\\dumpsine.raw", 1);
//    SNDsynSample_dump (soundSet2.samples[1], "E:\\temp\\dumpsine2.raw", 2);

//    SNDsynSample_dump (&testsound.samples[1], "E:\\temp\\dumpsine.raw", 1);
//    SNDsynSample_dump (&testsound.samples[1], "E:\\temp\\dumpsine2.raw", 2);
//    {      
//        u32 base = SYSreadVideoBase ();

//        SNDsynSound_drawCurve (&testsound.samples[0], (void*) base);
//        SNDsynSample_drawXorPass (640, (u8*) base);

//        EMULrender ();
//    }
}
*/




