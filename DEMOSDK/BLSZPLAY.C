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

#include "DEMOSDK\BASTYPES.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\BLSSND.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\TRACE.H"

#include "DEMOSDK\PC\EMUL.H"
#include "DEMOSDK\BLSZDUMP.H"


/* define wrapping to C or ASM version for each routine */
#include "DEMOSDK\BLSZPLAY.H"
		

#define BLZ_ALLOW_PLAYERTRACE() 0

#ifndef __TOS__

u32 blsSecondaryBufferFreq = BLS_DMA_FREQ;

void BLSsetSecondaryBufferFreq(u32 _freq)
{
    blsSecondaryBufferFreq = _freq;
}

void aBLZvbl(void) 
{
    *HW_COLOR_LUT = aBLZbackground;
    BLZupdate(aBLZplayer);
}

void aBLZ2vbl(void)
{
    aBLZvbl();
}

BLSplayer* aBLZplayer     = NULL;
u16        aBLZbackground = 0;

#endif /* __TOS__ */


enum BLSbufferState_
{
    BLSbs_STEP0 = 0,
    BLSbs_STEP1 = 4,
    BLSbs_STEP2 = 8,
    BLSbs_START = 12
};
typedef enum BLSbufferState_ BLSbufferState;

 
void BLZplayerInit(MEMallocator* _allocator, BLSplayer* _player, BLSsoundTrack* _sndtrack, BLZdmaMode _dmamode)
{
    u8  v;
    u16 buffersize = (BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD + BLS_NBBYTES_CLEARFLAGS) * BLS_NB_BUFFERS;

    DEFAULT_CONSTRUCT(_player);

    _player->volumeLeft = _player->volumeLeft2 = _player->volumeRight = _player->volumeRight2 = 0;
    _player->sndtrack = _sndtrack;

    _player->buffer   = (s8*) MEM_ALLOC (_allocator, buffersize);

    STDmset (_player->buffer, 0UL, buffersize);
      	
    for (v = 0 ; v < BLS_NBVOICES ; v++)
    {
        _player->voices[v].mask = 0xFFFF;

#       if BLS_ENABLE_OPTIMIZED_DUMP
        _player->voices[v].voicenum = v;
#       endif
    }

    if (BLSisBlitzSndtrack(_sndtrack))
    {
        u8  firstpatternindex = _player->sndtrack->track[0];
        u32 dataoffset = _player->sndtrack->patternsdata   [firstpatternindex];
        u32 datalen    = _player->sndtrack->patternslength [firstpatternindex];

        _player->blizcurrent = _player->sndtrack->framesdata + dataoffset;
        _player->patternend  = _player->blizcurrent + datalen;
    }

    if (_dmamode != BLZ_DMAMODE_NOAUDIO)
    {
        u32 buffer = (u32) _player->buffer;


		*HW_MICROWIRE_MASK = HW_MICROWIRE_MASK_SOUND;    
		*HW_MICROWIRE_DATA = HW_MICROWIRE_MIXER_YM; /* YM -12db does not work on default hardware unfortunately :( */
		while (*HW_MICROWIRE_MASK != HW_MICROWIRE_MASK_SOUND);

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

        *HW_DMASOUND_STARTADR_H = (u8)(buffer >> 16);
        *HW_DMASOUND_STARTADR_M = (u8)(buffer >> 8);
        *HW_DMASOUND_STARTADR_L = (u8) buffer;

        {
            u32 endbuf = buffer + BLS_NBBYTES_PERFRAME;

            if (_dmamode == BLZ_DMAMODE_LOOP)
            {
                endbuf += BLS_NBBYTES_OVERHEAD;
            }

            *HW_DMASOUND_ENDADR_H = (u8)(endbuf >> 16);
            *HW_DMASOUND_ENDADR_M = (u8)(endbuf >> 8);
            *HW_DMASOUND_ENDADR_L = (u8) endbuf;
        }
        
        _player->bufferstate = BLSbs_START;

        EMULcreateSoundBuffer (BLS_NBSAMPLES_PERFRAME * 2, true, blsSecondaryBufferFreq);
    }
}


/* -------------------------------------------------------------------------------
    C versions of play routines, useful for : 
    - PC version
    - easier development
    - test purpose by compairing C and ASM version results
    
    Similar routines are availble in ASM version
------------------------------------------------------------------------------- */

