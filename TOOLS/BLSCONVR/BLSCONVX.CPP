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

#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include "TOOLS\BINARIZE\HELPERS.H"

#include "TOOLS\BLSCONVR\BLSCONVR.H"


extern MEMallocator stdAllocator;

const u32 XM_KEYOFF = 97;

struct XMHeader
{
    char m_magic   [17];    // 0000h    17 char   ID="Extended module: "
    char m_title   [20];    // 0011h    20 char   Module name, padded with zeroes
    char m_whythisis1a;     // 0025h     1 char   ID=01Ah
    char m_tracker [20];    // 0026h    20 char   Tracker name
    u16  m_headerver;       // 003Ah     1 word   Tracker revision number, hi-byte is major version
    u32  m_headersize;      // 003Ch     1 dword  Header size// header start
    u16  m_ordnum;          // 0040h     1 word   Song length in patterns
    u16  m_restart;         // 0042h     1 word   Restart position
    u16  m_numchannels;     // 0044h     1 word   Number of channels
    u16  m_patnum;          // 0046h     1 word   Number of patterns (< 256) ="PAT"
    u16  m_insnum;          // 0048h     1 word   Number of instruments (<128)
    u16  m_freqtable;       // 004Ah     1 word   Flags : 0 - Linear frequency table / Amiga freq. table
    u16  m_tempo;           // 004Ch     1 word   Default tempo
    u16  m_speed;           // 004Eh     1 word   Default BPM
    u8   m_ord[256];        // 0050h   256 byte   Pattern order table

    
    XMHeader()
    {
        static_assert( (sizeof(*this) - (17+20+1+20+2)) == 276, "header size desync");
        memset(this, 0, sizeof(*this));
    }

    bool read           (FILE* _file)           { return fread(this, sizeof(*this), 1, _file) == 1; }
    bool checkFormat    ()              const   { return strncmp(m_magic, "Extended Module: ", sizeof(m_magic)) == 0; }
    bool checkHeaderSize()              const   { return m_headersize == 276; }
    
    std::string getTitle() const   
    { 
        std::string str; 
        str.assign(m_title, &m_title[sizeof(m_title) -1]); 
        return str; 
    }   
    
    std::string getTracker() const   
    { 
        std::string str; 
        str.assign(m_tracker, &m_tracker[sizeof(m_tracker) -1]); 
        return str; 
    }
};

struct XMPatternHeader
{
    u32  m_headersize;
    u8   m_packing;
    u16  m_nbrows;
    u16  m_packeddatasize;

    XMPatternHeader() { memset(this, 0, sizeof(*this)); }

    bool read (FILE* _file)           
    { 
        if ( fread(this, 5, 1, _file) == 1 )
        {
            return fread(&m_nbrows, 4, 1, _file) == 1;
        }

        return false;
    }
};

struct XMInstrumentHeader
{
    u32  m_instHeadersize;      // 0000h    1 dword  Instrument block/header size
    char m_name[22];            // 0004h   22 char   ASCII Instrument name, 0 padded ?
    u8   m_type;                // 001Ah    1 byte   Instrument type (always 0)
    u16  m_nbSamples;           // 001Bh    1 word   Number of samples in instrument
    u32  m_sampleHeaderSize;    // 001Dh    1 dword  Sample header size
    u8   m_samples[96];         // 0021h   96 byte   Sample numbers for all notes
    u8   m_volEnv[48];          // 0081h   64 byte   Points of volume envelope
    u8   m_panEnv[48];          // 00C1h   64 byte   Points of panning envelope
    u8   m_volEnvNbPoints;      // 0101h    1 byte   Number of volume points
    u8   m_panEnvNbPoints;      // 0102h    1 byte   Number of panning points
    u8   m_volSustainPoint;     // 0103h    1 byte   Volume sustain point
    u8   m_volLoopStartPoint;   // 0104h    1 byte   Volume loop start point
    u8   m_volLoopEndPoint;     // 0105h    1 byte   Volume loop end point
    u8   m_panSustainPoint;     // 0106h    1 byte   Panning sustain point
    u8   m_panLoopStartPoint;   // 0107h    1 byte   Panning loop start point
    u8   m_panLoopEndPoint;     // 0108h    1 byte   Panning loop end point
    u8   m_volType;             // 0109h    1 byte   Volume type, bitmapped   (0 - Volume on, 1 - Sustain on, 2 - Loop on)
    u8   m_panType;             // 010Ah    1 byte   Panning type, bitmapped  (0 - Panning on, 1 - Sustain on, 2 - Loop on)
    u8   m_vibratoType;         // 010Bh    1 byte   Vibrato type
    u8   m_vibratoSweep;        // 010Ch    1 byte   Vibrato sweep
    u8   m_vibratoDepth;        // 010Dh    1 byte   Vibrato depth
    u8   m_vibratoRate;         // 010Eh    1 byte   Vibrato rate
    u16  m_volFadeout;          // 010Fh    1 word   Volume fadeout
    u16  m_reserved[11];        // 0111h    11 word  Reserved

