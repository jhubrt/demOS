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


/*  BITMAP :
    - Implements optimized routine to unpack a picture from 4b by pixel to ST chunked planar 4bits format.
      Using 4bits pixels format achieves better compression ratio for storage 
	- Provides RLE bitplane real time decompress routine
*/

#ifndef BITMAP_H
#define BITMAP_H

/* when _endpitch line is 0, you can use _src == _dst */
void BITpl2chunk(void* _src, u16 _h, u16 _nbchunks, u16 _endlinepitch, void* _dst);

u16  BITcompress1P      (void* _frame, void* _opcode, u32 _opcodeBufferSize, void* _wdata, u32 _wdataBufferSize, u16 _pitch, u16 _frameheight, u32* _opcodeSize, u32* _wdataSize);
void BITuncompress1P_C  (void* _opcode, void* _wdata, void* _dest);

void BITcompressRLE1P     (void* _frame, void* _wdata, u32 _wdataBufferSize, u16 _pitch, u16 _frameheight, u32* _wdatasize);
void BITuncompressRLE1P_C (void* _wdata, void* _dest);

#ifdef __TOS__
void BIT1pUncompress    (void* _opcode, void* _wdata, u32 _dest);
void BIT1pRLEUncompress (void* _wdata, void* _dest);
#else
#   define BIT1pUncompress(_opcode, _wdata, _dest) BITuncompress1P_C(_opcode, _wdata, (void*)(_dest)); ASSERT((((u32)_wdata) & 1) == 0)
#   define BIT1pRLEUncompress(_wdata, _dest)       BITuncompressRLE1P_C(_wdata,_dest)
#endif

void BITspreadPlane (void* _src, void* _dst, u16 _wordspacing, u16 _nbwords);

struct BITStream_
{
	u8* 	m_buffer;
	u32		m_bufferSize;
	u8*		m_current;
	u16		m_nbframes;
};
typedef struct BITStream_ BITStream;

#define BIT_STREAM_FRAMETYPE_NOFRAME  0x0000
#define BIT_STREAM_FRAMETYPE_ERASE    0x2000
#define BIT_STREAM_FRAMETYPE_COPY     0x4000  
#define BIT_STREAM_FRAMETYPE_RLE      0x6000
#define BIT_STREAM_FRAMETYPE_COMP1    0x8000
#define BIT_STREAM_FRAMETYPE_RLEX     0xA000
#define BIT_STREAM_FRAMETYPE_COMP1X   0xC000
#define BIT_STREAM_FRAMETYPE_MASK     0xE000  

#define BIT_STREAM_FRAMEINDEX_MASK    0x1FFF  

struct BITStreamFrameDescriptor_
{
	u16 	m_frame_index_type;
	u16		m_framebytesize;
	u16		m_framewordsize;
};
typedef struct BITStreamFrameDescriptor_ BITStreamFrameDescriptor;

void BITstreamInit	   (BITStream* _desc, void* _buffer, u32 _bufferSize);
void BITstreamAddFrame (BITStreamFrameDescriptor* _framedesc, u8* _framebytes, u16* _framewords, BITStream* _stream);

#ifndef __TOS__
typedef void(*BITtraceCallback)(u16 t, u16 count, u8 operation, void* _clientdata);
void BITsetTraceCallback (BITtraceCallback _callback, void* _clientdata);
#endif

#ifdef DEMOS_UNITTEST
void BITunitTest (void);
#endif

#endif

