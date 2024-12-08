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
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\BITMAP.H"



void BITpl2chunk(void* _src, u16 _h, u16 _nbchunks, u16 _endlinepitch, void* _dst)
#ifdef __TOS__
;
#else
{
    u8* source = (u8*) _src;
    u8* dest   = (u8*) _dst;
    u16 planes[4];
    u16 y;


    for (y = 0 ; y < _h ; y++)
    {
        u8*  s = source;
        u16* d = (u16*) dest;
        u16 x;


        for (x = 0 ; x < _nbchunks ; x ++)
        {
            u16 i,p;


            for (p = 0 ; p < 4 ; p++)
            {
                planes[p] = 0;
            }

            for (i = 0 ; i < 8 ; i++)
            {
                u16 p;
                u8 pix1 = *s >> 4;
                u8 pix2 = *s & 0xF;

                s++;

                for (p = 0 ; p < 4 ; p++)
                {
                    planes[p] <<= 1;
                    planes[p] |= pix1 & 1;
                    pix1 >>= 1;

                    planes[p] <<= 1;
                    planes[p] |= pix2 & 1;
                    pix2 >>= 1;
                }
            }

            for (p = 0 ; p < 4 ; p++)
            {
                *d++ = PCENDIANSWAP16(planes[p]);
            }
        }

        source += (_nbchunks << 3) + _endlinepitch;
        dest   += (_nbchunks << 3) + _endlinepitch;
    }
}
#endif


void BITspreadPlane (void* _src, void* _dst, u16 _wordspacing, u16 _nbwords)
{
    u16* src = ((u16*) _src) + _nbwords - 1;
    u16* dst = ((u16*) _dst) + STDmulu(_nbwords - 1, _wordspacing);
    u16 t;


    for (t = _nbwords; t > 0; t--)
    {
        *dst = *src;
        src --;
        dst -= _wordspacing;
    }
}


/* outlined frame compression / decompress routines */

#define BIT_COMPRESS_OPCODE_X0	    0
#define BIT_COMPRESS_OPCODE_0X      6
#define BIT_COMPRESS_OPCODE_XX      2
#define BIT_COMPRESS_OPCODE_XXX     4

#define BIT_COMPRESS_OPCODE_MASK    6
#define BIT_COMPRESS_MAXREPEAT      64

#define BIT_COMPRESS_OPERATION_X0\
    bitWriteOpHeader(BIT_COMPRESS_OPCODE_X0, &lastt, &opcode, &wdata);\
    *opcode++ = frame[t++]; nbops++;

#define BIT_COMPRESS_OPERATION_0X\
    bitWriteOpHeader(BIT_COMPRESS_OPCODE_0X, &lastt, &opcode, &wdata);\
    *opcode++ = frame[t++]; nbops++;

#define BIT_COMPRESS_OPERATION_XX\
    bitWriteOpHeader(BIT_COMPRESS_OPCODE_XX, &lastt, &opcode, &wdata);\
    *wdata++ = *(u16*)&frame[t]; t += 2; ASSERT((t&1)==0); nbops++;
    /* no need to swap endian as frame is stored in big endian */

#ifdef __TOS__

#   define BIT_TRACE_CALLBACK(T, COUNT, OPERATION)

#else
	BITtraceCallback bit_g_traceCallback   = NULL;
	void*			 bit_g_traceClientData = NULL;

	void BITsetTraceCallback (BITtraceCallback _callback, void* _clientData)
	{
		bit_g_traceCallback   = _callback;
		bit_g_traceClientData = _clientData;
	}

#   define BIT_TRACE_CALLBACK(T, COUNT, OPERATION)\
		if (bit_g_traceCallback != NULL)\
			bit_g_traceCallback(T, COUNT, OPERATION, bit_g_traceClientData)
		
#endif	


static u8 bitCountToJump (u16 _count)
{	
	u16 v = (BIT_COMPRESS_MAXREPEAT - _count) << 3;
		

	if (v & 0x100)
	{
		v &= 0xFF;
		v |= 1;
	}
	
	return (u8) v;
}

