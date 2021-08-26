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

#ifndef BLSCONVR_H
#define BLSCONVR_H

const u32 NBPATTERNROWS = 64;
const u32 NBVOICES      = 4; 

const float AMIGA_NTSC_C2_FREQ = 8363.4229f;
const float AMIGA_PAL_C2_FREQ  = 8287.1369f;

const float STFREQ = (float) BLS_DMA_FREQ / 2.0f;

#define BLS_CONVERT_OUTPUT(FORMAT, ...) { printf (FORMAT, __VA_ARGS__ ); fprintf (stderr, FORMAT, __VA_ARGS__ ); }

struct Key
{
    u8      m_octave;
    u8      m_semitone;
    bool    m_supportArpeggio;

    static const char*             m_names[12];
    static std::map<unsigned, Key> m_period2Key;
    static float                   m_amigaFreq;

    static void Init (bool _palFreq);

    Key(u8 _octave, u8 _semitone) 
        : m_octave   (_octave)
        , m_semitone (_semitone)
        , m_supportArpeggio (false)
    {
        ASSERT (_semitone < 12);
    }

    Key() 
        : m_octave   (0x80)
        , m_semitone (0x80)
        , m_supportArpeggio (false)
    {}

    bool isDefined() const { return m_octave != 0x80; }

    void undefine() { m_octave = 0x80; }

    bool operator < (const Key& _o) const
    {
        return ((m_octave << 8) | m_semitone) < ((_o.m_octave << 8) | _o.m_semitone);
    }

    operator u16 () const 
    {
        return (m_octave << 4) | m_semitone;
    }

    s16 GetDistance (const Key& _o) const 
    {
        s16 octavedist = (m_octave - _o.m_octave) * 12;
        octavedist += m_semitone - _o.m_semitone;

        return octavedist;
    }
    
    float GetAmigaSampleFreq(s8 _fineTune) const
    {
        Key reference (2, 0);

        const float deltasemitones = (float)GetDistance(reference) + (float)_fineTune / 8.0f;

        const float amigaSampleFreq  = m_amigaFreq * powf(2.0f, (1.0f/12.0f) * deltasemitones);

        return amigaSampleFreq; 
    }

    void Transpose(s8 _nbsemitones)
    {
        s32 semitone = _nbsemitones + m_semitone;
    
        if (semitone < 0)
        {
            m_octave -= (u8)(( (1 - semitone) / 12 ) + 1);
            semitone = (semitone + (12 * 100)) % 12;
        }
        else if (semitone >= 12)
        {
            m_octave += (u8)(semitone / 12);
            semitone = semitone % 12;
        }

        m_semitone = (u8) semitone;
    }
};


struct Sample
{
    std::vector<s8> m_data;

    char m_sampleName[23];
    u16  m_sampleLen;
    s8   m_fineTune;
    u8   m_sampleVolume;
    u16  m_sampleLoopStart;
    u16  m_sampleLoopLength;
    u32  m_fixedLen;

    struct TransposeType
    {
        enum Enum
        {
            NO_TRANSPOSE,
            NO_INTERPOL,
            LINEAR_INTERPOL
        };
    };

    TransposeType::Enum m_transposeType;

    bool isLooping() const { return m_sampleLoopLength > 2; }

    bool isInterpolationRequired() const { return m_transposeType == TransposeType::LINEAR_INTERPOL; }
    bool isNoTranposeRequired()    const { return m_transposeType == TransposeType::NO_TRANSPOSE; }
    bool isFixedLengthRequired()   const { return m_fixedLen > 0; }

