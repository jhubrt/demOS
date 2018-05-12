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

extern "C"
{
#   include "DEMOSDK\BASTYPES.H"
#   include "DEMOSDK\STANDARD.H"
#   include "DEMOSDK\BLITSND.H"
}

#include <string.h>
#include <stdio.h>
#include <cctype>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include "TOOLS\BINARIZE\HELPERS.H"

#include "TOOLS\BLSCONVR\BLSCONVR.H"


#define BLS_CONVERT_OUTPUT(FORMAT, ...) { printf (FORMAT, __VA_ARGS__ ); fprintf (stderr, FORMAT, __VA_ARGS__ ); }

#define BLS_CONVERT_OUTPUT_POSITION(MESSAGE_TYPE, p, r, v) {\
    printf ("\n" MESSAGE_TYPE ": pat=0x%x|%d row=0x%x|%d voice=%d/4 - ", p, p, r, r, v + 1);\
    fprintf (stderr, "\n" MESSAGE_TYPE ": pat=0x%x|%d row=0x%x|%d voice=%d/4 - ", p, p, r, r, v + 1); }

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

const u32 NBINSTRUMENTS = 31;

u8 g_AmplitudeToLMC[] = 
{
    0,
    2 ,5 ,7 ,8 ,9 ,10,10,11,11,12,12,13,13,13,14,14,
    14,14,15,15,15,15,16,16,16,16,16,16,17,17,17,17,
    17,17,17,18,18,18,18,18,18,18,18,18,18,19,19,19,
    19,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20
};


/*  Protracker 16 note conversion table / MOD Period table
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
PT16 : I    1I    2I    3I    4I    5I    6I    7I    8I    9I   10I   11I   12I
MOD  : I 1712I 1616I 1524I 1440I 1356I 1280I 1208I 1140I 1076I 1016I  960I  906I
Note : I  C-0I  C#0I  D-0I  D#0I  E-0I  F-0I  F#0I  G-0I  G#0I  A-0I  A#0I  B-0I
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
I   13I   14I   15I   16I   17I   18I   19I   20I   21I   22I   23I   24I
I  856I  808I  762I  720I  678I  640I  604I  570I  538I  508I  480I  453I
I  C-1I  C#1I  D-1I  D#1I  E-1I  F-1I  F#1I  G-1I  G#1I  A-1I  A#1I  B-1I
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
I   25I   26I   27I   28I   29I   30I   31I   32I   33I   34I   35I   36I
I  428I  404I  381I  360I  339I  320I  302I  285I  269I  254I  240I  226I
I  C-2I  C#2I  D-2I  D#2I  E-2I  F-2I  F#2I  G-2I  G#2I  A-2I  A#2I  B-2I
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
I   37I   38I   39I   40I   41I   42I   43I   44I   45I   46I   47I   48I
I  214I  202I  190I  180I  170I  160I  151I  143I  135I  127I  120I  113I
I  C-3I  C#3I  D-3I  D#3I  E-3I  F-3I  F#3I  G-3I  G#3I  A-3I  A#3I  B-3I
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
I   49I   50I   51I   52I   53I   54I   55I   56I   57I   58I   59I   60I
I  107I  101I   95I   90I   85I   80I   75I   71I   67I   63I   60I   56I
I  C-4I  C#4I  D-4I  D#4I  E-4I  F-4I  F#4I  G-4I  G#4I  A-4I  A#4I  B-4I
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+ */

/* G-3 fine tune +1 may play around 25 khz on Amiga (C-2 + 19 semitones + finetune 1)
   => G-3 is the key to use for not interpolated samples */
const u16 NO_TRANSPOSE_PERIOD = 1; 

float Key::m_amigaFreq = AMIGA_NTSC_C2_FREQ;