    bool read (FILE* _file)           
    { 
        TOOLreadcheck(this, 27, _file);
        TOOLreadcheck(&m_nbSamples, 2, _file);

        if (m_nbSamples > 0)
        {
            TOOLreadcheck(&m_sampleHeaderSize, 210, _file);
            TOOLreadcheck(&m_volFadeout, 24, _file);
        }

        return false;
    }
};

struct XMSampleHeader
{
    u32   m_sampleLen;            // 0000h                   1 dword  Sample length = "LEN"
    u32   m_sampleLoopStart;      // 0004h                   1 dword  Sample loop start
    u32   m_sampleLoopLength;     // 0008h                   1 dword  Sample loop length
    u8    m_volume;               // 000Ch                   1 byte   Volume
    s8    m_finetune;             // 000Dh                   1 byte   Finetune for sample (-128..+127) +-127 is one half tone
    u8    m_sampleType;           // 000Eh                   1 byte   Sample type, bitmapped 0,1 : Loop type : (0 - no loop, 1 - forward loop, 2 - ping-pong loop, 3 - reserved, 4?: sample is 16-bit)
    u8    m_samplePan;            // 000Fh                   1 byte   Sample pan
    s8    m_relativeNoteNumber;   // 0010h                   1 byte   Relative note number (signed byte) (-96..+95), 0 -> C-4 sounds as C-4
    u8    m_reserved;             // 0011h                   1 byte   Reserved
    char  m_name[22];             // 0012h                  22 char   ASCII name of sample, 0 padded
                                  
                                  // 0013h               "LEN" byte   Sample data. The sample data is stored as delta compressed data like the ProTracker.
    void read (FILE* _file)           
    { 
        TOOLreadcheck(this, sizeof(*this), _file);
    }
};


static s32 unpackPattern(const u8* packedpattern, u8* pattern, s32 numRows, s32 numChannels)
{
    s32 outsize = 0;
    
    for (s32 x = 0 ; x < numRows ; x++)
    {
		for (s32 y = 0 ; y < numChannels ; y++) 
		{		        	
            u8 packhead = *packedpattern;

            s32 i = 0;

            if (packhead & 0x80)  // is pattern packed ?
			{
                packedpattern++;
                memset (&pattern[outsize], 0, 5);
                
                for (u8 bit = 0 ; bit < 5 ; bit++)
                {
                    if (packhead & (1 << bit))
                    {
                        pattern[outsize] = *packedpattern++;
                    }
                    outsize++;
                }
            }
            else
            {
                memcpy (&pattern[outsize], packedpattern, 5);
                outsize += 5;
                packedpattern += 5;
            }
		}
    }

	return outsize;
}


void Sndtrack::TryToLoadYMsounds (const char* _filename)
{
    char path[512];
    char dir[512];
    char filename[512];

    _splitpath (_filename, path, dir, filename, NULL);

    strcat (path, dir);
    strcat (path, filename);
    strcat (path, ".ini");

    m_ymloaded = SNDYMloadSounds (&stdAllocator, path, &m_ymSoundSet);

    if (m_ymloaded)
    {
        m_instrToYmSound.resize(m_samples.size(), -1);

        for (u32 t = 0 ; t < m_samples.size() ; t++)
        {
            const char* sampleName = m_samples[t].m_sampleName;

            for (u32 i = 0 ; i < m_ymSoundSet.nbSounds ; i++)
            {
                if ( strcmp(m_ymSoundSet.names[i], sampleName) == 0 )
                {
                    m_instrToYmSound [t] = i;
                    break;
                }
            }
        }

        m_ymSoundUsed.resize(m_ymSoundSet.nbSounds, false);
    }
}