    void Read (FILE* _file)
    {
        m_sampleName[sizeof(m_sampleName) - 1] = 0;
        TOOLreadcheck(m_sampleName, sizeof(m_sampleName) - 1, _file);

        m_transposeType = TransposeType::NO_INTERPOL;
        m_fixedLen = 0;

        if (m_sampleName[0] == '~')
        {
            bool fixedLen = false; 

            switch (m_sampleName[1])
            {
            case '0':
                m_transposeType = TransposeType::NO_TRANSPOSE;
                break;

            case '1':
                if (m_sampleName[2] == ':')
                    fixedLen = true;
                break;

            case '2':
                m_transposeType = TransposeType::LINEAR_INTERPOL;
                if (m_sampleName[2] == ':')
                    fixedLen = true;
                break;
            }

            if (fixedLen)
            {
                char* p = m_sampleName + 3;
                u16 t, i = 0; 
                char temp[sizeof(m_sampleName)];

                for (t = 3 ; t < (sizeof(m_sampleName) - 1) ; t++, i++)
                {
                    if (isdigit(m_sampleName[t]))
                    {
                        temp[i] = m_sampleName[t];
                    }
                    else
                    {
                        break;
                    }
                }

                temp[i] = 0;
                sscanf (temp, "%lu", &m_fixedLen);
            }
        }

        TOOLreadcheck(&m_sampleLen, sizeof(m_sampleLen), _file);
        m_sampleLen = PCENDIANSWAP16(m_sampleLen) << 1;

        TOOLreadcheck(&m_fineTune, sizeof(m_fineTune), _file);
        m_fineTune <<= 4;
        m_fineTune >>= 4;   /* 4 bits sign extension */ 

        TOOLreadcheck(&m_sampleVolume, sizeof(m_sampleVolume), _file);

        TOOLreadcheck(&m_sampleLoopStart, sizeof(m_sampleLoopStart), _file);
        m_sampleLoopStart = PCENDIANSWAP16(m_sampleLoopStart) << 1;

        TOOLreadcheck(&m_sampleLoopLength, sizeof(m_sampleLoopLength), _file);
        m_sampleLoopLength = PCENDIANSWAP16(m_sampleLoopLength) << 1;

        if (isLooping())
        {
            if (m_sampleLen > (m_sampleLoopStart + m_sampleLoopLength))
            {
                BLS_CONVERT_OUTPUT ("#WARNING cropping sample '%s' len from %u to %u\n", m_sampleName, m_sampleLen, m_sampleLoopStart + m_sampleLoopLength);
                m_sampleLen = m_sampleLoopStart + m_sampleLoopLength;
            }
        }
    }

    void Dump (const char* _filename)
    {
        FILE* file = fopen (_filename, "wb");

        fwrite(&(m_data[0]), m_data.size(), 1, file);

        fclose (file);
    }

    void Print ()
    {
        printf ("%22s | %6u | %6d | %6u | %6u | %6u\n", m_sampleName, m_sampleLen, m_fineTune, m_sampleVolume, m_sampleLoopStart, m_sampleLoopLength);
    }
};


struct Pattern
{
    struct Cell
    {
        u16 m_period;
        u8  m_instrument;
        u8  m_fx_h;
        u8  m_fx_m;
        u8  m_fx_l; 


        void Read (FILE* _file)
        {
            u16 fx;
            
            TOOLreadcheck(&m_period, sizeof(m_period), _file);
            TOOLreadcheck(&fx      , sizeof(fx), _file);

            m_period = PCENDIANSWAP16(m_period);
            fx       = PCENDIANSWAP16(fx);

            m_instrument = ((m_period >> 8) & 0xF0) | (fx >> 12);

            m_period &= 0xFFF;

            m_fx_h = (fx >> 8) & 0xF;
            m_fx_m = (fx >> 4) & 0xF;
            m_fx_l =  fx       & 0xF;
        }

        void Print ()
        {           
            if (m_period != 0)
            {
                auto it = FindKey();
                if (it != Key::m_period2Key.end())
                {
                    printf (" %-2s%x", Key::m_names[it->second.m_semitone], it->second.m_octave);
                }
                else
                {
                    printf ("\n#ERROR unknown period %d\n", m_period);
                }
            }
            else
            {
                printf(" - -");
            }

            printf (" %-2u [%x %x %x]", m_instrument, m_fx_h, m_fx_m, m_fx_l);
        }

        std::map<unsigned, Key>::iterator FindKey () const
        {            
            return Key::m_period2Key.find(m_period);
        }
    };

    Cell m_cells[NBPATTERNROWS][NBVOICES];
};


struct PrecomputedKey
{
    u8              m_instrument;
    s8              m_blitterTranspose;
    s8              m_fineTune;
    Key             m_key;
    u32             m_baseKeyIdentifier;
    u32             m_storagesize;
    bool            m_supportVolumeFX;
    double          m_freqmul;

