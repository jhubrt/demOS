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

#include "DEMOSDK\BASTYPES.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\BLITSND.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\TRACE.H"

#include "DEMOSDK\PC\EMUL.H"


#define blsBENCHMARK 1

#if blsBENCHMARK
#   define blsRASTER(COLOR) *HW_COLOR_LUT = COLOR;
#else
#   define blsRASTER(COLOR)
#endif

/* define wrapping to C or ASM version for each routine */
#if blsUSEASM
#   ifdef BLSupdate
#       define blsUpdAsync              ablsUpdAsync
        void blsUpdAsync                (BLSplayer* _player);
#   else
#       define blsUpdateAllVoices       ablsUpAllVoices
#       define blsUpdateSoundBuffers    ablsUpSoundBuffers
#       define blsUpdateScore           ablsUpScore
#       define blsUpdateRunningEffects  ablsUpRunningEffects
#       define blsSetDMABuffer          ablsSetDMABuffer
        void blsUpdateAllVoices         (BLSplayer* _player);
        void blsUpdateSoundBuffers      (BLSplayer* _player);
        void blsUpdateScore             (BLSplayer* _player);
        void blsUpdateRunningEffects    (BLSplayer* _player);
        void blsSetDMABuffer            (BLSplayer* _player);
#   endif
#endif

#ifndef __TOS__
u32 blsSecondaryBufferFreq = BLS_DMA_FREQ;

void BLSsetSecondaryBufferFreq(u32 _freq)
{
    blsSecondaryBufferFreq = _freq;
}

void aBLSvbl(void) {}
void* aBLSvblPlayer = NULL;

#endif

ENUM(BLSbufferState)
{
    BLSbs_STEP0 = 0,
    BLSbs_STEP1 = 4,
    BLSbs_STEP2 = 8,
    BLSbs_START = 12
};

 
void BLSplayerInit(MEMallocator* _allocator, BLSplayer* _player, BLSsoundTrack* _sndtrack, bool _initaudio)
{
    u16 v;
    u16 buffersize = (BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD + BLS_NBBYTES_CLEARFLAGS) * BLS_NB_BUFFERS;

    DEFAULT_CONSTRUCT(_player);

    _player->volumeLeft = _player->volumeLeft2 = _player->volumeRight = _player->volumeRight2 = 0;

    _player->speed = 6;         /* default speed    */
    _player->speedcount = 1;    /* play at next vbl */

    _player->sndtrack = _sndtrack;

    _player->buffer   = (s8*) MEM_ALLOC (_allocator, buffersize);

    STDmset (_player->buffer, 0UL, buffersize);
      	
    for (v = 0 ; v < BLS_NBVOICES ; v++)
    {
        _player->voices[v].mask = 0xFFFF;
    }

    if (_initaudio)
    {
        *HW_MICROWIRE_MASK = HW_MICROWIRE_MASK_SOUND;    
        *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME | 40;
        while (*HW_MICROWIRE_MASK != HW_MICROWIRE_MASK_SOUND);

        *HW_MICROWIRE_MASK = HW_MICROWIRE_MASK_SOUND;    
        *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME_LEFT | 20;
        while (*HW_MICROWIRE_MASK != HW_MICROWIRE_MASK_SOUND);

        *HW_MICROWIRE_MASK = HW_MICROWIRE_MASK_SOUND;    
        *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME_RIGHT | 20;
        while (*HW_MICROWIRE_MASK != HW_MICROWIRE_MASK_SOUND);

        *HW_DMASOUND_MODE = HW_DMASOUND_MODE_50066HZ | HW_DMASOUND_MODE_STEREO;

        _player->bufferstate = BLSbs_START;

        EMULcreateSoundBuffer (BLS_NBSAMPLES_PERFRAME * 2, true, blsSecondaryBufferFreq);
    }
}


void BLSgoto (BLSplayer* _player, u8 _trackIndex)
{
    u16 i;

    _player->trackindex = _trackIndex;
    _player->row = 0;
    _player->loopstart = 0;
    _player->loopcount = 0;
    _player->speedcount = 1;

    for (i = 0 ; i < BLS_NBVOICES ; i++)
    {
        _player->voices[i].samples[0]    = NULL;
        _player->voices[i].arpeggioState = ArpeggioState_STOPPED;
    }
}

/* -------------------------------------------------------------------------------
    C versions of play routines, useful for : 
    - PC version
    - easier development
    - test purpose by compairing C and ASM version results
    
    Similar routines are availble in ASM version
------------------------------------------------------------------------------- */

