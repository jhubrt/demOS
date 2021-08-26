/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2021 J.Hubert

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

#include "DEMOSDK\BASTYPES.H"

#define SOUND_C

#include "DEMOSDK\SOUND.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\LOAD.H"
#include "DEMOSDK\STANDARD.H"

SNDcore snd;

#ifndef __TOS__
volatile u8		SNDleftVolume;
volatile u8		SNDrightVolume;
volatile s8		SNDfademasterVolume;
volatile u8		SNDspeedfade;
volatile u16	SNDdmaLoopCount;
void*			SNDsourceTransfer;
volatile void*	SNDendSourceTransfer;
void*			SNDdestTransfer;

volatile void*	SNDlastDMAposition;	/* read only */
volatile s8		SNDmasterVolume;	/* ready only - debug purpose */

void SNDsoundtrackMonitor (void) {}
#endif

void SNDsoundtrackMonitor (void);

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

/* Sums of these channels achieve a quite constant global volume */
u8 SNDchannelVolume[] = { 20, 19, 18, 17, 16, 13, 0 };

void SNDplayNoise   (u16 _freq, u16 _level) PCSTUB;
void SNDstopNoise   (void) PCSTUB;

void SNDcopySample(u8* _source, u8* _dest, u32 _length)
{
	while (_length > 0)
	{
		*_dest++ = *_source++;
		_dest++;
		_length--;
	}
}

void SNDwaitDMALoop (void)
{
    u16 count = SNDdmaLoopCount;

    while (count == SNDdmaLoopCount);
}

void SNDwaitClientStep(u16 _clientStep)
{
	if (snd.syncWithSoundtrack)
	{
		while (snd.playerClientStep < _clientStep);
	}
}

static void SND_waitMicrowire (void)
{
	while (*HW_MICROWIRE_MASK != 0x7FF);
}

void SNDinit(RINGallocator* _allocator, u32 _sampleLen, u16 _routineindex)
{
	u32 dmaSampleBufferLen = ((_sampleLen * 2UL) + 1023UL) & 0xFFFFFC00UL;    /* stereo sample and round to 1024 for vbl copy routine */ 

    SYSvblroutines[_routineindex] = SNDsoundtrackMonitor;

	snd.cache		 = (u8*) RINGallocatorAlloc (_allocator, LOADroundBufferSize(_sampleLen));
	snd.dmaBuffer	 = (u8*) RINGallocatorAlloc (_allocator, dmaSampleBufferLen);
    snd.loaded       = NULL;
	snd.sampleLength = _sampleLen;
	snd.pcmLoaded	 = false;
	snd.currentLoadRequest = NULL;
	snd.dmaLoopstart = 0;
    snd.playerClientStep = 0;
	snd.syncWithSoundtrack = false;

	*HW_MICROWIRE_MASK = HW_MICROWIRE_MASK_SOUND;

	STDmset (snd.cache	 , 0, snd.sampleLength);
	*HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME;	/* volume = 0 at start */
    while (*HW_MICROWIRE_MASK != HW_MICROWIRE_MASK_SOUND);

	STDmset (snd.dmaBuffer, 0, snd.sampleLength * 2);
    *HW_MICROWIRE_DATA = HW_MICROWIRE_MIXER_YM;
    while (*HW_MICROWIRE_MASK != HW_MICROWIRE_MASK_SOUND);

	{
		u32 s = (u32) snd.dmaBuffer;

		(*HW_DMASOUND_STARTADR_H) = (u8)(s >> 16);
		(*HW_DMASOUND_STARTADR_M) = (u8)(s >> 8);
		(*HW_DMASOUND_STARTADR_L) = (u8) s;
	}

	{
		u32 end = (u32) snd.dmaBuffer + (snd.sampleLength - 1) * 2UL;

		(*HW_DMASOUND_ENDADR_H) = (u8)(end >> 16);
		(*HW_DMASOUND_ENDADR_M) = (u8)(end >> 8);
		(*HW_DMASOUND_ENDADR_L) = (u8) end;
	}

	*HW_DMASOUND_MODE	 = HW_DMASOUND_MODE_25033HZ | HW_DMASOUND_MODE_STEREO;
	*HW_DMASOUND_CONTROL = HW_DMASOUND_CONTROL_PLAYLOOP;

#   ifndef __TOS__
    SNDlastDMAposition = snd.dmaBuffer;
#   endif
}

void SNDshutdown(RINGallocator* _allocator)
{
	(*HW_DMASOUND_CONTROL) = HW_DMASOUND_CONTROL_OFF;

	RINGallocatorFree (_allocator, snd.cache);
	RINGallocatorFree (_allocator, snd.dmaBuffer);
}

#ifdef DEMOS_DEBUG
u16 SNDtrace (void* _image, u16 _pitch, u16 _planePitch, u16 _y)
{
	u32 dmastart = (((u32)*HW_DMASOUND_STARTADR_H) << 16) | (((u32)*HW_DMASOUND_STARTADR_M) << 8) | ((u32)*HW_DMASOUND_STARTADR_L);
	u32 dmaend   = (((u32)*HW_DMASOUND_ENDADR_H  ) << 16) | (((u32)*HW_DMASOUND_ENDADR_M  ) << 8) | ((u32)*HW_DMASOUND_ENDADR_L);
	u32 dmacount = (((u32)*HW_DMASOUND_COUNTER_H ) << 16) | (((u32)*HW_DMASOUND_COUNTER_M ) << 8) | ((u32)*HW_DMASOUND_COUNTER_L);

    {
        static char line[] = "play=    step=   loop=     req=      ";

        STDuxtoa (&line[5] , snd.playerContext, 3);
        STDuxtoa (&line[14], snd.playerClientStep, 2);
        STDuxtoa (&line[22], SNDdmaLoopCount, 4);
        STDuxtoa (&line[31], (u32) snd.currentLoadRequest, 6);

        SYSdebugPrint ( _image, _pitch, _planePitch, 0, _y, line);
    }

    {
        static char line[] = "left     right     fade     vol    ";

        line [5] = SNDleftVolume  & 0x80 ? ' ' : '>';
        STDuxtoa (&line[6], SNDleftVolume  & 0x7F, 2);

        line [15] = SNDrightVolume & 0x80 ? ' ' : '>';
        STDuxtoa (&line[16], SNDrightVolume & 0x7F, 2);

        STDuxtoa (&line[24], SNDfademasterVolume, 3);
        STDuxtoa (&line[32], SNDmasterVolume, 3);

	    SYSdebugPrint ( _image, _pitch, _planePitch, 0, _y + 8, line);
    }

    {
        static char line[] = "[       ;       ]=>        -        ";

        STDuxtoa (&line[1] , (u32)SNDsourceTransfer    , 6);
        STDuxtoa (&line[10], (u32)SNDendSourceTransfer , 6);
		STDuxtoa (&line[20], (u32)SNDdestTransfer      , 6);
		STDuxtoa (&line[29], (u32)SNDlastDMAposition   , 6);

        SYSdebugPrint ( _image, _pitch, _planePitch, 0, _y + 16, line);
    }

    {
        static char line[] = "dma=       start=       end=       ";

        STDuxtoa (&line[4] , dmacount, 6);
        STDuxtoa (&line[17], dmastart, 6);
        STDuxtoa (&line[28], dmaend  , 6);
            
        SYSdebugPrint ( _image, _pitch, _planePitch, 0, _y + 24, line);
    }

	return 36;
}
#endif