    static u32 sGetIdentifier(u8 _instrument, const Key& _key) { return ((_instrument << 16) | static_cast<u16>(_key)); }

    bool IsBaseKey     () const { return m_baseKeyIdentifier == 0; }
    u32  GetIdentifier () const { return sGetIdentifier(m_instrument, m_key); }

    void AssignStorageCharac (u32 _nbSTsamples, bool _supportVolumeFX, u32 _storageShift)
    {
        m_storagesize = _nbSTsamples;
        if (_supportVolumeFX)
        {
            m_storagesize <<= 1;
        }
        m_storagesize >>= _storageShift;
        m_supportVolumeFX = _supportVolumeFX;
    }
};

struct VolFXLevel
{
    u32             m_nbOccurences;
    std::set<u8>    m_modVolumes;

    static u8 GetShiftFromVolume (u16 vol)
    {
        if (vol > 48)
            return 0;
        else if (vol > 24)  
            return 1;
        else if (vol > 12) 
            return 2;
        else if (vol > 6) 
            return 3;
        else if (vol > 3) 
            return 4;
        else if (vol > 1) 
            return 5;
        else if (vol > 0)
            return 6;
        else if (vol == 0)
            return 8;

        ASSERT(0);
        return 8;
    }

    VolFXLevel (u8 _modVolume)
        : m_nbOccurences (1)
    {
        m_modVolumes.insert(_modVolume);
    }

    void add (u8 _modVolume)
    {
        m_modVolumes.insert(_modVolume);
        m_nbOccurences++;       
    }
};

class Sndtrack
{
    char m_title[21];
    u8   m_songLen;
    u8   m_restartSongLooping;
    u8   m_patternsSequence[128];
    char m_format[5]; 
    bool m_useVolumeForBalance;
    u16  m_bitmasks[16];

    std::vector<Sample>            m_samples;
    std::map<u8,std::set<Key>>     m_instrumentUsedKeys;
    std::vector<Pattern>           m_patterns;
    std::vector<PrecomputedKey>    m_precomputedKeys;
    std::map<u32,u32>              m_keyInstr2precomputedKeys;
    u16                            m_nbPrecomputedSamples;
    
    std::map<u8, std::map<u8,VolFXLevel>> m_instrFXVolumes;

    typedef std::multimap<u32,u16> SamplesMapOrderBySize;
    
public:

    u32 GetNbVolFXLevelForInstrument (u8 _instrument) const
    {
        auto it = m_instrFXVolumes.find(_instrument);
        
        if (it != m_instrFXVolumes.end())
            return it->second.size();
        else
            return 0;
    }
   
    Sndtrack(bool _useVolumeForBalance) : m_useVolumeForBalance (_useVolumeForBalance) {}

    bool LoadModule (const char* _filename);

    void AddKeyToInstrument (u8 _instrument, const Key& _key);
    void DisplayUsedInstruments ();
    void DumpSamples();
    void LookForLinkBetweenKeys ();
    void DisplayVolumeFXUsageByInstrument(const std::map<u8,u32>& instrToSize) const;
    void ComputeSamplesTransposeTable ();

    u8 FindPrecomputedKeyIndex (u8 _instrument, const Key& _key);
    void ConvertToBlitSoundtrack_InitBitmasks(BLSsoundTrack* sndtrack);
    void ConvertToBlitSoundtrack_SourceSamplesTable (BLSsoundTrack* sndtrack, std::map<u8,u8>& instrumentToSourceSample);
    void ConvertToBlitSoundtrack_PrecomputedKeys(BLSsoundTrack* sndtrack, const std::map<u8,u8>& instrumentToSourceSample, SamplesMapOrderBySize& sizeordertopkindex);
    void ConvertToBlitSoundtrack_SamplesAllocOrder(BLSsoundTrack* sndtrack, const SamplesMapOrderBySize& sizeordertopkindex);
    void ConvertToBlitSoundtrack_Patterns (BLSsoundTrack* sndtrack);
    BLSsoundTrack* ConvertToBlitSoundtrack ();
};

#endif