static u8 bitJumpToCount (u16 _jump)
{
	u16 v = _jump;

	
	if (v & 1)
	{
		v &= 0xFE;
		v |= 0x100;
	}
	
	v = BIT_COMPRESS_MAXREPEAT - (v >> 3);

    return (u8) v;
}

static void bitWriteOpHeader(u16 op, u16* _lastt, u8** _opcode, u16** _wdata)
{
    u16  lastt = *_lastt;
    u8*  opcode = *_opcode;
    u16* wdata = *_wdata;


    ASSERT((lastt & BIT_COMPRESS_OPCODE_MASK) == 0);
    ASSERT (lastt <= 0xFF);

    /*printf ("%d %d - ", lastt, op);*/

    *opcode++ = lastt | op;
    
    /* return vars */
    *_lastt  = 0;
    *_opcode = opcode;
    *_wdata  = wdata;
}

static u16 bitstoresequence (u8* _frame, u16 _nbbytes, u16* _t, u16** _wdata)
{
	u16  count = 0;
	u16  word;
    u16* wdata = *_wdata;
    u16  t = *_t;
	
	do
	{
		word = *(u16*) &_frame[t];
		
		if (word != 0)
		{
			*wdata++ = word;    /* frame was big endian => no need to swap */
			t += 2;
			count++;
			
			if (t >= _nbbytes)
			{
				break;
			}
			
			if (count >= (BIT_COMPRESS_MAXREPEAT - 1))
			{
				break;
			}
		}
	}
	while (word != 0);

    ASSERT(count > 0);

    /* return vars */
    *_wdata = wdata;
    *_t     = t;

	return count;
}


u16 BITcompress1P (void* _frame, void* _opcode, u32 _opcodeBufferSize, void* _wdata, u32 _wdataBufferSize, u16 _pitch, u16 _frameheight, u32* _opcodeSize, u32* _wdataSize)
{
    u16 nbbytes = _pitch * _frameheight;
    u8* frame   = (u8*) _frame;
    u8* opcode  = (u8*) _opcode;
    u16* wdata  = (u16*) _wdata;
    u16 t, lastt;
    u16 nbops = 0;

    lastt = 0;

    for (t = 0 ; t < nbbytes ; )
    {
        u8 pattern;


        pattern  = (frame[t]   != 0) << 3;
        pattern |= (frame[t+1] != 0) << 2;

        if ( (t + 2) < nbbytes )
        {
            pattern |= (frame[t+2] != 0) << 1;
            pattern |=  frame[t+3] != 0;
        }

        if (pattern & 0xC)
        {
            if (lastt > 0xFF)
            {
                pattern = 15;
            }
        }

        switch (pattern)
        {
        case 0:			/* _ _ _ _ */
            t     += 4; 
            lastt += 16;
            break;

        case 1:			/* _ _ _ X */
        case 2:			/* _ _ X _ */
        case 3:			/* _ _ X X */
            t     += 2; 
            lastt += 8;
            break;

        case 4:			/* _ X _ _ */
        case 5:			/* _ X _ X */
        case 6:			/* _ X X _ */
        case 7:			/* _ X X X */
            t++; 
            BIT_COMPRESS_OPERATION_0X;
            lastt=8;
            break;

        case 8:			/* X _ _ _ */
            BIT_COMPRESS_OPERATION_X0;
            t += 3; 
            lastt = 16;
            break;

        case 9:			/* X _ _ X */
        case 10:		/* X _ X _ */
            BIT_COMPRESS_OPERATION_X0;
            t++; 
            lastt = 8;
            break;					

        case 12:		/* X X _ _ */
            BIT_COMPRESS_OPERATION_XX;
            t += 2; 
            lastt = 16;
            break;

        case 13:		/* X X _ X */
            BIT_COMPRESS_OPERATION_XX;
            lastt = 8;
            break;

        case 11:		/* X _ X X */
        case 14:		/* X X X 0 */
        case 15:		/* X X X X */
            {   
                /* BIT_COMPRESS_OPERATION_XXX */ 
                u8 jump;

                ASSERT((lastt & BIT_COMPRESS_OPCODE_MASK) == 0);
                *wdata++ = PCENDIANSWAP16(lastt);
                jump = bitCountToJump( bitstoresequence(frame, nbbytes, &t, &wdata) );
                *opcode++ = BIT_COMPRESS_OPCODE_XXX | jump;
                lastt = 8;
                nbops++;
            }
            break;

        default:    
            ASSERT(0);
        }
    }			

    *opcode++ = BIT_COMPRESS_OPCODE_XXX | bitCountToJump(BIT_COMPRESS_MAXREPEAT);

    *_opcodeSize =       opcode - (u8*) _opcode;
    *_wdataSize  = (u8*) wdata  - (u8*) _wdata;

    ASSERT(*_opcodeSize <= _opcodeBufferSize);
    ASSERT(*_wdataSize  <= _wdataBufferSize);

    return nbops;
}



