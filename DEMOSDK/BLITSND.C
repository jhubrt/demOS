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


STATIC_ASSERT(sizeof(BLSvoice)          == 44);
STATIC_ASSERT(sizeof(BLSsoundTrack)     == 40);
STATIC_ASSERT(sizeof(BLSsample)         == 16);
STATIC_ASSERT(sizeof(BLSprecomputedKey) == 8);
STATIC_ASSERT(sizeof(BLScell)           == 4);
STATIC_ASSERT(sizeof(BLSrow)            == (sizeof(BLScell) * BLS_NBVOICES));

#ifdef __TOS__
STATIC_ASSERT(sizeof(BLSplayer)         == 218);
#else
STATIC_ASSERT(sizeof(BLSplayer)         == 220);
#endif


#if blsUSEASM
#   define blsINITUSEASM 1

#   if blsINITUSEASM
    void ablsIIWsample(void* sourcesample_sample, void* _dest, u32 samplelen, u32 divprecision_precisionmask, u32 freqdiv);
    void ablsIIBsample(void* sourcesample_sample, void* _dest, u32 samplelen, u32 divprecision_precisionmask, u32 freqdiv);
    void ablsINWsample(void* sourcesample_sample, void* _dest, u32 samplelen, u16 divprecision, u32 freqdiv);
    void ablsINBsample(void* sourcesample_sample, void* _dest, u32 samplelen, u16 divprecision, u32 freqdiv);
#   endif
#endif


