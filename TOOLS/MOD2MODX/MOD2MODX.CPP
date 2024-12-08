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
}

#include <string.h>
#include <stdio.h>
#include <vector>
#include <algorithm>

#include "TOOLS\BINARIZE\HELPERS.H"

#include "TOOLS\MOD2MODX\MOD2MODX.H"


const u32 NBMODVOICES = 4;
const u32 NBINSTRUMENTS = 31;
const u32 NBMODPATTERNROWS = 64;


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


static void PCM_DPCM(bool _pack, s8* _p, s32 _len)
{
    if (_pack)
    {
        for (s32 t = _len - 1 ; t > 0 ; t--)
        {
            _p[t] -= _p[t - 1];
        }
    }
    else
    {
        for (s32 t = 1 ; t < _len ; t++)
        {
            _p[t] += _p[t - 1];
        }
    }
}


bool Sndtrack::ProcessModuleMOD (const char* _filename, bool _pack)
{
    FILE* file = fopen(_filename, "rb");

    if (file != NULL)
    {
        u32 fileSize = TOOLfileSize(file);
        printf ("Source module size = %u\n\n", fileSize);

        char* buffer = new char[fileSize];
        fread(buffer, fileSize, 1, file);
        fseek(file, 0, SEEK_SET);

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

            if (sample.isLooping())
            {
                if (sample.m_sampleLen > (sample.m_sampleLoopStart + (u32)sample.m_sampleLoopLength))
                {
                    printf ("#WARNING you may crop sample '%s' len from %u to %u\n", sample.m_sampleName, sample.m_sampleLen, sample.m_sampleLoopStart + sample.m_sampleLoopLength);
                }
            }
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

        u32 nb = 1 + *std::max_element(m_patternsSequence, m_patternsSequence + sizeof(m_patternsSequence));
        printf ("Nb saved patterns %d\n", nb);

        m_nbVoices = NBMODVOICES;

        // Read patterns
        for (u32 p = 0 ; p < nb ; p++)
        {
            for (u32 r = 0 ; r < NBMODPATTERNROWS ; r++)
            {
                for (u32 v = 0 ; v < numchannels ; v++)
                {
                    u16 fx;
                    u16 period = 0;

                    TOOLreadcheck(&period, sizeof(period), file);
                    TOOLreadcheck(&fx      , sizeof(fx),   file);
                }
            }
        }

        // Read samples
        for (auto it = m_samples.begin() ; it != m_samples.end() ; it++)
        {
            auto len = it->m_sampleLen;

            if (len > 0)
            {
                signed char* p = (signed char*) (buffer + ftell(file));

                it->m_data.resize(len);
                TOOLreadcheck (&(it->m_data[0]), len, file);

                PCM_DPCM(_pack, p, it->m_data.size());
            }
        }

        fclose(file);

        printf ("LoadModule MOD End\n");

        // Save ----------------------------------------------------------------
        char filename[256];

        if (_pack)
            sprintf(filename, "%sX", _filename);
        else
        {
            strcpy(filename, _filename);
            filename[strlen(filename) - 1] = 0;
        }

        file = fopen(filename, "wb");

        if (file != NULL)
        {
            fwrite (buffer, 1, fileSize, file);
            fclose(file);

            printf ("Module saved MOD End \n");
        }
    }

    return file != NULL;
}



bool ProcessSample (const char* _filename, bool _pack)
{
    FILE* file = fopen(_filename, "rb");

    if (file != NULL)
    {
        u32 fileSize = TOOLfileSize(file);
        printf ("Source module size = %u\n\n", fileSize);

        s8* buffer = new s8[fileSize];
        fread(buffer, fileSize, 1, file);
        fclose(file);

        printf ("LoadSample End\n");

        PCM_DPCM(_pack, buffer, fileSize);

        // Save ----------------------------------------------------------------
        char filename[256];

        if (_pack)
            sprintf(filename, "%sX", _filename);
        else
        {
            strcpy(filename, _filename);
            filename[strlen(filename) - 1] = 0;
        }

        file = fopen(filename, "wb");

        if (file != NULL)
        {
            fwrite (buffer, 1, fileSize, file);
            fclose(file);

            printf ("Module saved PCM End \n");
        }
    }

    return file != NULL;
}



int main(char argc, char* _argv[])
{
    if (argc == 1)
    {
        printf("Mod2Modx v1.1.0\n\n");
        printf("mod2modx filename\n\n");

        return 1;
    }

    char filename[256];

    strcpy(filename, _argv[1]);
    _strupr(filename);

    bool pack = true;
    bool pcmonly = false;

    char* ext = strrchr(filename, '.');
    bool success = false;

    if (ext != NULL)
    {
        pack     = _stricmp(ext, ".MOD") == 0;
        pack    |= _stricmp(ext, ".PCM") == 0;

        pcmonly  = _stricmp(ext, ".PCM" ) == 0;
        pcmonly |= _stricmp(ext, ".PCMX") == 0;
    }        

    try
    {
        printf ("%s - %s\n", pack ? "dpcm to pcm" : "pcm to dpcm", pcmonly ? "mod" : "sample");

        if (pcmonly)
        {
            ProcessSample (filename, pack);
        }
        else
        {
            Sndtrack sndtrack;

            bool success = false;

            success = sndtrack.ProcessModuleMOD(filename, pack);

            if (success)
            {
                sndtrack.PrintSamplesTable();
            }
            else
            {
                MOD_CONVERT_OUTPUT("ERROR: cannot open module file '%s' for read\n", filename);
            }
        }
    }
    catch (const TOOLerror& error)
    {
        MOD_CONVERT_OUTPUT("#ERROR - exceptions catched : %s", error.m_message);
    }
}