void BITuncompress1P_C (void* _opcode, void* _wdata, void* _dest)
{
    u8*  opcode = (u8*)  _opcode;
    u16* wdata  = (u16*) _wdata;
    u8*  dest   = (u8*)  _dest;


    while(1)
    {
        u8 offset    = *opcode++;
        u8 operation = offset & BIT_COMPRESS_OPCODE_MASK;


        offset &= ~BIT_COMPRESS_OPCODE_MASK;
        /* printf ("%d %d - ", offset, operation); */
       
        switch(operation)
        {
        case BIT_COMPRESS_OPCODE_X0:
            dest += offset;				
            dest[0] = *opcode++;
			BIT_TRACE_CALLBACK(dest-(u8*)_dest, 1, operation);
            break;

        case BIT_COMPRESS_OPCODE_0X:
            dest += offset;
            dest[1] = *opcode++;
			BIT_TRACE_CALLBACK(dest-(u8*)_dest+1, 1, operation);
            break;

        case BIT_COMPRESS_OPCODE_XX:
            dest += offset;
            *(u16*)dest = *wdata++;   /* source and dest in big endian */
			BIT_TRACE_CALLBACK(dest-(u8*)_dest, 2, operation);
            break;

        case BIT_COMPRESS_OPCODE_XXX:
            {
                u8 nbrepeat;
                s16 step = PCENDIANSWAP16(*wdata);
				wdata++;
                
                /*printf ("%d - ", step); */
              
                nbrepeat = bitJumpToCount (offset);

                if (nbrepeat == BIT_COMPRESS_MAXREPEAT)
                {
                    return; /* END */ 
                }

                dest += step;
				BIT_TRACE_CALLBACK(dest-(u8*)_dest, nbrepeat << 1, operation);         

                if (nbrepeat > 0)
                {
					u16 t;
					
                    for (t = 1 ; t < nbrepeat ; t++)
                    {
                        *(u16*)dest = *wdata++;   /* source and dest into big endian */
                        dest += 8;
                    }

                    *(u16*)dest = *wdata++;       /* source and dest into big endian */
                }
            }

            break;
        }
    }
}

 
#define BIT_COMPRESSRLE_MAXREPEAT_LEN   16384

void BITcompressRLE1P (void* _frame, void* _wdata, u32 _wdataBufferSize, u16 _pitch, u16 _frameheight, u32* _wdatasize)
{
    u16 nbwords = (_pitch * _frameheight) >> 1;
    u16 t;
    u16* frame  = (u16*) _frame;
    u16* wdata  = (u16*) _wdata;
	u32 wdataSize;

	bool lastempty = true;
	bool currentempty, nextempty;	
	s16	 emptyjump = 0;
	u16* lastcount = NULL;

	
    for (t = 0 ; t < nbwords ; t++)
    {	
		u16 word = *frame++;
		
		currentempty = word == 0;

		if ((t + 1) >= nbwords)
		{
			nextempty = true;
		}
		else
		{
			nextempty = *frame == 0;
		}

		if (lastempty)
		{
			if (currentempty)
			{
				emptyjump -= 8;
			}
			else
			{
				if (emptyjump < 0)
				{
					*wdata++ = PCENDIANSWAP16(emptyjump);
					lastcount = wdata;
					*wdata++ = PCENDIANSWAP16(BIT_COMPRESSRLE_MAXREPEAT_LEN);
				}
                else if (lastcount == NULL)
                {
					lastcount = wdata;
					*wdata++ = PCENDIANSWAP16(BIT_COMPRESSRLE_MAXREPEAT_LEN);
                }

                *lastcount = PCENDIANSWAP16(*lastcount) - 4;
                *lastcount = PCENDIANSWAP16(*lastcount);
				*wdata++ = word;    /* no need to swap as frame is in big endian */
				
				lastempty = false;
			}
		}
		else /* last not empty */
		{
			if (currentempty && nextempty)
			{
				emptyjump = -8;
				lastempty = true;
			}
			else
			{
                *lastcount = PCENDIANSWAP16(*lastcount) - 4;
                *lastcount = PCENDIANSWAP16(*lastcount);
				*wdata++ = word;    /* no need to swap as frame is in big endian */
			}
		}
    }		

	*wdata++ = 0;
    
	wdataSize  = (u8*) wdata - (u8*) _wdata;
    ASSERT(wdataSize <= _wdataBufferSize);
    
    *_wdatasize = wdataSize;
}

