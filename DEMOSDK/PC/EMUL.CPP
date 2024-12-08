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

#ifndef _CRT_SECURE_NO_WARNINGS
#	define _CRT_SECURE_NO_WARNINGS
#endif

#include <windows.h>
#include <string.h>
#include <mmsystem.h>
#include <dsound.h>
#include <math.h>
#include <malloc.h>
#include <vector>
#include <conio.h>

#undef ARRAY_SIZE // collision...
#undef MEM_FREE   // collision...

extern "C"
{
    #include "DEMOSDK\BASTYPES.H"
    #include "DEMOSDK\STANDARD.H"
    #include "DEMOSDK\HARDWARE.H"
    #include "DEMOSDK\SYSTEM.H"
    #include "DEMOSDK\TRACE.H"
    
    #include "DEMOSDK\PC\WINDOW.H"
    #include "DEMOSDK\PC\EMUL.H"
    #include "DEMOSDK\PC\BITCONVR.H"
}


#define emul_WIDTH  1024
#define emul_HEIGHT (313*2)

#define emul_SOUND_FREQ 50000

#define EMUL_YM_CHIPFREQ 2*1000*1000

#define EMUL_MAX_FRAMETIME 20

extern "C"
{
#   include "EXTERN\sound.c"
}
    

enum emulVideoMode
{
    emulVideoMode_NORMAL,
    emulVideoMode_EXTENDED,
    emulVideoMode_DISABLE
};


struct FBext
{    
    emulVideoMode mode;
    s16     width;
    s16     height;
    s16     pitch;

    u32     currentindex;

    u32     commands [1024*1024];
};

enum FBextCommand
{
    FBEXT_SETCOLOR,
    FBEXT_SETWIDTH,
    FBEXT_SETADR,
    FBEXT_SETPIXOFFSET,
    FBEXT_SETVIDEOMODE
};

struct emulTraceValue
{
    const static u32 LENGTH = 319;

    const s32 m_min;
    const s32 m_max;
    const u32 m_color;

    u32 m_index;

    std::vector<s32> m_values;

    emulTraceValue (s32 _min, s32 _max, u32 _color) 
        : m_min(_min)
        , m_max(_max)
        , m_color(_color)
        , m_index (0)
    {
        m_values.reserve(LENGTH);
    }

    void clear()
    {
        m_values.resize(0);
        m_index = 0;
    }

    void push(s32 _value)
    {
        if (_value < m_min)
        {
            _value = m_min;
        }
        else if (_value > m_max)
        {
            _value = m_max;
        }

        if (m_values.size() < LENGTH)
        {
            m_values.push_back(_value);
        }
        else
        {
            m_values[m_index++] = _value;
            if (m_index >= LENGTH)
            {
                m_index = 0;
            }
        }
    }
};

struct EMULlmcMixer
{
    enum Enum
    {
        NO_YM   = 0x0,
        YM      = 0x1,
        YM_12DB = 0x2
    };
};

struct MiniEmul
{
	WINdow* w;

    u16                     width;
    u16                     height;
    u16                     width_ext;
    bool                    vstretch;
    bool                    updateRequest;
    u32                     lastTimeUpdate;
    void*                   latchvideobuffer;

    FBext                   fbext;

    size_t                  buffermask;
    IDirectSound8*          directSound;
	IDirectSoundBuffer*     primaryBuffer;
	IDirectSoundBuffer8*    secondaryBuffer;
    u32                     lastExtraKey;
    bool                    captureSound;
    bool                    mono;
    bool                    fastforward;
    bool                    drawcolors;
    u16                     pause;

    u8                      lastmode;
    u32                     imagelut [256];
    u8                      image [emul_WIDTH*emul_HEIGHT*sizeof(u32)];
    s16                     ymbuffer [emul_SOUND_FREQ / 50];
    s16                     ymbuffercopy [(emul_SOUND_FREQ / 50) * 2];
    s16                     lastdelay = 0;
    s32                     LMCleftChannelAmp;
    s32                     LMCrightChannelAmp;    
    EMULlmcMixer::Enum      LMCmixer;

    std::vector<emulTraceValue*> m_tracedValues;
};

static MiniEmul g_miniEmul;

static u8 emulYMselectedReg = 0;

static void emulfromChunkToRVB (void* _base, u8 _videomode);

extern "C" void emulYMselectReg(u8 _regnum)
{
    emulYMselectedReg = _regnum;
}

extern "C" void emulYMwriteReg(u8 _data)
{
    Sound_WriteReg (emulYMselectedReg, _data);
}

static uint8_t emuYMinPortCallback (int port_id, void* user_data)
{
    return 0;
}

static void emuYMoutPortCallback (int port_id, uint8_t data, void* user_data)
{
}


static void emuInitYM (void)
{
    Ym2149_Init();
}

#if EMUL_TRACED_VALUE_ENABLE

extern "C" void EMULtraceNewValue (u16 _valueindex, s32 _min, s32 _max, u8 _r, u8 _g, u8 _b)
{
    g_miniEmul.m_tracedValues.resize(_valueindex + 1, NULL);
    ASSERT(g_miniEmul.m_tracedValues[_valueindex] == NULL);
    g_miniEmul.m_tracedValues[_valueindex] = new emulTraceValue(_min, _max, (_r << 16) | (_g << 8) | _b);
}

extern "C" void EMULtracePushValue (u16 _valueindex, s32 _val)
{
    g_miniEmul.m_tracedValues[_valueindex]->push(_val);
}

extern "C" void EMULtraceClearValue (u16 _valueindex)
{
    g_miniEmul.m_tracedValues[_valueindex]->clear();
}

#endif

