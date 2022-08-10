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

#ifdef __TOS__ /* in order to check ASM alignement */
STATIC_ASSERT(sizeof(BLSvoice)          == 32);
STATIC_ASSERT(sizeof(BLSsample)         == 16);
STATIC_ASSERT(sizeof(BLSprecomputedKey) == 8);
STATIC_ASSERT(sizeof(BLScell)           == 4);
#endif

#ifdef __TOS__
#   define blsINITUSEASM 1

#   if blsINITUSEASM
    void ablsIIWsample(void* sourcesample_sample, void* _dest, u32 samplelen, u32 divprecision_precisionmask, u32 freqdiv);
    void ablsIIBsample(void* sourcesample_sample, void* _dest, u32 samplelen, u32 divprecision_precisionmask, u32 freqdiv);
    void ablsINWsample(void* sourcesample_sample, void* _dest, u32 samplelen, u16 divprecision, u32 freqdiv);
    void ablsINBsample(void* sourcesample_sample, void* _dest, u32 samplelen, u16 divprecision, u32 freqdiv);
    void ablsUndelta(s8* _s1, u32 _len);
#   endif
#endif


void BLSinitSample (s8* _sampleBuffer, u32* _sampleHeap, BLSsample* _sourceSample, BLSsample* _transposedSample, BLSprecomputedKey* _key, u16 _storagemode)
{
    u32 roundvalue       = 1UL << (_key->freqmulshift - 1);
    u16 sampleLenH       = (u16)(_sourceSample->sampleLen >> 16);
    u16 sampleLenL       = (u16)_sourceSample->sampleLen;
    u16 sampleLoopStartH = (u16)(_sourceSample->sampleLoopStart >> 16);
    u16 sampleLoopStartL = (u16)_sourceSample->sampleLoopStart;
    s8* d                = NULL;
    u32 size;


    ASSERT(_sourceSample->sampleLen > 0);

    _transposedSample->sampleLen         = STDmulu(sampleLenL, _key->freqmul);
    _transposedSample->sampleLen        += STDmulu(sampleLenH, _key->freqmul) << 16;
    _transposedSample->sampleLen        += roundvalue;
    _transposedSample->sampleLen       >>= _key->freqmulshift;

    _transposedSample->sampleLoopStart   = STDmulu(sampleLoopStartL, _key->freqmul);
    _transposedSample->sampleLoopStart  += STDmulu(sampleLoopStartH, _key->freqmul) << 16;
    _transposedSample->sampleLoopStart  += roundvalue;
    _transposedSample->sampleLoopStart >>= _key->freqmulshift;

    _transposedSample->sampleLoopLength = (u16) (STDmulu(_sourceSample->sampleLoopLength, _key->freqmul) >> _key->freqmulshift);
    _transposedSample->flags            = _sourceSample->flags;

    size = (_transposedSample->sampleLen + 1) * 2;

    switch (_storagemode)
    {
    case BLS_STORAGE_WORD:
        _transposedSample->sample = _sampleBuffer + _sampleHeap[0];
        _sampleHeap[0] += size;
        _sampleHeap[1] += size;
        d = _transposedSample->sample;
        break;
    case BLS_STORAGE_BYTE_0:
        _transposedSample->sample = _sampleBuffer + _sampleHeap[0];
        _sampleHeap[0] += size;
        _transposedSample->flags |= BLS_SAMPLE_STORAGE_INTERLACE;
        d = _transposedSample->sample;
        break;
    case BLS_STORAGE_BYTE_1:
        _transposedSample->sample = _sampleBuffer + _sampleHeap[1];
        _sampleHeap[1] += size;
        _transposedSample->flags |= BLS_SAMPLE_STORAGE_SHIFT;
        _transposedSample->flags |= BLS_SAMPLE_STORAGE_INTERLACE;
        d = _transposedSample->sample - 1;
        break;
    }
    
    {
        u32 sampleLen = _transposedSample->sampleLen;
        s8* s = _sourceSample->sample;
        u8  divprecision = _key->freqdivshift;
        u16 freqdiv = _key->freqdiv;
#       if blsINITUSEASM==0
        u32 i, acc = 0;
        s32 sampl1, sampl2;
        s8  sampl;
        s16 interp1, interp2;
#       endif

        if (_sourceSample->flags & BLS_SAMPLE_INTERPOLATE)
        {
            u16 precisionmask;

            if (divprecision == 16) /* interp do not fit with 16 bits shift */
            {
                divprecision = 15;
                freqdiv >>= 1;
            }
           
            precisionmask = (1 << divprecision) - 1;

#           if blsINITUSEASM
            if (_storagemode == BLS_STORAGE_WORD)
                ablsIIWsample(s, d, sampleLen, ((u32)divprecision << 16) | (u32)precisionmask, freqdiv);
            else
                ablsIIBsample(s, d, sampleLen, ((u32)divprecision << 16) | (u32)precisionmask, freqdiv);
            d += sampleLen << 1;
#           else
            for (i = 0 ; i < sampleLen ; i++)
            {
                u32 index = acc  >> divprecision;

                sampl1 = *(s + index);
                sampl2 = *(s + (index + 1));

                interp2 = acc & precisionmask;
                interp1 = precisionmask - interp2;

                sampl1 = STDmuls((s16)sampl1, interp1);
                sampl2 = STDmuls((s16)sampl2, interp2);

                sampl = (s8)((sampl1 + sampl2) >> divprecision);

                if (_storagemode == BLS_STORAGE_WORD)
                {
                    *d = sampl < 0 ? 0xFF : 0;
                }
                d++;

                *d++ = sampl;

                acc += freqdiv;
            }
#           endif
        }
        else
        {
#           if blsINITUSEASM
            if (_storagemode == BLS_STORAGE_WORD)
                ablsINWsample(s, d, sampleLen, divprecision, freqdiv);
            else
                ablsINBsample(s, d, sampleLen, divprecision, freqdiv);
            d += sampleLen << 1;
#           else
            for (i = 0 ; i < sampleLen ; i++)
            {
                sampl = *(s + (acc  >> divprecision));

                if (_storagemode == BLS_STORAGE_WORD)
                {
                    *d = sampl < 0 ? 0xFF : 0;
                }
                d++;

                *d++ = sampl;

                acc += freqdiv;
            }
#           endif
        }

        if (_storagemode == BLS_STORAGE_WORD)
        {
            *d = *(d-2);
        }
        d++;

        *d = *(d-2);
    }
}