#ifndef blsUpdateSoundBuffers
static void blsUpdateSoundBuffers (BLSplayer* _player)
{
    u32 buffer      = (u32) _player->buffer; 
    u32 readcursor  = SYSlmovep(HW_DMASOUND_COUNTER_H) >> 8;
    u32 dmabufstart = _player->dmabufstart;
    u32 dmabufend   = _player->dmabufend;


    if (_player->volumeLeft)
    {
        *HW_MICROWIRE_MASK = HW_MICROWIRE_MASK_SOUND;    
        *HW_MICROWIRE_DATA = _player->volumeLeft;
    }

    switch (_player->bufferstate)
    {
    case BLSbs_STEP0:
        _player->buffertoupdate = _player->buffer;
        _player->dmabufstart    = buffer + BLS_STEP_PERFRAME + BLS_STEP_PERFRAME;
        _player->bufferstate    = BLSbs_STEP1;
        break;

    case BLSbs_STEP1:
        _player->buffertoupdate = _player->buffer + BLS_STEP_PERFRAME;
        _player->dmabufstart    = buffer;
        _player->bufferstate    = BLSbs_STEP2;
        break;
    
    case BLSbs_START:

        *HW_DMASOUND_STARTADR_H = (u8)(buffer >> 16);
        *HW_DMASOUND_STARTADR_M = (u8)(buffer >> 8);
        *HW_DMASOUND_STARTADR_L = (u8) buffer;

        {
            u32 endbuf = buffer + BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD;

            *HW_DMASOUND_ENDADR_H = (u8)(endbuf >> 16);
            *HW_DMASOUND_ENDADR_M = (u8)(endbuf >> 8);
            *HW_DMASOUND_ENDADR_L = (u8) endbuf;
        }

        *HW_DMASOUND_CONTROL = HW_DMASOUND_CONTROL_PLAYLOOP;

        readcursor = -1;

        /* !!! no break here !!! */

    case BLSbs_STEP2:
        _player->buffertoupdate = _player->buffer + BLS_STEP_PERFRAME + BLS_STEP_PERFRAME;
        _player->dmabufstart    = buffer + BLS_STEP_PERFRAME;
        _player->bufferstate    = BLSbs_STEP0;
        break;
    }

    _player->dmabufend = _player->dmabufstart + BLS_NBBYTES_PERFRAME;

    if ((readcursor >= dmabufstart) && (readcursor <= dmabufend))
    {
        _player->dmabufend += BLS_NBBYTES_OVERHEAD;
    }

    if (_player->volumeRight)
    {
        *HW_MICROWIRE_MASK = HW_MICROWIRE_MASK_SOUND;    
        *HW_MICROWIRE_DATA = _player->volumeRight;
    }

#   ifdef __TOS__
    _player->volumeLeft   = _player->volumeLeft2;
    _player->volumeRight  = _player->volumeRight2;
    _player->volumeLeft2  = 0;
    _player->volumeRight2 = 0;
#   endif

#   if blsLOGDMA
    TRAClogNumberS("frame"    , (u32) _player->framenum, 4, 0);

    if ((readcursor >= dmabufstart) && (readcursor <= dmabufend))
    {
        TRAClog(" noskip ", 0);
    }
    else
    {
        TRAClog(" skip 4 ", 0);
    }

    TRAClogNumberS("dmastart" , (u32) dmabufstart            , 4, 0);
    TRAClogNumberS("dmaend"   , (u32) dmabufend              , 4, 0);
    TRAClogNumberS("readcurs" , (u32) readcursor             , 4, 0);
    TRAClogNumberS("nextdmas" , (u32) _player->dmabufstart   , 4, 0);
    TRAClogNumberS("nextdmae" , (u32) _player->dmabufend     , 4, 0);
    TRAClogNumberS("update"   , (u32) _player->buffertoupdate, 4, 0);

    TRAClog("\n", _player->bufferstate == BLSbs_STEP2 ? '\n' : 0);
#   endif
}
#endif