extern "C" void EMULsetVStretch(bool _vstretch)
{
    g_miniEmul.vstretch = _vstretch;
}

extern "C" void EMULfbStdEnable(void)
{
    g_miniEmul.fbext.mode = emulVideoMode_NORMAL;
}

extern "C" void EMULfbDisableAll(void)
{
    g_miniEmul.fbext.mode = emulVideoMode_DISABLE;
}


extern "C" void EMULfbExStart (u8 _videomode, u16 _startcyclex, u16 _starty, u16 _endcyclex, u16 _endy, u16 _pitch, u16 _pixoffset)
{
    g_miniEmul.fbext.currentindex = 0;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _videomode;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _startcyclex;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _starty;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _endcyclex;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _endy;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _pitch;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _pixoffset;
}

extern "C" void EMULfbExSetVideoMode (u32 _execcycle, u8 _videomode)
{
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _execcycle;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = FBEXT_SETVIDEOMODE;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _videomode;
}

extern "C" void EMULfbExSetPixOffset (u32 _execcycle, u8 _offsetx)
{
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _execcycle;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = FBEXT_SETPIXOFFSET;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _offsetx;
}

extern "C" void EMULfbExSetColor (u32 _execcycle, u16 _index, u16 _value)
{
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _execcycle;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = FBEXT_SETCOLOR;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _index;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _value;
}

extern "C" void EMULfbExSetAdr (u32 _execcycle, u32 _adr)
{
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _execcycle;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = FBEXT_SETADR;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _adr;
}

extern "C" void EMULfbExSetWidth (u32 _execcycle, u16 _startx, u16 _endx, u16 _pitch)
{
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _execcycle;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = FBEXT_SETWIDTH;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _startx;    
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _endx;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _pitch;
}

extern "C" void EMULfbExEnd (void)
{
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = 0xFFFFFFFF;
    g_miniEmul.fbext.mode = emulVideoMode_EXTENDED;
}

extern "C" void EMULfbExDisable (void)
{
    g_miniEmul.fbext.mode = emulVideoMode_NORMAL;
    g_miniEmul.lastmode   = -1;
}

extern "C" u32 EMULfbExComputeCycle (u8 _videosync, u16 _x, u16 _y)
{
    u32 cyclesperline = 0;

    switch (_videosync)
    {
    case HW_VIDEO_SYNC_50HZ:
        cyclesperline = 512;
        break;

    case HW_VIDEO_SYNC_60HZ:
        cyclesperline = 508;
        break;

    case HW_VIDEO_SYNC_EXTERN:
        cyclesperline = 256; /* ? */
        break;

    default: ASSERT(0);
    }

    return ((u32)_y * cyclesperline) + (u32)_x;
}


extern "C" WINdow* EMULgetWindow (void)
{
	return g_miniEmul.w;
}


