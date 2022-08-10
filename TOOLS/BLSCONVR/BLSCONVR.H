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

#ifndef BLSCONVR_H
#define BLSCONVR_H

const u32 NBMAXVOICES   = 8; 
const u32 NBMODVOICES   = 4;

const float STFREQ = (float) BLS_DMA_FREQ / 2.0f;

const u16 NO_TRANSPOSE_PERIOD = 1; 

#define BLS_CONVERT_OUTPUT(FORMAT, ...) { printf (FORMAT, __VA_ARGS__ ); fprintf (stderr, FORMAT, __VA_ARGS__ ); }

#define BLS_CONVERT_OUTPUT_POSITION(MESSAGE_TYPE, p, r, v) {\
    printf (MESSAGE_TYPE ": pat=0x%x|%d row=0x%x|%d voice=%d - ", p, p, r, r, v + 1);\
    fprintf (stderr, MESSAGE_TYPE ": pat=0x%x|%d row=0x%x|%d voice=%d - ", p, p, r, r, v + 1); }

struct Key
{
    static const u8 INVALID = 0x80;
    static const u8 KEYOFF  = 0x79;

    u8      m_octave;
    u8      m_semitone;
    bool    m_supportArpeggio;

    static const char* m_names[12];

    Key(u8 _octave, u8 _semitone) 
        : m_octave   (_octave)
        , m_semitone (_semitone)
        , m_supportArpeggio (false)
    {
        ASSERT (_semitone < 12);
    }

    Key() 
        : m_octave   (INVALID)
        , m_semitone (INVALID)
        , m_supportArpeggio (false)
    {}

    bool isDefined() const { return m_octave != INVALID; }
    bool isKeyOff () const { return m_octave == KEYOFF; }

    void undefine()     { m_octave = INVALID; }
    void setKeyOff()    { m_octave = KEYOFF;  } 

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
    
    void Transpose(s8 _nbsemitones)
    {
        s32 semitone = _nbsemitones + m_semitone;
    
        if (semitone < 0)
        {
            m_octave -= (u8)(( (- semitone - 1) / 12 ) + 1);
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

struct FreqRef 
{
    float m_amigaFreq;

    FreqRef();

    void Init(bool _palfreq);

    float GetAmigaSampleFreq(const Key& _key, s8 _fineTune);
};

struct Sample
{
    struct TransposeType
    {
        enum Enum
        {
            NO_TRANSPOSE,
            NO_INTERPOL,
            LINEAR_INTERPOL
        };
    };

    std::vector<s8> m_data;

	char m_sampleName[23]	= {0};
    u32  m_sampleLen		= 0;
    s8   m_fineTune			= 0;
	s8	 m_relativeNoteNum	= 0;
    u8   m_sampleVolume		= 0;
    u32  m_sampleLoopStart	= 0;
    u16  m_sampleLoopLength = 0;
    u32  m_fixedLen			= 0;
    TransposeType::Enum m_transposeType = TransposeType::NO_INTERPOL;

   
	bool isLooping() const { return m_sampleLoopLength > 2; }

    bool isInterpolationRequired() const { return m_transposeType == TransposeType::LINEAR_INTERPOL; }
    bool isNoTranposeRequired()    const { return m_transposeType == TransposeType::NO_TRANSPOSE; }
    bool isFixedLengthRequired()   const { return m_fixedLen > 0; }

    void SetName (const char* _name)
    {
        strncpy(m_sampleName, _name, sizeof(m_sampleName) - 1);
        m_sampleName[sizeof(m_sampleName) - 1] = 0;

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
    }

    void CheckLength ()
    {
        if (isLooping())
        {
            if (m_sampleLen > (m_sampleLoopStart + (u32)m_sampleLoopLength))
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
};


struct Pattern
{
    struct Cell
    {
        Key m_key;
        u8  m_instrument;
        u8  m_volume;
        u8  m_fx_h;
        u8  m_fx_m;
        u8  m_fx_l; 


        void Print ()
        {           
            if (m_key.isDefined())
            {
				if (m_key.isKeyOff())
				{
					printf (" OFF");
				}
				else
				{
					printf (" %-2s%x", Key::m_names[m_key.m_semitone], m_key.m_octave);
				}
            }
			else
			{
                printf(" - -");
            }

            printf (" %-2u{%2x %x %x %x", m_instrument, m_volume, m_fx_h, m_fx_m, m_fx_l);
        }
    };

    typedef Cell Row[NBMAXVOICES];

    Row* m_cells;
    u16  m_nbRows;

    Pattern () 
        : m_cells(NULL) 
        , m_nbRows(0)
    {}

    ~Pattern () 
    {
        delete[] m_cells;
        m_cells = NULL;
        m_nbRows = 0;
    }

    void SetSize (u16 _nbRows) 
    {
        assert (m_cells == NULL);
        m_cells = new Row[_nbRows];
        m_nbRows = _nbRows;
    }
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
    u8   m_patternsSequence[256];
    char m_format[5]; 
    bool m_useVolumeForBalance;
    u16  m_bitmasks[16];

    std::vector<Sample>            m_samples;
    std::map<u8,std::set<Key>>     m_instrumentUsedKeys;
    std::vector<int>               m_instrToYmSound;
    std::vector<bool>              m_ymSoundUsed;
    std::vector<Pattern>           m_patterns;
    u16                            m_nbVoices;
    std::vector<PrecomputedKey>    m_precomputedKeys;
    std::map<u32,u32>              m_keyInstr2precomputedKeys;
    u16                            m_nbPrecomputedSamples;

    std::map<u8, std::map<u8,VolFXLevel>> m_instrFXVolumes;

    typedef std::multimap<u32,u16> SamplesMapOrderBySize;
    
    SNDYMsoundSet   m_ymSoundSet;
    bool            m_ymloaded;

    void TryToLoadYMsounds (const char* _filename);

public:

    u32 GetNbVolFXLevelForInstrument (u8 _instrument) const
    {
        auto it = m_instrFXVolumes.find(_instrument);
        
        if (it != m_instrFXVolumes.end())
            return it->second.size();
        else
            return 0;
    }
   
    Sndtrack(bool _useVolumeForBalance) 
        : m_useVolumeForBalance (_useVolumeForBalance)
        , m_ymloaded (false)    
    {
        memset(&m_ymSoundSet,0,sizeof(m_ymSoundSet));
        memset(m_patternsSequence,0,sizeof(m_patternsSequence));
    }

    bool LoadModuleMOD (const char* _filename);
    bool LoadModuleXM  (const char* _filename);
    void PrintSamplesTable ();
    int  RetrievePattern (u32 _patternNum, u32 _startIndex = 0);

    void DetermineInstrumentsFromScore ();
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
    void ConvertToBlitSoundtrack_ToBLScell (BLSsoundTrack* sndtrack, u32 p, u32 r, u32 v, const Pattern::Cell& _sc, BLScell& dc, Key lastkey[NBMAXVOICES], u8 lastinstrument[NBMAXVOICES], bool balanceused[NBMAXVOICES]);
    void ConvertToBlitSoundtrack_Patterns (BLSsoundTrack* sndtrack);
    BLSsoundTrack* ConvertToBlitSoundtrack ();
};

#endif