void BITuncompressRLE1P_C (void* _wdata, void* _dest)
{
    s16* wdata  = (s16*) _wdata;
    u16* dest   = (u16*) _dest;
    s16	 word;


    word = PCENDIANSWAP16(*wdata);
    wdata++;

    if (word < 0)
    {
        dest -= (word >> 1);
        word = PCENDIANSWAP16(*wdata);
        wdata++;
    }

    while(word != 0)
    {
        u16 t;


        BIT_TRACE_CALLBACK(((dest-(u16*)_dest) << 1), (BIT_COMPRESSRLE_MAXREPEAT_LEN - word) >> 1, 0);

        for (t = word ; t < BIT_COMPRESSRLE_MAXREPEAT_LEN ; t += 4)
        {
            *dest = *wdata; /* do not need to swap : source and dest in big endian */
            dest += 4; 
            wdata++;
        }

        word = PCENDIANSWAP16(*wdata);
        wdata++;

        if (word != 0)
        {
            ASSERT(word < 0);
            dest -= (word >> 1);

            word = PCENDIANSWAP16(*wdata);
            wdata++;
        }
    }	
}

#ifndef __TOS__

#ifndef DEMOS_OPTIMIZED

#define BITcompressRLE4_MAXSIZE 128000UL

#define BIT_COMPRESS_TRON() 0

#if BIT_COMPRESS_TRON()

static bitDumpRLEsecondPass (s8* repeatStart, s8* repeatEnd)
{
    static int number = 0;
    FILE* file;
    char filename[256];
    s8* p;
    u32 total = 0;

    sprintf(filename, "..\\_logs\\RLEp2_%d.log", number++);

    file = fopen(filename, "wt");

    for (p = repeatStart; p < repeatEnd; p++)
    {
        if (*p >= 0)
            total += *p;
        else
            total -= *p;
    }

    fprintf (file, "Length = %d Total = %d\n\n", repeatEnd - repeatStart, total);

    for (p = repeatStart; p < repeatEnd; p++)
    {
        fprintf (file, "%d\n", *p);
    }

    fclose(file);
}
#else
#define bitDumpRLEsecondPass(repeatStart, repeatEnd)
#endif

static u8* bitRLEsecondPass(MEMallocator* allocator_, s8* repeatBase, s8* repeat, u32* data, u32* dataBase, s16 lineInc, s32 columnInc)
{
    BITrleHeader header, *h;
    u8* result;
    u8* p;
    s8* p1;

    /* Second pass : Optimize removing count 1 */

    for (p1 = repeatBase; (p1 + 1) < repeat; p1++)
    {
        if (*p1 == 1)
        {
            s16 count = 1;
            s8* p2 = p1 + 1;

            for (; p2 < repeat; p2++)
            {
                if (count >= 128)
                    break;

                if (*p2 != 1)
                    break;

                count++;
            }

            if (count > 1)
            {
                bitDumpRLEsecondPass(repeatBase, repeat);

                memmove(p1 + 1, p2, repeat - p2);
                repeat -= p2 - (p1 + 1);
                *p1 = -count;
                p1++;
            }
        }
    }

    header.repeatLen = repeat - repeatBase;
    header.dataLen = (data - dataBase) << 2;
    header.lineInc = lineInc; 
    header.columnInc = columnInc; 

    p = result = (u8*)MEM_ALLOC(allocator_, BIT_RLE_HEADERSIZE + header.dataLen + header.repeatLen);

    STDmcpy(p, &header, BIT_RLE_HEADERSIZE);
    p += BIT_RLE_HEADERSIZE;

    h = (BITrleHeader*) result;

    h->columnInc = PCENDIANSWAP32(h->columnInc);
    h->lineInc   = PCENDIANSWAP16(h->lineInc);
    h->dataLen   = PCENDIANSWAP16(h->dataLen);
    h->repeatLen = PCENDIANSWAP16(h->repeatLen);

    STDmcpy(p, dataBase, header.dataLen);
    p += header.dataLen;

    STDmcpy(p, repeatBase, header.repeatLen);
    p += header.repeatLen;

    MEM_FREE(allocator_, repeatBase);
    MEM_FREE(allocator_, dataBase);

    return result;
}