static bool InitializeDirectSound(MiniEmul* _emul)
{
	HRESULT result;
	DSBUFFERDESC bufferDesc;
	WAVEFORMATEX waveFormat;


	// Initialize the direct sound interface pointer for the default sound device.
	result = DirectSoundCreate8(NULL, &(_emul->directSound), NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Set the cooperative level to priority so the format of the primary sound buffer can be modified.
    result = _emul->directSound->SetCooperativeLevel((HWND)WINgetWindowHandle(_emul->w), DSSCL_PRIORITY);
	if(FAILED(result))
	{
		return false;
	}

	// Setup the primary buffer description.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = NULL;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// Get control of the primary sound buffer on the default sound device.
	result = _emul->directSound->CreateSoundBuffer(&bufferDesc, &(_emul->primaryBuffer), NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Setup the format of the primary sound bufffer.
	// In this case it is a .WAV file recorded at 44,100 samples per second in 16-bit stereo (cd audio format).
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = emul_SOUND_FREQ;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	// Set the primary buffer to be the wave format specified.
	result = _emul->primaryBuffer->SetFormat(&waveFormat);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}

extern "C" void EMULmixer(void)
{
    g_miniEmul.LMCmixer = (EMULlmcMixer::Enum)(*HW_MICROWIRE_DATA & 0x3F);
}


extern "C" void EMULleftChannel(void)
{
    const float dbleft  = -2.0f * (float)(20 - (*HW_MICROWIRE_DATA & 0x3F));   
    g_miniEmul.LMCleftChannelAmp = (s32) (powf(10.0f, dbleft / 10.0f) * 64.0f);
}

extern "C" void EMULrightChannel(void)
{
    const float dbright  = -2.0f * float(20 - (*HW_MICROWIRE_DATA & 0x3F));   
    g_miniEmul.LMCrightChannelAmp = (s32) (powf(10.0f, dbright / 10.0f) * 64.0f);
}


static int emulComputeCrossCorrelation(s16* x, s16* y, u16 n, u16 maxdelay)
{
    // Means computation
    double mx = 0.0, my = 0.0;
    for (int i = 0 ; i < n ; i++) 
    {
        mx += x[i];
        my += y[i];
    }
    mx /= n;
    my /= n;

    // Denominators computation
    double sx = 0.0, sy = 0.0;
    for (int i = 0 ; i < n ; i++) 
    {
        const double xval = x[i] - mx;
        const double yval = y[i] - my;

        sx += xval * xval;
        sy += yval * yval;
    }
    double d = sqrt(sx*sy);

    // Compute the best correlation 
    double maxr = 0.0;
    int bestdelay = 0;

    for (int delay = 0 ; delay < maxdelay; delay++) 
    {
        double sxy = 0.0;
        for (int i = 0; i < n; i++) 
        {
            int j = i + delay;
            if (j < 0 || j >= n)
                sxy += (x[i] - mx) * (-my);
            else
                sxy += (x[i] - mx) * (y[j] - my);           
        }
    
        double r = sxy / d;
        r -= (double) delay / (double) maxdelay * 0.01;  // favor left side

        if (r > maxr)
        {
            bestdelay = delay;
            maxr = r;
        }
    }
    
    return bestdelay;
}


extern "C" void EMULdrawYMbuffer(s16 _x, s16 _y, bool _sync)
{
    u16 len = ARRAYSIZE(g_miniEmul.ymbuffer);

    WINsetColor(g_miniEmul.w, 0, 0, 0);
    WINfilledRectangle (g_miniEmul.w, _x, _y, _x + len + 1, _y + 256);

    {
        u16 drawlen = len;
        u16 x = _x;

        if (_sync)
        {
            s16 delay = emulComputeCrossCorrelation(g_miniEmul.ymbuffer, g_miniEmul.ymbuffercopy, len + g_miniEmul.lastdelay, len / 2);
            memcpy(&g_miniEmul.ymbuffercopy[delay], g_miniEmul.ymbuffer, len * sizeof(s16));
            memset(g_miniEmul.ymbuffercopy, 0, delay * sizeof(s16));

            if (delay > 0)
            {
                x += delay;
                drawlen -= delay;
            }

            g_miniEmul.lastdelay = delay;

            WINsetColor(g_miniEmul.w, 0, 255, 0);
            WINline(g_miniEmul.w, x, _y, x, _y + 120);
        }

        WINsetColor(g_miniEmul.w, 192, 192, 192);

        u16 lasty = (g_miniEmul.ymbuffer[0] >> 8) + _y;

        for (u16 t = 0; t < drawlen; t++, x++)
        {
            s16 y = (g_miniEmul.ymbuffer[t] >> 8) + _y;

            WINline(g_miniEmul.w, x, lasty, 1 + x, y);
            lasty = y;
        }
    }
}


extern "C" void EMULplaysound (void* _data, u32 _nbSamples, u32 _offset)
{
    void* bufferPtr = NULL;
    DWORD bufferSize = 0;
    void* bufferPtr2 = NULL;
    DWORD bufferSize2 = 0;

    u32 ymbufferlen = 0; 
    u8  shift = g_miniEmul.LMCmixer == EMULlmcMixer::YM_12DB ? 2 : 0;

   
    ASSERT((_nbSamples >> 1) <= sizeof(g_miniEmul.ymbuffer));
    
    _offset <<= 1;

    if (g_miniEmul.LMCmixer == EMULlmcMixer::NO_YM)
    {
        memset (g_miniEmul.ymbuffer, 0, sizeof(g_miniEmul.ymbuffer));
    }
    else
    {
        ymsample v[3];

        do
        {
            s16 value = YM2149_NextSample(v) >> shift;
            g_miniEmul.ymbuffer[ymbufferlen] = value;
            ymbufferlen++;
        } 
        while (ymbufferlen < (_nbSamples >> 1));
    }

    s16 buffercopy[4096];

    ASSERT(_nbSamples < ARRAYSIZE(buffercopy)*2);

    u32 t, i;
    s8* s = (s8*)_data;
    s16* d = buffercopy;
    
    const auto GAIN_MUL = 20;
    const auto GAIN_DIV = 69;

    if (g_miniEmul.mono)
    {
        for (t = 0, i = 0 ; t < _nbSamples ; t += 2, i++)
        {
            s32 ymvalue = g_miniEmul.ymbuffer[i];

            s32 value1 = ((*s++) * GAIN_MUL + ymvalue) * g_miniEmul.LMCleftChannelAmp;
            s32 value2 = ((*s++) * GAIN_MUL + ymvalue) * g_miniEmul.LMCrightChannelAmp;

            value1 = (value1 + value2) / (2 * GAIN_DIV);

            *d++ = (s16) value1;
            *d++ = (s16) value1;
        }
    }
    else
    {
        for (t = 0, i = 0; t < _nbSamples; t += 2, i++)
        {
            s32 ymvalue = g_miniEmul.ymbuffer[i];
            s32 value;

            value = ((*s++) * GAIN_MUL + ymvalue) * g_miniEmul.LMCleftChannelAmp;
            value /= GAIN_DIV;

            ASSERT(value < 32768);
            ASSERT(value > -32769);

            *d++ = (s16) value;

            value = ((*s++) * GAIN_MUL + ymvalue) * g_miniEmul.LMCrightChannelAmp;
            value /= GAIN_DIV;

            ASSERT(value < 32768);
            ASSERT(value > -32769);

            *d++ = (s16) value;
        }
    }

    HRESULT result = g_miniEmul.secondaryBuffer->Lock(_offset, _nbSamples << 1, (void**)&bufferPtr, (DWORD*)&bufferSize, (void**)&bufferPtr2, (DWORD*)&bufferSize2, 0);
    ASSERT (result == DS_OK);
    ASSERT (bufferPtr2 == NULL);
    
    memcpy (bufferPtr, buffercopy, _nbSamples * sizeof(s16));

    result = g_miniEmul.secondaryBuffer->Unlock((void*)bufferPtr, bufferSize, bufferPtr2, bufferSize2);
    ASSERT (result == DS_OK);

    if (g_miniEmul.captureSound)
    {
        static FILE* file = NULL;

        if (file == NULL)
        {
            file = fopen (".\\_logs\\dmadump.raw", "wb");
        }

        fwrite (buffercopy, sizeof(s16), _nbSamples, file);
    }   
}

extern "C" u32 EMULgetPlayOffset (void)
{
    DWORD playCursor = 0, writeCursor = 0;

    g_miniEmul.secondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor);

    return writeCursor >> 1;
}


extern "C" void EMULcreateSoundBuffer (u32 _length, bool _stereo, u32 _dmafreq)
{
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	HRESULT result;
	IDirectSoundBuffer* tempBuffer;
    unsigned nbChannels = _stereo ? 2 : 1;


    if ( g_miniEmul.secondaryBuffer != NULL )
    {
        g_miniEmul.secondaryBuffer->Release();
        g_miniEmul.secondaryBuffer = NULL;
    }

	// Set the wave format of secondary buffer that this wave file will be loaded onto.
	waveFormat.wFormatTag      = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec  = _dmafreq;
	waveFormat.wBitsPerSample  = 16;
	waveFormat.nChannels       = nbChannels;
	waveFormat.nBlockAlign     = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize          = 0;

	// Set the buffer description of the secondary sound buffer that the wave file will be loaded onto.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = _length * 2 * 2; // double buffer in 16 bits
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// Create a temporary sound buffer with the specific buffer settings.
	result = g_miniEmul.directSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
    ASSERT ( result == DS_OK );

	// Test the buffer format against the direct sound 8 interface and create the secondary buffer.
	result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&g_miniEmul.secondaryBuffer);
    ASSERT ( result == DS_OK );

	// Release the temporary buffer.
	tempBuffer->Release();
	tempBuffer = 0;

	// Create a temporary buffer to hold the wave file data.
	// Lock the secondary buffer to write wave data into it.
    void* bufferPtr = NULL;
    DWORD bufferSize = 0;

    result = g_miniEmul.secondaryBuffer->Lock(0, _length * 2, (void**)&bufferPtr, (DWORD*)&bufferSize, NULL, 0, DSBLOCK_ENTIREBUFFER);
    ASSERT ( result == DS_OK );

    STDmset (bufferPtr, 0, bufferSize);
    
    // Unlock the secondary buffer after the data has been written to it.
	result = g_miniEmul.secondaryBuffer->Unlock((void*)bufferPtr, bufferSize, NULL, 0);
    ASSERT ( result == DS_OK );
	
    // Set position at the beginning of the sound buffer.
	result = g_miniEmul.secondaryBuffer->SetCurrentPosition(0);
    ASSERT ( result == DS_OK );

	// Set volume of the buffer to 100%.
	result = g_miniEmul.secondaryBuffer->SetVolume(DSBVOLUME_MAX);
    ASSERT ( result == DS_OK );

	// Play the contents of the secondary sound buffer.
	result = g_miniEmul.secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
    ASSERT ( result == DS_OK );   
}