#if !blsINITUSEASM
static void ablsUndelta(s8* _s1, u32 _len)
{
    s8* s2 = _s1 + 1;
    u32 t;

    for (t = _len - 1 ; t > 0 ; t--)
    {
        *s2++ += *_s1++;
    }
}
#endif

void BLSinit (MEMallocator* _allocator, MEMallocator* _allocatorTemp, BLSsoundTrack* _sndtrack, BLSinitCallback _statCallback)
{
    u16 k, i;
    u32 sampleHeap[2] = {0,0};


    _sndtrack->sampleHeap = (s8*) MEM_ALLOC(_allocator, _sndtrack->sampleHeapSize);

    /* Delta decode */
    for (i = 0; i < _sndtrack->nbSourceSamples; i++)
    {
        u32 len = _sndtrack->sourceSamples[i].sampleLen;
        s8* s1  = _sndtrack->sourceSamples[i].sample;

        if (len > 1)
        {
            if (_statCallback != NULL)
            {
                _statCallback(true, i, _sndtrack->nbSourceSamples);
            }

            ablsUndelta(s1, len);
        }
    }
    
    for (i = 0 ; i < _sndtrack->nbSamples ; i++)
    {
        u16 k = _sndtrack->sampleAllocOrder[i] & BLS_STORAGE_ORDER_MASK;
        BLSprecomputedKey* key = &_sndtrack->keys[k];       

        BLSinitSample(_sndtrack->sampleHeap, sampleHeap, &_sndtrack->sourceSamples[key->sampleIndex], &_sndtrack->samples[i], key, _sndtrack->sampleAllocOrder[i] & ~BLS_STORAGE_ORDER_MASK);
        key->sampleIndex = (u8) i;

        if (_statCallback != NULL)
        {
            _statCallback(false, i, _sndtrack->nbSamples);
        }

        ASSERT(sampleHeap[0] <= _sndtrack->sampleHeapSize);
        ASSERT(sampleHeap[1] <= _sndtrack->sampleHeapSize);
    }

    for (k = 0 ; k < _sndtrack->nbSourceSamples ; k++)
    {
        MEM_FREE(_allocatorTemp, _sndtrack->sourceSamples[k].sample);
    }

    MEM_FREE(_allocatorTemp, _sndtrack->sourceSamples);
    _sndtrack->sourceSamples = NULL;
    _sndtrack->nbSourceSamples = 0;

    MEM_FREE(_allocatorTemp, _sndtrack->sampleAllocOrder);
    _sndtrack->sampleAllocOrder = NULL;
}