const char* Key::m_names [12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

std::map<unsigned, Key> Key::m_period2Key;

void Key::Init (bool _palFreq)
{
    m_amigaFreq = _palFreq ? AMIGA_PAL_C2_FREQ : AMIGA_NTSC_C2_FREQ;

    m_period2Key[NO_TRANSPOSE_PERIOD] = Key(10,0);

    // MOD  : I 1712I 1616I 1524I 1440I 1356I 1280I 1208I 1140I 1076I 1016I  960I  906I
    // Note : I  C-0I  C#0I  D-0I  D#0I  E-0I  F-0I  F#0I  G-0I  G#0I  A-0I  A#0I  B-0I
    m_period2Key[1712] = Key(0,0);
    m_period2Key[1616] = Key(0,1);
    m_period2Key[1524] = Key(0,2);
    m_period2Key[1440] = Key(0,3);
    m_period2Key[1356] = Key(0,4);
    m_period2Key[1280] = Key(0,5);
    m_period2Key[1208] = Key(0,6);
    m_period2Key[1140] = Key(0,7);
    m_period2Key[1076] = Key(0,8);
    m_period2Key[1016] = Key(0,9);
    m_period2Key[960 ] = Key(0,10);
    m_period2Key[906 ] = Key(0,11);

    // 856I  808I  762I  720I  678I  640I  604I  570I  538I  508I  480I  453I
    // C-1I  C#1I  D-1I  D#1I  E-1I  F-1I  F#1I  G-1I  G#1I  A-1I  A#1I  B-1I
    m_period2Key[856] = Key(1,0);
    m_period2Key[808] = Key(1,1);
    m_period2Key[762] = Key(1,2);
    m_period2Key[720] = Key(1,3);
    m_period2Key[678] = Key(1,4);
    m_period2Key[640] = Key(1,5);
    m_period2Key[604] = Key(1,6);
    m_period2Key[570] = Key(1,7);
    m_period2Key[538] = Key(1,8);
    m_period2Key[508] = Key(1,9);
    m_period2Key[480] = Key(1,10);
    m_period2Key[453] = Key(1,11);

    // 428I  404I  381I  360I  339I  320I  302I  285I  269I  254I  240I  226I
    // C-2I  C#2I  D-2I  D#2I  E-2I  F-2I  F#2I  G-2I  G#2I  A-2I  A#2I  B-2I
    m_period2Key[428] = Key(2,0);
    m_period2Key[404] = Key(2,1);
    m_period2Key[381] = Key(2,2);
    m_period2Key[360] = Key(2,3);
    m_period2Key[339] = Key(2,4);
    m_period2Key[320] = Key(2,5);
    m_period2Key[302] = Key(2,6);
    m_period2Key[285] = Key(2,7);
    m_period2Key[269] = Key(2,8);
    m_period2Key[254] = Key(2,9);
    m_period2Key[240] = Key(2,10);
    m_period2Key[226] = Key(2,11);

    // 214I  202I  190I  180I  170I  160I  151I  143I  135I  127I  120I  113I
    // C-3I  C#3I  D-3I  D#3I  E-3I  F-3I  F#3I  G-3I  G#3I  A-3I  A#3I  B-3I
    m_period2Key[214] = Key(3,0);
    m_period2Key[202] = Key(3,1);
    m_period2Key[190] = Key(3,2);
    m_period2Key[180] = Key(3,3);
    m_period2Key[170] = Key(3,4);
    m_period2Key[160] = Key(3,5);
    m_period2Key[151] = Key(3,6);
    m_period2Key[143] = Key(3,7);
    m_period2Key[135] = Key(3,8);
    m_period2Key[127] = Key(3,9);
    m_period2Key[120] = Key(3,10);
    m_period2Key[113] = Key(3,11);

    // 107I  101I   95I   90I   85I   80I   75I   71I   67I   63I   60I   56I
    // C-4I  C#4I  D-4I  D#4I  E-4I  F-4I  F#4I  G-4I  G#4I  A-4I  A#4I  B-4I
    m_period2Key[107] = Key(4,0);
    m_period2Key[101] = Key(4,1);
    m_period2Key[95 ] = Key(4,2);
    m_period2Key[90 ] = Key(4,3);
    m_period2Key[85 ] = Key(4,4);
    m_period2Key[80 ] = Key(4,5);
    m_period2Key[75 ] = Key(4,6);
    m_period2Key[71 ] = Key(4,7);
    m_period2Key[67 ] = Key(4,8);
    m_period2Key[63 ] = Key(4,9);
    m_period2Key[60 ] = Key(4,10);
    m_period2Key[56 ] = Key(4,11);
}


void Sndtrack::AddKeyToInstrument (u8 _instrument, const Key& _key)
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


bool Sndtrack::LoadModule (const char* _filename)
{
    FILE* file = fopen(_filename, "rb");

    if (file != NULL)
    {
        printf ("Source module size = %u\n\n", TOOLfileSize(file));

        m_title[sizeof(m_title) - 1] = 0;

        fread(m_title, sizeof(m_title) - 1, 1, file);

        printf ("Title = %20s\n\n", m_title);

        printf ("    %22s |%8s|%8s|%8s|%8s|%8s", "Sample name", "Length", "FineTune", "Volume", "LoopStrt", "LoopLen\n\n");

        m_samples.resize(NBINSTRUMENTS);

        for (u32 t = 0 ; t < NBINSTRUMENTS ; t++)
        {
            Sample& sample = m_samples[t];

            sample.Read(file);

            printf ("%2u: ", t+1);
            sample.Print();
        }

        TOOLreadcheck(&m_songLen, sizeof(m_songLen), file);
        printf ("\nSong length = %u\n", m_songLen);

        TOOLreadcheck(&m_restartSongLooping, sizeof(m_restartSongLooping), file);
        printf ("\nRestartSongLooping = %u\n", m_restartSongLooping);

        TOOLreadcheck(&m_patternsSequence, sizeof(m_patternsSequence), file);
        for (u32 t = 0 ; t < sizeof(m_patternsSequence) ; t++)
        {
            if ((t & 7) == 0)
            {
                printf ("\n%3d: ", t);
            }

            printf ("%3u ", m_patternsSequence[t]);
        }

        m_format[4] = 0;
        TOOLreadcheck(&m_format, sizeof(m_format) -1 , file);
        printf ("\n\n%4s\n", m_format);

        u32 nb = 1 + *std::max_element(m_patternsSequence, m_patternsSequence + sizeof(m_patternsSequence));
        printf ("Nb saved patterns %d\n", nb);

        m_patterns.resize(nb);

        for (u32 p = 0 ; p < nb ; p++)
        {
            printf ("\n\nPattern %d\n", p);
            u8  lastinstrument[NBVOICES] = {0,0,0,0};
            Key lastkey[NBVOICES];

            for (u32 r = 0 ; r < NBPATTERNROWS ; r++)
            {
                printf ("\n%2d: ", r);

                for (u32 v = 0 ; v < NBVOICES ; v++)
                {
                    Pattern::Cell& cell = m_patterns[p].m_cells[r][v];

                    cell.Read (file);

                    if (cell.m_period != 0)
                    {
                        ASSERT(cell.m_instrument >= 1);
                        if (cell.m_instrument > m_samples.size()) // Manage invalid cells
                        {
                            BLS_CONVERT_OUTPUT_POSITION("ERROR", p, r, v);
                            BLS_CONVERT_OUTPUT("use invalid instrument (0x%x|%d)\n" , cell.m_instrument, cell.m_instrument);
                        }
                        else
                        {
                            const Sample& sample = m_samples[cell.m_instrument - 1];

                            if (sample.m_sampleLen == 0) // Manage "silence" sample
                            {
                                lastkey[v].undefine();
                                lastinstrument[v] = 0;
                            }
                            else
                            {
                                if (sample.isNoTranposeRequired() || sample.isFixedLengthRequired())
                                {
                                    cell.m_period = NO_TRANSPOSE_PERIOD;
                                }

                                auto it = cell.FindKey();
                                if (it != Key::m_period2Key.end())
                                {
                                    AddKeyToInstrument (cell.m_instrument, it->second);

                                    lastkey[v]        = it->second;
                                    lastinstrument[v] = cell.m_instrument;
                                }
                                else
                                {
                                    BLS_CONVERT_OUTPUT_POSITION("ERROR", p, r, v);
                                    BLS_CONVERT_OUTPUT("unknown key period (0x%x|%d)\n" , cell.m_period, cell.m_period);
                                }
                            }
                        }
                    }

                    switch (cell.m_fx_h)
                    {
                    case 0xC:   // volume 
                        if (lastinstrument[v] != 0)
                        {
                            // When use volume for balance => odd value for balance control
                            if ((m_useVolumeForBalance == false) || ((cell.m_fx_l & 1) == 0))
                            {
                                m_instrHasVolumeEffect.insert(lastinstrument[v]);
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

                                AddKeyToInstrument (lastinstrument[v], key1);
                                AddKeyToInstrument (lastinstrument[v], key2);
                            }
                        }
                        break;
                    }

                    cell.Print();

                    if ((v + 1) < NBVOICES)
                    {
                        printf (" | ");
                    }
                }
            }
        }

        // Read samples
        for (auto it = m_samples.begin() ; it != m_samples.end() ; it++)
        {
            auto len = it->m_sampleLen;

            if (len > 0)
            {
                it->m_data.resize(len);
                TOOLreadcheck (&(it->m_data[0]), len, file);
            }
        }

        fclose(file);

        printf ("\n\n-------------------- LoadModule End --------------------\n\n");
    }

    return file != NULL;
}

void Sndtrack::DisplayUsedInstruments ()
{
    printf ("\n\nInstrument keys used...\n\n");

    for (auto it = m_instrumentUsedKeys.begin() ; it != m_instrumentUsedKeys.end() ; it++)
    {
        for (auto k = it->second.begin() ; k != it->second.end() ; k++)
        {
            printf ("Instr = %u - Key %2s %u\n", it->first, Key::m_names[k->m_semitone], k->m_octave);
        }
    }
}

void Sndtrack::DumpSamples()
{
    u32 t = 0;

    for (auto spl = m_samples.begin() ; spl != m_samples.end() ; spl++)
    {
        auto len = spl->m_sampleLen;

        if (len > 0)
        {
            char temp[64];

            sprintf(temp, "sample_%d.raw", t);

            spl->Dump (temp);
        }

        t++;
    }
}

void Sndtrack::LookForLinkBetweenKeys ()
{
    for (auto it = m_instrumentUsedKeys.begin() ; it != m_instrumentUsedKeys.end() ; it++)
    {
        PrecomputedKey pKey;
        pKey.m_instrument = it->first;

        u8 sampleindex = pKey.m_instrument - 1;

        const auto& keyMap = it->second;

        for (auto ikey = keyMap.begin() ; ikey != keyMap.end() ; ikey++)
        {
            pKey.m_key = *ikey;
            pKey.m_blitterTranspose = 0;
            pKey.m_baseKeyIdentifier = 0;
            pKey.m_fineTune = m_samples[sampleindex].m_fineTune;

            /* Look if (in the beginning of the map) we found same semitone in a lower octave */
            for (auto ikey2 = keyMap.begin() ; ikey2 != ikey ; ikey2++)
            {
                if (ikey->m_semitone == ikey2->m_semitone)
                {
                    pKey.m_blitterTranspose  = ikey->m_octave - ikey2->m_octave;
                    pKey.m_baseKeyIdentifier = PrecomputedKey::sGetIdentifier( pKey.m_instrument, *ikey2 );
                    break;
                }
            }

            u32 keyidentifier = pKey.GetIdentifier ();
            m_keyInstr2precomputedKeys[keyidentifier] = m_precomputedKeys.size();
            m_precomputedKeys.push_back(pKey);
        }
    }    
}

void Sndtrack::DisplayVolumeFXUsageByInstrument(const std::map<u8,u32>& instrToSize) const
{
    for (auto it = instrToSize.begin() ; it != instrToSize.end() ; it++)
    {
        printf ("instrument %ld size = %-7u ", it->first, it->second);

        if ( m_instrHasVolumeEffect.find(it->first) == m_instrHasVolumeEffect.end() )
        {
            printf ("no volume effect found\n");
        }
        else
        {
            printf ("volume used\n");
        }
    }
}

void Sndtrack::ComputeSamplesTransposeTable ()
{
    LookForLinkBetweenKeys();

    printf ("\n\nInstrument precompute...\n\n");

    m_nbPrecomputedSamples = 0;

    u32 totalSize = 0;

    std::map<u8,u32> instrToSize;
    u8 lastinstru = 0;

    for (auto ipkey = m_precomputedKeys.begin() ; ipkey != m_precomputedKeys.end() ; ipkey++)
    {
        const Sample& sourceSample = m_samples[ipkey->m_instrument - 1];

        if (lastinstru != ipkey->m_instrument)
        {
            printf ("\nInstr = %3u (%5u): ", ipkey->m_instrument, sourceSample.m_sampleLen);
            lastinstru = ipkey->m_instrument;
        }
    
        if ( ipkey->IsBaseKey() )
        {
            assert(ipkey->m_blitterTranspose == 0);

            u32     nbSTsamples = 0;
            u32     nbSTsamplesLoop = 0;
            u16     freqmullog2 = 0;
            double  freqmul = 0.0;


            if (sourceSample.isNoTranposeRequired())
            {
                nbSTsamples      = sourceSample.m_sampleLen;
                nbSTsamplesLoop  = sourceSample.m_sampleLoopLength;

                freqmul = 1.0;

                printf ("notranspose ");
            }
            else
            {
                if (sourceSample.isFixedLengthRequired())
                {
                    printf ("fixed len ");

                    freqmul = (double) sourceSample.m_fixedLen / (double)sourceSample.m_sampleLen;
                }
                else
                {
                    double amigaSampleFreq = ipkey->m_key.GetAmigaSampleFreq(ipkey->m_fineTune);
                    freqmul = STFREQ / amigaSampleFreq;
                }

                nbSTsamples      = (u32) (0.5 + freqmul * (float)sourceSample.m_sampleLen);
                nbSTsamplesLoop  = (u32) (0.5 + freqmul * (float)sourceSample.m_sampleLoopLength);

                if ((sourceSample.isInterpolationRequired() == false) && (sourceSample.isLooping() == false))
                {
                    u16 freqmulint  = (u16)freqmul;

                    for (freqmullog2 = 14 ; ((1 << freqmullog2) & freqmulint) == 0 ; freqmullog2--)
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

                        for (auto iharmonickeys = m_precomputedKeys.begin() ; iharmonickeys != m_precomputedKeys.end() ; iharmonickeys++)
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

            printf ("[%u/%u (%u) >%d] ", ipkey->m_key.m_octave, ipkey->m_key.m_semitone, nbSTsamples >> freqmullog2, ipkey->m_blitterTranspose);            

            m_nbPrecomputedSamples++;

            const bool hasVolumeEffect = m_instrHasVolumeEffect.find(ipkey->m_instrument) != m_instrHasVolumeEffect.end();
            ipkey->AssignStorageCharac (nbSTsamples, hasVolumeEffect, freqmullog2 );

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
            nbSTsamples     >>= freqmullog2;

            if ( nbSTsamplesLoop > 16384 )
            {
                BLS_CONVERT_OUTPUT ("\nERROR: loop length exceeds 16384 samples for key [%u/%u (%lu)] : %d\n", ipkey->m_key.m_octave, ipkey->m_key.m_semitone, nbSTsamples, nbSTsamplesLoop);
            }

            if (ipkey->m_key.m_supportArpeggio)
            {
                if (nbSTsamples > 65535)
                {
                    BLS_CONVERT_OUTPUT ("\nERROR: sample length used in arpeggio exceeds 65535 samples for key [%u/%u (%lu)]\n", ipkey->m_key.m_octave, ipkey->m_key.m_semitone, nbSTsamples);
                }
            }
        }
        else
        {
            printf ("{%u/%u >%d} ", ipkey->m_key.m_octave, ipkey->m_key.m_semitone, ipkey->m_blitterTranspose);
            ipkey->m_freqmul = 0.0;
        }
    }

    printf("\n\n");

    DisplayVolumeFXUsageByInstrument(instrToSize);

    printf ("\ntotal size = %u for %u precomputed samples and %u keys\n", totalSize, m_nbPrecomputedSamples, m_precomputedKeys.size());
}


void Sndtrack::ConvertToBlitSoundtrack_InitBitmasks(BLSsoundTrack* sndtrack)
{
    m_bitmasks [ 0] = 0xFFFF;
    m_bitmasks [ 1] = 0xFEFE;
    m_bitmasks [ 2] = 0xFCFC;
    m_bitmasks [ 3] = 0xF8F8;
    m_bitmasks [ 4] = 0xF0F0;
    m_bitmasks [ 5] = 0xE0E0;
    m_bitmasks [ 6] = 0xC0C0;
    m_bitmasks [ 7] = 0x8080;
    m_bitmasks [ 8] = 0xEEEE;
    m_bitmasks [ 9] = 0xCCCC;
    m_bitmasks [10] = 0x8888;
    m_bitmasks [11] = 0x7F7F;
    m_bitmasks [12] = 0x3F3F;
    m_bitmasks [13] = 0x1F1F;
    m_bitmasks [14] = 0x0F0F;
    m_bitmasks [15] = 0x0000;

    for (u32 i = 0 ; i < m_samples.size() ; i++)
    {
        const char* name = m_samples[i].m_sampleName;

        const char* p = strstr(name, "#$");

        while (p != NULL)
        {
            char i[2];
            char m[3];
            u32 mask  = 0;
            u32 index = 0;

            i [0] = p[2];
            i [1] = 0;

            m [0] = p[3];
            m [1] = p[4];
            m [2] = 0;

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

                printf ("found bitmask 0x%x = 0x%x\n", index, m_bitmasks[index]);

                p = strstr(p+2, "#$");
            }
        }
    }
}

void Sndtrack::ConvertToBlitSoundtrack_SourceSamplesTable (BLSsoundTrack* sndtrack, std::map<u8,u8>& instrumentToSourceSample)
{
    // create source samples table
    u8 ssi = 0;

    for (auto i = m_instrumentUsedKeys.begin() ; i != m_instrumentUsedKeys.end() ; i++, ssi++)
    {
        u8 instrumentIndex = i->first;
        instrumentToSourceSample[instrumentIndex] = ssi;

        auto ss = &m_samples[instrumentIndex - 1];
        auto sd = &sndtrack->sourceSamples[ssi];

        sd->sampleLen         = ss->m_sampleLen;
        sd->sampleLoopLength  = ss->m_sampleLoopLength <= 2 ? 0 : ss->m_sampleLoopLength;
        sd->sampleLoopStart   = ss->m_sampleLoopStart;
        sd->sample            = new s8[sd->sampleLen];
        sd->flags             = 0;
        
        switch (ss->m_transposeType)
        {
        case Sample::TransposeType::LINEAR_INTERPOL:
            sd->flags |= BLS_SAMPLE_INTERPOLATE;
            break;

/*        case Sample::TransposeType::NO_TRANSPOSE:
            sd->flags |= BLS_SAMPLE_NOTRANSPOSE;
            break;*/
        }

        if (ss->m_sampleVolume == 64)
        {
            memcpy (sd->sample, &ss->m_data[0], sd->sampleLen);
        }
        else
        {
            for (u32 t = 0 ; t < sd->sampleLen ; t++)
            {
                sd->sample[t] = (ss->m_data[t] * ss->m_sampleVolume + 32) >> 6;
            }
        }
    }
}

static void computeFloat16 (double _value, u16* _num, u8* _shift)
{
    assert(_value >= 0.0);
    u16 intval = (u16) _value;
    s16 b;

    for (b = 15 ; b >= 0 ; b--)
    {
        if ((1 << b) & intval)
        {
            break;
        }
    }

    b++;

    *_shift = 16 - b;
    u32 value = (u32)(_value * float(1 << *_shift));
    assert ((value & 0xFFFF0000) == 0);
    *_num = (u16)value;
}


void Sndtrack::ConvertToBlitSoundtrack_PrecomputedKeys(BLSsoundTrack* sndtrack, const std::map<u8,u8>& instrumentToSourceSample, std::map<u32,u16> &sizeordertopkindex)
{
    for (u32 pi = 0, si = 0 ; pi < m_precomputedKeys.size() ; pi++)
    {
        auto pks = &m_precomputedKeys[pi];
        auto pkd = &sndtrack->keys[pi];
        u8   baseKeyFlag = 0;


        if (pks->IsBaseKey())
        {
            auto ssi = instrumentToSourceSample.find(pks->m_instrument);
            ASSERT (ssi != instrumentToSourceSample.end());

            ASSERT (pks->m_blitterTranspose <= 0);

            /* assign precomputed sample transposition parameter */
            computeFloat16 (pks->m_freqmul      , &pkd->freqmul, &pkd->freqmulshift);
            computeFloat16 (1.0 / pks->m_freqmul, &pkd->freqdiv, &pkd->freqdivshift);

            pkd->sampleIndex = ssi->second;

            u32 size = pks->m_storagesize;
            if (pks->m_supportVolumeFX)
            {
                size += 0x8000000;
            }
            sizeordertopkindex[size] = (u16) pi;

            baseKeyFlag = BLS_BASEKEY_FLAG;
        }
        else
        {
            auto it = m_keyInstr2precomputedKeys.find(pks->m_baseKeyIdentifier);
            ASSERT (it != m_keyInstr2precomputedKeys.end());
            ASSERT (m_precomputedKeys[it->second].m_blitterTranspose <= 0);

            pkd->sampleIndex = static_cast<u8>(it->second);

            const auto& pbks = m_precomputedKeys[it->second];

            computeFloat16 (pbks.m_freqmul      , &pkd->freqmul, &pkd->freqmulshift);
            computeFloat16 (1.0 / pbks.m_freqmul, &pkd->freqdiv, &pkd->freqdivshift);
        }

        /* assign key */
        pkd->blitterTranspose = (pks->m_blitterTranspose << 1) | baseKeyFlag;
    }
}


void Sndtrack::ConvertToBlitSoundtrack_SamplesAllocOrder(BLSsoundTrack* sndtrack, const std::map<u32,u16>& sizeordertopkindex)
{
    u32 heapsize[2] = {0, 0};

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
    while(it != sizeordertopkindex.begin());

    sndtrack->sampleHeapSize = std::max(heapsize[0], heapsize[1]);
    printf ("heapsize %d %d\n", heapsize[0], heapsize[1]);
}


u8 Sndtrack::FindPrecomputedKeyIndex (u8 _instrument, const Key& _key)
{
    u32 identifier = PrecomputedKey::sGetIdentifier(_instrument, _key);
    
    auto it = m_keyInstr2precomputedKeys.find(identifier);
    ASSERT (it != m_keyInstr2precomputedKeys.end());

    return static_cast<u8>(it->second);
}


static void Sndtrack_SetBalanceValue (const Pattern::Cell& _sc, u16 _value, bool _balanceused[4], u32 _p, u32 _r, u32 _voice, BLScell& _dc)
{
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
        break;

    case 2:
        if (_balanceused[1])
        {
            BLS_CONVERT_OUTPUT_POSITION("WARNING", _p, _r, _voice);
            BLS_CONVERT_OUTPUT("balance FX used on other voice 1 from same channel\n");
        }
    }

    if (_value < ARRAYSIZE(g_AmplitudeToLMC))
    {
        _dc.value = g_AmplitudeToLMC[_value];
    }
    else
    {
        BLS_CONVERT_OUTPUT_POSITION("ERROR", _p, _r, _voice);
        BLS_CONVERT_OUTPUT("balance volume exceeds %d (0x%x|%d)\n", sizeof(g_AmplitudeToLMC), _value, _value);
        _dc.value = g_AmplitudeToLMC[ARRAYSIZE(g_AmplitudeToLMC)-1];
    }
}


void Sndtrack::ConvertToBlitSoundtrack_Patterns (BLSsoundTrack* sndtrack)
{
    for (u32 p = 0 ; p < m_patterns.size() ; p++)
    {
        const auto& sp = m_patterns[p];
        auto& dp       = sndtrack->patterns[p];
        Key lastkey[NBVOICES];
        u8  lastinstrument[NBVOICES] = {0,0,0,0};


        for (u32 r = 0 ; r < NBPATTERNROWS ; r++)
        {
            bool balanceused[4] = {false, false, false, false};

            for (u32 v = 0 ; v < NBVOICES ; v++)
            {
                const auto& sc = sp.m_cells[r][v];
                auto&       dc = dp.rows[r].cells[v];


                dc.fx          = BLSfx_NONE;
                dc.value       = 0;

                if ((sc.m_instrument != 0) && (m_samples[sc.m_instrument - 1].m_sampleLen == 0))
                {
                    lastkey[v].undefine();
                    lastinstrument[v] = 0;
                    dc.fx = BLSfx_VOICE_OFF;
                }
                else if (sc.m_period != 0)
                {
                    const Key& key = sc.FindKey()->second;
                    dc.precomputedKey = FindPrecomputedKeyIndex(sc.m_instrument, key) + 1;

                    lastkey[v] = key;
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
                else
                {
                    dc.precomputedKey = 0;
                }

                if (dc.fx != BLSfx_VOICE_OFF)
                {
                    // source : [12][x][y] set volume x*16+y - legal volumes are 0..64
                    // dest   : 0 means do not set, else bit shift = volume - 1
                    bool unknownfx = false;

                    switch (sc.m_fx_h)
                    {
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

                                dc.value  = pk1;
                                dc.value2 = pk2;
                            }
                        }
                        break;

                    case 8: 
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

                    case 9:
                        {
                            u16 volume = (sc.m_fx_m << 12) + (sc.m_fx_l << 8);
                            dc.fx = BLSfx_SETSAMPLEOFFSET;
                            *(u16*)&dc.value = PCENDIANSWAP16(volume);
                        }
                        break;

                    case 0xB:
                        {
                            dc.fx = BLSfx_JUMP;
                            dc.value = sc.m_fx_m * 16 + sc.m_fx_l;
                            if (dc.value >=  m_songLen)
                            {
                                BLS_CONVERT_OUTPUT_POSITION("ERROR", p, r, v);
                                BLS_CONVERT_OUTPUT("in jump ($B) track position exceeds songlen (0x%x|%d >= 0x%x|%d)\n", dc.value, m_songLen, dc.value, m_songLen);
                                dc.value = m_songLen - 1;
                            }
                        }
                        break;

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
                            else
                            {
                                if (vol > 48)
                                    dc.value = 0;
                                else if (vol > 24)  
                                    dc.value = 1;
                                else if (vol > 12) 
                                    dc.value = 2;
                                else if (vol > 6) 
                                    dc.value = 3;
                                else if (vol > 3) 
                                    dc.value = 4;
                                else if (vol > 1) 
                                    dc.value = 5;
                                else if (vol > 0)
                                    dc.value = 6;
                                else if (vol == 0)
                                    dc.value = 8;

                                dc.fx = BLSfx_VOLUME;
                            }
                        }
                        break;

                    case 0xD:
                        {
                            dc.fx = BLSfx_PATTERN_BREAK;
                            dc.value = sc.m_fx_m * 10 + sc.m_fx_l;
                            if (dc.value > 63)
                            {
                                BLS_CONVERT_OUTPUT_POSITION("ERROR", p, r, v);
                                BLS_CONVERT_OUTPUT("in pattern break ($D) row exceeds 63 (0x%x|%d)\n", dc.value, dc.value);
                                dc.value = 63;
                            }
                        }
                        break;

                    case 0xE:
                        switch (sc.m_fx_m)
                        {
                        case 0:
                            dc.fx    = BLSfx_BITMASK;
                            *(u16*)&dc.value = PCENDIANSWAP16(m_bitmasks[sc.m_fx_l]);
                            break;
                        case 6:
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
                        case 8:
                            dc.fx    = BLSfx_CLIENT_EVENT;
                            dc.value = sc.m_fx_l;
                            break;
                        case 9:
                            dc.fx    = BLSfx_RETRIGGER_SAMPLE;
                            dc.value = sc.m_fx_l;
                            break;
                        case 0xE:
                            dc.fx    = BLSfx_DELAY_PATTERN;
                            dc.value = sc.m_fx_l;
                            break;
                        }
                        break;

                    case 0xF:
                        {
                            u8 speed = sc.m_fx_m * 16 + sc.m_fx_l;
                            dc.fx = BLSfx_SPEED;
                            if (speed < 32)
                            {
                                dc.value = speed;
                            }
                            else
                            {
                                dc.value = 60*50 / speed;
                            }
                        }
                        break;

                    default:
                        unknownfx = sc.m_fx_h > 0;
                    }

                    if (unknownfx)
                    {
                        BLS_CONVERT_OUTPUT_POSITION("WARNING", p, r, v);
                        BLS_CONVERT_OUTPUT("unmanaged FX=0x%x [0x%x]\n", sc.m_fx_h, sc.m_fx_m);
                    }
                }
            }
        }
    }
}