static void ShutdownDirectSound(MiniEmul* _emul)
{
    // Release the secondary sound buffer.
	if (_emul->secondaryBuffer)
	{
		_emul->secondaryBuffer->Release();
		_emul->secondaryBuffer = NULL;
	}
    
    // Release the primary sound buffer pointer.
	if (_emul->primaryBuffer)
	{
		_emul->primaryBuffer->Release();
		_emul->primaryBuffer = NULL;
	}

	// Release the direct sound interface pointer.
	if(_emul->directSound)
	{
		_emul->directSound->Release();
		_emul->directSound = NULL;
	}
}


extern "C" void EMULinit (void* _buffer, s16 _width, s16 _height, s16 _width_ext, char* _title)
{
	WINinitParam param;
    bool result;

    DEFAULT_CONSTRUCT(&g_miniEmul);

	param.x = param.y = WINDOW_CENTER;
    param.w = _width  == -1 ? EMUL_WINDOW_WIDTH  : _width;
    param.h = _height == -1 ? EMUL_WINDOW_HEIGHT : _height;
	param.title = _title;
	param.hInstance = NULL;

    g_miniEmul.width    = (u16) param.w;
    g_miniEmul.height   = (u16) param.h;
    g_miniEmul.vstretch = false;
    g_miniEmul.lastmode = 0;
    g_miniEmul.updateRequest = false;
    g_miniEmul.latchvideobuffer = NULL;

    memset (g_miniEmul.image, 0, sizeof(g_miniEmul.image));

    param.w += _width_ext;

	g_miniEmul.w = WINconstruct (&param);

    result = InitializeDirectSound(&g_miniEmul);
    ASSERT (result);    

    memset (g_miniEmul.ymbuffer, 0, sizeof(g_miniEmul.ymbuffer));

    g_miniEmul.LMCmixer = EMULlmcMixer::YM;
    g_miniEmul.LMCleftChannelAmp = 64;
    g_miniEmul.LMCrightChannelAmp = 64;    

    emuInitYM();
}

EXTERN_C void EMULnewFrame(void)
{
    if (g_miniEmul.fastforward)
    {
        g_miniEmul.updateRequest = true;
    }
    else
    {
        u32 newTime = timeGetTime();

        if ((newTime - g_miniEmul.lastTimeUpdate) > EMUL_MAX_FRAMETIME)
        {
            g_miniEmul.updateRequest = true;
        }
    }
}

EXTERN_C bool EMULupdateRequested(void)
{
    return g_miniEmul.updateRequest;
}

static u32 ST2RVBcolor (u16 _color)
{
    u16 color = PCENDIANSWAP16(_color);

    u8 R = (u8)(color >> 8);
    u8 G = (u8)(color >> 4);
    u8 B = (u8)(color);

    R <<= 1;
    G <<= 1;
    B <<= 1;

    R |= (R & 0x10) != 0;
    G |= (G & 0x10) != 0;
    B |= (B & 0x10) != 0;

    R &= 0xF;
    G &= 0xF;
    B &= 0xF;

    R <<= 4;
    G <<= 4;
    B <<= 4;

	return ((u32)R << 16) | ((u32)G << 8) | (u32)B;
}