u8* BITcompressRLE4v (MEMallocator* allocator_, void* _frame, u16 _pitch, u16 _frameheight)
{
    s8*  repeatBase = (s8*)  MEM_ALLOCTEMP(allocator_, BITcompressRLE4_MAXSIZE);
    u32* dataBase   = (u32*) MEM_ALLOCTEMP(allocator_, BITcompressRLE4_MAXSIZE);

    u32* frame  = (u32*) _frame;
    s8*  repeat = repeatBase;
    u32* data   = dataBase;

    u16 offx;

    s8 count;


    for (offx = 0 ; offx < _pitch ; offx += 4)
    {
        u16 totalcount = 0;
        u32 p01last = *frame;
        u32 p01;
        bool equal;
        u16 line;

        u32* p = frame + (_pitch >> 2);

        count = 1;

        for (line = 1 ; line < _frameheight ; line++)
        {
            p01 = *p;

            equal = (p01 == p01last);

            if (equal && (count < 127))
            {
                count++;
            }
            else
            {
                totalcount += count;
                *repeat++ = count;
                *data++ = p01last;
                count = 1;
            }

            p01last = p01;

            p += _pitch >> 2;
        }

        totalcount += count;
        *repeat++ = count;
        *data++ = p01last;

        ASSERT(totalcount == _frameheight);

        *repeat++ = 0;

        frame++;
    }

    repeat--;

    return bitRLEsecondPass(allocator_, repeatBase, repeat, data, dataBase, _pitch, -STDmuls(_frameheight, _pitch) + 4);
}


u8* BITcompressRLE4h (MEMallocator* allocator_, void* _frame, u16 _pitch, u16 _frameheight)
{
    s8*  repeatBase = (s8*)  MEM_ALLOCTEMP(allocator_, BITcompressRLE4_MAXSIZE);
    u32* dataBase   = (u32*) MEM_ALLOCTEMP(allocator_, BITcompressRLE4_MAXSIZE);

    u32* frame  = (u32*) _frame;
    s8*  repeat = repeatBase;
    u32* data   = dataBase;

    u16 biplanes;
    u32 framesize = STDmulu(_pitch, _frameheight);

    s8 count;


    for (biplanes = 0 ; biplanes < 2 ; biplanes++)
    {
        u16 totalcount = 0;
        u32 p01last = *frame;
        u32 p01;
        bool equal;
        u32* p = frame + 2;

        count = 1;

        for (u32 off = 8 ; off < framesize ; off += 8)
        {
            p01 = *p;

            equal = (p01 == p01last);

            if (equal && (count < 127))
            {
                count++;
            }
            else
            {
                totalcount += count;
                *repeat++ = count;
                *data++ = p01last;
                count = 1;
            }

            p01last = p01;

            p += 2;
        }

        totalcount += count;
        *repeat++ = count;
        *data++ = p01last;

        ASSERT((totalcount << 3) == framesize);

        *repeat++ = 0;

        frame++;
    }

    repeat--;

    return bitRLEsecondPass(allocator_, repeatBase, repeat, data, dataBase, 8, 4UL - framesize);
}

#endif
#endif