BLSsoundTrack* Sndtrack::ConvertToBlitSoundtrack ()
{
    BLSsoundTrack* sndtrack = new BLSsoundTrack;

    sndtrack->nbSourceSamples = m_instrumentUsedKeys.size();
    sndtrack->sourceSamples   = new BLSsample[sndtrack->nbSourceSamples];

    sndtrack->nbSamples = m_nbPrecomputedSamples;
    sndtrack->samples   = new BLSsample[sndtrack->nbSamples];

    sndtrack->nbKeys    = m_precomputedKeys.size();
    sndtrack->keys      = new BLSprecomputedKey[sndtrack->nbKeys];

    sndtrack->nbPatterns = m_patterns.size();
    sndtrack->patterns  = new BLSpattern[sndtrack->nbPatterns];

    sndtrack->trackLen  = m_songLen;
    sndtrack->track     = new u8 [sndtrack->trackLen];

    ConvertToBlitSoundtrack_InitBitmasks(sndtrack);

    std::map<u8,u8> instrumentToSourceSample;

    ConvertToBlitSoundtrack_SourceSamplesTable(sndtrack, instrumentToSourceSample);

    std::map<u32,u16> sizeordertopkindex;

    ConvertToBlitSoundtrack_PrecomputedKeys(sndtrack, instrumentToSourceSample, sizeordertopkindex);

    ASSERT(m_nbPrecomputedSamples == sizeordertopkindex.size());
    sndtrack->sampleAllocOrder = new u16[m_nbPrecomputedSamples];

    ConvertToBlitSoundtrack_SamplesAllocOrder (sndtrack, sizeordertopkindex);

    ConvertToBlitSoundtrack_Patterns (sndtrack);

    sndtrack->trackLen  = m_songLen;           
    STDmcpy (sndtrack->track, m_patternsSequence, m_songLen);

    return sndtrack;
}