static void ST2PCLut(u16* _STlut, u32* _PClut)
{
    u8 i;

    for (i = 0 ; i < 16 ; i++)
    {
        _PClut[i] = ST2RVBcolor(_STlut[i]);
    }
}

static u8 emulVideoMode2NBP(u8 _videomode)
{
    switch (_videomode)
    {
    case HW_VIDEO_MODE_1P: return 1;
    case HW_VIDEO_MODE_2P: return 2;
    case HW_VIDEO_MODE_4P: return 4;
    default: ASSERT(0);    return 0;
    }
}


extern "C" void EMULwait (u32 _ms)
{
    Sleep(_ms);
}

extern "C" void EMULcls (void)
{
    if (g_miniEmul.fbext.mode != emulVideoMode_NORMAL)
        memset(g_miniEmul.image, 0, sizeof(g_miniEmul.image));
    WINsetColor (g_miniEmul.w, 0,0,0);
    WINclear (g_miniEmul.w);
}


static void emulTraceDrawCurves(const u32 offsety, const u32 offsetx)
{
    s32 y1 = -1;

    for (const auto tracedvalue : g_miniEmul.m_tracedValues)
    {
        if (tracedvalue != NULL)
        {
            const u32 count = tracedvalue->m_values.size();
            const u32 color = tracedvalue->m_color;

            WINsetColor(g_miniEmul.w, (u8)(color >> 16), (u8)(color >> 8), (u8)color);

            for (u32 t = 0, i = tracedvalue->m_index; t < count; t++, i++)
            {
                if (i >= count)
                {
                    i = 0;
                }

                s32 y2 = (tracedvalue->m_values[i] - tracedvalue->m_min) * 199 / (tracedvalue->m_max - tracedvalue->m_min) + offsety;

                if (y1 >= 0)
                {
                    WINline(g_miniEmul.w, offsetx + t, y1, offsetx + t + 1, y2);
                }

                y1 = y2;
            }
        }
    }
}

static void emulIncPause(void)
{
    switch (g_miniEmul.pause)
    {
    case 0:     g_miniEmul.pause = 10;      break;
    case 10:    g_miniEmul.pause = 100;     break;
    case 100:   g_miniEmul.pause = 1000;    break;
    case 1000:  g_miniEmul.pause = 0;       break;
    default: ASSERT(0);
    }
}


void EMULdebugSurface(void* buffer, u16 pitch, u16 width, u16 height, u16 nbplanes)
{
    switch (nbplanes)
    {
    case 1:
        BITfromChunk1PTo8b(buffer, pitch, NULL, g_miniEmul.image, width, height, width);
        break;
    case 2:
        BITfromChunk2PTo8b(buffer, pitch, NULL, g_miniEmul.image, width, height, width);
        break;
    case 4:
        BITfromChunk4PTo8b(buffer, pitch, NULL, g_miniEmul.image, width, height, width); 
        break;
    }

    ST2PCLut(HW_COLOR_LUT, g_miniEmul.imagelut);
    WINdrawImage (g_miniEmul.w, g_miniEmul.image, width, height, 8, (u8*) g_miniEmul.imagelut, 0, 0);
    WINrender(g_miniEmul.w);
}


