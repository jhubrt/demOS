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


const u32 NBINSTRUMENTS = 31;
const u32 NBMODPATTERNROWS = 64;



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

class MODPeriodKeyMap
{
    std::map<unsigned, Key> m_period2Key;
       
public:    
    MODPeriodKeyMap ()
    {
        m_period2Key[0]    = Key(Key::INVALID, 0);

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

        // extended octave...
        m_period2Key[53 ] = Key(5,0);
        m_period2Key[50 ] = Key(5,1);
        m_period2Key[47 ] = Key(5,2);
        m_period2Key[45 ] = Key(5,3);
        m_period2Key[42 ] = Key(5,4);
        m_period2Key[40 ] = Key(5,5);
        m_period2Key[37 ] = Key(5,6);
        m_period2Key[35 ] = Key(5,7);
        m_period2Key[33 ] = Key(5,8);
        m_period2Key[31 ] = Key(5,9);
        m_period2Key[30 ] = Key(5,10);
        m_period2Key[28 ] = Key(5,11);
    }

    std::map<unsigned, Key>::const_iterator FindKey (u16 _period) const
    {            
        return m_period2Key.find(_period);
    }

    std::map<unsigned, Key>::const_iterator End() const 
    { 
        return m_period2Key.end(); 
    }
};

MODPeriodKeyMap g_MODPeriodKeyMap;


static bool ReadMODCell (FILE* _file, Pattern::Cell& _cell)
{
    u16 fx;
    u16 period = 0;

    TOOLreadcheck(&period, sizeof(period), _file);
    TOOLreadcheck(&fx      , sizeof(fx), _file);

    period = PCENDIANSWAP16(period);
    fx     = PCENDIANSWAP16(fx);

    _cell.m_instrument = ((period >> 8) & 0xF0) | (fx >> 12);

    period &= 0xFFF;

    _cell.m_fx_h   = (fx >> 8) & 0xF;
    _cell.m_fx_m   = (fx >> 4) & 0xF;
    _cell.m_fx_l   =  fx       & 0xF;
    _cell.m_volume = 0;

    auto it = g_MODPeriodKeyMap.FindKey(period);

    if (it != g_MODPeriodKeyMap.End())
    {
        _cell.m_key = it->second;
        return true;
    }
    else
    {
        BLS_CONVERT_OUTPUT ("\n#ERROR unknown period %d\n", period);
        return false;
    }
}

static u8 determineChannelsNum(const char* _f)
{
    if (memcmp(_f, "M.K.", 4) == 0 ||
        memcmp(_f, "M!K!", 4) == 0 ||
        memcmp(_f, "FLT4", 4) == 0)
    {
        return 4;
    }

    if (memcmp(_f, "FLT8", 4) == 0 || 
        memcmp(_f, "OKTA", 4) == 0 ||
        memcmp(_f, "OCTA", 4) == 0 ||
        memcmp(_f, "FA08", 4) == 0 ||
        memcmp(_f, "CD81", 4) == 0)
    {
        return 8;
    }

    if (memcmp(_f + 1, "CHN", 3) == 0)
    {
        if(_f[0] >= '1' && _f[0] <= '9')
        {
            return _f[0] - '0';
        }
    }

    if (memcmp(_f + 2, "CH", 2) == 0 ||
        memcmp(_f + 2, "CN", 2) == 0)
    {
        if (_f[0] >= '1' && _f[0] <= '9' && _f[1] >= '0' && _f[1] <= '9')
        {
            return (_f[0] - '0') * 10 + _f[1] - '0';
        }
    }

    return 0;
}


bool Sndtrack::LoadModuleMOD (const char* _filename)
{
    FILE* file = fopen(_filename, "rb");

    if (file != NULL)
    {
        printf ("Source module size = %u\n\n", TOOLfileSize(file));

        m_title[sizeof(m_title) - 1] = 0;

        fread(m_title, sizeof(m_title) - 1, 1, file);

        m_samples.resize(NBINSTRUMENTS);

        for (u32 t = 0 ; t < m_samples.size() ; t++)
        {
            Sample& sample = m_samples[t];
            char name[22];
            u16 sampleLen = 0;
            u16 sampleLoopStart = 0;


            TOOLreadcheck(name, sizeof(name), file);

            TOOLreadcheck(&sampleLen, sizeof(sampleLen), file);
            sample.m_sampleLen = PCENDIANSWAP16(sampleLen) << 1;

            TOOLreadcheck(&sample.m_fineTune, sizeof(sample.m_fineTune), file);
            sample.m_fineTune <<= 4; /* 4 bits signed * 16 => 8 bits signed */ 

            TOOLreadcheck(&sample.m_sampleVolume, sizeof(sample.m_sampleVolume), file);

            TOOLreadcheck(&sampleLoopStart, sizeof(sampleLoopStart), file);
            sample.m_sampleLoopStart = PCENDIANSWAP16(sampleLoopStart) << 1;

            TOOLreadcheck(&sample.m_sampleLoopLength, sizeof(sample.m_sampleLoopLength), file);
            sample.m_sampleLoopLength = PCENDIANSWAP16(sample.m_sampleLoopLength) << 1;

            sample.SetName(name);
            sample.CheckLength();
        }

        TOOLreadcheck(&m_songLen, sizeof(m_songLen), file);
        printf ("\nSong length = %u\n", m_songLen);

        TOOLreadcheck(&m_restartSongLooping, sizeof(m_restartSongLooping), file);
        printf ("\nRestartSongLooping = %u\n", m_restartSongLooping);

        TOOLreadcheck(&m_patternsSequence, 128, file);

        m_format[4] = 0;
        TOOLreadcheck(&m_format, sizeof(m_format) -1 , file);
        printf ("\n\n%4s\n", m_format);

        u8 numchannels = determineChannelsNum(m_format);
        ASSERT(numchannels >= 4);

        u32 nb = 1 + *std::max_element(m_patternsSequence, m_patternsSequence + sizeof(m_patternsSequence));
        printf ("Nb saved patterns %d\n", nb);

        m_patterns.resize(nb);

        m_nbVoices = NBMODVOICES;

        // Read patterns
        for (u32 p = 0 ; p < nb ; p++)
        {
            Pattern& pattern = m_patterns[p];

            pattern.SetSize(NBMODPATTERNROWS);            

            for (u32 r = 0 ; r < pattern.m_nbRows ; r++)
            {
                for (u32 v = 0 ; v < m_nbVoices ; v++)
                {
                    Pattern::Cell& cell = pattern.m_cells[r][v];

                    if ( ReadMODCell (file, cell) == false )
                    {
                        BLS_CONVERT_OUTPUT_POSITION("#ERROR", p, r, v);
                    }
                }

                // bypass channel >= 4
                for (u32 v = m_nbVoices; v < numchannels; v++)
                {
                    Pattern::Cell tempcell;

                    if ( ReadMODCell (file, tempcell) == false )
                    {
                        BLS_CONVERT_OUTPUT_POSITION("#ERROR", p, r, v);
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

		m_ymloaded = false;
		DEFAULT_CONSTRUCT(&m_ymSoundSet);

        fclose(file);

        printf ("\n\n-------------------- LoadModule MOD End --------------------\n\n");
    }

    return file != NULL;
}