#ifndef blzUpdateSoundBuffers
void blzUpdateSoundBuffers (BLSplayer* _player)
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


#ifndef blzUpdateSoundBuffers2
void blzUpdateSoundBuffers2 (BLSplayer* _player)
{
    u32 buffer = (u32) _player->buffer; 


    *HW_DMASOUND_CONTROL = HW_DMASOUND_CONTROL_PLAYONCE;

    if (_player->volumeLeft)
    {
        *HW_MICROWIRE_MASK = HW_MICROWIRE_MASK_SOUND;    
        *HW_MICROWIRE_DATA = _player->volumeLeft;
    }

    switch (_player->bufferstate)
    {
    case BLSbs_START:
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

    case BLSbs_STEP2:
        _player->buffertoupdate = _player->buffer + BLS_STEP_PERFRAME + BLS_STEP_PERFRAME;
        _player->dmabufstart    = buffer + BLS_STEP_PERFRAME;
        _player->bufferstate    = BLSbs_STEP0;
        break;
    }

    _player->dmabufend = _player->dmabufstart + BLS_NBBYTES_PERFRAME;

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
#endif /* blsUpdateSoundBuffers2 */


#ifndef blzUpdateAllVoices
static void blzUpdateVoiceFrame(BLSplayer* _player, BLSvoice* _voice, u8* _buffer, bool _firstpass)
{
    u16  remain = BLS_NBSAMPLES_PERFRAME / 2;    /* nb samples per voice per frame */
    u8   arpeggioIndex = _voice->arpeggioOffset >> 2;
    u8*  cleared2 = _buffer + BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD;
    u8*  cleared = cleared2 + _firstpass;
    BLSsoundTrack* sndtrack = _player->sndtrack;


    blsRASTER(0x70);

    *HW_BLITTER_ADDR_DEST = (u32)_buffer;

    ASSERT(((*cleared) & 0xFE) == 0);

    blsOptDump_mask(_player->blitzDumper, _voice, _voice->mask);

    if ((_voice->keys[0] == NULL) || (_voice->mute != 0) || (_voice->sampledelay != 0))
    {
        blsOptDump_voiceactivation(_player->blitzDumper, _voice, false);

#       if BLS_SCOREMODE_ENABLE
        _voice->mirroractivation = false; /* for trace */
#       endif

        if (*cleared)
        {
            if (_voice->sampledelay)
            {
                _voice->sampledelay--;
            }
            return;
        }

        *cleared = true;
        *cleared2 = true;

        goto clear;
    }

#   if BLS_SCOREMODE_ENABLE
    _voice->mirroractivation = true; /* for trace */
#   endif

    *cleared = false;
    *cleared2 = false;

    blsOptDump_voiceactivation(_player->blitzDumper, _voice, true);

    {
        BLSprecomputedKey* key = _voice->keys[arpeggioIndex];
        BLSsample* sample;
        u8 sampleIndex = key->sampleIndex;

        if (BLS_IS_BASEKEY(key) == false)
        {
            ASSERT(key->sampleIndex <= sndtrack->nbKeys);
            sampleIndex = sndtrack->keys[sampleIndex].sampleIndex;
        }

        ASSERT(sampleIndex <= sndtrack->nbSamples);
        sample = &sndtrack->samples[sampleIndex];

        {
            u32  scrCurrentSampl = _voice->current;
            s8   transpose = key->blitterTranspose >> 1;
            u32  sampleLen = sample->sampleLen;
            u32  sampleLoopStart = sample->sampleLoopStart;
            u16  sampleLoopLength = sample->sampleLoopLength;
            s8*  sampleData = sample->sample;
            u32  transfer;
            bool storageshift = (sample->flags & BLS_SAMPLE_STORAGE_SHIFT) != 0;
            u8   volume = _voice->volume;


            blsOptDump_volume(_player->blitzDumper, _voice, volume);

            ASSERT((volume == 0) || (volume == 8) || ((sample->flags & BLS_SAMPLE_STORAGE_INTERLACE) == 0));

            if ((sample->flags & BLS_SAMPLE_STORAGE_INTERLACE) == 0)
            {
                volume += _voice->volumeoffset;
                if (volume > 8)
                {
                    volume = 8;
                }
            }

            if (volume == 8)
            {
                *HW_BLITTER_HOP = HW_BLITTER_HOP_BIT1;
                *HW_BLITTER_OP = HW_BLITTER_OP_BIT0;
                *HW_BLITTER_CTRL2 = 0;
            }
            else
            {
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

                if (_voice->mask != 0xFFFF)
                {
                    u32 mask = _voice->mask;
                    STDmset(HW_BLITTER_HTONE, (mask << 16) | mask, 32);
                    *HW_BLITTER_HOP = HW_BLITTER_HOP_SOURCE_AND_HTONE;
                }
                else
                {
                    *HW_BLITTER_HOP = HW_BLITTER_HOP_SOURCE;
                }
                *HW_BLITTER_OP = HW_BLITTER_OP_S;
            }

            ASSERT(scrCurrentSampl < sampleLen);

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

                remain -= (u16)transfer;
                transfer >>= transpose;

                *HW_BLITTER_ADDR_SOURCE = (u32)(sampleData + (scrCurrentSampl << 1));
                *HW_BLITTER_XSIZE = 1 << transpose;
                *HW_BLITTER_YSIZE = (u16)transfer;

                blsRASTER(0x700);
                *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
                EMULblit();
                blsRASTER(0x70);

                scrCurrentSampl += transfer;
            }
            else
            {
                *HW_BLITTER_XINC_SOURCE = 2 << transpose;

                transfer = (sampleLen - scrCurrentSampl) >> transpose;

                if (transfer > 0) /* can be 0 when transpose > 0 */
                {
                    if (transfer > remain)
                    {
                        transfer = remain;
                    }

                    *HW_BLITTER_ADDR_SOURCE = (u32)(sampleData + (scrCurrentSampl << 1));
                    *HW_BLITTER_YINC_SOURCE = *HW_BLITTER_XINC_SOURCE;
                    *HW_BLITTER_XSIZE = (u16)transfer;
                    *HW_BLITTER_YSIZE = (transfer >> 16) + 1;

                    blsRASTER(0x700);
                    *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
                    EMULblit();
                    blsRASTER(0x70);

                    scrCurrentSampl += transfer << transpose;
                    remain -= (u16)transfer;

                    if ((remain > 0) && (sampleLoopLength > 0))
                    {
                        u16 translooplen = (u16)(sampleLoopLength >> transpose);
                        u32 div = STDdivu(remain, translooplen);
                        u16 nbloops = (u16)div;
                        u16 incXSource = *HW_BLITTER_XINC_SOURCE;
                        u32 sloopData = (u32)(sampleData + (sampleLoopStart << 1));

                        scrCurrentSampl = sampleLoopStart;

                        if (nbloops > 0)
                        {
                            *HW_BLITTER_ADDR_SOURCE = sloopData;
                            *HW_BLITTER_YINC_SOURCE = incXSource - (translooplen << (transpose + 1));
                            *HW_BLITTER_XSIZE = translooplen;
                            *HW_BLITTER_YSIZE = nbloops;

                            blsRASTER(0x700);
                            *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
                            EMULblit();
                            blsRASTER(0x70);

                            remain = div >> 16;
                        }

                        if (remain > 0)
                        {
                            *HW_BLITTER_ADDR_SOURCE = sloopData;
                            *HW_BLITTER_YINC_SOURCE = incXSource;
                            *HW_BLITTER_XSIZE = remain;
                            *HW_BLITTER_YSIZE = 1;

                            blsRASTER(0x700);
                            *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
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
                    _voice->keys[0] = NULL;
                    _voice->arpeggioState = ArpeggioState_STOPPED;
                    blsOptDump_nullkeypost(_player->blitzDumper, _voice);
                }
                else
                {
                    ASSERT(remain == 0);
                    _voice->current = sampleLoopStart;
                }
            }
        }

        if (remain > 0)
        {
        clear:
            *HW_BLITTER_CTRL2 = 0;
            *HW_BLITTER_ADDR_SOURCE = (u32)_buffer;
            *HW_BLITTER_HOP = HW_BLITTER_HOP_BIT1;
            *HW_BLITTER_OP = HW_BLITTER_OP_BIT0;
            *HW_BLITTER_XSIZE = (u16)remain;
            *HW_BLITTER_YSIZE = 1;

            blsRASTER(0x700);
            *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
            EMULblit();
            blsRASTER(0x70);

            if (_voice->sampledelay)
            {
                _voice->sampledelay--;
                return;
            }

            if (_voice->keys[0] != NULL)
                blsOptDump_nullkeypost(_player->blitzDumper, _voice);

            _voice->keys[0] = NULL;
            _voice->arpeggioState = ArpeggioState_STOPPED;
        }
        else if (_voice->arpeggioState != ArpeggioState_STOPPED)
        {
            u16 freqdiv = key->freqdiv;
            u8  freqdivshift = key->freqdivshift;
            u16 current = (u16)_voice->current;
            u32 v;


            v = STDmulu(current, freqdiv);
            v >>= freqdivshift;

            ASSERT(v <= 0xFFFF);
            _voice->currentsource = v;
        }

        ASSERT(*HW_BLITTER_ADDR_DEST == (u32)(_buffer + BLS_NBBYTES_PERFRAME));
    }
}


void blzUpdateAllVoices (BLSplayer* _player)
{
    u32  backbuf  = (u32) _player->buffertoupdate;
    u32* overhead = (u32*)(backbuf + BLS_NBBYTES_PERFRAME);


    *overhead = 0x12341234UL;

    *HW_BLITTER_XINC_DEST   = 4;    /* multiplexing */
    *HW_BLITTER_YINC_DEST   = 4;

    *HW_BLITTER_ENDMASK1  = 0xFFFF;
    *HW_BLITTER_ENDMASK2  = 0xFFFF;
    *HW_BLITTER_ENDMASK3  = 0xFFFF;

    blzUpdateVoiceFrame (_player, &_player->voices[1], (u8*) backbuf    , true );
    blzUpdateVoiceFrame (_player, &_player->voices[2], (u8*) backbuf + 2, true );   

    *HW_BLITTER_ENDMASK1  = PCENDIANSWAP16(0xFF00);
    *HW_BLITTER_ENDMASK2  = PCENDIANSWAP16(0xFF00);
    *HW_BLITTER_ENDMASK3  = PCENDIANSWAP16(0xFF00);

    blzUpdateVoiceFrame (_player, &_player->voices[0], (u8*) backbuf    , false);
    blzUpdateVoiceFrame (_player, &_player->voices[3], (u8*) backbuf + 2, false);

    ASSERT( *(overhead) == 0x12341234UL );

    *overhead = overhead[-1];
}
#endif /* blsUpdateAllVoices */


#ifndef blzBlitzDecode
static void blzManageEndPattern(BLSplayer * _player)
{
    BLSsoundTrack* sndtrack = _player->sndtrack;

    if (_player->blizcurrent >= _player->patternend)
    {
        u32 patterndata;
        u32 patterndatalen;
        u8  patternindex;

        _player->trackindex++;
        if (_player->trackindex >= sndtrack->trackLen)
        {
            _player->maxframenum = _player->framenum;
            _player->framenum    = 0;
            _player->tracklooped = true;
            _player->trackindex  = 0;
        }

        patternindex   = sndtrack->track[_player->trackindex];
        patterndata    = sndtrack->patternsdata[patternindex];
        patterndatalen = sndtrack->patternslength[patternindex];

        _player->blizcurrent = sndtrack->framesdata + patterndata;
        _player->patternend = _player->blizcurrent + patterndatalen;
    }
}

static void blzBlitzDecode (BLSplayer* _player)
{
    u8*  framedata  = _player->blizcurrent;
    u8*  blitzdata  = framedata;
    BLSsoundTrack* sndtrack = _player->sndtrack;
    u8   t;

#   ifndef __TOS__
#   if BLZ_ALLOW_PLAYERTRACE()
    bool printline = voicedata != 0;
#   else
    const bool printline = false;
#   endif

    if (printline)
    {
        printf("\n%2x %2x %5d ($%-6x) : |", _player->trackindex, sndtrack->track[_player->trackindex], _player->framenum, blitzdata - sndtrack->framesdata);
    }
#   endif

    /* YM  1 byte : Mixer (FreqA | restart sqr A) (FreqB | restart sqr B) (FreqC | restart sqr C) FreqEnvelop (LevelA | Freq noise) (LevelB | EnvelopShape) LevelC

    if Mixer
        1 byte mixer 

    if (FreqX | restart sqr X)
        1 byte FreqX low
        1 byte FreqX hight (4 low bits) | 0x80 if square X need to be restarted

    if FreqEnvelop
        1 byte FreqEnvelopL
        1 byte FreqEnvelopH

    if LevelA | freqnoise
        1 byte Level A (5 bits low) | 0x80 if freqnoise changed
        if Freqnoise changed
            1 byte freqnoise
    
    if LevelB | Envshape
        1 byte Level A (5 bits low) | 0x80 if envshape changed
        if envshape changed
            1 byte envshape

    if LevelC
        1 byte Level C
    */

    {
        u8 ymvoicedata = *blitzdata++;

        if (ymvoicedata != 0)
        {
            if (ymvoicedata & 0x80)
            { 
                u8 mixer = *blitzdata++;
                HW_YM_SET_REG(HW_YM_SEL_IO_AND_MIXER, mixer);
            }
            ymvoicedata <<= 1;

            for (t = 0 ; t < SND_YM_NB_CHANNELS ; t++)
            {
                if (ymvoicedata & 0x80)
                { 
                    u8 freqL = *blitzdata++;
                    s8 freqH = *blitzdata++;

                    if (freqH < 0)
                    {
                        HW_YM_SET_REG(HW_YM_SEL_FREQCHA_L + (t << 1), 0);
                        HW_YM_SET_REG(HW_YM_SEL_FREQCHA_H + (t << 1), 0);
                    }

                    HW_YM_SET_REG(HW_YM_SEL_FREQCHA_L + (t << 1), freqL);
                    HW_YM_SET_REG(HW_YM_SEL_FREQCHA_H + (t << 1), freqH);
                }
                ymvoicedata <<= 1;
            }

            if (ymvoicedata & 0x80)
            {
                u8 freqL = *blitzdata++;
                u8 freqH = *blitzdata++;

                HW_YM_SET_REG(HW_YM_SEL_FREQENVELOPE_L, freqL);
                HW_YM_SET_REG(HW_YM_SEL_FREQENVELOPE_H, freqH);
            }
            ymvoicedata <<= 1;

            if (ymvoicedata & 0x80)
            {
                s8 levelA = *blitzdata++;

                if (levelA < 0)
                {
                    u8 noisefreq = *blitzdata++;
                    HW_YM_SET_REG(HW_YM_SEL_FREQNOISE, noisefreq);
                }
                HW_YM_SET_REG(HW_YM_SEL_LEVELCHA, levelA);
            }
            ymvoicedata <<= 1;

            if (ymvoicedata & 0x80)
            {
                s8 levelB = *blitzdata++;

                if (levelB < 0)
                {
                    u8 envshape = *blitzdata++;
                    HW_YM_SET_REG(HW_YM_SEL_ENVELOPESHAPE, envshape);
                }
                HW_YM_SET_REG(HW_YM_SEL_LEVELCHB, levelB);
            }
            ymvoicedata <<= 1;

            if (ymvoicedata & 0x80)
            {
                u8 levelC = *blitzdata++;

                HW_YM_SET_REG(HW_YM_SEL_LEVELCHC, levelC);
            }
            ymvoicedata <<= 1;
        }
    }

    /* PCM  1 byte : V0desc V1desc V2desc V3desc V0active V1active V2active V3active

    foreach V<x>desc
        1 byte  ArpegOn SampleIndexChanged TranposeChanged MaskChanged 0 Volume

    if ArpegOn
        2 bytes Offset

    if SampleIndexChanged
        1 byte SampleIndex

    if TranposeChanged
        1 byte Transpose (4 bits used)

    if MaskChanged
        1 byte Mask
    */

    {
        BLSprecomputedKey* keys = sndtrack->keys;
        BLSvoice* voice = _player->voices;
        u8   voicedata = *blitzdata++;


        for (t = 0; t < BLS_NBVOICES; t++, voice++)
        {
            bool hasvoicedesc = (voicedata & 0x80) != 0;
            bool voiceactive = (voicedata & 8) != 0;
            bool retriggersample = false;
            bool haskeynum = false;
            bool arpegiostart = false;
            bool arpegiostop = false;
            bool volumeset = false;
            u8   keynum = 0;


            /*voice->mute = !voiceon;*/
            if (hasvoicedesc)
            {
                u8 voicedesc = *blitzdata++;

                /*
                arpeggiostatechanged     1 bit
                sampleindexchanged       1 bit
                transposechanged         1 bit
                volumechanged            1 bit
                volume                   4 bits unsigned

                arpeggio                 16 bits
                sampleindex              8  bits
                transpose                4  bits signed
                currentsource            16 bits ? 24 bits
                */

                /* implemented the same way than in ASM (by shifting) */
                haskeynum = (voicedesc & 0x80) != 0;
                if (haskeynum)
                {
                    voice->current = 0;
                    voice->currentsource = 0;
                    voice->volume = 0;
                    voice->arpeggioState = ArpeggioState_STOPPED;
                    voice->retrigger = 0;
                    voice->sampledelay = 0;

                    keynum = *blitzdata++;

                    if (keynum == 0)
                        voice->keys[0] = NULL;
                    else
                        voice->keys[0] = &keys[keynum - 1];
                }
                voicedesc <<= 1;

                arpegiostart = (voicedesc & 0x80) != 0;
                if (arpegiostart)
                {
                    voice->keys[1] = &keys[*blitzdata++];
                    voice->keys[2] = &keys[*blitzdata++];

                    if (voice->arpeggioState == ArpeggioState_STOPPED)
                    {
                        voice->arpeggioState = ArpeggioState_STARTING;
                    }
                }
                voicedesc <<= 1;

                arpegiostop = (voicedesc & 0x80) != 0;
                if (arpegiostop)
                {
                    /*if (haskeynum == false)*/
                    {
                        if (voice->arpeggioState != ArpeggioState_STOPPED)
                        {
                            voice->arpeggioState = ArpeggioState_STOPPING;
                        }
                    }
                }
                voicedesc <<= 1;

                if (voicedesc & 0x80)
                {
                    voice->mask = *blitzdata++;
                    voice->mask |= (u16)voice->mask << 8;
                }
                voicedesc <<= 1;

                retriggersample = (voicedesc & 0x80) != 0;
                if (retriggersample)
                {
                    voice->current = 0;
                }
                voicedesc <<= 1;

                volumeset = (voicedesc & 0x80) != 0;
                if (volumeset)
                {
                    voice->volume = *blitzdata++;
                }
                voicedesc <<= 1;

                if (voicedesc & 0x80)
                {
                    u32 offset = *blitzdata++;
                    
                    offset <<= 8;
                    offset |= *blitzdata++;
                    offset <<= 8;
                    offset |= *blitzdata++;

                    voice->current = offset;
                }
                voicedesc <<= 1;
            }

            voice->sampledelay = voiceactive ? 0 : 0xFF;

#           ifndef __TOS__
            if (printline)
            {
                printf("%c ", haskeynum ? '}' : ' ');
                if (haskeynum)
                    printf("%02x ", keynum);
                else
                    printf("   ");

                printf("|");
            }
#           endif

            voicedata <<= 1;

            /* Update arpeggio */
            switch (voice->arpeggioState)
            {
            case ArpeggioState_STARTING:
                voice->arpeggioState = ArpeggioState_RUNNING;
                /* no break here !!! */

            case ArpeggioState_STOPPED:
                voice->arpeggioOffset = 0;
                break;

            case ArpeggioState_STOPPING:
                voice->arpeggioState = ArpeggioState_STOPPED;
                voice->arpeggioOffset = 8;
                /* no break here */

            case ArpeggioState_RUNNING:
            {
                u32 v;
                u16 freqmul;
                u8  freqmulshift;
                u16 currentSource = (u16)voice->currentsource;
                BLSprecomputedKey* key;

                voice->arpeggioOffset += 4;
                if (voice->arpeggioOffset == 12)
                {
                    voice->arpeggioOffset = 0;
                }

                key = voice->keys[voice->arpeggioOffset >> 2];
                freqmul = key->freqmul;
                freqmulshift = key->freqmulshift;

                v = STDmulu(currentSource, freqmul);
                v >>= freqmulshift;
                voice->current = v;
            }
            break;
            }
        }
    }

    _player->blizcurrent = blitzdata;

#   ifndef __TOS__
#   if BLZ_ALLOW_PLAYERTRACE()
    ASSERT(printline || ((blitzdata - framedata) == 1));
#   endif
#   endif

#   ifndef __TOS__
    if (printline)
    {
        printf(" %d", blitzdata - framedata);
    }
#   endif

    blzManageEndPattern(_player);
}
#endif /* blsBlitzDecode */


#ifndef blzSetDMABuffer
void blzSetDMABuffer(BLSplayer* _player)
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
#endif /* blsSetDMABuffer */

#ifndef __TOS__
void blzUpdateMicrowireEmul(BLSplayer* _player)
{
    if (_player->volumeLeft2)
    {
        _player->volumeLeft = _player->volumeLeft2;
        *HW_MICROWIRE_MASK = HW_MICROWIRE_MASK_SOUND;
        *HW_MICROWIRE_DATA = _player->volumeLeft;
        _player->volumeLeft2 = 0;
        EMULleftChannel();
    }

    if (_player->volumeRight2)
    {
        _player->volumeRight = _player->volumeRight2;
        *HW_MICROWIRE_MASK = HW_MICROWIRE_MASK_SOUND;
        *HW_MICROWIRE_DATA = _player->volumeRight;
        _player->volumeRight2 = 0;
        EMULrightChannel();
    }
}
#endif

#ifndef BLZupdate
void BLZupdate (BLSplayer* _player)
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

    ASSERT( BLSisBlitzSndtrack(_player->sndtrack) );

    _player->framenum++;

    blsRASTER(0x50);

    blzUpdateSoundBuffers(_player);

    blzBlitzDecode(_player);

    blzUpdateAllVoices(_player);

    blzSetDMABuffer (_player);

#   ifndef __TOS__
    blzUpdateMicrowireEmul(_player);

    {
        static void* PCdelayedbuffer = NULL;

        if (PCdelayedbuffer != NULL)
            EMULplaysound (PCdelayedbuffer, BLS_NBBYTES_PERFRAME, playBuffer != 0 ? 0 : BLS_NBBYTES_PERFRAME);

        PCdelayedbuffer = (void*)_player->dmabufstart;
    }
#   endif
}

void BLZupdAsync(BLSplayer* _player)
{
    u8* cleared = (u8*) _player->buffertoupdate;


    _player->framenum++;

    /* as we do not re-use buffer, enforce update by disabling clear channel optimization */
    cleared += BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD;
    *(u32*) cleared = 0;

    blzBlitzDecode (_player);

    blzUpdateAllVoices (_player);
}
#endif /* BLSupdate */


#ifndef BLZ2update
void BLZ2update (BLSplayer* _player)
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

    ASSERT( BLSisBlitzSndtrack(_player->sndtrack) );

    _player->framenum++;

    blsRASTER(0x50);

    blzUpdateSoundBuffers2(_player);

    blzBlitzDecode(_player);

    blzUpdateAllVoices(_player);

    blzSetDMABuffer (_player);

#   ifndef __TOS__
    blzUpdateMicrowireEmul(_player);

    {
        static void* PCdelayedbuffer = NULL;

        if (PCdelayedbuffer != NULL)
            EMULplaysound (PCdelayedbuffer, BLS_NBBYTES_PERFRAME, playBuffer != 0 ? 0 : BLS_NBBYTES_PERFRAME);

        PCdelayedbuffer = (void*)_player->dmabufstart;
    }
#   endif
}
#endif /* BLZ2update */


