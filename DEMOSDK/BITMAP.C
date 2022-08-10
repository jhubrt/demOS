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
