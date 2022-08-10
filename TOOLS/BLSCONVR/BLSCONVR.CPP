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

extern "C"
{
#   include "DEMOSDK\BASTYPES.H"
#   include "DEMOSDK\STANDARD.H"
#   include "DEMOSDK\BLSSND.H"
#   include "DEMOSDK\HARDWARE.H"
#   include "DEMOSDK\BLSZDUMP.H"
}

#include <string.h>
#include <stdio.h>
#include <cctype>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>

#include "TOOLS\BINARIZE\HELPERS.H"

#include "TOOLS\BLSCONVR\BLSCONVR.H"

// allocator
static void* stdAlloc(void* _alloc, u32 _size)
{
    IGNORE_PARAM(_alloc);
    return malloc(_size);
}

static void stdFree(void* _alloc, void* _adr)
{
    IGNORE_PARAM(_alloc);
    free(_adr);
}

MEMallocator stdAllocator = { NULL, stdAlloc, stdAlloc, stdFree };

const char* Key::m_names[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

const Key NOTRANSPOSE_KEY(250, 0);


u8 g_AmplitudeToLMC[] =
{
    0,
    2 ,5 ,7 ,8 ,9 ,10,10,11,11,12,12,13,13,13,14,14,
    14,14,15,15,15,15,16,16,16,16,16,16,17,17,17,17,
    17,17,17,18,18,18,18,18,18,18,18,18,18,19,19,19,
    19,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20
};

const float AMIGA_NTSC_C2_FREQ = 8363.4229f;
const float AMIGA_PAL_C2_FREQ  = 8287.1369f;

FreqRef::FreqRef() : m_amigaFreq(AMIGA_NTSC_C2_FREQ) {}

void FreqRef::Init(bool _palfreq)
{
    m_amigaFreq = _palfreq ? AMIGA_PAL_C2_FREQ : AMIGA_NTSC_C2_FREQ;
}

float FreqRef::GetAmigaSampleFreq(const Key& _key, s8 _fineTune)
{
    Key reference (2, 0);

    const float deltasemitones = (float)_key.GetDistance(reference) + (float)_fineTune / 128.0f;

    const float amigaSampleFreq  = m_amigaFreq * powf(2.0f, (1.0f/12.0f) * deltasemitones);

    return amigaSampleFreq; 
}

FreqRef g_freqRef;

void Sndtrack::PrintSamplesTable()
{
    printf("Title = %20s\n\n", m_title);

    printf("    %22s |%8s|%8s|%8s|%8s|%8s|%8s", "Sample name", "Length", "RelativK", "FineTune", "Volume", "LoopStrt", "LoopLen\n\n");

    for (u32 t = 0; t < m_samples.size(); t++)
    {
        const auto& s = m_samples[t];

        printf("%2u: ", t + 1);
        printf("%22s | %6u | %6d | %6d | %6u | %6u | %6u\n", s.m_sampleName, s.m_sampleLen, s.m_relativeNoteNum + 24, s.m_fineTune, s.m_sampleVolume, s.m_sampleLoopStart, s.m_sampleLoopLength);
    }
}


void Sndtrack::AddKeyToInstrument(u8 _instrument, const Key& _key)
{
    auto it2 = m_instrumentUsedKeys.find(_instrument);
    if (it2 == m_instrumentUsedKeys.end())
    {
        m_instrumentUsedKeys[_instrument].insert(_key);
    }
    else
    {
        it2->second.insert(_key);
    }
}


int Sndtrack::RetrievePattern(u32 _patternNum, u32 _startIndex)
{
    for (u32 t = _startIndex ; t < m_songLen ; t++)
    {
        if (m_patternsSequence[t] == _patternNum)
        {
            return t;
        }
    }

    return -1;
}


void Sndtrack::DetermineInstrumentsFromScore()
{
    for (u32 p = 0; p < m_patterns.size(); p++)
    {
        u8  lastinstrument[NBMAXVOICES] = { 0,0,0,0,0,0,0,0 };
        Key lastkey[NBMAXVOICES];

        Pattern& pattern = m_patterns[p];

        printf("\n\nPattern 0x%x|%d (%d)\n", p, p, pattern.m_nbRows);

        if (RetrievePattern(p) < 0)
        {
            printf ("pattern unused\n");
        }
        else
        {
            for (u32 r = 0; r < pattern.m_nbRows; r++)
            {
                printf("\n%3d:", r);

                for (u32 v = 0; v < NBMODVOICES; v++)
                {
                    Pattern::Cell& cell = pattern.m_cells[r][v];
                    bool needToRegisterVolume = false;
                    u8 currentVolume = 64;


                    if (cell.m_key.isDefined())
                    {
                        if (cell.m_instrument == 0)
                        {
                            ASSERT((lastinstrument[v] > 0) || cell.m_key.isKeyOff());
                            cell.m_instrument = lastinstrument[v];
                        }

                        if (cell.m_instrument > m_samples.size()) // Manage invalid cells
                        {
                            BLS_CONVERT_OUTPUT_POSITION("\n\nERROR", p, r, v);
                            BLS_CONVERT_OUTPUT("use invalid instrument (0x%x|%d)\n\n", cell.m_instrument, cell.m_instrument);
                        }
                        else if (cell.m_instrument > 0)
                        {
                            const Sample& sample = m_samples[cell.m_instrument - 1];

                            if ((sample.m_sampleLen == 0) || (cell.m_key.isKeyOff())) // Manage "silence" sample (mod) of key off (xm)
                            {
                                lastkey[v].undefine();
                                lastinstrument[v] = 0;
                            }
                            else
                            {
                                cell.m_key.Transpose(sample.m_relativeNoteNum);

                                needToRegisterVolume = true;

                                if (sample.isNoTranposeRequired() || sample.isFixedLengthRequired())
                                {
                                    cell.m_key = NOTRANSPOSE_KEY;
                                }

                                AddKeyToInstrument(cell.m_instrument, cell.m_key);

                                lastkey[v] = cell.m_key;
                                lastinstrument[v] = cell.m_instrument;
                            }
                        }
                    }

                    switch (cell.m_fx_h)
                    {
                    case 0xC:   // volume 
                        if (lastinstrument[v] != 0)
                        {
                            needToRegisterVolume = true;

                            // When use volume for balance => odd value for balance control
                            if ((m_useVolumeForBalance == false) || ((cell.m_fx_l & 1) == 0))
                            {
                                u8 vol = (cell.m_fx_m << 4) | cell.m_fx_l;

                                if (vol > 64)
                                {
                                    BLS_CONVERT_OUTPUT_POSITION("\n\nERROR", p, r, v);
                                    BLS_CONVERT_OUTPUT("volume exceed 64 (0x%x|%d)\n\n", vol, vol);
                                    vol = 64;
                                }
                                currentVolume = vol;
                            }
                        }
                        break;

                    case 0:     // arpeggio
                        if ((cell.m_fx_l != 0) || (cell.m_fx_m != 0))
                        {
                            if (lastkey[v].isDefined())
                            {
                                lastkey[v].m_supportArpeggio = true;

                                Key key1 = lastkey[v];
                                Key key2 = lastkey[v];

                                key1.Transpose(cell.m_fx_m);
                                key2.Transpose(cell.m_fx_l);

                                key1.m_supportArpeggio = key2.m_supportArpeggio = true;

                                AddKeyToInstrument(lastinstrument[v], key1);
                                AddKeyToInstrument(lastinstrument[v], key2);
                            }
                        }
                        break;
                    }

                    if (cell.m_volume > 0)
                    {
                        if (lastinstrument[v] != 0)
                        {
                            needToRegisterVolume = true;
                            currentVolume = cell.m_volume - 0x10;
                        }
                    }

                    cell.Print();

                    if ((v + 1) < m_nbVoices)
                    {
                        printf("|");
                    }

                    if (needToRegisterVolume)
                    {
                        auto it = m_instrFXVolumes.find(lastinstrument[v]);
                        if (it == m_instrFXVolumes.end())
                        {
                            m_instrFXVolumes.insert(std::make_pair(lastinstrument[v], std::map<u8, VolFXLevel>()));
                            it = m_instrFXVolumes.find(lastinstrument[v]);
                        }

                        u8 shift = VolFXLevel::GetShiftFromVolume(currentVolume);
                        VolFXLevel volFXLevel(currentVolume);

                        auto& volumemap = it->second;
                        auto it2 = volumemap.find(shift);
                        if (it2 == volumemap.end())
                        {
                            volumemap.insert(std::make_pair(shift, volFXLevel));
                        }
                        else
                        {
                            it2->second.add(currentVolume);
                        }
                    }
                }

                {
                    u32 max = m_nbVoices < (NBMODVOICES + SND_YM_NB_CHANNELS) ? m_nbVoices : (NBMODVOICES + SND_YM_NB_CHANNELS);

                    for (u32 v = NBMODVOICES; v < max; v++)
                    {
                        Pattern::Cell& cell = pattern.m_cells[r][v];


                        if (cell.m_key.isDefined())
                        {
                            if (cell.m_key.isKeyOff())
                            {
                            }
                            else if (
                                (cell.m_instrument > m_instrToYmSound.size()) ||
                                (m_instrToYmSound[cell.m_instrument - 1] == -1)) // Manage invalid cells
                            {
                                BLS_CONVERT_OUTPUT_POSITION("\n\nERROR", p, r, v);
                                BLS_CONVERT_OUTPUT("use invalid YM instrument (0x%x|%d)\n\n", cell.m_instrument, cell.m_instrument);
                            }
                            else
                            {
                                m_ymSoundUsed[m_instrToYmSound[cell.m_instrument - 1]] = true;
                            }
                        }

                        if (v < 7)
                        {
                            cell.Print();
                        }

                        if (v < 6)
                        {
                            printf("|");
                        }
                    }
                }
            }
        }
    }
}

void Sndtrack::DisplayUsedInstruments()
{
    printf("\n\nInstrument keys used...\n\n");

    for (auto it = m_instrumentUsedKeys.begin(); it != m_instrumentUsedKeys.end(); it++)
    {
        for (auto k = it->second.begin(); k != it->second.end(); k++)
        {
            printf("instr = %-2u - Key %2s %u\n", it->first, Key::m_names[k->m_semitone], k->m_octave);
        }
    }
}

void Sndtrack::DumpSamples()
{
    u32 t = 0;

    for (auto spl = m_samples.begin(); spl != m_samples.end(); spl++)
    {
        auto len = spl->m_sampleLen;

        if (len > 0)
        {
            char temp[64];

            sprintf(temp, "sample_%d.raw", t);

            spl->Dump(temp);
        }

        t++;
    }
}

void Sndtrack::LookForLinkBetweenKeys()
{
    for (auto it = m_instrumentUsedKeys.begin(); it != m_instrumentUsedKeys.end(); it++)
    {
        PrecomputedKey pKey;
        pKey.m_instrument = it->first;

        u8 sampleindex = pKey.m_instrument - 1;

        const auto& keyMap = it->second;

        for (auto ikey = keyMap.begin(); ikey != keyMap.end(); ikey++)
        {
            pKey.m_key = *ikey;
            pKey.m_blitterTranspose = 0;
            pKey.m_baseKeyIdentifier = 0;
            pKey.m_fineTune = m_samples[sampleindex].m_fineTune;

            /* Look if (in the beginning of the map) we found same semitone in a lower octave */
            for (auto ikey2 = keyMap.begin(); ikey2 != ikey; ikey2++)
            {
                if (ikey->m_semitone == ikey2->m_semitone)
                {
                    pKey.m_blitterTranspose = ikey->m_octave - ikey2->m_octave;
                    pKey.m_baseKeyIdentifier = PrecomputedKey::sGetIdentifier(pKey.m_instrument, *ikey2);
                    break;
                }
            }

            u32 keyidentifier = pKey.GetIdentifier();
            m_keyInstr2precomputedKeys[keyidentifier] = m_precomputedKeys.size();
            m_precomputedKeys.push_back(pKey);
        }
    }
}

void Sndtrack::DisplayVolumeFXUsageByInstrument(const std::map<u8, u32>& instrToSize) const
{
    for (auto it = instrToSize.begin(); it != instrToSize.end(); it++)
    {
        u32 nbVolFXLevel = GetNbVolFXLevelForInstrument(it->first);
        auto itv = m_instrFXVolumes.find(it->first);
        ASSERT(itv != m_instrFXVolumes.end());


        printf("instr %-2ld '%-22s' size = %-7u ", it->first, m_samples[it->first - 1].m_sampleName, it->second);

        switch (nbVolFXLevel)
        {
        case 0:
        case 1:
            printf("no volume effect found ");

            if (nbVolFXLevel == 1)
            {
                printf(">%d", itv->second.begin()->first);
            }

            printf("\n");
            break;

        default:
            printf("volume used ");

            for (auto itl = itv->second.begin(); itl != itv->second.end(); itl++)
            {
                printf(">%d [%d](", itl->first, itl->second.m_nbOccurences);

                for (auto itmv = itl->second.m_modVolumes.begin(); itmv != itl->second.m_modVolumes.end(); )
                {
                    printf("0x%x", *itmv);

                    itmv++;

                    if (itmv != itl->second.m_modVolumes.end())
                    {
                        printf(",");
                    }
                }

                printf(")  ");
            }

            printf("\n");
            break;
        }
    }
}

void Sndtrack::ComputeSamplesTransposeTable()
{
    LookForLinkBetweenKeys();

    printf("\n\nInstrument precompute...\n\n");

    m_nbPrecomputedSamples = 0;

    u32 totalSize = 0;

    std::map<u8, u32> instrToSize;
    u8 lastinstru = 0;

    for (auto ipkey = m_precomputedKeys.begin(); ipkey != m_precomputedKeys.end(); ipkey++)
    {
        const Sample& sourceSample = m_samples[ipkey->m_instrument - 1];

        if (lastinstru != ipkey->m_instrument)
        {
            printf("\ninstr %-2u '%-22s' (%6u): ", ipkey->m_instrument, m_samples[ipkey->m_instrument - 1].m_sampleName, sourceSample.m_sampleLen);
            lastinstru = ipkey->m_instrument;
        }

        if (ipkey->IsBaseKey())
        {
            assert(ipkey->m_blitterTranspose == 0);

            u32     nbSTsamples = 0;
            u32     nbSTsamplesLoop = 0;
            u16     freqmullog2 = 0;
            double  freqmul = 0.0;


            if (sourceSample.isNoTranposeRequired())
            {
                nbSTsamples = sourceSample.m_sampleLen;
                nbSTsamplesLoop = sourceSample.m_sampleLoopLength;

                freqmul = 1.0;

                printf("notranspose ");
            }
            else
            {
                if (sourceSample.isFixedLengthRequired())
                {
                    printf("fixed len ");

                    freqmul = (double)sourceSample.m_fixedLen / (double)sourceSample.m_sampleLen;
                }
                else
                {
                    double amigaSampleFreq = g_freqRef.GetAmigaSampleFreq(ipkey->m_key, ipkey->m_fineTune);
                    freqmul = STFREQ / amigaSampleFreq;
                }

                nbSTsamples = (u32)(0.5 + freqmul * (float)sourceSample.m_sampleLen);
                nbSTsamplesLoop = (u32)(0.5 + freqmul * (float)sourceSample.m_sampleLoopLength);

                if ((sourceSample.isInterpolationRequired() == false) && (sourceSample.isLooping() == false))
                {
                    u16 freqmulint = (u16)freqmul;

                    for (freqmullog2 = 14; ((1 << freqmullog2) & freqmulint) == 0; freqmullog2--)
                    {
                        if (freqmullog2 == 0)
                        {
                            break;
                        }
                    }

                    if (freqmullog2 >= 1)
                    {
                        ipkey->m_blitterTranspose -= freqmullog2;

                        u32 identifier = ipkey->GetIdentifier();

                        for (auto iharmonickeys = m_precomputedKeys.begin(); iharmonickeys != m_precomputedKeys.end(); iharmonickeys++)
                        {
                            if (iharmonickeys->m_baseKeyIdentifier == identifier)
                            {
                                iharmonickeys->m_blitterTranspose -= freqmullog2;
                            }
                        }

                        freqmul /= double(1 << freqmullog2);
                    }
                }
            }

            ipkey->m_freqmul = freqmul;

            printf("[%s %u/%u (%u) >%d] ", Key::m_names[ipkey->m_key.m_semitone], ipkey->m_key.m_octave, ipkey->m_key.m_semitone, nbSTsamples >> freqmullog2, ipkey->m_blitterTranspose);

            m_nbPrecomputedSamples++;

            const bool hasVolumeEffect = GetNbVolFXLevelForInstrument(ipkey->m_instrument) > 1;
            ipkey->AssignStorageCharac(nbSTsamples, hasVolumeEffect, freqmullog2);

            auto its = instrToSize.find(ipkey->m_instrument);
            if (its == instrToSize.end())
            {
                instrToSize[ipkey->m_instrument] = ipkey->m_storagesize;
            }
            else
            {
                its->second += ipkey->m_storagesize;
            }

            totalSize += ipkey->m_storagesize;

            nbSTsamplesLoop >>= freqmullog2;
            nbSTsamples >>= freqmullog2;

            if (nbSTsamplesLoop > 16384)
            {
                BLS_CONVERT_OUTPUT("\nERROR: loop length exceeds 16384 samples for key [%u/%u (%lu)] : %d\n", ipkey->m_key.m_octave, ipkey->m_key.m_semitone, nbSTsamples, nbSTsamplesLoop);
            }

            if (ipkey->m_key.m_supportArpeggio)
            {
                if (nbSTsamples > 65535)
                {
                    BLS_CONVERT_OUTPUT("\nERROR: sample length used in arpeggio exceeds 65535 samples for key [%u/%u (%lu)]\n", ipkey->m_key.m_octave, ipkey->m_key.m_semitone, nbSTsamples);
                }
            }
        }
        else
        {
            printf("{%u/%u >%d} ", ipkey->m_key.m_octave, ipkey->m_key.m_semitone, ipkey->m_blitterTranspose);
            ipkey->m_freqmul = 0.0;
        }
    }

    printf("\n\n");

    DisplayVolumeFXUsageByInstrument(instrToSize);

    printf("\ntotal size = %u for %u precomputed samples and %u keys\n", totalSize, m_nbPrecomputedSamples, m_precomputedKeys.size());
}


void Sndtrack::ConvertToBlitSoundtrack_InitBitmasks(BLSsoundTrack* sndtrack)
{
    m_bitmasks[0] = 0xFFFF;
    m_bitmasks[1] = 0xFEFE;
    m_bitmasks[2] = 0xFCFC;
    m_bitmasks[3] = 0xF8F8;
    m_bitmasks[4] = 0xF0F0;
    m_bitmasks[5] = 0xE0E0;
    m_bitmasks[6] = 0xC0C0;
    m_bitmasks[7] = 0x8080;
    m_bitmasks[8] = 0xEEEE;
    m_bitmasks[9] = 0xCCCC;
    m_bitmasks[10] = 0x8888;
    m_bitmasks[11] = 0x7F7F;
    m_bitmasks[12] = 0x3F3F;
    m_bitmasks[13] = 0x1F1F;
    m_bitmasks[14] = 0x0F0F;
    m_bitmasks[15] = 0x0000;

    for (u32 i = 0; i < m_samples.size(); i++)
    {
        const char* name = m_samples[i].m_sampleName;

        const char* p = strstr(name, "#$");

        while (p != NULL)
        {
            char i[2];
            char m[3];
            u32 mask = 0;
            u32 index = 0;

            i[0] = p[2];
            i[1] = 0;

            m[0] = p[3];
            m[1] = p[4];
            m[2] = 0;

            sscanf(i, "%x", &index);
            sscanf(m, "%x", &mask);

            if (index == 0)
            {
                BLS_CONVERT_OUTPUT("ERROR: you cannot customize bitmask 0 (always = 0xFFFF)\n");
            }
            else
            {
                ASSERT(index < ARRAYSIZE(m_bitmasks));
                m_bitmasks[index] = (u16)((mask << 8) | mask);

                printf("found bitmask 0x%x = 0x%x\n", index, m_bitmasks[index]);

                p = strstr(p + 2, "#$");
            }
        }
    }
}

void Sndtrack::ConvertToBlitSoundtrack_SourceSamplesTable(BLSsoundTrack* sndtrack, std::map<u8, u8>& instrumentToSourceSample)
{
    // create source samples table
    u8 ssi = 0;

    for (auto i = m_instrumentUsedKeys.begin(); i != m_instrumentUsedKeys.end(); i++, ssi++)
    {
        bool copysample = true;
        u8 instrumentIndex = i->first;
        u8 shift = 0;
        instrumentToSourceSample[instrumentIndex] = ssi;

        auto ss = &m_samples[instrumentIndex - 1];
        auto sd = &sndtrack->sourceSamples[ssi];

        sd->sampleLen = ss->m_sampleLen;
        sd->sampleLoopLength = ss->m_sampleLoopLength <= 2 ? 0 : ss->m_sampleLoopLength;
        sd->sampleLoopStart = ss->m_sampleLoopStart;
        sd->sample = new s8[sd->sampleLen];
        sd->flags = 0;

        if (ss->m_sampleVolume != 64)
        {
            copysample = false;
        }

        if (GetNbVolFXLevelForInstrument(instrumentIndex) == 1)
        {
            shift = m_instrFXVolumes.find(instrumentIndex)->second.begin()->first;
            ASSERT(shift <= 8);
            if (shift != 0)
                copysample = false;
        }

        switch (ss->m_transposeType)
        {
        case Sample::TransposeType::LINEAR_INTERPOL:
            sd->flags |= BLS_SAMPLE_INTERPOLATE;
            break;

            /*        case Sample::TransposeType::NO_TRANSPOSE:
            sd->flags |= BLS_SAMPLE_NOTRANSPOSE;
            break;*/
        }

        if (copysample)
        {
            memcpy(sd->sample, &ss->m_data[0], sd->sampleLen);
        }
        else
        {
            for (u32 t = 0; t < sd->sampleLen; t++)
            {
                sd->sample[t] = (ss->m_data[t] * ss->m_sampleVolume + 32) >> (6 + shift);
            }
        }

        // Delta storage to help compression
        for (s32 t = sd->sampleLen - 1; t > 0; t--)
        {
            sd->sample[t] -= sd->sample[t - 1];
        }
    }
}

static void computeFloat16(double _value, u16* _num, u8* _shift)
{
    assert(_value >= 0.0);
    u16 intval = (u16)_value;
    s16 b;

    for (b = 15; b >= 0; b--)
    {
        if ((1 << b) & intval)
        {
            break;
        }
    }

    b++;

    *_shift = 16 - b;
    u32 value = (u32)(_value * float(1 << *_shift));
    assert((value & 0xFFFF0000) == 0);
    *_num = (u16)value;
}


void Sndtrack::ConvertToBlitSoundtrack_PrecomputedKeys(BLSsoundTrack* sndtrack, const std::map<u8, u8>& instrumentToSourceSample, SamplesMapOrderBySize& sizeordertopkindex)
{
    for (u32 pi = 0, si = 0; pi < m_precomputedKeys.size(); pi++)
    {
        auto pks = &m_precomputedKeys[pi];
        auto pkd = &sndtrack->keys[pi];
        u8   baseKeyFlag = 0;


        sndtrack->keysnoteinfo[pi] = (u8)(u16) pks->m_key;

        if (pks->IsBaseKey())
        {
            auto ssi = instrumentToSourceSample.find(pks->m_instrument);
            ASSERT(ssi != instrumentToSourceSample.end());

            ASSERT(pks->m_blitterTranspose <= 0);

            /* assign precomputed sample transposition parameter */
            computeFloat16(pks->m_freqmul, &pkd->freqmul, &pkd->freqmulshift);
            computeFloat16(1.0 / pks->m_freqmul, &pkd->freqdiv, &pkd->freqdivshift);

            pkd->sampleIndex = ssi->second;

            u32 size = pks->m_storagesize;
            if (pks->m_supportVolumeFX)
            {
                size += 0x8000000;
            }
            sizeordertopkindex.insert(std::pair<u32, u16>(size, (u16)pi));

            baseKeyFlag = BLS_BASEKEY_FLAG;
        }
        else
        {
            auto it = m_keyInstr2precomputedKeys.find(pks->m_baseKeyIdentifier);
            ASSERT(it != m_keyInstr2precomputedKeys.end());
            ASSERT(m_precomputedKeys[it->second].m_blitterTranspose <= 0);

            pkd->sampleIndex = static_cast<u8>(it->second);

            const auto& pbks = m_precomputedKeys[it->second];

            computeFloat16(pbks.m_freqmul, &pkd->freqmul, &pkd->freqmulshift);
            computeFloat16(1.0 / pbks.m_freqmul, &pkd->freqdiv, &pkd->freqdivshift);
        }

        /* assign key */
        pkd->blitterTranspose = (pks->m_blitterTranspose << 1) | baseKeyFlag;
    }
}


void Sndtrack::ConvertToBlitSoundtrack_SamplesAllocOrder(BLSsoundTrack* sndtrack, const SamplesMapOrderBySize& sizeordertopkindex)
{
    if (m_nbPrecomputedSamples > 0)
    {
        u32 heapsize[2] = { 0, 0 };

        assert(sizeordertopkindex.begin() != sizeordertopkindex.end());

        auto it = sizeordertopkindex.end();
        u16 index = 0;

        do
        {
            it--;

            u16 pkindex = it->second;
            auto& pk = m_precomputedKeys[pkindex];

            if (pk.m_supportVolumeFX)
            {
                heapsize[0] += pk.m_storagesize + 2;
                heapsize[1] += pk.m_storagesize + 2;
                sndtrack->sampleAllocOrder[index++] = pkindex | BLS_STORAGE_WORD;
            }
            else
            {
                if (heapsize[0] > heapsize[1])
                {
                    heapsize[1] += (pk.m_storagesize + 1) << 1;
                    sndtrack->sampleAllocOrder[index++] = pkindex | BLS_STORAGE_BYTE_1;
                }
                else
                {
                    heapsize[0] += (pk.m_storagesize + 2) << 1;
                    sndtrack->sampleAllocOrder[index++] = pkindex | BLS_STORAGE_BYTE_0;
                }
            }
        } 
        while (it != sizeordertopkindex.begin());

        sndtrack->sampleHeapSize = std::max(heapsize[0], heapsize[1]) + 4;
        printf("heapsize %d %d\n", heapsize[0], heapsize[1]);
    }
}


u8 Sndtrack::FindPrecomputedKeyIndex(u8 _instrument, const Key& _key)
{
    u32 identifier = PrecomputedKey::sGetIdentifier(_instrument, _key);

    auto it = m_keyInstr2precomputedKeys.find(identifier);
    ASSERT(it != m_keyInstr2precomputedKeys.end());

    return static_cast<u8>(it->second);
}


static void Sndtrack_SetBalanceValue(const Pattern::Cell& _sc, u16 _value, bool _balanceused[4], u32 _p, u32 _r, u32 _voice, BLScell& _dc)
{
    u16 val;

    _dc.fx = BLSfx_SETBALANCE;

    _balanceused[_voice] = true;

    // 0x8 fx goes from 0 -> 0x80 -> 0xff for left -> middle (l+r) -> right
    switch (_voice)
    {
    case 3:
        if (_balanceused[0])
        {
            BLS_CONVERT_OUTPUT_POSITION("WARNING", _p, _r, _voice);
            BLS_CONVERT_OUTPUT("balance FX used on other voice 0 from same channel\n");
        }
    case 0:
        val = HW_MICROWIRE_VOLUME_LEFT;
        break;

    case 2:
        if (_balanceused[1])
        {
            BLS_CONVERT_OUTPUT_POSITION("WARNING", _p, _r, _voice);
            BLS_CONVERT_OUTPUT("balance FX used on other voice 1 from same channel\n");
        }
    case 1:
        val = HW_MICROWIRE_VOLUME_RIGHT;
        break;
    }

    if (_value < ARRAYSIZE(g_AmplitudeToLMC))
    {
        val |= g_AmplitudeToLMC[_value];
    }
    else
    {
        BLS_CONVERT_OUTPUT_POSITION("ERROR", _p, _r, _voice);
        BLS_CONVERT_OUTPUT("balance volume exceeds %d (0x%x|%d)\n", sizeof(g_AmplitudeToLMC), _value, _value);
        val |= g_AmplitudeToLMC[ARRAYSIZE(g_AmplitudeToLMC) - 1];
    }

    *(u16*)&_dc.value = PCENDIANSWAP16(val);
}





void Sndtrack::ConvertToBlitSoundtrack_ToBLScell(BLSsoundTrack* sndtrack, u32 p, u32 r, u32 v, const Pattern::Cell& sc, BLScell& dc, Key lastkey[NBMAXVOICES], u8 lastinstrument[NBMAXVOICES], bool balanceused[NBMAXVOICES])
{
    u16 ymfxoffset = (v >= NBMODVOICES) ? 0x100 : 0;
    bool unknownfx = false;

    switch (sc.m_fx_h | ymfxoffset)
    {
        //--------------------------------------------------------------
        // ARPEGGIO
        //--------------------------------------------------------------
    case 0x100:
        if ((sc.m_fx_m != 0) || (sc.m_fx_l != 0))
        {
            BLS_CONVERT_OUTPUT_POSITION("WARNING", p, r, v);
            BLS_CONVERT_OUTPUT("arpeggio ignored on ym channel : define arpeggio into the ym sound\n");
        }
        break;

    case 0:
        if ((sc.m_fx_l != 0) || (sc.m_fx_m != 0))
        {
            if ((lastkey[v].isDefined() == false) || (lastinstrument[v] == 0))
            {
                BLS_CONVERT_OUTPUT_POSITION("WARNING", p, r, v);
                BLS_CONVERT_OUTPUT("arpeggio without prior instrument / key\n");
            }
            else
            {
                const Sample& sample = m_samples[lastinstrument[v] - 1];

                if (sample.isNoTranposeRequired() || sample.isFixedLengthRequired())
                {
                    BLS_CONVERT_OUTPUT_POSITION("ERROR", p, r, v);
                    BLS_CONVERT_OUTPUT("arpeggio triggered on 'no transpose sample'\n");
                }

                dc.fx = BLSfx_ARPEGGIO;

                Key key1 = lastkey[v];
                Key key2 = key1;

                key1.Transpose(sc.m_fx_m);
                key2.Transpose(sc.m_fx_l);

                u8 pk1 = FindPrecomputedKeyIndex(lastinstrument[v], key1);
                u8 pk2 = FindPrecomputedKeyIndex(lastinstrument[v], key2);

                dc.value = pk1;
                dc.value2 = pk2;
            }
        }
        break;

        //--------------------------------------------------------------
        // SLIDE UP
        //--------------------------------------------------------------
    case 1:
        BLS_CONVERT_OUTPUT_POSITION("WARNING", p, r, v);
        BLS_CONVERT_OUTPUT("slide up ignored on pcm channel (can only be used on ym voice)\n");
        break;

    case 0x101:
        dc.value = sc.m_fx_m * 16 + sc.m_fx_l;
        dc.fx = BLSfx_PITCHBEND_RANGE;
        break;

        //--------------------------------------------------------------
        // SLIDE DOWN
        //--------------------------------------------------------------
    case 2:
        BLS_CONVERT_OUTPUT_POSITION("WARNING", p, r, v);
        BLS_CONVERT_OUTPUT("slide down ignored on pcm channel (can only be used on ym voice)\n");
        break;

    case 0x102:
        dc.value = -(sc.m_fx_m * 16 + sc.m_fx_l);
        dc.fx = BLSfx_PITCHBEND_RANGE;
        break;

        //--------------------------------------------------------------
        // PITCHBEND TICKS
        //--------------------------------------------------------------
    case 3:
        BLS_CONVERT_OUTPUT_POSITION("WARNING", p, r, v);
        BLS_CONVERT_OUTPUT("portamento ignored on pcm channel (can only be used on ym voice to set pitchbend ticks)\n");
        break;
    case 0x103:
        dc.value = sc.m_fx_m * 16 + sc.m_fx_l;
        dc.fx = BLSfx_PITCHBEND_TICKS;
        break;

        //--------------------------------------------------------------
        // PORTAMENTO TICKS
        //--------------------------------------------------------------
    case 5:
        BLS_CONVERT_OUTPUT_POSITION("WARNING", p, r, v);
        BLS_CONVERT_OUTPUT("Continue fx 'Slide to note' + 'do Volume slide' ignored on pcm channel (can be used on ym voice to set portamento ticks)\n");
        break;
    case 0x105:
        dc.value = sc.m_fx_m * 16 + sc.m_fx_l;
        dc.fx = BLSfx_PORTAMENTO_TICKS;
        break;

        //--------------------------------------------------------------
        // BALANCE VOLUME
        //--------------------------------------------------------------
    case 8:
    case 0x108:
        // When use volume for balance => odd value for balance control
        if (m_useVolumeForBalance == false)
        {
            u8 value = sc.m_fx_m * 16 + sc.m_fx_l;

            // 0x8 fx goes from 0 -> 0x80 -> 0xff for left -> middle (l+r) -> right
            switch (v)
            {
            case 3:
            case 0:
                if (value <= 0x80)
                    value = 64;
                else
                    value = (255 - value) >> 1;
                break;
            case 2:
            case 1:
                if (value >= 0x80)
                    value = 64;
                else
                    value >>= 1;
                break;
            }

            Sndtrack_SetBalanceValue(sc, value, balanceused, p, r, v, dc);
        }
        break;

    //--------------------------------------------------------------
    // SET SAMPLE OFFSET / SET FINE TUNE
    //--------------------------------------------------------------
    case 0x109:
        dc.value = sc.m_fx_m * 16 + sc.m_fx_l;
        dc.fx = BLSfx_SET_FINETUNE;
        break;
    case 9:
    {
        u16 offset = (sc.m_fx_m << 12) + (sc.m_fx_l << 8);
        dc.fx = BLSfx_SETSAMPLEOFFSET;
        *(u16*)&dc.value = PCENDIANSWAP16(offset);
    }
    break;

    //--------------------------------------------------------------
    // JUMP
    //--------------------------------------------------------------
    case 0xB:
    case 0x10B:
        dc.fx = BLSfx_JUMP;
        dc.value = sc.m_fx_m * 16 + sc.m_fx_l;
        if (dc.value >= m_songLen)
        {
            BLS_CONVERT_OUTPUT_POSITION("ERROR", p, r, v);
            BLS_CONVERT_OUTPUT("in jump ($B) track position exceeds songlen (0x%x|%d >= 0x%x|%d)\n", dc.value, m_songLen, dc.value, m_songLen);
            dc.value = m_songLen - 1;
        }
        break;

        //--------------------------------------------------------------
        // VOLUME
        //--------------------------------------------------------------
    case 0xC:
    {
        u16 vol = sc.m_fx_m * 16 + sc.m_fx_l;
        if (vol > 64)
        {
            BLS_CONVERT_OUTPUT_POSITION("ERROR", p, r, v);
            BLS_CONVERT_OUTPUT("volume exceeds 64 (0x%x|%d)\n", vol, vol);
            vol = 64;
        }

        if (m_useVolumeForBalance && ((sc.m_fx_l & 1) != 0))
        {
            Sndtrack_SetBalanceValue(sc, vol, balanceused, p, r, v, dc);
        }
        else if ((lastinstrument[v] == 0) || (GetNbVolFXLevelForInstrument(lastinstrument[v]) > 1))
        {
            dc.value = VolFXLevel::GetShiftFromVolume(vol);
            if (dc.value < 8)
            {
                if (GetNbVolFXLevelForInstrument(sc.m_instrument) == 1)
                {
                    dc.value = 0;
                }
            }
            dc.fx = BLSfx_VOLUME;
        }
    }
    break;

    case 0x10C:
    {
        u8 vol = sc.m_fx_m * 16 + sc.m_fx_l;
        dc.value = std::min<u8>(vol, 15);
        dc.fx = BLSfx_VOLUME;
    }
    break;

    //--------------------------------------------------------------
    // PATTERN BREAK
    //--------------------------------------------------------------
    case 0xD:
    case 0x10D:
        dc.fx = BLSfx_PATTERN_BREAK;
        dc.value = sc.m_fx_m * 10 + sc.m_fx_l;
        if (dc.value > 63)
        {
            BLS_CONVERT_OUTPUT_POSITION("ERROR", p, r, v);
            BLS_CONVERT_OUTPUT("in pattern break ($D) row exceeds 63 (0x%x|%d)\n", dc.value, dc.value);
            dc.value = 63;
        }
        break;

        //--------------------------------------------------------------
        // EXTENDED
        //--------------------------------------------------------------
    case 0xE:
        switch (sc.m_fx_m | ymfxoffset)
        {
            //--------------------------------------------------------------
            // SET BITMASK
            //--------------------------------------------------------------
        case 0x100:
            BLS_CONVERT_OUTPUT_POSITION("WARNING", p, r, v);
            BLS_CONVERT_OUTPUT("set bitmask ignored on ym sound\n");
            break;

        case 0:
            dc.fx = BLSfx_BITMASK;
            *(u16*)&dc.value = PCENDIANSWAP16(m_bitmasks[sc.m_fx_l]);
            break;

            //--------------------------------------------------------------
            // LOOP
            //--------------------------------------------------------------
        case 6:
        case 0x106:
            if (sc.m_fx_l == 0)
            {
                dc.fx = BLSfx_LOOP_START_SET;
            }
            else
            {
                dc.fx = BLSfx_LOOP;
                dc.value = sc.m_fx_l;
            }
            break;

            //--------------------------------------------------------------
            // FREE CODE TO IMPLEMENT SOMETHING ELSE...
            //--------------------------------------------------------------
//      case 8:
//      case 0x108:
//          dc.fx = BLSfx_CLIENT_EVENT;
//          dc.value = sc.m_fx_l;
//          break;

            //--------------------------------------------------------------
            // RETRIGGER SAMPLE
            //--------------------------------------------------------------
        case 0x109:
            BLS_CONVERT_OUTPUT_POSITION("WARNING", p, r, v);
            BLS_CONVERT_OUTPUT("retrigger sample ignored on ym sound\n");
            break;

        case 9:
            dc.fx = BLSfx_RETRIGGER_SAMPLE;
            dc.value = sc.m_fx_l;
            break;

            //--------------------------------------------------------------
            // DELAY SAMPLE
            //--------------------------------------------------------------
        case 0x10D:
            BLS_CONVERT_OUTPUT_POSITION("WARNING", p, r, v);
            BLS_CONVERT_OUTPUT("delay sample ignored on ym sound\n");
            break;

        case 0xD:
            dc.fx = BLSfx_DELAY_SAMPLE;
            dc.value = sc.m_fx_l;
            break;

            //--------------------------------------------------------------
            // DELAY PATTERN
            //--------------------------------------------------------------
        case 0xE:
        case 0x10E:
            dc.fx = BLSfx_DELAY_PATTERN;
            dc.value = sc.m_fx_l;
            break;

            //--------------------------------------------------------------
            // UNKNOWN
            //--------------------------------------------------------------		
        default:
            unknownfx = true;
        }
        break;

        //--------------------------------------------------------------
        // SET SPEED
        //--------------------------------------------------------------
    case 0xF:
    case 0x10F:
    {
        u8 speed = sc.m_fx_m * 16 + sc.m_fx_l;
        dc.fx = BLSfx_SPEED;
        if (speed < 32)
        {
            dc.value = speed;
        }
        else
        {
            dc.value = 60 * 50 / speed;
        }
    }
    break;

//  Free code to implement something else
//  case 0x1E: // Blitracker specific FX Uxx
//  {
//      u8 clientdata = sc.m_fx_m * 16 + sc.m_fx_l;
//      dc.fx = BLSfx_CLIENT_EVENT;
//      dc.value = clientdata;
//      break;
//  }

    default:
        unknownfx = true;
    }

    if (unknownfx)
    {
        BLS_CONVERT_OUTPUT_POSITION("WARNING", p, r, v);
        BLS_CONVERT_OUTPUT("unmanaged FX=0x%x [0x%x]\n", sc.m_fx_h, sc.m_fx_m);
    }
}




void Sndtrack::ConvertToBlitSoundtrack_Patterns(BLSsoundTrack* sndtrack)
{
    for (u32 p = 0; p < m_patterns.size(); p++)
    {
        std::vector<BLScell> cells;

        const auto& sp = m_patterns[p];
        auto& dp = sndtrack->patterns[p];
        Key lastkey[NBMAXVOICES];
        u8  lastinstrument[NBMAXVOICES] = { 0,0,0,0,0,0,0,0 };


        dp.nbrows = (u8)sp.m_nbRows;
        dp.rowtocellindex = new u16[dp.nbrows + 1];

        if (RetrievePattern(p) < 0)
        {
            printf ("pattern unused\n");
        }
        else
        {
            for (u32 r = 0; r < sp.m_nbRows; r++)
            {
                bool balanceused[NBMAXVOICES] = { false, false, false, false, false, false, false, false };


                ASSERT(cells.size() < 65536);
                dp.rowtocellindex[r] = (u16)cells.size();

                for (u32 v = 0; v < NBMODVOICES; v++)
                {
                    const auto& sc = sp.m_cells[r][v];
                    BLScell     dc;


                    dc.precomputedKey = 0;
                    dc.fx = BLSfx_NONE;
                    dc.value = 0;
                    dc.value2 = 0;

                    if (((sc.m_instrument != 0) && (m_samples[sc.m_instrument - 1].m_sampleLen == 0)) || sc.m_key.isKeyOff())
                    {
                        lastkey[v].undefine();
                        lastinstrument[v] = 0;
                        dc.precomputedKey = BLS_PRECOMPUTED_KEY_OFF;
                    }
                    else if (sc.m_key.isDefined())
                    {
                        dc.precomputedKey = FindPrecomputedKeyIndex(sc.m_instrument, sc.m_key) + 1;

                        lastkey[v] = sc.m_key;
                        lastinstrument[v] = sc.m_instrument;
                    }
                    else if ((sc.m_instrument != 0) && lastkey[v].isDefined())
                    {
                        if (lastinstrument[v] == sc.m_instrument)
                        {
                            dc.precomputedKey = FindPrecomputedKeyIndex(sc.m_instrument, lastkey[v]) + 1;
                            lastinstrument[v] = sc.m_instrument;
                        }
                        else
                        {
                            BLS_CONVERT_OUTPUT_POSITION("ERROR", p, r, v);
                            BLS_CONVERT_OUTPUT("use new instrument without setting key => not managed\n");
                        }
                    }

                    ConvertToBlitSoundtrack_ToBLScell(sndtrack, p, r, v, sc, dc, lastkey, lastinstrument, balanceused);

                    if ((dc.precomputedKey != 0) || (dc.fx != BLSfx_NONE))
                    {
                        dc.fx |= v << BLS_FX_VOICENUM_SHIFT;
                        cells.push_back(dc);
                    }

                    if (sc.m_volume > 0)
                    {
                        dc.precomputedKey = 0;
                        dc.fx = (u8)(BLSfx_VOLUME | (v << BLS_FX_VOICENUM_SHIFT));
                        dc.value = VolFXLevel::GetShiftFromVolume(sc.m_volume - 0x10);
                        if (dc.value < 8)
                        {
                            if (GetNbVolFXLevelForInstrument(sc.m_instrument) == 1)
                            {
                                dc.value = 0;
                            }
                        }
                        dc.value2 = 0;
                        cells.push_back(dc);
                    }
                }

                if (m_nbVoices > NBMODVOICES)
                {
                    for (u32 v = NBMODVOICES; v < (NBMODVOICES + SND_YM_NB_CHANNELS); v++)
                    {
                        const auto& sc = sp.m_cells[r][v];
                        BLScell     dc;


                        dc.precomputedKey = 0;
                        dc.fx = BLSfx_NONE;
                        dc.value = 0;
                        dc.value2 = 0;

                        if (sc.m_key.isDefined())
                        {
                            if (sc.m_key.isKeyOff())
                            {
                                dc.precomputedKey = BLS_PRECOMPUTED_KEY_OFF;
                            }
                            else if (sc.m_instrument != 0)
                            {
                                dc.precomputedKey = (sc.m_key.m_octave * 12 + sc.m_key.m_semitone) + 1;
                                dc.value2 = m_instrToYmSound[sc.m_instrument - 1];
                            }
                        }

                        ConvertToBlitSoundtrack_ToBLScell(sndtrack, p, r, v, sc, dc, lastkey, lastinstrument, balanceused);

                        if ((dc.precomputedKey != 0) || (dc.fx != BLSfx_NONE))
                        {
                            dc.fx |= v << BLS_FX_VOICENUM_SHIFT;
                            cells.push_back(dc);
                        }

                        if (sc.m_volume > 0)
                        {
                            dc.precomputedKey = 0;
                            dc.fx = (u8)(BLSfx_VOLUME | (v << BLS_FX_VOICENUM_SHIFT));
                            dc.value = std::min(sc.m_volume - 0x10, 15);
                            dc.value2 = 0;
                            cells.push_back(dc);
                        }
                    }
                }
            }
        }
        ASSERT(cells.size() < 65536);

        u16 nbcells = (u16)cells.size();
        dp.rowtocellindex[dp.nbrows] = nbcells;

        if (nbcells > 0)
        {
            dp.cells = new BLScell[cells.size()];
            memcpy(dp.cells, &cells[0], cells.size() * sizeof(BLScell));
        }
    }
}


BLSsoundTrack* Sndtrack::ConvertToBlitSoundtrack()
{
    BLSsoundTrack* sndtrack = new BLSsoundTrack;


    memset (sndtrack, 0, sizeof(*sndtrack));

    ASSERT(m_instrumentUsedKeys.size() < 256);
    sndtrack->nbSourceSamples = (u8)m_instrumentUsedKeys.size();
    sndtrack->sourceSamples = new BLSsample[sndtrack->nbSourceSamples];

    sndtrack->nbSamples = m_nbPrecomputedSamples;
    sndtrack->samples = new BLSsample[sndtrack->nbSamples];

    ASSERT(m_precomputedKeys.size() < 65536);
    sndtrack->nbKeys       = (u16)m_precomputedKeys.size();
    sndtrack->keys         = new BLSprecomputedKey[sndtrack->nbKeys];
    sndtrack->keysnoteinfo = new u8[sndtrack->nbKeys];

    ASSERT(m_patterns.size() < 256);
    sndtrack->nbPatterns = (u8)m_patterns.size();
    sndtrack->patterns = new BLSpattern[sndtrack->nbPatterns];

    sndtrack->trackLen = m_songLen;
    sndtrack->track = new u8[sndtrack->trackLen];

    ConvertToBlitSoundtrack_InitBitmasks(sndtrack);

    std::map<u8, u8> instrumentToSourceSample;

    ConvertToBlitSoundtrack_SourceSamplesTable(sndtrack, instrumentToSourceSample);

    SamplesMapOrderBySize sizeordertopkindex;

    ConvertToBlitSoundtrack_PrecomputedKeys(sndtrack, instrumentToSourceSample, sizeordertopkindex);

    ASSERT(m_nbPrecomputedSamples == sizeordertopkindex.size());
    sndtrack->sampleAllocOrder = new u16[m_nbPrecomputedSamples];

    ConvertToBlitSoundtrack_SamplesAllocOrder(sndtrack, sizeordertopkindex);

    ConvertToBlitSoundtrack_Patterns(sndtrack);

    sndtrack->trackLen = m_songLen;
    STDmcpy(sndtrack->track, m_patternsSequence, m_songLen);

    sndtrack->YMsoundSet = m_ymSoundSet;    // hacky temp

    return sndtrack;
}


struct SequenceScriptParserState
{
    int                         trackindex = -1;
    std::map<std::string, u32>  macros;
    std::string                 basepath;

    SequenceScriptParserState(const char* filename_)
    {
        char drive[256];
        char dir  [256];

        _splitpath(filename_, drive, dir, NULL, NULL);
        strcat(drive, dir);
        basepath = drive;
    }
};

static bool LoadSequenceScript(const char* _filename, std::vector<BLSextSequenceRow>& sequence, SequenceScriptParserState& state_)
{
    const char* delims = "\t \r\n";
    u32         linecount = 1;
    char        errormsg[256] = "";
    char        line[256];

    FILE* file = fopen (_filename, "rt");

    if (file == NULL)
    {
        sprintf(errormsg, "cannot open file %s", _filename);
        goto Error;
    }

    while (fgets(line, sizeof(line), file))
    {
        char* p = strtok(line, delims);

        if ((p != NULL) && (*p != 0))
        {
            switch (line[0])
            {
            default:
            {
                if (state_.trackindex != -1)
                {
                    BLSextSequenceRow row;
                    int i = 0;


                    if (sscanf(p, "%x", &i) == 0)
                    {
                        strcpy(errormsg, "row not an hexa number");
                        goto Error;
                    }

                    row.row = (u8)i;
                    row.nbcommands = 0;
                    row.framenum = BLS_SEQ_FRAMENUM_UNDEFINED;
                    row.trackindex = (u8)state_.trackindex;

                    while (p = strtok(NULL, delims))
                    {
                        auto it = state_.macros.find(p);

                        if (it == state_.macros.end())
                        {
                            i = 0;
                            if (sscanf(p, "%x", &i) == 0)
                            {
                                sprintf(errormsg, "command number not an hexa number nor macro name '%s'", p);
                                goto Error;
                            }
                        }
                        else
                        {
                            i = (u8)it->second;
                        }

                        row.commands[row.nbcommands++] = (u8)i;
                    }

                    if (row.nbcommands == 0)
                    {
                        strcpy(errormsg, "no command defined into this row");
                        goto Error;
                    }

                    sequence.push_back(row);
                }
                else
                {
                    strcpy(errormsg, "commands defined without previous track index");
                    goto Error;
                }
            }
            break;

            case 't':
            {
                p = strtok(NULL, delims);
                if (p == NULL)
                {
                    strcpy(errormsg, "track index missing");
                    goto Error;
                }
                if (sscanf(p, "%x", &state_.trackindex) == 0)
                {
                    strcpy(errormsg, "track index not an hexa number");
                    goto Error;
                }
            }
            break;

            case '$':
            {
                p = strtok(NULL, delims);
                const char* key = p;

                if (p != NULL)
                    p = strtok(NULL, delims);

                if ((key == NULL) || (p == NULL))
                {
                    strcpy(errormsg, "macro definition");
                    goto Error;
                }
                u32 value = 0;
                if (sscanf(p, "%x", &value) == 0)
                {
                    strcpy(errormsg, "macro value not an hexa number");
                    goto Error;
                }
                state_.macros[key] = value;
            }
            break;

            case '#':
            {
                char filename[256];

                sprintf (filename, "%s\\%s", state_.basepath.c_str(), line+1);
                if (LoadSequenceScript(filename, sequence, state_) == false)
                {
                    sprintf(errormsg, "cannot include file %s", filename);
                    goto Error;
                }
            }
            break;

            case ';':
                break; // comment => do nothing
            }
        }

        linecount++;
    }

    fclose(file);

    return true;

Error:
    BLS_CONVERT_OUTPUT("\nERROR: in sequence file '%s' line %u : %s\n", _filename, linecount, errormsg);

    if (file != NULL)
        fclose(file);

    return false;
}

static void saveExtSequence(BLSplayer* player_, FILE* seqfile_)
{
    u16* sequenceframenummap = player_->sequenceframenummap;
    u16  tracklen = player_->sndtrack->trackLen;
    BLSextSequenceRow* seq = player_->sequence;
    u16  seqsize = player_->sequencesize;
    u16 t;


    for (t = 0; t < tracklen; t++)
    {
        TOOLwritecheck(sequenceframenummap[t], seqfile_);
    }

    for (t = 0; t < seqsize; t++, seq++)
    {
        u16 i;

        if (seq->framenum == BLS_SEQ_FRAMENUM_UNDEFINED)
        {
            BLS_CONVERT_OUTPUT ("WARNING: seqfile contains trackindex unknown in .xm file\n");
        }

        TOOLwritecheck(seq->framenum  , seqfile_);
        TOOLwritecheck(seq->nbcommands, seqfile_);

        for (i = 0; i < seq->nbcommands; i++)
        {
            TOOLwritecheck(seq->commands[i], seqfile_);
        }

        if ((seq->nbcommands & 1) == 0)
        {
            TOOLwritecheck((u8)0, seqfile_);
        }
    }

    TOOLwritecheck((u16)BLS_SEQ_FRAMENUM_UNDEFINED, seqfile_);
}



static void GenerateBlitzFile(const char* _filename, BLSsoundTrack * _bsndtrack, bool generateseq_)
{
    char blitzfilename[256];
    std::vector<BLSextSequenceRow> sequence;


    strcpy(blitzfilename, _filename);
    char* p = strrchr(blitzfilename, '.');

    if (p != NULL)
    {
        *p = 0;
    }

    FILE* seqfile = NULL;

    if (generateseq_)
    {
        char seqfilename[256];

        strcpy(seqfilename, blitzfilename);
        strcat(seqfilename, ".SEQ");
        SequenceScriptParserState state(seqfilename);
        bool result = LoadSequenceScript(seqfilename, sequence, state);
        ASSERT(result);

        strcpy(seqfilename, blitzfilename);
        strcat(seqfilename, ".BSQ");
        printf("Generate sequence file : %s\n", seqfilename);
        seqfile = fopen(seqfilename, "wb");
        ASSERT(seqfile != NULL);
    }

    strcat(blitzfilename, ".BLZ");
    printf("Generate blitz file : %s\n", blitzfilename);

    FILE* blitzfile = fopen(blitzfilename, "wb");
    ASSERT(blitzfile != NULL);

    BLSwriteHeader(true, blitzfile);
    BLSwriteSounds(_bsndtrack, blitzfile);
    
    {
        u16 sequenceframenummap[256];
        BLSplayer player;

        

        struct BLSoptDump_* dumper = BLSoptDumpInit(blitzfile, _bsndtrack);
          
        BLSinit(&stdAllocator, &stdAllocator, _bsndtrack, (BLSinitCallback)NULL);
        BLSplayerInit(&stdAllocator, &player, _bsndtrack, BLZ_DMAMODE_NOAUDIO);
        
        player.blitzDumper = dumper;

        memset(sequenceframenummap,-1,sizeof(sequenceframenummap));

        player.sequence     = sequence.size() > 0 ? &sequence[0] : NULL;
        player.sequencesize = (u16) sequence.size();
        player.sequenceframenummap = sequenceframenummap;

        BLStestPlay(&player, NULL, NULL, 0);

        if (generateseq_)
        {
            saveExtSequence(&player, seqfile);
        }

        BLSplayerFree(&stdAllocator, &player);

        BLSoptDumpShutdown(dumper);

        printf ("\nDuration: nbframes= %d time= %dmin %ds\n", player.maxframenum, player.maxframenum / (50*60), (player.maxframenum / 50) % 60);
    }

    if (blitzfile != NULL)
    {
        fclose(blitzfile);
    }

    if (seqfile != NULL)
    {
        fclose(seqfile);
    }
}


int main(char argc, char* _argv[])
{
    bool useVolumeForBalance = false;
    bool amigaPALFreq = false;
    bool generateblitz = false;
    bool generateseq = false;

    if (argc == 1)
    {
        printf("BlsConvert v 2.3.1\n\n");
        printf("blsconvert filename [-v1] [-amigapal] [-blitz] [-seq]\n\n");

        return 1;
    }

    char filename[256];

    strcpy(filename, _argv[1]);
    _strupr(filename);

    if (argc >= 3)
    {
        for (u16 t = 2; t < argc; t++)
        {
            if (strcmp(_argv[t], "-v1") == 0)
            {
                useVolumeForBalance = true;
            }
            else if (strcmp(_argv[t], "-amigapal") == 0)
            {
                amigaPALFreq = true;
            }
            else if (strcmp(_argv[t], "-blitz") == 0)
            {
                generateblitz = true;
            }
            else if (strcmp(_argv[t], "-seq") == 0)
            {
                generateseq = true;
            }
        }
    }

    try
    {
        g_freqRef.Init(amigaPALFreq);
        
        Sndtrack sndtrack(useVolumeForBalance);

        char* ext = strrchr(filename, '.');
        bool success = false;

        if (ext != NULL)
        {
            if (strcmp(ext, ".MOD") == 0)
            {
                success = sndtrack.LoadModuleMOD(filename);
            }
            else if (strcmp(ext, ".XM") == 0)
            {
                success = sndtrack.LoadModuleXM(filename);
            }
        }

        if (success)
        {
            sndtrack.PrintSamplesTable();

            sndtrack.DetermineInstrumentsFromScore();
            sndtrack.DisplayUsedInstruments();
            //sndtrack.DumpSamples();
            sndtrack.ComputeSamplesTransposeTable();

            auto bsndtrack = sndtrack.ConvertToBlitSoundtrack();

            strcpy(ext, ".BLS");
            BLSsave(bsndtrack, filename);

            if (generateblitz)
            {
                GenerateBlitzFile(filename, bsndtrack, generateseq);
            }

            delete bsndtrack; bsndtrack = NULL;
        }
        else
        {
            BLS_CONVERT_OUTPUT("ERROR: cannot open module file '%s' for read\n", filename);
        }
    }
    catch (const TOOLerror& error)
    {
        BLS_CONVERT_OUTPUT("#ERROR - exceptions catched : %s", error.m_message);
    }
}
