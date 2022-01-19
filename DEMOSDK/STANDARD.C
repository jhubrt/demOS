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


#include "DEMOSDK\BASTYPES.H"
#include "DEMOSDK\STANDARD.H"


u16 STDswap16(u16 _v)
{
    u16 v;
    u8* s = (u8*) &_v;
    u8* d = (u8*) &v;

    d[0] = s[1];
    d[1] = s[0];

    return v;
}

u32 STDswap32(u32 _v)
{
    u32 v;
    u8* s = (u8*) &_v;
    u8* d = (u8*) &v;

    d[0] = s[3];
    d[1] = s[2];
    d[2] = s[1];
    d[3] = s[0];

    return v;
}

void STDuxtoa(char* _text, u32 _value, s16 _nbchars)
{
    s16 i = 0;

	ASSERT(_nbchars >= 1);

	_text += _nbchars;

	if (_value ==  0)
	{
		*(--_text) = '0';
		_nbchars--;
	}

	for (i = _nbchars ; i >= 1 ; i--)
    {
		if (_value == 0)
		{
			*(--_text) = ' ';
		}
		else
        {
            u16 v = _value & 0xF;
            
            if ( v < 10 )
                *(--_text) = '0' + v;
            else
                *(--_text) = 'A' - 10 + v; 

			_value >>= 4;
        }
    } 
}

void STDutoa(char* _text, u32 _value, s16 _nbchars) 
{
    static u32 powof10[] = {1000000000UL, 100000000UL, 10000000UL, 1000000UL, 100000UL, 10000UL, 1000UL, 100UL, 10UL, 1UL};
    u32*  p = powof10;
    u8    i = 10;
    u8*   end = _text + _nbchars;


    while (_value < *p) 
    {
        p++;
        if (--i == 1)
            break;
    }

    while (i--) 
    {
        u32 sub = *p++;
        char n = '0';

        while (_value >= sub) 
        {
            _value -= sub;
            n++;
        }
        *_text++ = n;
    }

    while (_text < end)
    {
        *_text++ = ' ';
    }
}

void STDstoa(char* _text, s32 _value, s16 _nbchars)
{
    if ( _value < 0 )
    {
        _value = -_value;
        *_text++ = '-';
        _nbchars--;
    }

    STDutoa(_text, _value, _nbchars);
}



u16 STDstrLen(char* _str)
{
    u16 nb = 0;

    while (*_str++ != 0)
    {
        nb++;
    }

    return nb;
}


#ifdef DEMOS_LOAD_FROMHD
void STDwriteB (FILE* _file, u8  _b)
{
    bool result;
    
    result = fwrite (&_b, sizeof(_b), 1, _file) == 1;
    ASSERT(result);
}

void STDwriteW (FILE* _file, u16 _w)
{
    bool result;
    
    _w = PCENDIANSWAP16(_w);
    result = fwrite (&_w, sizeof(_w), 1, _file) == 1;
    ASSERT(result);
}

void STDwriteL (FILE* _file, u32 _l)
{
    bool result;
    
    _l = PCENDIANSWAP32(_l);
    result = fwrite (&_l, sizeof(_l), 1, _file) == 1;
    ASSERT(result);
}

void STDwrite(FILE* _file, void* _buf, u32 _size)
{
    bool result;

    result = fwrite (_buf, 1, _size, _file) == _size;
    ASSERT(result);
}
#endif

#ifndef __TOS__
static u32 seed1 = 0x12345678;
static u32 seed2 = 0x87654321;

void STDstop2300(void)
{
}

u16 STDmfrnd(void)
{
	u32 t = seed1;
    seed1 += seed2;
	seed2 = t;

	return t >> 16;
}

static u32 seed3 = 0x12345678;
static u32 seed4 = 0x87654321;

u16 STDifrnd(void)
{
	u32 t = seed3;
    seed3 += seed4;
	seed4 = t;

	return t >> 16;
}

u32 STDmulu (u16 _x, u16 _y)
{
    u32 result = (u32)_x * (u32)_y;
    return result;
}

s32 STDmuls (s16 _x, s16 _y)
{
    s32 result = (s32)_x * (s32)_y;
    return result;
}

u32 STDdivu (u32 _x, u16 _y)
{
    u32 q = _x / _y;
    u32 r = _x % _y;

    return (r << 16) | (q & 0xFFFF);
}

u32 STDdivs (s32 _x, s16 _y)
{
    s32 q = _x / _y;
    s32 r = _x % _y;

    return (r << 16) | (q & 0xFFFF);
}

void STDmset (void* _adr, u32 _value, u32 _length)
{
	memset (_adr, (u8) _value, _length);
}

void STDmcpy (void* _dest, void* _src, u32 _length)
{
    ASSERT(_length < (64*65536));
    memcpy(_dest, _src, _length);
}

void STDmcpy2 (void* _dest, void* _src, u32 _length)
{
    ASSERT((_length & 1) == 0);
    ASSERT(_length < (64*65536));
    ASSERT(((u32)_dest & 1) == 0);
    ASSERT(((u32)_src  & 1) == 0);

	memcpy(_dest, _src, _length);
}

void STDfastmset (void* _adr, u32 _value, u32 _length)
{
	u32* d = (u32*) _adr;
    u32 t;

    ASSERT((_length & 3) == 0);

    for (t = 0 ; t < _length ; t += 4)
    {
        *d++ = _value;
    }
}

void  STDcpuSetSR (u16 _status) { IGNORE_PARAM(_status); }
void* STDgetSP(void)            { return NULL; }
void* STDgetUSP(void)           { return NULL; }
void  STDsetUSP(void* _adr)     {}

#endif