#ifndef blsUpdateAllVoices
static void blsUpdateVoiceFrame (BLSvoice* _voice, u8* _buffer, bool _firstpass)
{
    u16  remain = BLS_NBSAMPLES_PERFRAME / 2;    /* nb samples per voice per frame */
    u8   arpeggioIndex = _voice->arpeggioOffset >> 2;
    u8*  cleared2 = _buffer + BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD;
    u8*  cleared  = cleared2 + _firstpass;
    u16* mute_sampledelay = (u16*)&_voice->mute;


    blsRASTER(0x70);

    *HW_BLITTER_ADDR_DEST = (u32) _buffer;
    
    ASSERT( ((*cleared) & 0xFE) == 0 );

    if ((_voice->samples[0] == NULL) || (*mute_sampledelay != 0))
    {
        if (*cleared)
        {
            if (_voice->sampledelay)
            {
                _voice->sampledelay--;
            }
            return;
        }

        *cleared  = true;
        *cleared2 = true;

        goto clear;
    }

    *cleared  = false;    
    *cleared2 = false;

    {
        BLSsample* sample = _voice->samples[arpeggioIndex];       

        {
            u32  scrCurrentSampl  = _voice->current;
            s8   transpose        = _voice->keys[arpeggioIndex]->blitterTranspose >> 1;
            u32  sampleLen        = sample->sampleLen;
            u32  sampleLoopStart  = sample->sampleLoopStart;
            u16  sampleLoopLength = sample->sampleLoopLength;
            s8*  sampleData       = sample->sample;
            u32  transfer;
            bool storageshift     = (sample->flags & BLS_SAMPLE_STORAGE_SHIFT) != 0;
            u8   volume           = _voice->volume;

            ASSERT((volume == 0) || (volume == 8) || ((sample->flags & BLS_SAMPLE_STORAGE_INTERLACE) == 0));

            if ((sample->flags & BLS_SAMPLE_STORAGE_INTERLACE) == 0)
            {
                volume += _voice->volumeoffset;
                if (volume > 8)
                {
                    volume = 8;
                }
            }

            if (_firstpass)
            {
                if (storageshift)
                {
                    *HW_BLITTER_CTRL2 = 8;
                }
                else
                {
                    *HW_BLITTER_CTRL2 = volume;
                }
            }
            else
            {
                if (storageshift)
                {
                    *HW_BLITTER_CTRL2 = 0;
                }
                else
                {
                    *HW_BLITTER_CTRL2 = (8 | HW_BLITTER_CTRL2_FORCE_XTRA_SRC | HW_BLITTER_CTRL2_NO_FINAL_SRC_READ) + volume;
                }
            }

            if ( volume == 8 )
            {
                *HW_BLITTER_HOP = HW_BLITTER_HOP_BIT1;
                *HW_BLITTER_OP  = HW_BLITTER_OP_BIT0; 
            }
            else if ( _voice->mask != 0xFFFF )
            {
                u32 mask = _voice->mask;
                STDmset (HW_BLITTER_HTONE, (mask << 16) | mask, 32);
                *HW_BLITTER_HOP = HW_BLITTER_HOP_SOURCE_AND_HTONE;
                *HW_BLITTER_OP  = HW_BLITTER_OP_S; 
            }
            else
            {
                *HW_BLITTER_HOP = HW_BLITTER_HOP_SOURCE;
                *HW_BLITTER_OP  = HW_BLITTER_OP_S; 
            }

            ASSERT (scrCurrentSampl < sampleLen);

            if (transpose < 0)
            {
                transpose = -transpose;

                *HW_BLITTER_XINC_SOURCE = 0;
                *HW_BLITTER_YINC_SOURCE = 2;

                transfer = (sampleLen - scrCurrentSampl) << transpose;

                if (transfer > remain)
                {
                    transfer = remain;
                }

                remain -= (u16) transfer;
                transfer >>= transpose;

                *HW_BLITTER_ADDR_SOURCE = (u32) (sampleData + (scrCurrentSampl << 1));
                *HW_BLITTER_XSIZE = 1 << transpose;
                *HW_BLITTER_YSIZE = (u16) transfer;

                blsRASTER(0x700);
                *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
                EMULblit();
                blsRASTER(0x70);

                scrCurrentSampl += transfer;
            }
            else
            {
                *HW_BLITTER_XINC_SOURCE = 2 << transpose;

                {
                    transfer = (sampleLen - scrCurrentSampl) >> transpose;

                    if (transfer > remain)
                    {
                        transfer = remain;
                    }

                    *HW_BLITTER_ADDR_SOURCE = (u32) (sampleData + (scrCurrentSampl << 1));
                    *HW_BLITTER_YINC_SOURCE = *HW_BLITTER_XINC_SOURCE;
                    *HW_BLITTER_XSIZE = (u16) transfer;
                    *HW_BLITTER_YSIZE = (transfer >> 16) + 1;

                    blsRASTER(0x700);
                    *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
                    EMULblit();
                    blsRASTER(0x70);

                    scrCurrentSampl += transfer << transpose;
                    remain          -= (u16) transfer;

                    if ((remain > 0) && (sampleLoopLength > 0))
                    {
                        u16 translooplen = (u16)(sampleLoopLength >> transpose);
                        u32 div          = STDdivu(remain, translooplen);
                        u16 nbloops      = (u16) div;
                        u16 incXSource   = *HW_BLITTER_XINC_SOURCE;
                        u32 sloopData    = (u32) (sampleData + (sampleLoopStart << 1));

                        scrCurrentSampl = sampleLoopStart;
                        
                        if (nbloops > 0)
                        {
                            *HW_BLITTER_ADDR_SOURCE = sloopData;
                            *HW_BLITTER_YINC_SOURCE = incXSource - (translooplen << (transpose + 1));
                            *HW_BLITTER_XSIZE       = translooplen;
                            *HW_BLITTER_YSIZE       = nbloops;

                            blsRASTER(0x700);
                            *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
                            EMULblit();
                            blsRASTER(0x70);

                            remain = div >> 16; 
                        }
                        
                        if (remain > 0)
                        {
                            *HW_BLITTER_ADDR_SOURCE = sloopData;
                            *HW_BLITTER_YINC_SOURCE = incXSource;
                            *HW_BLITTER_XSIZE       = remain;
                            *HW_BLITTER_YSIZE       = 1;

                            blsRASTER(0x700);
                            *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
                            EMULblit();
                            blsRASTER(0x70);
                            
                            scrCurrentSampl += (u32)remain << transpose;
                            remain = 0;                        
                        }
                    }
                }
            }

            _voice->current = scrCurrentSampl;
            if (scrCurrentSampl >= sampleLen)
            {
                if (sampleLoopLength == 0)
                {
                    _voice->samples[0]    = NULL;
                    _voice->arpeggioState = ArpeggioState_STOPPED;
                }
                else
                {
                    ASSERT(remain == 0);
                    _voice->current = sampleLoopStart;
                }
            }
        }
    }   

    if (remain > 0)
    {
clear:
        *HW_BLITTER_CTRL2 = 0;
        *HW_BLITTER_ADDR_SOURCE = (u32)_buffer;
        *HW_BLITTER_HOP   = HW_BLITTER_HOP_BIT1;
        *HW_BLITTER_OP    = HW_BLITTER_OP_BIT0; 
        *HW_BLITTER_XSIZE = (u16) remain;
        *HW_BLITTER_YSIZE = 1;    

        blsRASTER(0x700);
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();
        blsRASTER(0x70);

        if ( _voice->sampledelay ) 
        {
            _voice->sampledelay--;
            return;
        }

        _voice->samples[0]    = NULL;
        _voice->arpeggioState = ArpeggioState_STOPPED;
    }

    ASSERT (*HW_BLITTER_ADDR_DEST == (u32)(_buffer + BLS_NBBYTES_PERFRAME) );

    if (_voice->arpeggioState != ArpeggioState_STOPPED)
    {
        u16 freqdiv      = _voice->keys[arpeggioIndex]->freqdiv;
        u8  freqdivshift = _voice->keys[arpeggioIndex]->freqdivshift;
        u16 current      = (u16) _voice->current;
        u32 v;

        v = STDmulu(current, freqdiv);
        v >>= freqdivshift;

        ASSERT(v <= 0xFFFF);
        _voice->currentsource = v;
    }
}