void EMULrender (void)
{
    void* adr = NULL;
    u8    videomode = *HW_VIDEO_MODE;
    u16   steaddpixels = 0;


    if (g_miniEmul.updateRequest == false)
    {
        return;
    }
    g_miniEmul.updateRequest = false;

    g_miniEmul.lastTimeUpdate = timeGetTime();

    adr = g_miniEmul.latchvideobuffer;
    g_miniEmul.latchvideobuffer = (void*) SYSreadVideoBase();

    if (adr == NULL)
        return;

    ST2PCLut(HW_COLOR_LUT, g_miniEmul.imagelut);

    if ((*HW_VIDEO_PIXOFFSET != 0) && (*HW_VIDEO_PIXOFFSET_HIDDEN == 0))
    {
        steaddpixels = 16;
    }

    switch (g_miniEmul.fbext.mode)
    {
    case emulVideoMode_EXTENDED:
        emulfromChunkToRVB (adr, videomode);
        WINdrawImage (g_miniEmul.w, g_miniEmul.image, emul_WIDTH, emul_HEIGHT, 32, NULL, 0, 0);
        break;
    
    case emulVideoMode_NORMAL:
    {
        u16  width  = 0;
        u16  height = 0;
        u16  pitch  = 0;


        if (videomode != g_miniEmul.lastmode) // && (g_miniEmul.vstretch) && (videomode == HW_VIDEO_MODE_2P))
        {
            WINsetColor(g_miniEmul.w, 0,0,0);
            WINclear(g_miniEmul.w);
            memset(g_miniEmul.image, 0, sizeof(g_miniEmul.image));
            g_miniEmul.lastmode = videomode;
        }

        switch (videomode)
        {
        case HW_VIDEO_MODE_1P:
            width  = 640 + steaddpixels;
            height = 400;
            pitch  = width / 8;
            BITfromChunk1PTo8b (adr, pitch, NULL, g_miniEmul.image, width, height, width);
            break;

        case HW_VIDEO_MODE_2P:
            {
            auto stretch = g_miniEmul.vstretch ? 2 : 1;

            width = 640 + steaddpixels;
            height = 200;
            pitch = width / 4;
            BITfromChunk2PTo8b(adr, pitch, NULL, g_miniEmul.image, width, height, width * stretch);
            height *= stretch;
            }
            break;

        case HW_VIDEO_MODE_4P:
            width  = 320 + steaddpixels;
            height = 200;
            pitch  = width / 2;
            BITfromChunk4PTo8b (adr, pitch, NULL, g_miniEmul.image, width, height, width);
            break;
        }

        u32 offsetx = (g_miniEmul.width  - width) / 2;
        u32 offsety = (g_miniEmul.height - height) / 2;

        WINdrawImage (g_miniEmul.w, g_miniEmul.image, width, height, 8, (u8*) g_miniEmul.imagelut, offsetx, offsety);

        emulTraceDrawCurves(offsety, offsetx);
    }
    break;
    }

    if (g_miniEmul.drawcolors)
    {
        u16 t, i = 0;
        u16* colorlut = &(g_STHardware.reg_HW_COLOR_LUT);

        for (t = 0; t < 16; t++)
        {            
            u32 c = ST2RVBcolor( colorlut[t] ) << 8;
            WINsetColor32     (g_miniEmul.w, PCENDIANSWAP32(c));
            WINfilledRectangle(g_miniEmul.w, i, 0, i + 50, 30);
            i += 60;
        }
    }

    if (g_miniEmul.captureSound)
    {
        WINsetColor       (g_miniEmul.w, 255, 0, 0);
        WINfilledRectangle(g_miniEmul.w, 5, 5, 30, 30);
    }

    if (g_miniEmul.fastforward)
    {
        WINsetColor(g_miniEmul.w, 255, 255, 0);
        WINtext(g_miniEmul.w, 5, 5, ">>");
    }

    WINrender (g_miniEmul.w);

    {
        u32 extrakey = WINgetKeyExtraState (g_miniEmul.w);

        if ( extrakey != g_miniEmul.lastExtraKey )
        {
            g_miniEmul.lastExtraKey = extrakey;

            switch ( extrakey )
            {
            case VK_OEM_7:
                g_miniEmul.fastforward ^= true;
                // g_miniEmul.ymEmulator = (emulYMemulator) !g_miniEmul.ymEmulator;
                break;

            case VK_F11:
                g_miniEmul.mono ^= true;
                WINresetKeyExtraState(g_miniEmul.w);
                break;

            case VK_SCROLL:
                g_miniEmul.captureSound ^= 1;
                if (g_miniEmul.captureSound == 0)
                {
                    WINsetColor       (g_miniEmul.w, 0, 0, 0);
                    WINfilledRectangle(g_miniEmul.w, 5, 5, 30, 30);
                }
                WINresetKeyExtraState(g_miniEmul.w);
                break;

            case VK_END:
            {
                printf ("Your choice ?\n\n");
                printf ("[C] draw colors lut\n");
                printf ("[L] save log\n");

                while (_kbhit())
                    _getch();

                switch (_getch())
                {
                case 'C':
                case 'c':
                    g_miniEmul.drawcolors ^= 1;
                    break;

                case 'L':
                case 'l':
                    TRACsaveLog("logPC.txt");
                    break;
                }

                WINresetKeyExtraState(g_miniEmul.w);
                break;
            }

            case VK_PAUSE:			
                emulIncPause();
                WINresetKeyExtraState(g_miniEmul.w);
                break;
            }
        }

        if (g_miniEmul.pause > 0)
        {
            s16 pause = g_miniEmul.pause;
            
            while (pause > 0)
            {
                Sleep(10);
                pause -= 10;

                if (WINgetKeyExtraState(g_miniEmul.w) == VK_PAUSE)
                {
                    emulIncPause();
                    WINresetKeyExtraState(g_miniEmul.w);
                    break;
                }
            }
        }
    }

    *HW_KEYBOARD_DATA = (u8) WINgetKeyState(g_miniEmul.w);
}

