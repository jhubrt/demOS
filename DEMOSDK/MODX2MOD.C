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
#include "DEMOSDK\STANDARD.H"

#define MX2M_NBINSTRUMENTS    31
#define MX2M_NBMODPATTERNROWS 64
#define MX2M_NBVOICES         4

struct MX2MSample
{
    char m_name[22];
    u16  m_sampleLen;
    u8   m_fineTune;
    u8   m_volume;
    u16  m_sampleLoopStart;
    u16  m_sampleLoopLength;
};

struct MX2MPattern
{
    u16 m_cells[MX2M_NBMODPATTERNROWS][MX2M_NBVOICES][2];    
};

struct MX2MHeader
{
    char m_title[20];

    struct MX2MSample m_sampleHeaders[MX2M_NBINSTRUMENTS];

    u8   m_songLen;
    u8   m_songRestart;

    u8   m_sequence[128];
    u8   m_format[4];
};


#ifdef __TOS__

void aMX2MdpcmTOpcm(s8* adr_, u16 count_);

#else

void aMX2MdpcmTOpcm(s8* adr_, u16 count_)
{
    u16 i;

    for (i = 1 ; i < count_ ; i++)
    {
        adr_[i] += adr_[i-1];
    }
}

#endif


void MX2MconvertDPCM2Pulse (void* buffer_, u32 size_)
{
    struct MX2MHeader* header = (struct MX2MHeader*) buffer_;
    s8* p = (s8*)(header + 1);

    u16 t;
    u16 maxPatternIndex = 0;


    for (t = 0; t < ARRAYSIZE(header->m_sequence); t++)
        if (header->m_sequence[t] > maxPatternIndex)
            maxPatternIndex = header->m_sequence[t];

    p += (maxPatternIndex + 1) * (u16) sizeof(struct MX2MPattern);

    {
        struct MX2MSample* sampleHeader = header->m_sampleHeaders;

        for (t = 0 ; t < MX2M_NBINSTRUMENTS ; t++, sampleHeader++)
        {
            u16 sampleLen = PCENDIANSWAP16(sampleHeader->m_sampleLen) << 1;

            if (sampleLen > 0)
            {
                aMX2MdpcmTOpcm(p, sampleLen - 1);
                p += sampleLen;
            }
        }
    }

    ASSERT((p - (s8*) buffer_) == size_);
}