static void blsUpdateAllVoices (BLSplayer* _player)
{
    u32  backbuf  = (u32) _player->buffertoupdate;
    u32* overhead = (u32*)(backbuf + BLS_NBBYTES_PERFRAME);

    *overhead = 0x12341234UL;

    *HW_BLITTER_XINC_DEST   = 4;    /* multiplexing */
    *HW_BLITTER_YINC_DEST   = 4;

    *HW_BLITTER_ENDMASK1  = 0xFFFF;
    *HW_BLITTER_ENDMASK2  = 0xFFFF;
    *HW_BLITTER_ENDMASK3  = 0xFFFF;

    blsUpdateVoiceFrame (&_player->voices[1], (u8*) backbuf    , true );
    blsUpdateVoiceFrame (&_player->voices[2], (u8*) backbuf + 2, true );   

    *HW_BLITTER_ENDMASK1  = PCENDIANSWAP16(0xFF00);
    *HW_BLITTER_ENDMASK2  = PCENDIANSWAP16(0xFF00);
    *HW_BLITTER_ENDMASK3  = PCENDIANSWAP16(0xFF00);

    blsUpdateVoiceFrame (&_player->voices[0], (u8*) backbuf    , false);
    blsUpdateVoiceFrame (&_player->voices[3], (u8*) backbuf + 2, false);

    ASSERT( *(overhead) == 0x12341234UL );

    *overhead = overhead[-1];
}
#endif

#ifndef blsSetDMABuffer
static void blsSetDMABuffer(BLSplayer* _player)
{
    u32 dmastart = _player->dmabufstart;
    u32 dmaend   = _player->dmabufend;  

    *HW_DMASOUND_STARTADR_H = (u8)(dmastart >> 16);
    *HW_DMASOUND_STARTADR_M = (u8)(dmastart >> 8);
    *HW_DMASOUND_STARTADR_L = (u8) dmastart;

    *HW_DMASOUND_ENDADR_H   = (u8)(dmaend >> 16);
    *HW_DMASOUND_ENDADR_M   = (u8)(dmaend >> 8);
    *HW_DMASOUND_ENDADR_L   = (u8) dmaend;
}
#endif