void BLZfree (MEMallocator* _allocator, BLSsoundTrack* _sndtrack)
{
    if (_sndtrack->sampleHeap != NULL)
    {
        MEM_FREE(_allocator, _sndtrack->sampleHeap);
        _sndtrack->sampleHeap = NULL;
    }

    MEM_FREE(_allocator, _sndtrack->samples);
    _sndtrack->samples = NULL;
    MEM_FREE(_allocator, _sndtrack->keys);
    _sndtrack->keys = NULL;
    MEM_FREE(_allocator, _sndtrack->keysnoteinfo);
    _sndtrack->keysnoteinfo = NULL;

    if (_sndtrack->framesdata != NULL)
    {
        MEM_FREE(_allocator, _sndtrack->framesdata);
        _sndtrack->framesdata = NULL;
    }

    if (_sndtrack->patternsdata != NULL)
    {
        MEM_FREE(_allocator, _sndtrack->patternsdata);
        _sndtrack->patternsdata = NULL;
    }

    if (_sndtrack->patternslength != NULL)
    {
        MEM_FREE(_allocator, _sndtrack->patternslength);
        _sndtrack->patternslength = NULL;
    }

    if (_sndtrack->track != NULL)
    {
        MEM_FREE(_allocator, _sndtrack->track);
        _sndtrack->track = NULL;
    }

    MEM_FREE(_allocator, _sndtrack);
}


void BLZplayerFree (MEMallocator* _allocator, BLSplayer* _player)
{
    MEM_FREE(_allocator, _player->buffer);
    _player->buffer = NULL;

    BLZresetYM ();
}

static char* blsReadBlitzTrack(MEMallocator * _allocator, char* p, BLSsoundTrack* sndtrack)
{
    u32 framesdatalen = 0;
    u16 t;


    STD_READ_L(p, framesdatalen);
    sndtrack->framesdata = (u8*) MEM_ALLOC(_allocator, framesdatalen);
    ASSERT(sndtrack->framesdata != NULL);
    STDmcpy (sndtrack->framesdata, p, framesdatalen);
    p += framesdatalen;

    STD_READ_B(p, sndtrack->trackLen);
    sndtrack->track = (u8*) MEM_ALLOC(_allocator, sndtrack->trackLen);
    ASSERT(sndtrack->track != NULL);
    STDmcpy(sndtrack->track, p, sndtrack->trackLen);
    p += sndtrack->trackLen;

    STD_READ_B(p, sndtrack->nbPatterns);
    sndtrack->patternsdata = (u32*) MEM_ALLOC(_allocator, sndtrack->nbPatterns * sizeof(u32));
    ASSERT(sndtrack->patternsdata != NULL);

    for (t = 0; t < sndtrack->nbPatterns; t++)
    {
        STD_READ_L(p, sndtrack->patternsdata[t]);
    }

    sndtrack->patternslength = (u16*) MEM_ALLOC(_allocator, sndtrack->nbPatterns * sizeof(u16));
    ASSERT(sndtrack->patternslength != NULL);

    for (t = 0; t < sndtrack->nbPatterns; t++)
    {
        STD_READ_W(p, sndtrack->patternslength[t]);
    }

    return p;
}