void BITuncompressRLE4_C (void* _compressedBuffer, void* _dest)
{
    BITrleHeader* header = (BITrleHeader*) _compressedBuffer;

    header->columnInc = PCENDIANSWAP32(header->columnInc);
    header->lineInc   = PCENDIANSWAP16(header->lineInc);
    header->dataLen   = PCENDIANSWAP16(header->dataLen);
    header->repeatLen = PCENDIANSWAP16(header->repeatLen);

    {
        u32* data = (u32*)((u8*)_compressedBuffer + BIT_RLE_HEADERSIZE);   /* sizeof(BITrelHeader) on ST */
        s8* repeat = ((s8*)data) + header->dataLen;
        u32* dataend = (u32*)repeat;
        s8* repeatend = repeat + header->repeatLen;
        u8* dest = (u8*)_dest;
        s8   count;

        while (repeat < repeatend)
        {
            count = *repeat++;
            ASSERT(repeat <= repeatend);

            if (count == 0)
            {
                dest += header->columnInc;
                count = *repeat++;
                ASSERT(repeat <= repeatend);
                ASSERT(count != 0);
            }

            if (count > 0)
            {
                u32 data0 = *data++;
                ASSERT(data <= dataend);

                while (count-- != 0)
                {
                    ASSERT(dest >= (u8*)_dest);
                    ASSERT(dest < ((u8*)_dest + 32000));
                    *(u32*)dest = data0;
                    dest += header->lineInc;
                }
            }
            else if (count < 0)
            {
                while (count++ != 0)
                {
                    ASSERT(dest >= (u8*)_dest);
                    ASSERT(dest < ((u8*)_dest + 32000));
                    *(u32*)dest = *data++;
                    ASSERT(data <= dataend);
                    dest += header->lineInc;
                }
            }
        }
    }
}




void BITstreamInit(BITStream* _desc, void* _buffer, u32 _bufferSize)
{
	_desc->m_current = _desc->m_buffer = (u8*) _buffer;
	_desc->m_bufferSize = _bufferSize;
	_desc->m_nbframes = 0;
}

void BITstreamAddFrame (BITStreamFrameDescriptor* _framedesc, u8* _framebytes, u16* _framewords, BITStream* _stream)
{
	BITStreamFrameDescriptor* p = (BITStreamFrameDescriptor*)_stream->m_current;
	
	ASSERT((_framedesc->m_framewordsize & 1) == 0);
    ASSERT((_framedesc->m_framebytesize + _framedesc->m_framewordsize + sizeof(BITStreamFrameDescriptor) + (_stream->m_current - _stream->m_buffer)) < _stream->m_bufferSize);

    p->m_frame_index_type = PCENDIANSWAP16(_framedesc->m_frame_index_type);
	p->m_framebytesize    = PCENDIANSWAP16(_framedesc->m_framebytesize);
	p->m_framewordsize    = PCENDIANSWAP16(_framedesc->m_framewordsize);
	
	_stream->m_current += sizeof(BITStreamFrameDescriptor);

    if (_framedesc->m_framebytesize > 0)
	{
		STDmcpy (_stream->m_current, _framebytes, _framedesc->m_framebytesize);
		_stream->m_current += _framedesc->m_framebytesize;
	}
	
	if ( ((u32)(_stream->m_current)) & 1 )
	{
		_stream->m_current++;
	}		

    if (_framedesc->m_framewordsize > 0)
	{
        STDmcpy (_stream->m_current, _framewords, _framedesc->m_framewordsize);
        _stream->m_current += _framedesc->m_framewordsize;
	}
	
    _stream->m_nbframes++;
}


#ifdef DEMOS_UNITTEST
void BITunitTest (void)
{
	FILE* file;
	u16* dest = (u16*)(((u32)*HW_VIDEO_BASE_H << 16) | ((u32)*HW_VIDEO_BASE_M << 8) | ((u32)*HW_VIDEO_BASE_L));

	void* image = malloc(32000);

	*HW_VIDEO_MODE = HW_VIDEO_MODE_4P;

	file = fopen ("d:\\MANGA.PLN", "rb");
	fread(image, 32, 1, file);
	memcpy (HW_COLOR_LUT, image, 32);
	fread(image, 32000, 1, file);
	fclose (file);

	BITpl2chunk(image, 200, 20, 0, dest);

	free(image);

	while(1);
}
#endif