void BLSinitSample (s8* _sampleBuffer, u32* _sampleHeap, BLSsample* _sourceSample, BLSsample* _transposedSample, BLSprecomputedKey* _key, u16 _storagemode)
{
    u32 size;
    s8* d = NULL;
    u32 roundvalue = 1UL << (_key->freqmulshift - 1);

    ASSERT(_sourceSample->sampleLen > 0);

    _transposedSample->sampleLen        =      ((STDmulu((u16)_sourceSample->sampleLen       , _key->freqmul) + roundvalue) >> _key->freqmulshift);
    _transposedSample->sampleLoopLength = (u16) (STDmulu((u16)_sourceSample->sampleLoopLength, _key->freqmul)               >> _key->freqmulshift);
    _transposedSample->sampleLoopStart  =       (STDmulu((u16)_sourceSample->sampleLoopStart , _key->freqmul)               >> _key->freqmulshift);
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

void BLSinit (MEMallocator* _allocator, BLSsoundTrack* _sndtrack, BLSinitCallback _statCallback)
{
    u16 k, i;
    u32 sampleHeap[2] = {0,0};


    _sndtrack->sampleHeap = (s8*) MEM_ALLOC(_allocator, _sndtrack->sampleHeapSize);

    for (i = 0 ; i < _sndtrack->nbSamples ; i++)
    {
        u16 k = _sndtrack->sampleAllocOrder[i] & BLS_STORAGE_ORDER_MASK;
        BLSprecomputedKey* key = &_sndtrack->keys[k];       

        BLSinitSample(_sndtrack->sampleHeap, sampleHeap, &_sndtrack->sourceSamples[key->sampleIndex], &_sndtrack->samples[i], key, _sndtrack->sampleAllocOrder[i] & ~BLS_STORAGE_ORDER_MASK);
        key->sampleIndex = (u8) i;

        ASSERT(sampleHeap[0] <= _sndtrack->sampleHeapSize);
        ASSERT(sampleHeap[1] <= _sndtrack->sampleHeapSize);

        if (_statCallback != NULL)
        {
            _statCallback(i, _sndtrack->nbSamples);
        }
    }

    for (k = 0 ; k < _sndtrack->nbSourceSamples ; k++)
    {
        MEM_FREE(_allocator, _sndtrack->sourceSamples[k].sample);
    }

    MEM_FREE(_allocator, _sndtrack->sourceSamples);
    _sndtrack->sourceSamples = NULL;
    _sndtrack->nbSourceSamples = 0;
    MEM_FREE(_allocator, _sndtrack->sampleAllocOrder);
    _sndtrack->sampleAllocOrder = NULL;
}

#if blsDUMP
void BLSdumpHeap (BLSsoundTrack* _sndtrack, char* _filename)
{
    FILE* file = fopen (_filename, "wb");    
    fwrite (_sndtrack->sampleHeap, _sndtrack->sampleHeapSize, 1, file);
    fclose (file);
}
#endif

void BLSfree (MEMallocator* _allocator, BLSsoundTrack* _sndtrack)
{
    MEM_FREE(_allocator, _sndtrack->sampleHeap);
    MEM_FREE(_allocator, _sndtrack->samples);
    MEM_FREE(_allocator, _sndtrack->keys);
    MEM_FREE(_allocator, _sndtrack->patterns);
    MEM_FREE(_allocator, _sndtrack->track);
    MEM_FREE(_allocator, _sndtrack);
}

void BLSplayerFree (MEMallocator* _allocator, BLSplayer* _player)
{
    MEM_FREE(_allocator, _player->buffer);
    _player->buffer = NULL;
}


BLSsoundTrack* BLSread(MEMallocator* _allocator, void* _buffer)
{
    BLSsoundTrack* sndtrack;
    u16 formatid, formatrevision, i;
    u8* p = (u8*) _buffer;

    sndtrack = MEM_ALLOC_STRUCT(_allocator, BLSsoundTrack);
    DEFAULT_CONSTRUCT(sndtrack);

    /* BLS format */
    STD_READ_W(p,formatid);
    ASSERT(BLS_FORMAT == formatid);

    STD_READ_W(p,formatrevision);
    ASSERT(BLS_FORMAT_REVISION == formatrevision);

    /* Source samples */
    STD_READ_B(p,sndtrack->nbSourceSamples);
    sndtrack->sourceSamples = (BLSsample*) MEM_ALLOCTEMP(_allocator, sizeof(BLSsample) * sndtrack->nbSourceSamples);

    {
        BLSsample* s = sndtrack->sourceSamples;

        for (i = 0 ; i < sndtrack->nbSourceSamples ; i++, s++)
        {
            STD_READ_L(p,s->sampleLen); 
            STD_READ_L(p,s->sampleLoopStart);
            STD_READ_W(p,s->sampleLoopLength);
            STD_READ_W(p,s->flags);

            s->sample = (s8*) MEM_ALLOCTEMP(_allocator, s->sampleLen + 1UL);
            STDmcpy(s->sample, p, s->sampleLen); 
            s->sample[s->sampleLen] = s->sample[s->sampleLen - 1];  /* duplicate last byte to simplify interpolate */
            p += s->sampleLen;
        }
    }

    /* Nb precomputed samples */
    STD_READ_W(p,sndtrack->nbSamples);
    sndtrack->samples = (BLSsample*) MEM_ALLOC(_allocator, sndtrack->nbSamples * sizeof(BLSsample));
    
    /* Load keys */
    STD_READ_W(p,sndtrack->nbKeys);
    sndtrack->keys   = (BLSprecomputedKey*) MEM_ALLOC(_allocator, sndtrack->nbKeys * sizeof(BLSprecomputedKey));

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

    /* Load alloc order */
    STD_READ_L(p,sndtrack->sampleHeapSize);
    
    sndtrack->sampleAllocOrder = (u16*) MEM_ALLOCTEMP(_allocator, sizeof(u16) * sndtrack->nbSamples);
    STDmcpy (sndtrack->sampleAllocOrder, p, sizeof(u16) * sndtrack->nbSamples);
    p += sndtrack->nbSamples * sizeof(u16);

    for (i = 0 ; i < sndtrack->nbSamples ; i++)
    {
        sndtrack->sampleAllocOrder[i] = PCENDIANSWAP16(sndtrack->sampleAllocOrder[i]);
    }
    
    /* Read patterns */
    STD_READ_B(p,sndtrack->nbPatterns);    
    sndtrack->patterns = (BLSpattern*) MEM_ALLOC(_allocator, sizeof(BLSpattern) * sndtrack->nbPatterns);
    STDmcpy(sndtrack->patterns, p, sizeof(BLSpattern) * sndtrack->nbPatterns);
    p += sizeof(BLSpattern) * sndtrack->nbPatterns;

    /* Read song */
    STD_READ_B(p,sndtrack->trackLen);
    sndtrack->track = (u8*) MEM_ALLOC(_allocator, sndtrack->trackLen);
    STDmcpy(sndtrack->track, p, sndtrack->trackLen);
    p += sndtrack->trackLen;

    return sndtrack;
}


#ifdef DEMOS_LOAD_FROMHD
BLSsoundTrack* BLSload(MEMallocator* _allocator, char* _filename)
{
    BLSsoundTrack* sndtrack;
    void* buffer;
    u32 size, readbytes;
    FILE* file = fopen(_filename, "rb");
    ASSERT(file != NULL);
    
    fseek (file, 0, SEEK_END);
    size = ftell(file);
    fseek (file, 0, SEEK_SET);

    buffer = MEM_ALLOCTEMP (_allocator, size);

    readbytes = fread (buffer, 1, size, file);
    ASSERT(size == readbytes);

    fclose(file);

    sndtrack = BLSread(_allocator, buffer);

    MEM_FREE(_allocator, buffer);

    return sndtrack;
}

void BLSwrite(BLSsoundTrack* _sndtrack, char* _filename)
{
    u32 i;
    FILE* file = fopen(_filename, "wb");
    ASSERT(file != NULL);
    

    { /* BLS format */ 
        u16 formatid        = PCENDIANSWAP16(BLS_FORMAT);
        u16 formatrevision  = PCENDIANSWAP16(BLS_FORMAT_REVISION);

        fwrite(&formatid      , sizeof(formatid)      , 1, file);
        fwrite(&formatrevision, sizeof(formatrevision), 1, file);
    }
    
    /* write source samples */
    fwrite(&_sndtrack->nbSourceSamples, sizeof(_sndtrack->nbSourceSamples), 1, file);

    for (i = 0 ; i < _sndtrack->nbSourceSamples ; i++)
    {
        BLSsample* s = &(_sndtrack->sourceSamples[i]);
        {   
            u32 sampleLen = PCENDIANSWAP32(s->sampleLen);
            fwrite(&sampleLen, sizeof(sampleLen), 1, file); 
        }
        {
            u32 sampleLoopStart = PCENDIANSWAP32(s->sampleLoopStart);
            fwrite(&sampleLoopStart, sizeof(sampleLoopStart), 1, file);
        }
        {
            u16 sampleLoopLength = PCENDIANSWAP16(s->sampleLoopLength);
            fwrite(&sampleLoopLength, sizeof(sampleLoopLength), 1, file);
        }
        {   
            u16 flags = PCENDIANSWAP16(s->flags);
            fwrite(&flags, sizeof(flags), 1, file); 
        }
        fwrite(s->sample, s->sampleLen, 1, file);
    }

    { /* write nb precomputed samples */
        u16 nbSamples = PCENDIANSWAP16(_sndtrack->nbSamples);
        fwrite(&nbSamples, sizeof(nbSamples), 1, file);
    }
    
    { /* write keys */
        u16 nbKeys = PCENDIANSWAP16(_sndtrack->nbKeys);
        fwrite(&nbKeys, sizeof(nbKeys), 1, file);
    }

    for (i = 0 ; i < _sndtrack->nbKeys ; i++)
    {
        BLSprecomputedKey* k = &(_sndtrack->keys[i]);

        fwrite(&k->blitterTranspose, sizeof(u8), 1, file);
        fwrite(&k->sampleIndex     , sizeof(u8), 1, file);
        {
            u16 freqmul = PCENDIANSWAP16(k->freqmul);
            u16 freqdiv = PCENDIANSWAP16(k->freqdiv);
            fwrite(&freqmul,        sizeof(freqmul), 1, file);
            fwrite(&freqdiv,        sizeof(freqdiv), 1, file);
            fwrite(&k->freqmulshift,sizeof(k->freqmulshift), 1, file);
            fwrite(&k->freqdivshift,sizeof(k->freqdivshift), 1, file);
        }
    }

    {
        u32 heapsize = PCENDIANSWAP32(_sndtrack->sampleHeapSize);
        fwrite(&heapsize, sizeof(heapsize), 1, file);
    }

    for (i = 0 ; i < _sndtrack->nbSamples ; i++)
    {
        u16 sampleAllocOrder = PCENDIANSWAP16(_sndtrack->sampleAllocOrder[i]);
        fwrite(&sampleAllocOrder, sizeof(sampleAllocOrder), 1, file);
    }

    /* write patterns */
    fwrite(&_sndtrack->nbPatterns, sizeof(_sndtrack->nbPatterns), 1, file);
    fwrite(_sndtrack->patterns, sizeof(BLSpattern) * _sndtrack->nbPatterns, 1, file);
    
    /* write song */
    fwrite(&(_sndtrack->trackLen), sizeof(_sndtrack->trackLen), 1, file);
    fwrite(_sndtrack->track, _sndtrack->trackLen, 1, file);

    fclose(file);
}

#endif
