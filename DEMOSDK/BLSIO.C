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
#include "DEMOSDK\BLSSND.H"

#if BLS_SCOREMODE_ENABLE

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\HARDWARE.H"


void* blsReadSounds(MEMallocator* _allocator, MEMallocator* _allocatorTemp, u8* p, BLSsoundTrack* sndtrack);


static char* blsReadScore(MEMallocator * _allocator, char* p, BLSsoundTrack* sndtrack)
{
    /* Read patterns */
    STD_READ_B(p, sndtrack->nbPatterns);
    sndtrack->patterns = (BLSpattern*)MEM_ALLOC(_allocator, sizeof(BLSpattern) * sndtrack->nbPatterns);
    ASSERT(sndtrack->patterns != NULL);

    {
        u16 t;
        BLSpattern* pattern = sndtrack->patterns;
        for (t = 0; t < sndtrack->nbPatterns; t++, pattern++)
        {
            STD_READ_B(p, pattern->nbrows);
            {
                u16 size = sizeof(u16) * (pattern->nbrows + 1);

                pattern->rowtocellindex = (u16*)MEM_ALLOC(_allocator, size);
                STDmcpy(pattern->rowtocellindex, p, size);
                p += size;

#				ifndef __TOS__
                {
                    u16 i;
                    for (i = 0; i <= pattern->nbrows; i++)
                    {
                        u16 v = pattern->rowtocellindex[i];
                        pattern->rowtocellindex[i] = PCENDIANSWAP16(v);
                    }
                }
#				endif

                size = pattern->rowtocellindex[pattern->nbrows] * sizeof(BLScell);

                if (size > 0)
                {
                    pattern->cells = (BLScell*)MEM_ALLOC(_allocator, size);
                    STDmcpy(pattern->cells, p, size);
                    p += size;
                }
                else
                {
                    pattern->cells = NULL;
                }
            }
        }
    }

    /* Read song */
    STD_READ_B(p, sndtrack->trackLen);
    sndtrack->track = (u8*)MEM_ALLOC(_allocator, sndtrack->trackLen);
    ASSERT(sndtrack->track != NULL);

    STDmcpy(sndtrack->track, p, sndtrack->trackLen);
    p += sndtrack->trackLen;

    return p;
}

void* BLSread(MEMallocator* _allocator, MEMallocator* _allocatorTemp, void* _buffer, BLSsoundTrack** _sndtrack)
{
    BLSsoundTrack* sndtrack;
    u16 formatid;
    u8* p = (u8*) _buffer;

    sndtrack = MEM_ALLOC_STRUCT(_allocator, BLSsoundTrack);
    ASSERT(sndtrack != NULL);
    DEFAULT_CONSTRUCT(sndtrack);

    /* BLS format */
    STD_READ_W(p,formatid);
    ASSERT(BLS_FORMAT == formatid);

    p = blsReadSounds   (_allocator, _allocatorTemp, p, sndtrack);
    p = SNDYMreadSounds (_allocator, p, &sndtrack->YMsoundSet);
    p = blsReadScore    (_allocator, p, sndtrack);

    *_sndtrack = sndtrack;

    return p;
}


void BLSfree (MEMallocator* _allocator, BLSsoundTrack* _sndtrack)
{
    BLZfree(_allocator, _sndtrack);

    SNDYMfreeSounds(_allocator, &_sndtrack->YMsoundSet);

    if (_sndtrack->patterns != NULL)
    {
        u16 t;
        BLSpattern* p = _sndtrack->patterns;

        for (t = 0; t < _sndtrack->nbPatterns; t++, p++)
        {
            MEM_FREE(_allocator, p->rowtocellindex);

            if (p->cells != NULL)
            {
                MEM_FREE(_allocator, p->cells);
            }
        }

        MEM_FREE(_allocator, _sndtrack->patterns);
    }
}


void BLSplayerFree (MEMallocator* _allocator, BLSplayer* _player)
{
    BLZplayerFree(_allocator, _player);

    SNDYMfreePlayer(_allocator, &_player->ymplayer);
}


#ifdef DEMOS_LOAD_FROMHD
void BLSwriteHeader(bool _blitzMode, FILE* file)
{
    /* BLS format */ 
    STDwriteW(file, _blitzMode ? BLS_FORMAT_BLITZ : BLS_FORMAT);
    STDwriteW(file, BLS_FORMAT_REVISION);
}


void BLSwriteSounds(BLSsoundTrack* _sndtrack, FILE* file)
{
    u16 i;
   
   
    /* write source samples */
    STDwriteB(file, _sndtrack->nbSourceSamples);

    for (i = 0 ; i < _sndtrack->nbSourceSamples ; i++)
    {
        BLSsample* s = &(_sndtrack->sourceSamples[i]);
        u32 written;

        STDwriteL(file, s->sampleLen);
        STDwriteL(file, s->sampleLoopStart);
        STDwriteW(file, s->sampleLoopLength);
        STDwriteW(file, s->flags);
        
        written = fwrite(s->sample, s->sampleLen, 1, file);
        ASSERT(written == 1);
    }

    /* write nb precomputed samples */
    STDwriteW (file, _sndtrack->nbSamples);
    
    /* write keys */
    STDwriteW (file, _sndtrack->nbKeys);

    for (i = 0 ; i < _sndtrack->nbKeys ; i++)
    {
        BLSprecomputedKey* k = &(_sndtrack->keys[i]);

        STDwriteB(file, k->blitterTranspose);
        STDwriteB(file, k->sampleIndex);
        {
            STDwriteW(file, k->freqmul);
            STDwriteW(file, k->freqdiv);
            STDwriteB(file, k->freqmulshift);
            STDwriteB(file, k->freqdivshift);
        }
    }

    STDwrite(file, _sndtrack->keysnoteinfo, _sndtrack->nbKeys);

    STDwriteL(file, _sndtrack->sampleHeapSize);

    for (i = 0 ; i < _sndtrack->nbSamples ; i++)
    {
        STDwriteW(file, _sndtrack->sampleAllocOrder[i]);
    }
}

void BLSwriteScore(BLSsoundTrack* _sndtrack, FILE* file)
{
    u16 i;    /* write patterns */
    STDwriteB (file, _sndtrack->nbPatterns);
   
	{
		BLSpattern* pattern = _sndtrack->patterns;
		for (i = 0; i < _sndtrack->nbPatterns; i++, pattern++)
		{
            u16 t;
            u16 nbcells;

            STDwriteB (file, pattern->nbrows);

            for (t = 0 ; t <= pattern->nbrows ; t++)
            {
                STDwriteW (file, pattern->rowtocellindex[t]);
            }

            nbcells = pattern->rowtocellindex[pattern->nbrows];

            if (nbcells > 0)
            {
                fwrite (pattern->cells, sizeof(BLScell), nbcells, file);
            }
		}
	}

    /* write song */
    STDwriteB (file, _sndtrack->trackLen);    
    fwrite(_sndtrack->track, _sndtrack->trackLen, 1, file);
}

void BLSsave(BLSsoundTrack* _sndtrack, char* _filename)
{
    FILE* file = fopen (_filename, "wb");
    ASSERT(file != NULL);

    BLSwriteHeader(false    , file);
    BLSwriteSounds(_sndtrack, file);
    SNDYMwriteSounds (&_sndtrack->YMsoundSet, file);
    BLSwriteScore (_sndtrack, file);

    fclose(file);
}

#endif /* DEMOS_LOAD_FROMHD */

#endif /* BLS_SCOREMODE_ENABLE */