#ifndef blsUpdateScore
static void AssignBaseKeyToVoice (BLSsoundTrack* sndtrack, u8 pkindex, u8 arpeggioIndex, BLSvoice* voice)
{
    BLSprecomputedKey* key  = &sndtrack->keys[pkindex];
    u8 sampleIndex = key->sampleIndex;

    voice->keys [arpeggioIndex] = key;

    if (BLS_IS_BASEKEY(key))
    {
        ASSERT (sndtrack->keys->sampleIndex <= sndtrack->nbSamples);
    }
    else
    {
        ASSERT (key->sampleIndex <= sndtrack->nbKeys);
        sampleIndex = sndtrack->keys[sampleIndex].sampleIndex;
        ASSERT (sndtrack->keys[sampleIndex].sampleIndex <= sndtrack->nbSamples);
    }

    voice->samples[arpeggioIndex] = &sndtrack->samples[sampleIndex];
}


static void blsUpdateScore(BLSplayer* _player)
{
    BLSsoundTrack* sndtrack = _player->sndtrack;

    _player->speedcount--;
    if ( _player->speedcount == 0 )
    {
        u8             patternindex = sndtrack->track[_player->trackindex];
        u8             currentrow   = _player->row;
        BLSrow*        row          = &(sndtrack->patterns[patternindex].rows[currentrow]);
        u8             delay        = 0;

        _player->row++;
        if (_player->row >= BLS_NBPATTERNROWS)
        {
            _player->row = 0;
            _player->loopstart = 0;
            _player->loopcount = 0;
            _player->trackindex++;
            if (_player->trackindex >= sndtrack->trackLen)
            {
                _player->trackindex = 0;
            }
        }

        /* interpret pattern */
        {
            BLScell*    cell  = row->cells;
            BLSvoice*   voice = _player->voices;
            u16         i;

            for (i = 0 ; i < BLS_NBVOICES ; i++, cell++, voice++)
            {
                if ( cell->precomputedKey != 0 )
                {
                    AssignBaseKeyToVoice(sndtrack, cell->precomputedKey-1, 0, voice);

                    voice->current       = 0;
                    voice->currentsource = 0;
                    voice->volume        = 0;
                    voice->arpeggioState = ArpeggioState_STOPPED;
                    voice->retrigger     = 0;
                    voice->sampledelay   = 0;
                }

                switch (cell->fx)
                {
                case BLSfx_NONE:
                    if (voice->arpeggioState != ArpeggioState_STOPPED)
                    {
                        voice->arpeggioState = ArpeggioState_STOPPING;
                    }
                    voice->retrigger = 0;
                    break;

                case BLSfx_VOLUME:
                    voice->volume = cell->value;
                    break;
                
                case BLSfx_SPEED:
                    _player->speed = cell->value;
                    break;
                
                case BLSfx_JUMP:
                    _player->trackindex = cell->value;
                    _player->row        = 0;
                    break;
                
                case BLSfx_PATTERN_BREAK:
                    _player->trackindex++;
                    if (_player->trackindex >= sndtrack->trackLen)
                    {
                        _player->trackindex = 0;
                    }
                    _player->row = cell->value;
                    break;
                
                case BLSfx_ARPEGGIO:

                    switch (voice->arpeggioState)
                    {
                    case ArpeggioState_STOPPED:
                        voice->arpeggioState = ArpeggioState_STARTING;
                        break;
                    }

                    {
                        BLSprecomputedKey* key1 = &sndtrack->keys[cell->value];
                        BLSprecomputedKey* key2 = &sndtrack->keys[cell->value2];

                        if (( key1 != voice->keys[1] ) || ( key2 != voice->keys[2] ))
                        {
                            voice->arpeggioState = ArpeggioState_STARTING;
                            voice->current = voice->currentsource = 0;

                            AssignBaseKeyToVoice(sndtrack, cell->value , 1, voice);
                            AssignBaseKeyToVoice(sndtrack, cell->value2, 2, voice);
                        }
                    }
                    break;

                case BLSfx_SETBALANCE:
                    if ((i == 0) || (i == 3))
                    {
                        _player->volumeLeft2 = *(u16*)&cell->value;
                        _player->volumeLeft2 = PCENDIANSWAP16(_player->volumeLeft2);
                    }
                    else
                    {
                        _player->volumeRight2 = *(u16*)&cell->value;
                        _player->volumeRight2 = PCENDIANSWAP16(_player->volumeRight2);
                    }
                    break;

                case BLSfx_BITMASK:
                    voice->mask = *(u16*)&cell->value;
                    voice->mask = PCENDIANSWAP16(voice->mask);
                    break;

                case BLSfx_SETSAMPLEOFFSET:
                    if ( voice->samples[0] != NULL )
                    {
                        u16 offset = *(u16*)(&cell->value);
                        offset = PCENDIANSWAP16(offset);
                        voice->current = STDmulu(voice->keys[0]->freqmul, offset) >> voice->keys[0]->freqmulshift;
                    }
                    break;

                case BLSfx_RETRIGGER_SAMPLE:
                    voice->retrigger = voice->retriggercount = cell->value;
                    break;

                case BLSfx_CLIENT_EVENT:
                    _player->clientEvent = cell->value;
                    break;

                case BLSfx_VOICE_OFF:
                    voice->samples[0] = NULL;
                    voice->arpeggioState = ArpeggioState_STOPPED;
                    break;

                case BLSfx_LOOP_START_SET:
                    _player->loopstart = currentrow;
                    break;

                case BLSfx_LOOP:
                    if (_player->loopcount == 0)
                    {
                        _player->loopcount = cell->value;
                        _player->row = _player->loopstart;
                    }
                    else if (_player->loopcount > 0)
                    {
                        _player->loopcount--;
                        if (_player->loopcount > 0)
                        {
                            _player->row = _player->loopstart;
                        }
                    }
                    break;

                case BLSfx_DELAY_SAMPLE:
                    voice->sampledelay = cell->value;
                    break;

                case BLSfx_DELAY_PATTERN:
                    delay = _player->speed * cell->value;
                    break;

                default:
                    ASSERT(0);
                }
            }
        }

        /* reset speed for next interpretation */
        _player->speedcount = _player->speed + delay;
    }
}
#endif


