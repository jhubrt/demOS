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


/*! @defgroup SND

    SND provides SouNDtrack management. 
    It is designed to be cheap in terms of CPU usage.                                       <br>

    SND:                                                                                    <br>
    - plays PCM sample with STe DMA                                                         <br>
    - manages volumes of left / right channels : the idea is to listen in mono 
      and use them as (free) mixers                                                         <br>
    - manages the musical score as a DSL code (coded as a co-routine)                       <br>
    - requires / synchronizes loading of samples and transfer PCM to the wished channel     <br>

    Volume slides are managed by VBL interrupt
    PCM transfers are also sliced into VBL interrupt.
*/
    
#ifndef SOUND_H
#define SOUND_H

#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\ALLOC.H"
#include "DEMOSDK\LOAD.H"

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

struct SNDcore_
{
	u8*		     dmaBuffer;
	u8*		     cache;
    u8*          loaded;
	u32		     sampleLength;

	bool	     pcmLoaded;
	bool		 syncWithSoundtrack;
	
	LOADrequest* currentLoadRequest;
	u16			 dmaLoopstart;

	volatile s16 playerContext;
	u16			 playerStart;
    volatile u16 playerClientStep;
};
typedef struct SNDcore_ SNDcore;

#ifndef SOUND_C

extern SNDcore snd;
extern u8 SNDchannelVolume[];

#endif

#define SND_BEGIN							case 0: snd.playerStart = __LINE__; 

#define SND_LOADSAMPLE(MEDIA,RESOURCE)		snd.loaded = (u8*) RSC_##MEDIA .preload[RSC_##MEDIA##_ZIK_##RESOURCE##_RAW]; if ( snd.loaded == NULL ) { snd.currentLoadRequest = LOADrequestLoad(&RSC_##MEDIA , RSC_##MEDIA##_ZIK_##RESOURCE##_RAW, snd.cache, LOAD_PRIOTITY_HIGH); snd.loaded = snd.cache; }
#define SND_WAIT_SAMPLELOADED				case __LINE__:	if ( (snd.currentLoadRequest != NULL) && (snd.currentLoadRequest->processed != LOADrequestState_DONE) )  return __LINE__; if ( snd.currentLoadRequest != NULL) snd.currentLoadRequest->allocated = false; snd.currentLoadRequest = NULL;

#define SND_SYNC_COPYCACHE_TO_LEFT(OFFSET,LEN)		SNDcopySample(snd.loaded, snd.dmaBuffer + (OFFSET << 1)		, LEN);
#define SND_SYNC_COPYCACHE_TO_RIGHT(OFFSET,LEN)		SNDcopySample(snd.loaded, snd.dmaBuffer + (OFFSET << 1) +1	, LEN);

#define SND_COPYCACHE_TO_LEFT(OFFSET,LEN)	SNDdestTransfer = snd.dmaBuffer + (OFFSET << 1);     SNDsourceTransfer = snd.loaded; SNDendSourceTransfer = snd.loaded + LEN;
#define SND_COPYCACHE_TO_RIGHT(OFFSET,LEN)	SNDdestTransfer = snd.dmaBuffer + (OFFSET << 1) + 1; SNDsourceTransfer = snd.loaded; SNDendSourceTransfer = snd.loaded + LEN; 
#define SND_WAIT_CACHECOPIED				case __LINE__: if ( SNDsourceTransfer < SNDendSourceTransfer ) return __LINE__;

#define SND_VOLUME_SET(VOL)					SNDmasterVolume = VOL; (*HW_MICROWIRE_MASK) = HW_MICROWIRE_MASK_SOUND; (*HW_MICROWIRE_DATA) = HW_MICROWIRE_VOLUME | VOL;
#define SND_VOLUME_FADEIN(STEP)				SNDspeedfade = STEP; SNDmasterVolume = 0; SNDfademasterVolume = 1;
#define SND_VOLUME_FADEOUT(STEP)			SNDspeedfade = STEP; SNDmasterVolume = 40; SNDfademasterVolume = -1;

#define SND_SETPANNING(PAN)					SNDleftVolume = SNDchannelVolume[PAN]; SNDrightVolume = SNDchannelVolume[6 - PAN]; { STATIC_ASSERT(PAN<7); }

#define SND_DMALOOP_RESET					snd.dmaLoopstart = SNDdmaLoopCount;
#define SND_WAIT_DMALOOP_REACH(COUNT)		case __LINE__: if ( (SNDdmaLoopCount - snd.dmaLoopstart) < COUNT ) return __LINE__;

#define	SND_IDLE							case __LINE__: return __LINE__;
#define SND_RESTART							return 0;
#define SND_WAIT_CLIENTSTEP(COUNT)          case __LINE__: if (( snd.playerClientStep < COUNT ) && snd.syncWithSoundtrack ) return __LINE__;

void SNDinit        (RINGallocator* _allocator, u32 _sampleLen, u16 _routineindex);
void SNDshutdown    (RINGallocator* _allocator);
void SNDcopySample  (u8* _source, u8* _dest, u32 _length);
void SNDwaitDMALoop (void);
void SNDwaitClientStep(u16 _clientStep);	/* shoud be used from idle thread only ! */
void SNDplayNoise   (u16 _freq, u16 _level);
void SNDstopNoise   (void);

#ifdef DEMOS_DEBUG
u16  SNDtrace       (void* _image, u16 _pitch, u16 _planePitch, u16 _y);
#endif

#endif