extern "C" void EMULblit (void)
{
    u8   rshift = *HW_BLITTER_CTRL2 & 15;
    bool fxsr   = (*HW_BLITTER_CTRL2 & HW_BLITTER_CTRL2_FORCE_XTRA_SRC   ) != 0;
    bool nfsr   = (*HW_BLITTER_CTRL2 & HW_BLITTER_CTRL2_NO_FINAL_SRC_READ) != 0;
    bool smudge = (*HW_BLITTER_CTRL1 & HW_BLITTER_CTRL1_SMUDGE) != 0;

    u16 htone = (*HW_BLITTER_CTRL1) & 15;
    u16* s = (u16*) *HW_BLITTER_ADDR_SOURCE;
    u16* d = (u16*) *HW_BLITTER_ADDR_DEST;

    s16 incSx = *HW_BLITTER_XINC_SOURCE;
    s16 incSy = *HW_BLITTER_YINC_SOURCE;
    s16 incDx = *HW_BLITTER_XINC_DEST;
    s16 incDy = *HW_BLITTER_YINC_DEST;

    incSx >>= 1;
    incSy >>= 1;
    incDx >>= 1;
    incDy >>= 1;

    do
    {
        u16 x;
        u16 w1 = 0;
        u16 endmask = *HW_BLITTER_ENDMASK1;


        endmask = PCENDIANSWAP16 (endmask);
        
        if (fxsr)
        {
            w1 = *s;
            w1 = PCENDIANSWAP16(w1);
            s += incSx;        
        }

        for (x = *HW_BLITTER_XSIZE ; x > 0 ; x--)
        {
            u16 w2 = 0;
            u16 sw = -1;
            u16 dw;


            if ((x != 1) || (nfsr == 0))
            {
                if (((*HW_BLITTER_HOP) & HW_BLITTER_HOP_SOURCE) != 0)
                {
                    w2 = *s;
                    w2 = PCENDIANSWAP16(w2);
                }
            }

            if (smudge)
            {
                htone = ((w1 << (16 - rshift)) | (PCENDIANSWAP16(*s) >> rshift)) & 15;
            }

            switch (*HW_BLITTER_HOP)
            {
            case HW_BLITTER_HOP_BIT1:
                sw = 0xFFFF;
                break;

            case HW_BLITTER_HOP_HTONE:
                sw = HW_BLITTER_HTONE[htone & 15];
                sw = PCENDIANSWAP16(sw);
                break;

            case HW_BLITTER_HOP_SOURCE:
            case HW_BLITTER_HOP_SOURCE_AND_HTONE:
                
                sw = (w1 << (16 - rshift)) | (w2 >> rshift);

                if ((*HW_BLITTER_HOP) == HW_BLITTER_HOP_SOURCE_AND_HTONE)
                {
                    u16 ht = HW_BLITTER_HTONE[htone & 15];
                    ht = PCENDIANSWAP16(ht);
                    sw &= ht;
                }
                break;
            }

            w1 = w2;
            
            /* apply logical operation */
            dw = *d;
            dw = PCENDIANSWAP16(dw);

            switch (*HW_BLITTER_OP)
            {
            case HW_BLITTER_OP_BIT0:                dw = 0;              break;
            case HW_BLITTER_OP_S_AND_D:             dw =  sw &  dw;      break;
            case HW_BLITTER_OP_S_AND_NOT_D:         dw =  sw & ~dw;      break;
            case HW_BLITTER_OP_S:                   dw =  sw;            break;
            case HW_BLITTER_OP_NOT_S_AND_D:         dw = ~sw &  dw;      break;
            case HW_BLITTER_OP_D:                   dw = *d;             break;
            case HW_BLITTER_OP_S_XOR_D:             dw =  sw ^  dw;      break;
            case HW_BLITTER_OP_S_OR_D:              dw =  sw |  dw;      break;
            case HW_BLITTER_OP_NOT_S_AND_NOT_D:     dw = ~sw & ~dw;      break;
            case HW_BLITTER_OP_NOT_S_XOR_D:         dw = ~sw ^  dw;      break;
            case HW_BLITTER_OP_NOT_D:               dw = ~dw;            break;
            case HW_BLITTER_OP_NOT_S:               dw = ~sw;            break;
            case HW_BLITTER_OP_S_OR_NOT_D:          dw = sw  | ~dw;      break;
            case HW_BLITTER_OP_NOT_S_OR_D:          dw = ~sw |  dw;      break;
            case HW_BLITTER_OP_NOT_S_OR_NOT_D:      dw = ~sw | ~dw;      break;
            case HW_BLITTER_OP_BIT1:                dw = -1;             break;
            }

            /* end mask and destination write */
            {
                u16 dm   = PCENDIANSWAP16(*d);
                u16 dbak = dm & ~endmask;
                dw = dbak | (dw & endmask);
                dw = PCENDIANSWAP16(dw);
                *d = dw;
            }

            if (x != 1)
            {
                s += incSx;
                d += incDx;
            }

            if (x == 2)
            {
                if ( nfsr )
                {
                    s -= incSx;
                }
                endmask = *HW_BLITTER_ENDMASK3;
                endmask = PCENDIANSWAP16 (endmask);
            }
            else
            {
                endmask = *HW_BLITTER_ENDMASK2;
                endmask = PCENDIANSWAP16 (endmask);
            }
        }
        
        s += incSy;
        d += incDy;

        if ((s16)*HW_BLITTER_YINC_DEST >= 0)
            htone++;
        else
            htone--;

        (*HW_BLITTER_YSIZE)--;
    }
    while (*HW_BLITTER_YSIZE);

    *HW_BLITTER_ADDR_SOURCE = (u32) s;
    *HW_BLITTER_ADDR_DEST   = (u32) d;
}

// 68k port helpers

extern "C" u8* EMUL_B_I (EMULregister* _reg)
{
    u8* adr = (u8*) _reg->l;
    _reg->l++;

    return adr;
}


extern "C" u16* EMUL_W_I (EMULregister* _reg)
{
    u16* adr = (u16*) _reg->l;
    _reg->l += 2;

    return adr;
}

extern "C" u32* EMUL_L_I (EMULregister* _reg)
{
    u32* adr = (u32*) _reg->l;
    _reg->l += 4;

    return adr;
}

extern "C" bool EMUL_ROR_W (EMULregister* _reg, u8 _shift)
{
    bool carry = false;
    
    if (_shift > 0)
    {
	    u16 bits = _reg->w;

        _shift &= 15;

        carry = (bits & (1 << (_shift - 1))) != 0;

        bits <<= 16 - _shift;

	    _reg->w >>= _shift;	
    	_reg->w |= bits;
    }

    return carry;
}

extern "C" void EMUL_ROL_L (EMULregister* _reg, u8 _shift)
{
    if (_shift > 0)
    {
    	u32 bits = _reg->l;
	
        _shift &= 31;

        bits >>= 32 - _shift;
       
	    _reg->l <<= _shift;	
	    _reg->l |= bits;
    }
}

extern "C" void EMUL_LSL_L (EMULregister* _reg, u8 _shift)
{
    if (_shift >= 32)
    {
        _reg->l = 0;
    }
    else
    {
        _reg->l <<= _shift;
    }
}

extern "C" void EMUL_SUB_B (EMULregister* _source, EMULregister* _dest, EMUL68k* _p)
{
	u16 source = _source->b;
	u16 dest   = _dest->b;
	
	dest -= source;
	
	_dest->b = (u8) dest;
	_p->carry = (dest & 0x100) != 0;
}