#ifndef blsUpdateRunningEffects
static void blsUpdateRunningEffects (BLSplayer* _player)
{
    u16 t;
    BLSvoice*   voice = _player->voices;        

    for (t = 0 ; t < BLS_NBVOICES ; t++, voice++)
    {
        if (voice->retrigger > 0)
        {
            voice->retriggercount--;
            if (voice->retriggercount == 0)
            {
                voice->current = 0;
                voice->retriggercount = voice->retrigger;
            }
        }

        switch (voice->arpeggioState)
        {
        case ArpeggioState_STARTING:
            voice->arpeggioState = ArpeggioState_RUNNING;
            /* no break here !!! */

        case ArpeggioState_STOPPED:
            voice->arpeggioOffset = 0;
            break;

        case ArpeggioState_STOPPING:
            voice->arpeggioState  = ArpeggioState_STOPPED;
            voice->arpeggioOffset = 8;
            /* no break here */

        case ArpeggioState_RUNNING:
            {
                u32 v;
                u16 freqmul;
                u8  freqmulshift;
                u8  arpeggioIndex;
                u16 currentSource = (u16) voice->currentsource;

                voice->arpeggioOffset += 4;
                if (voice->arpeggioOffset == 12)
                {
                    voice->arpeggioOffset = 0;
                }

                arpeggioIndex = voice->arpeggioOffset >> 2;
                freqmul       = voice->keys[arpeggioIndex]->freqmul;
                freqmulshift  = voice->keys[arpeggioIndex]->freqmulshift;

                v = STDmulu(currentSource, freqmul);
                v >>= freqmulshift;
                voice->current = v;

                ASSERT(voice->current < voice->samples[arpeggioIndex]->sampleLen);
            }
            break;
        }
    }
}
#endif


#ifndef BLSupdate
void BLSupdate (BLSplayer* _player)
{
#   ifndef __TOS__
    u8 playBuffer = EMULgetPlayOffset () >= BLS_NBBYTES_PERFRAME;
    {
        static u8 oldBuffer = 0;

        if (oldBuffer == playBuffer)
        {
            return;
        }
        oldBuffer = playBuffer;
    }
#   endif

    _player->framenum++;

    blsRASTER(0x50);

    blsUpdateSoundBuffers(_player);

    blsUpdateScore (_player);

    blsUpdateRunningEffects (_player);

    blsUpdateAllVoices (_player);

    blsSetDMABuffer (_player);

#   ifndef __TOS__
    if (_player->volumeLeft2)
    {
        _player->volumeLeft = _player->volumeLeft2;
        *HW_MICROWIRE_MASK  = HW_MICROWIRE_MASK_SOUND;    
        *HW_MICROWIRE_DATA  = _player->volumeLeft;
        _player->volumeLeft2 = 0;
        EMULleftChannel();
    }

    if (_player->volumeRight2)
    {
        _player->volumeRight = _player->volumeRight2;
        *HW_MICROWIRE_MASK  = HW_MICROWIRE_MASK_SOUND;    
        *HW_MICROWIRE_DATA  = _player->volumeRight;
        _player->volumeRight2 = 0;
        EMULrightChannel();
    }

    EMULplaysound (_player->buffertoupdate, BLS_NBBYTES_PERFRAME, playBuffer != 0 ? 0 : BLS_NBBYTES_PERFRAME);
#   endif
}