void* blsReadSounds(MEMallocator* _allocator, MEMallocator* _allocatorTemp, u8* p, BLSsoundTrack* sndtrack)
{
    u16 formatrevision, i;


    /* Format rev */
    STD_READ_W(p,formatrevision);
    ASSERT(BLS_FORMAT_REVISION == formatrevision);

    /* Source samples */
    STD_READ_B(p,sndtrack->nbSourceSamples);
    sndtrack->sourceSamples = (BLSsample*) MEM_ALLOCTEMP(_allocatorTemp, sizeof(BLSsample) * sndtrack->nbSourceSamples);
    ASSERT(sndtrack->sourceSamples!= NULL);

    {
        BLSsample* s = sndtrack->sourceSamples;

        for (i = 0 ; i < sndtrack->nbSourceSamples ; i++, s++)
        {
            STD_READ_L(p,s->sampleLen); 
            STD_READ_L(p,s->sampleLoopStart);
            STD_READ_W(p,s->sampleLoopLength);
            STD_READ_W(p,s->flags);

            s->sample = (s8*) MEM_ALLOCTEMP(_allocatorTemp, s->sampleLen + 1UL);
            ASSERT(s->sample != NULL);

            STDmcpy(s->sample, p, s->sampleLen); 
            s->sample[s->sampleLen] = s->sample[s->sampleLen - 1];  /* duplicate last byte to simplify interpolate */
            p += s->sampleLen;
        }
    }

    /* Nb precomputed samples */
    STD_READ_W(p,sndtrack->nbSamples);
    sndtrack->samples = (BLSsample*) MEM_ALLOC(_allocator, sndtrack->nbSamples * sizeof(BLSsample));
    ASSERT(sndtrack->samples != NULL);

    /* Load keys */
    STD_READ_W(p,sndtrack->nbKeys);
    sndtrack->keys = (BLSprecomputedKey*) MEM_ALLOC(_allocator, sndtrack->nbKeys * sizeof(BLSprecomputedKey));
    ASSERT(sndtrack->keys != NULL);

    for (i = 0 ; i < sndtrack->nbKeys ; i++)
    {
        BLSprecomputedKey* k = &(sndtrack->keys[i]);

        STD_READ_B(p,k->blitterTranspose);
        STD_READ_B(p,k->sampleIndex);
        STD_READ_W(p,k->freqmul);
        STD_READ_W(p,k->freqdiv);
        STD_READ_B(p,k->freqmulshift);
        STD_READ_B(p,k->freqdivshift);
    }

    sndtrack->keysnoteinfo = (u8*) MEM_ALLOC(_allocator, sndtrack->nbKeys);
    STDmcpy(sndtrack->keysnoteinfo, p, sndtrack->nbKeys);
    p += sndtrack->nbKeys;
    ASSERT(sndtrack->keysnoteinfo != NULL);

    /* Load alloc order */
    STD_READ_L(p,sndtrack->sampleHeapSize);
    
    sndtrack->sampleAllocOrder = (u16*) MEM_ALLOCTEMP(_allocatorTemp, sizeof(u16) * sndtrack->nbSamples);
    ASSERT(sndtrack->sampleAllocOrder != NULL);
    STDmcpy (sndtrack->sampleAllocOrder, p, sizeof(u16) * sndtrack->nbSamples);
    p += sndtrack->nbSamples * sizeof(u16);

    for (i = 0 ; i < sndtrack->nbSamples ; i++)
    {
        sndtrack->sampleAllocOrder[i] = PCENDIANSWAP16(sndtrack->sampleAllocOrder[i]);
    }
    
    return p;
}

void* BLZread(MEMallocator* _allocator, MEMallocator* _allocatorTemp, void* _buffer, BLSsoundTrack** _sndtrack)
{
    BLSsoundTrack* sndtrack;
    u16 formatid;
    u8* p = (u8*) _buffer;

    sndtrack = MEM_ALLOC_STRUCT(_allocator, BLSsoundTrack);
    ASSERT(sndtrack != NULL);
    DEFAULT_CONSTRUCT(sndtrack);

    /* BLS format */
    STD_READ_W(p,formatid);
    ASSERT(BLS_FORMAT_BLITZ == formatid);
    
    p = blsReadSounds    (_allocator, _allocatorTemp, p, sndtrack);
    p = blsReadBlitzTrack(_allocator, p, sndtrack);

    *_sndtrack = sndtrack;

    return p;
}

#ifdef DEMOS_LOAD_FROMHD

void BLSdumpHeap (BLSsoundTrack* _sndtrack, char* _filename)
{
    FILE* file = fopen (_filename, "wb");    
    fwrite (_sndtrack->sampleHeap, _sndtrack->sampleHeapSize, 1, file);
    fclose (file);
}

#endif /* DEMOS_LOAD_FROMHD */