#ifdef DEMOS_LOAD_FROMHD
void* STDloadfile(MEMallocator* _allocator, char* _filename)
{
    void* buffer;
    u32 size, readbytes;
    FILE* file = fopen(_filename, "rb");
    ASSERT(file != NULL);

    if (file != NULL)
    {
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        fseek(file, 0, SEEK_SET);

        buffer = MEM_ALLOCTEMP(_allocator, size);
        ASSERT(buffer != NULL);

        readbytes = fread(buffer, 1, size, file);
        ASSERT(size == readbytes);

        fclose(file);
    }

    return buffer;
}
#endif

#ifdef DEMOS_UNITTEST
static void STD_unitTest_convertions (void)
{
	char line[] = "|________|";
	s16 t;

	for (t = 0 ; t < 200 ; t += 10)
	{
		STDuxtoa (line + 1, t, 8);
		printf ("%s ", line);
	}

	for (t = 0 ; t > -200 ; t -= 10)
	{
		STDuxtoa (line + 1, t, 8);
		printf ("%s ", line);
	}
}

static void STD_unitTest_mset (void)
{
    u8* buf[2];
	u16 t;
    
    
    buf[0] = (u8*) malloc(32768UL);
    buf[1] = (u8*) malloc(32768UL);

    for (t = 0 ; t < 20000 ; t++)
    {
        u8  val   = rand();

        u16 index = rand() & 32767;
        u16 size  = rand();

        memset (buf[0],0,32768UL);
        memset (buf[1],0,32768UL);

        if (( index + size ) > 32768UL )
        {
            size = 32768UL - index;
        }

        memset  (&(buf[0][index]), val, size);
        STDmset(&(buf[1][index]), (((u32) val) << 24) | (((u32) val) << 16) | (((u32) val) << 8) | (u32) val, size);

        if ( memcmp(buf[0], buf[1], 32768UL) != 0 )
        {
            printf ("\nbuf=%p ind=%u siz=%u v=%u\n", buf[1], index, size, val);
            ASSERT(0);
        }

		if ((t & 127) == 0)
		{
			printf ("%u\n", t);
		}
    }

    free(buf[1]);
    free(buf[0]);
}

static void STD_unitTest_mcpy (void)
{
    u8* buf[3];
    u16 t;
    
    
    buf[0] = (u8*) malloc(32768UL);
    buf[1] = (u8*) malloc(32768UL);
    buf[2] = (u8*) malloc(32768UL);

    /* test STD_mcpy */
    for (t = 0 ; t < 32768UL ; t++)
    {
        buf[2][t] = (u8) t;
    }

    for (t = 0 ; t < 20000 ; t++)
    {
        u8  val    = rand();
        u16 index1 = rand() & 32767;
        u16 index2 = rand() & 32767;
        u16 size   = rand();

        memset (buf[0],0,32768UL);
        memset (buf[1],0,32768UL);

        if (( index1 + size ) > 32768UL )
        {
            size = 32768UL - index1;
        }

        if (( index2 + size ) > 32768UL )
        {
            size = 32768UL - index2;
        }

        memcpy  (&buf[0][index1], &buf[2][index1], size);
        STDmcpy(&buf[1][index1], &buf[2][index1], size);

        if ( memcmp(buf[0], buf[1], 32768UL) != 0 )
        {
            printf ("buf=%p i1=%u i2=%u siz=%u v=%u\n", buf[1], index1, index2, size, val);
            ASSERT(0);
        }
		
		if ((t & 127) == 0)
		{
			printf ("%u\n", t);
		}    }

    free(buf[2]);
    free(buf[1]);
    free(buf[0]);
}



void STD_unitTest_mcpy2 (void)
{
    u16* buf[3];
    u16 t;


    buf[0] = (u16*) malloc(65536UL);
    buf[1] = (u16*) malloc(65536UL);
    buf[2] = (u16*) malloc(65536UL);

    /* test STD_mcpy */
    for (t = 0 ; t < 32768UL ; t++)
    {
        buf[2][t] = t & 0xFF;
    }

    for (t = 0 ; t < 20000 ; t++)
    {
        u8  val    = rand();
        u16 index1 = rand() & 32767;
        u16 index2 = rand() & 32767;
        u16 size   = rand();

        memset (buf[0],0,65536UL);
        memset (buf[1],0,65536UL);

        if (( index1 + size ) > 32768UL )
        {
            size = 32768UL - index1;
        }

        if (( index2 + size ) > 32768UL )
        {
            size = 32768UL - index2;
        }

        memcpy  (&buf[0][index1], &buf[2][index1], size << 1);
        STDmcpy2(&buf[1][index1], &buf[2][index1], size << 1);

        if ( memcmp(buf[0], buf[1], 65536UL) != 0 )
        {
            printf ("buf=%p i1=%u i2=%u siz=%u v=%u\n", buf[1], index1, index2, size << 1, val);
            ASSERT(0);
        }

        if ((t & 127) == 0)
        {
            printf ("%u\n", t);
        }    
    }

    free(buf[2]);
    free(buf[1]);
    free(buf[0]);
}



void STD_unitTest_utoa (void)
{
    char temp[32];
    char temp2[32];
    u32 i;

    
    memset (temp, 0, sizeof(temp));

    for (i = 0; i < 0x20000UL; i++)
    {
        STDutoa(temp, i, 10);
        sprintf(temp2, "%-10d", i);
        ASSERT(strcmp(temp, temp2) == 0);
    }
}

void STD_unitTest (void)
{
    STD_unitTest_utoa();
    STD_unitTest_convertions();
    STD_unitTest_mcpy();
    STD_unitTest_mcpy2();
    STD_unitTest_mset();
}

#endif