static void blsUpdAsync(BLSplayer* _player)
{
    u8* cleared = (u8*) _player->buffertoupdate;

    /* as we do not re-use buffer, enforce update by disabling clear channel optimization */
    cleared += BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD;
    *(u32*) cleared = 0;

    blsUpdateScore (_player);

    blsUpdateRunningEffects (_player);

    blsUpdateAllVoices (_player);
}
#endif


/* -------------------------------------------------------------------------------
    ASYNC PLAY ROUTINE
------------------------------------------------------------------------------- */

u32 BLSasyncPlay (BLSplayer* _player, u8 _trackindex, void* _buffer, u32 _buffersize)
{
    u32 size = 0;


    _player->buffertoupdate = (s8*) _buffer;

    do
    {
        if (_trackindex == _player->trackindex)
        {
            if (_player->speedcount == 1)
            {
                break;
            }
        }

        ASSERT ( (size + BLS_NBBYTES_PERFRAME) < _buffersize );

        blsUpdAsync(_player);

        _player->buffertoupdate += BLS_NBBYTES_PERFRAME;
        size += BLS_NBBYTES_PERFRAME;
    }
    while (1);

    return size;
}


/* -------------------------------------------------------------------------------
    TEST MODE
------------------------------------------------------------------------------- */

#if blsDUMP
static void blsDumpPlayerState (BLSplayer* _player, u32 _offset, FILE* _file)
{
    BLSvoice* voice = _player->voices;
    u16 i = 15;
    u16 w = 24;
	u16 w2 = w * BLS_NBVOICES;
    u16 t, j;

    static char tracep[] = 
    {
        "PLAYER --------        \n"
        "offset       =         \n"
        "speed        =         \n"
        "speedcount   =         \n"
        "trackindex   =         \n"
        "pattern      =         \n"
        "row          =         \n"
        "loopstart    =         \n"
        "loopcount    =         \n"
        "volumeLeft   =         \n"
        "volumeRight  =         \n"
        "clientEvent  =         \n"
    };

    static char trace[] = 
    {
        "key0         =          " "key0         =          " "key0         =          " "key0         =         \n"
        "key1         =          " "key1         =          " "key1         =          " "key1         =         \n"
        "key2         =          " "key2         =          " "key2         =          " "key2         =         \n"
        "current      =          " "current      =          " "current      =          " "current      =         \n"
        "currentsourc =          " "currentsourc =          " "currentsourc =          " "currentsourc =         \n"
        "mask         =          " "mask         =          " "mask         =          " "mask         =         \n"
        "volume       =          " "volume       =          " "volume       =          " "volume       =         \n"
        "sampledelay  =          " "sampledelay  =          " "sampledelay  =          " "sampledelay  =         \n"
        "retrigr      =          " "retrigr      =          " "retrigr      =          " "retrigr      =         \n"
        "retrigrcount =          " "retrigrcount =          " "retrigrcount =          " "retrigrcount =         \n"
        "arpegioState =          " "arpegioState =          " "arpegioState =          " "arpegioState =         \n"
        "arpegioOfset =          " "arpegioOfset =          " "arpegioOfset =          " "arpegioOfset =         \n"
		"\n\n"
    };

    STDuxtoa(&tracep[i], _player->framenum, 6);     i += w;
    STDuxtoa(&tracep[i], _offset            , 6);   i += w;
    STDuxtoa(&tracep[i], _player->speed, 2);        i += w;
    STDuxtoa(&tracep[i], _player->speedcount, 2);   i += w;
    STDuxtoa(&tracep[i], _player->trackindex, 2);   i += w;
    STDuxtoa(&tracep[i], _player->pattern, 2);      i += w;
    STDuxtoa(&tracep[i], _player->row, 2);          i += w;
    STDuxtoa(&tracep[i], _player->loopstart, 2);    i += w;
    STDuxtoa(&tracep[i], _player->loopcount, 2);    i += w;
    STDuxtoa(&tracep[i], _player->volumeLeft2, 4);  i += w;
    STDuxtoa(&tracep[i], _player->volumeRight2, 4); i += w;
    STDuxtoa(&tracep[i], _player->clientEvent, 2);

    fwrite (tracep, sizeof(tracep) - 1, 1, _file);

    for (t = 0 ; t < BLS_NBVOICES ; t++)
    {
        i = 15 + t * w;

        for (j = 0 ; j < 3 ; j++)
        {
            if (voice->keys[j] != NULL)
                STDuxtoa(&trace[i], voice->keys[j]->sampleIndex, 2);
            else
                STDuxtoa(&trace[i], -1, 2);
            i += w2;
        }

        STDuxtoa(&trace[i], voice->current        , 8); i += w2;
        STDuxtoa(&trace[i], voice->currentsource  , 8); i += w2;
        STDuxtoa(&trace[i], voice->mask           , 4); i += w2;
        STDuxtoa(&trace[i], voice->volume         , 2); i += w2;
        STDuxtoa(&trace[i], voice->sampledelay    , 2); i += w2;
        STDuxtoa(&trace[i], voice->retrigger      , 2); i += w2;
        STDuxtoa(&trace[i], voice->retriggercount , 2); i += w2;
        STDuxtoa(&trace[i], voice->arpeggioState  , 2); i += w2;
        STDuxtoa(&trace[i], voice->arpeggioOffset , 2); i += w2;
      
        voice++;
    }   

	fwrite (trace, sizeof(trace) - 1, 1, _file);
}


