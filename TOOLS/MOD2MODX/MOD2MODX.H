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

#ifndef MOD2MODX_H
#define MOD2DMOX_H

const u32 NBMAXVOICES   = 8; 

#define MOD_CONVERT_OUTPUT(FORMAT, ...) { printf (FORMAT, __VA_ARGS__ ); fprintf (stderr, FORMAT, __VA_ARGS__ ); }


struct Sample
{
    std::vector<s8> m_data;

	char m_sampleName[23]	= {0};
    u32  m_sampleLen		= 0;
    s8   m_fineTune			= 0;
	s8	 m_relativeNoteNum	= 0;
    u8   m_sampleVolume		= 0;
    u32  m_sampleLoopStart	= 0;
    u16  m_sampleLoopLength = 0;
    u32  m_fixedLen			= 0;

   
	bool isLooping() const { return m_sampleLoopLength > 2; }

    void SetName (const char* _name)
    {
        strncpy(m_sampleName, _name, sizeof(m_sampleName) - 1);
        m_sampleName[sizeof(m_sampleName) - 1] = 0;
    }

    void CheckLength ()
    {
        if (isLooping())
        {
            if (m_sampleLen > (m_sampleLoopStart + (u32)m_sampleLoopLength))
            {
                MOD_CONVERT_OUTPUT ("#WARNING cropping sample '%s' len from %u to %u\n", m_sampleName, m_sampleLen, m_sampleLoopStart + m_sampleLoopLength);
                m_sampleLen = m_sampleLoopStart + m_sampleLoopLength;
            }
        }
    }
};


class Sndtrack
{
    char                 m_title[21];
    u8                   m_songLen;
    u8                   m_restartSongLooping;
    u8                   m_patternsSequence[256];
    char                 m_format[5]; 
 
    std::vector<Sample>  m_samples;
    u16                  m_nbVoices;
   
public:
  
    bool ProcessModuleMOD (const char* _filename, bool _pack);
    void PrintSamplesTable ();
};

#endif