int main (char argc, char* _argv[])
{
    bool useVolumeForBalance = false;
    bool amigaPALFreq = false;

    if (argc >= 3)
    {
        for (u16 t = 2 ; t < argc ; t++)
        {
            if (strcmp(_argv[t], "-v1") == 0)
            {
                useVolumeForBalance = true;
            }
            else if (strcmp(_argv[t], "-amigapal") == 0)
            {
                amigaPALFreq = true;
            }        
        }
    }

    Key::Init(amigaPALFreq);

    Sndtrack sndtrack(useVolumeForBalance);

    if ( sndtrack.LoadModule (_argv[1]) )
    {
        sndtrack.DisplayUsedInstruments ();
        //sndtrack.DumpSamples();
        sndtrack.ComputeSamplesTransposeTable();

        auto bsndtrack = sndtrack.ConvertToBlitSoundtrack();

        char temp[256];

        strcpy(temp, _argv[1]);
        _strupr(temp);
        char* p = strstr(temp, ".MOD");
        ASSERT(p!=NULL);
        strcpy (p, ".BLS");

        BLSwrite(bsndtrack, temp);

        delete bsndtrack; bsndtrack = NULL;
    }
    else
    {
        BLS_CONVERT_OUTPUT("ERROR: cannot open module file '%s' for read\n", _argv[1]);
    }
}