extern "C" bool EMUL_BTST (u32 _data, u8 _bit)
{
    return (_data & (1 << _bit)) != 0;
}

extern "C" void EMUL_SWAP(EMULregister * _reg)
{
    u32 h = _reg->l >> 16;
    u32 l = _reg->w;

    _reg->l = (l << 16) | h;
}

extern "C" void EMULlog(char* str)
{
    OutputDebugStringA(str);
}

#pragma optimize("", on)

static void emulfromChunkToRVB (void* _base, u8 _videomode)
{
    u8*  source        = (u8*)  _base;
    u32* dest          = (u32*) g_miniEmul.image;
    u32* commands      = g_miniEmul.fbext.commands;
    u8   videomode     = (u8)  *commands++;
    u16  startcyclex   = (u16) *commands++;
    u16  starty        = (u16) *commands++;
    u16  endcyclex     = (u16) *commands++;
    u16  endy          = (u16) *commands++;
    u16  pitch         = (u16) *commands++;
    u16  pixoffset     = (u16) *commands++;
    u32  currentcycley = 0;
    u16  y;
    u8   nbP = emulVideoMode2NBP(videomode);


    ASSERT(g_miniEmul.fbext.mode == emulVideoMode_EXTENDED);
    ASSERT(nbP <= 4);

    for (y = 0 ; y < emul_HEIGHT ; y++)
    {
        u16 currentcyclex = 0;
        u16 planes[4];
        u16 x, shifterx = 0;
        u16 currentpixoffset = pixoffset;
        u16* s = (u16*) source;
        u32* d = dest;


        if (nbP != 1)
        {
            if (y == (emul_HEIGHT / 2))
            {
                break;
            }
        }

        for (x = 0 ; x < emul_WIDTH ; x++)
        {
            u32 currentcycle = currentcycley + currentcyclex;
            u16 p;


            while (currentcycle == *commands)
            {
                commands++;

                switch (*commands++)
                {
                case FBEXT_SETCOLOR:
                {
                    u16 index = (u16) *commands++;
                    u16 value = (u16) *commands++;

                    HW_COLOR_LUT[index] = value;
                }
                break;

                case FBEXT_SETVIDEOMODE:
                    videomode = (u8) *commands++;
                    nbP = emulVideoMode2NBP(videomode);
                    break;

                case FBEXT_SETWIDTH:
                    startcyclex = (u16) *commands++;
                    endcyclex   = (u16) *commands++;
                    pitch       = (u16) *commands++;
                    break;

                case FBEXT_SETADR:
                    source = (u8*) *commands++;
                    s = (u16*)source;
                    break;

                case FBEXT_SETPIXOFFSET:
                    currentpixoffset = pixoffset = (u16) *commands++;
                    break;

                default:
                    ASSERT(0);
                }
            }

            if ((currentcyclex >= startcyclex) && (currentcyclex <= endcyclex) && (y >= starty) && (y <= endy))
            {
                u8 pix = 0;
                u8 val = 1 << nbP;

                if ((shifterx & 15) == 0)
                {
                    for (p = 0 ; p < nbP ; p++)
                    {
                        planes[p] = PCENDIANSWAP16( *s );
                        s++;
                    }

                    if (currentpixoffset > 0)
                    {
                        for (p = 0 ; p < nbP ; p++)
                        {
                            planes[p] <<= currentpixoffset;
                            shifterx = currentpixoffset;
                        }

                        currentpixoffset = 0;
                    }
                }
                shifterx++;

                switch (nbP)
                {
                case 4:
                    pix = ((planes[3] & 0x8000) >> 12) | ((planes[2] & 0x8000) >> 13) | ((planes[1] & 0x8000) >> 14) | ((planes[0] & 0x8000) >> 15);
                    planes[3] <<= 1;
                    planes[2] <<= 1;
                    planes[1] <<= 1;
                    planes[0] <<= 1;
                    break;
                case 2:
                    pix = ((planes[1] & 0x8000) >> 14) | ((planes[0] & 0x8000) >> 15);
                    planes[1] <<= 1;
                    planes[0] <<= 1;
                    break;
                case 1:
                    pix = ((planes[0] & 0x8000) >> 15);
                    planes[0] <<= 1;
                    break;
                default:
                    ASSERT(0);
                }

                /*                for (p = 0 ; p < nbP ; p++)
                {
                pix |= (planes[p] & 0x8000) != 0 ? val : 0;
                pix >>= 1;
                planes[p] <<= 1;
                }*/

                {
                    u32 v = ST2RVBcolor( HW_COLOR_LUT[pix] );                        

                    *d++ = v;

                    if (nbP == 4)
                    {
                        *d++ = v;
                    }
                }
            }
            else
            {
                u32 v = ST2RVBcolor( HW_COLOR_LUT[0] );   

                *d++ = v;
                if (nbP == 4)
                {
                    *d++ = v;
                }
            }

            switch (nbP)
            {
            case 4:
                currentcyclex++;
                x++;
                break;
            case 2:
                currentcyclex += x & 1;
                break;
            case 1:
                currentcyclex += (x & 3) == 3;
                break;
            }
        }

        switch (nbP)
        {
        case 4:
        case 2:
            currentcycley += 512;
            break;
        case 1:
            currentcycley += 256;
            break;
        }

        if ((y >= starty) && (y <= endy))
        {
            source += pitch;
        }

        if (nbP != 1)
        {
            memset(dest + emul_WIDTH, 0, emul_WIDTH * sizeof(u32));
            dest += emul_WIDTH;
        }

        dest += emul_WIDTH;
    }
}