void BLZresetYM(void)
{
    HW_YM_SET_REG(HW_YM_SEL_IO_AND_MIXER, 0xFF);
    HW_YM_SET_REG(HW_YM_SEL_LEVELCHA,     0 );
    HW_YM_SET_REG(HW_YM_SEL_LEVELCHB,     0 );
    HW_YM_SET_REG(HW_YM_SEL_LEVELCHC,     0 );
}

void BLZgoto (BLSplayer* _player, u8 _trackIndex)
{
    u16 i;
    BLSsoundTrack* sndtrack = _player->sndtrack;
    u8 patternindex;
    u32 patterndata;
    u32 patterndatalen;
    
    
    BLZresetYM ();

    _player->trackindex = _trackIndex;
    patternindex    = sndtrack->track[_trackIndex];
    patterndata     = sndtrack->patternsdata [patternindex];
    patterndatalen  = sndtrack->patternslength [patternindex];
    
    _player->blizcurrent = sndtrack->framesdata + patterndata;
    _player->patternend  = _player->blizcurrent + patterndatalen;

    for (i = 0 ; i < BLS_NBVOICES ; i++)
    {
        _player->voices[i].keys[0]       = NULL;
        _player->voices[i].arpeggioState = ArpeggioState_STOPPED;
        _player->voices[i].mask          = 0xFFFF;
    }

    _player->volumeLeft2  = 20 | HW_MICROWIRE_VOLUME_LEFT;
    _player->volumeRight2 = 20 | HW_MICROWIRE_VOLUME_RIGHT;
}