bool Sndtrack::LoadModuleXM (const char* _filename)
{
    bool success = false;
    FILE* file = fopen(_filename, "rb");


    if (file != NULL)
    {
        printf ("Source module size = %u\n\n", TOOLfileSize(file));

        XMHeader header;

        if (header.read(file) == false)
        {
            printf ("Format XM cannot read header\n");
            goto Error;        
        }

        if (header.checkFormat() == false)
        {
            printf ("Format XM not recognized\n");
            goto Error;
        }

        strcpy(m_title, header.getTitle().c_str());

        printf ("Tracker = %20s - Ver = 0x%x\n", header.getTracker().c_str(), header.m_headerver);

        if (header.checkHeaderSize() == false)
        {
            printf ("Wrong header size %d != 276\n", header.m_headersize);
            goto Error;
        }

        m_nbVoices = header.m_numchannels;

		ASSERT(header.m_ordnum < 256);
        m_songLen = (u8) header.m_ordnum;
        memcpy(m_patternsSequence, header.m_ord, sizeof(m_patternsSequence));

		ASSERT(header.m_restart < 256);
        m_restartSongLooping = (u8) header.m_restart;

        m_patterns.resize(header.m_patnum);

        for (u32 i = 0 ; i < header.m_patnum ; i++)
        {
            Pattern& pattern = m_patterns[i];

            XMPatternHeader xmpatternHeader;

            if ( xmpatternHeader.read(file) == false)
            {
                printf ("XM file corrupted\n");
                goto Error;
            }

            u8 patternpackeddata[32768];

            assert(xmpatternHeader.m_packeddatasize <= sizeof(patternpackeddata));

            if ( fread(patternpackeddata, xmpatternHeader.m_packeddatasize, 1, file) != 1)
            {
                printf ("XM file corrupted (2)\n");
                goto Error;            
            }

            u8 patterndata[32768];

            u32 unpacklen = unpackPattern (patternpackeddata, patterndata, xmpatternHeader.m_nbrows, header.m_numchannels);
            assert (unpacklen <= sizeof(patterndata));
            assert (unpacklen == (5 * xmpatternHeader.m_nbrows * header.m_numchannels));

            pattern.SetSize(xmpatternHeader.m_nbrows);

            u8* p = patterndata;

            for (u32 y = 0 ; y < xmpatternHeader.m_nbrows ; y++)
            {
                for (u32 x = 0 ; x < header.m_numchannels ; x++)
                {
                    auto& cell = pattern.m_cells[y][x];
                    //decaler la conversion period => note dans le decodage specifique du MOD

                    switch (p[0])
                    {                    
                    case 0:
                        cell.m_key.undefine();
                        break;
                    case XM_KEYOFF:
                        cell.m_key.setKeyOff();
                        break;
                    default:
                        if (p[0] < XM_KEYOFF)
                        {
                            s16 key = p[0] - 1;
                                                        
                            if (x >= 4)
                            {
							    key -= 12;
                            }

                            if (key < 0)
                            {
                                BLS_CONVERT_OUTPUT_POSITION("#ERROR ", i, y, x );
                                BLS_CONVERT_OUTPUT("#ERROR key too low (%ld)\n", key);
                            }
                            else
                            {
                                cell.m_key = Key(key / 12, key % 12);
                            }
                        }
                    }

                    cell.m_instrument = p[1];       // instrument  - 1 to 128 or 0
                    cell.m_volume     = p[2];       // fx type//p[2] // volume
                    cell.m_fx_h       = p[3];       // fx type
                    cell.m_fx_m       = p[4] >> 4;  // fx param
                    cell.m_fx_l       = p[4] & 0xF; // fx param
                    
                    p += 5;
                }
            }
        }

        m_samples.resize(header.m_insnum);

        for (u32 i = 0 ; i < header.m_insnum ; i++)
        {
            XMInstrumentHeader instrumentHeader;
            auto& sample = m_samples[i];

            instrumentHeader.read(file);

            if (instrumentHeader.m_nbSamples > 0)
            {
                ASSERT(instrumentHeader.m_nbSamples == 1);

                std::vector<XMSampleHeader> sampleHeaders;

                sampleHeaders.resize(instrumentHeader.m_nbSamples);

                for (u32 s = 0 ; s < instrumentHeader.m_nbSamples ; s++)
                {
                    sampleHeaders[s].read(file);
                }

                for (u32 s = 0 ; s < instrumentHeader.m_nbSamples ; s++)
                {
                    const auto& xmsample = sampleHeaders[s];
                    
                    if (s == 0)
                    {
                        sample.m_sampleVolume     = xmsample.m_volume; 
                        sample.m_fineTune         = xmsample.m_finetune;
						sample.m_relativeNoteNum  = xmsample.m_relativeNoteNumber - 24;
                        sample.m_sampleLen        = xmsample.m_sampleLen;
                        sample.m_sampleLoopStart  = xmsample.m_sampleLoopStart;
                        sample.m_sampleLoopLength = (u16) xmsample.m_sampleLoopLength;
						
						if (xmsample.m_sampleLoopLength >= 0x10000)
						{
							BLS_CONVERT_OUTPUT ("#ERROR sample loop exceed 64k\n");
						}

                        if (xmsample.m_sampleType & 4)
                        {
                            BLS_CONVERT_OUTPUT ("#ERROR 16 bits samples not managed => convert to 8 bits\n");
                        }

                        if (xmsample.m_sampleType & 2)
                        {
                            BLS_CONVERT_OUTPUT ("#ERROR ping pong samples not managed\n");
                        }

                        if (xmsample.m_sampleLen > 0)
                        {
                            sample.m_data.resize(xmsample.m_sampleLen);
                            TOOLreadcheck (&sample.m_data[0], xmsample.m_sampleLen, file);
                            
                            // Sample data (signed): The samples are stored as delta values. 
                            // Even 16-bit data is stored as delta values. (It only makes really sens to store every other byte as delta value, but this is as easy)
                            s8 oldval = 0;
                            for (u32 t = 0 ; t < xmsample.m_sampleLen ; t++)
                            {
                                s8 newval = sample.m_data[t] + oldval;
                                sample.m_data[t] = oldval = newval;
                            }
                        }

                        sample.CheckLength ();
                    }
                    else
                    {
                        fseek (file, xmsample.m_sampleLen, SEEK_CUR);
                    }
                }
            }

            sample.SetName (instrumentHeader.m_name);
        }       

        printf ("\n\n-------------------- LoadModule XM End --------------------\n\n");

        success = true;

        TryToLoadYMsounds (_filename);
    }

Error:
    if (file != NULL)
    {
        fclose(file);
    }

    return success;
}