static void blsDumpSndtrackState (BLSsoundTrack* _sndtrack, FILE* _file)
{
    u16 w = 24;
    u16 t, i;
    BLSsample*         sample = _sndtrack->samples;
    BLSprecomputedKey* pkey   = _sndtrack->keys;


    for (t = 0 ; t < _sndtrack->nbKeys; t++)
    {
        static char tracep[] = 
        {
            "PKEY ----------        \n"
            "sampleindex  =         \n"
            "blittertrans =         \n"
            "freqmul      =         \n"
            "freqdiv      =         \n"
            "freqmulshift =         \n"
            "freqdivshift =         \n\n"
        };

        i = 15;
        STDuxtoa(&tracep[i], t                       , 2);  i += w;
        STDuxtoa(&tracep[i], pkey->sampleIndex       , 2);  i += w;
        STDuxtoa(&tracep[i], pkey->blitterTranspose  , 2);  i += w;
        STDuxtoa(&tracep[i], pkey->freqmul           , 4);  i += w;
        STDuxtoa(&tracep[i], pkey->freqdiv           , 4);  i += w;
        STDuxtoa(&tracep[i], pkey->freqmulshift      , 2);  i += w;
        STDuxtoa(&tracep[i], pkey->freqdivshift      , 2);  i += w;
        
        pkey++;

        fwrite (tracep, sizeof(tracep) - 1, 1, _file);
    }

    for (t = 0 ; t < _sndtrack->nbSamples ; t++)
    {
        static char tracep[] = 
        {
            "SAMPLE --------        \n"
            "length       =         \n"
            "loopstart    =         \n"
            "looplength   =         \n"
            "flags        =         \n\n"
        };

        i = 15;
        STDuxtoa(&tracep[i], t                       , 2);  i += w;
        STDuxtoa(&tracep[i], sample->sampleLen       , 6);  i += w;
        STDuxtoa(&tracep[i], sample->sampleLoopStart , 6);  i += w;
        STDuxtoa(&tracep[i], sample->sampleLoopLength, 4);  i += w;
        STDuxtoa(&tracep[i], sample->flags           , 4);  i += w;
        
        sample++;

        fwrite (tracep, sizeof(tracep) - 1, 1, _file);
    }
}


void BLStestPlay (BLSplayer* _player, u8 _trackindex, char* _filesamplename, char* _filetracename)
{
    bool reachindex = false;
    u8  lastrow = _player->row;
    bool endreached = false;
    u32 offset = 0;

    FILE* filetrace  = fopen(_filetracename , "wb");
    FILE* filesample = fopen(_filesamplename, "wb");

    blsDumpSndtrackState(_player->sndtrack, filetrace);

    _player->buffertoupdate = _player->buffer;

    do
    {
        _player->framenum++;

        blsDumpPlayerState(_player, offset, filetrace);

        if (reachindex)
        {
            if ( lastrow > _player->row )
            {
                endreached = true;
            }
        }
        else
        {
            reachindex = _trackindex == _player->trackindex;
        }

        if (endreached)
        {
            if (_player->speedcount == 1)
            {
                break;
            }
        }

        lastrow = _player->row;

        blsUpdAsync(_player);

		fwrite (_player->buffer, BLS_NBBYTES_PERFRAME, 1, filesample);
        offset += BLS_NBBYTES_PERFRAME;
    }
    while (1);

    fclose(filesample);
    fclose(filetrace);
}
#endif /* blsDUMP */
