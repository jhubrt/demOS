/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2018 J.Hubert

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


#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <string.h>
#include <mmsystem.h>
#include <dsound.h>
#include <math.h>

#undef MEM_FREE  // collision...

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

STRUCT(FBext)
{    
    bool    enable;
    s16     width;
    s16     height;
    s16     pitch;

    u32     currentindex;

    u32     commands [1024*1024];
};

ENUM(FBextCommand)
{
    FBEXT_SETCOLOR,
    FBEXT_SETWIDTH,
    FBEXT_SETADR,
    FBEXT_SETVIDEOMODE
};

STRUCT(MiniEmul)
{
	WINdow* w;

    u16                     width;
    u16                     height;
    u16                     width_ext;

    FBext                   fbext;

    size_t                  buffermask;
    IDirectSound8*          directSound;
	IDirectSoundBuffer*     primaryBuffer;
	IDirectSoundBuffer8*    secondaryBuffer;
    u32                     lastExtraKey;
    bool                    captureSound;

    u32                     imagelut [256];
    u8                      image [emul_WIDTH*emul_HEIGHT*sizeof(u32)];
};

MiniEmul g_miniEmul;



extern "C" void EMULfbExStart (u8 _videomode, u16 _startx, u16 _starty, u16 _endx, u16 _endy, u16 _pitch)
{
    g_miniEmul.fbext.currentindex = 0;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _videomode;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _startx;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _starty;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _endx;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _endy;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _pitch;
}

extern "C" void EMULfbExSetVideoMode (u32 _startcycle, u8 _videomode)
{
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _startcycle;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = FBEXT_SETVIDEOMODE;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _videomode;
}

extern "C" void EMULfbExSetColor (u32 _startcycle, u16 _index, u16 _value)
{
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _startcycle;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = FBEXT_SETCOLOR;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _index;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _value;
}

extern "C" void EMULfbExSetAdr (u32 _startcycle, u32 _adr)
{
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _startcycle;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = FBEXT_SETADR;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _adr;
}

extern "C" void EMULfbExSetWidth (u32 _startcycle, u16 _startx, u16 _endx, u16 _pitch)
{
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _startcycle;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = FBEXT_SETWIDTH;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _startx;    
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _endx;
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = _pitch;
}

extern "C" void EMULfbExEnd (void)
{
    g_miniEmul.fbext.commands[g_miniEmul.fbext.currentindex++] = 0xFFFFFFFF;
    g_miniEmul.fbext.enable = true;
}

extern "C" void EMULfbExDisable (void)
{
    g_miniEmul.fbext.enable = false;
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
	waveFormat.nSamplesPerSec = 44100;
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

static s32 g_EMULleftChannelAmp = 64;

extern "C" void EMULleftChannel(void)
{
    const float dbleft  = -2.0f * (float)(20 - (*HW_MICROWIRE_DATA & 0x3F));   
    g_EMULleftChannelAmp = (s32) (powf(10.0f, dbleft / 10.0f) * 64.0f);
}

static s32 g_EMULrightChannelAmp = 64;

extern "C" void EMULrightChannel(void)
{
    const float dbright  = -2.0f * float(20 - (*HW_MICROWIRE_DATA & 0x3F));   
    g_EMULrightChannelAmp = (s32) (powf(10.0f, dbright / 10.0f) * 64.0f);
}

extern "C" void EMULplaysound (void* _data, u32 _byteslength, u32 _offset)
{
    void* bufferPtr = NULL;
    DWORD bufferSize = 0;
    void* bufferPtr2 = NULL;
    DWORD bufferSize2 = 0;


    HRESULT result = g_miniEmul.secondaryBuffer->Lock(_offset, _byteslength, (void**)&bufferPtr, (DWORD*)&bufferSize, (void**)&bufferPtr2, (DWORD*)&bufferSize2, 0);
    ASSERT (result == DS_OK);
    ASSERT (bufferPtr2 == NULL);

    u32 t;
    u8* d = (u8*)bufferPtr;
    s8* s = (s8*)_data;

    for (t = 0 ; t < _byteslength ; t += 2)
    {
        *d++ = (s8) ( (((*s++) * g_EMULleftChannelAmp)  >> 6) + 128);
        *d++ = (s8) ( (((*s++) * g_EMULrightChannelAmp) >> 6) + 128);
    }
 
	result = g_miniEmul.secondaryBuffer->Unlock((void*)bufferPtr, bufferSize, bufferPtr2, bufferSize2);
    ASSERT (result == DS_OK);

    if (g_miniEmul.captureSound)
    {
        static FILE* file = NULL;

        if (file == NULL)
        {
            file = fopen (".\\_logs\\dmadump.raw", "wb");
        }

        fwrite (_data, 1, _byteslength, file);

        WINsetColor       (g_miniEmul.w, 255, 0, 0);
        WINfilledRectangle(g_miniEmul.w, 5, 5, 30, 30);
    }
}


extern "C" void EMULgetSound (void* _data, u32 _byteslength)
{
    void* bufferPtr = NULL;
    DWORD bufferSize = 0;


    HRESULT result = g_miniEmul.secondaryBuffer->Lock(0, _byteslength, (void**)&bufferPtr, (DWORD*)&bufferSize, NULL, 0, DSBLOCK_ENTIREBUFFER);    
    ASSERT (result == DS_OK);

    {
        u32 t;
        u8* d = (u8*)bufferPtr;
        s8* s = (s8*)_data;

        for (t = 0 ; t < _byteslength ; t++)
        {
            *s++ = (*d++) - 128;
        }
    }

    // Unlock the secondary buffer after the data has been written to it.
	result = g_miniEmul.secondaryBuffer->Unlock((void*)bufferPtr, bufferSize, NULL, 0);
    ASSERT (result == DS_OK);
}



extern "C" u32 EMULgetPlayOffset (void)
{
    DWORD playCursor = 0, writeCursor = 0;

    g_miniEmul.secondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor);

    return writeCursor;
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
	waveFormat.wBitsPerSample  = 8;
	waveFormat.nChannels       = nbChannels;
	waveFormat.nBlockAlign     = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize          = 0;

	// Set the buffer description of the secondary sound buffer that the wave file will be loaded onto.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = _length * 2; // double buffer
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


extern "C" void EMULinit (void* _buffer, s16 _width, s16 _height, s16 _width_ext)
{
	WINinitParam param;
    bool result;

    DEFAULT_CONSTRUCT(&g_miniEmul);

	param.x = param.y = WINDOW_CENTER;
    param.w = _width  == -1 ? EMUL_WINDOW_WIDTH  : _width;
    param.h = _height == -1 ? EMUL_WINDOW_HEIGHT : _height;
	param.title = NULL;
	param.hInstance = NULL;

    g_miniEmul.width  = (u16) param.w;
    g_miniEmul.height = (u16) param.h;

    memset (g_miniEmul.image, 0, sizeof(g_miniEmul.image));

    param.w += _width_ext;

	g_miniEmul.w = WINconstruct (&param);

    result = InitializeDirectSound(&g_miniEmul);
    ASSERT (result);    
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

static void emulfromChunkToRVB (void* _base, u8 _videomode)
{
    u8*  source       = (u8*)  _base;
    u32* dest         = (u32*) g_miniEmul.image;
    u32* commands     = g_miniEmul.fbext.commands;
    u8   videomode    = (u8)  *commands++;
    u16  startx       = (u16) *commands++;
    u16  starty       = (u16) *commands++;
    u16  endx         = (u16) *commands++;
    u16  endy         = (u16) *commands++;
    u16  pitch        = (u16) *commands++;
    u32  currentcycle = 0;
    u16  y;
    u8   nbP = emulVideoMode2NBP(videomode);


    ASSERT(g_miniEmul.fbext.enable);
    ASSERT(nbP <= 4);

    for (y = 0 ; y < emul_HEIGHT ; y++)
    {
        u16 planes[4];
        u16 x;
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
            u16 p,i;

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
                    startx = (u16) *commands++;
                    endx   = (u16) *commands++;
                    pitch  = (u16) *commands++;
                    break;

                case FBEXT_SETADR:
                    source = (u8*) *commands++;
                    break;

                default:
                    ASSERT(0);
                }
            }

            if ((x & 15) == 0)
            {
                if ((x >= startx) && (x <= endx) && (y >= starty) && (y <= endy))
                {
                    for (p = 0 ; p < nbP ; p++)
                    {
                        planes[p] = PCENDIANSWAP16( *s );
                        s++;
                    }

                    for (i = 0 ; i < 8 ; i++)
                    {
                        u8  pix1 = 0;
                        u8  pix2 = 0;
                        u8  val  = 1 << nbP;


                        for (p = 0 ; p < nbP ; p++)
                        {
                            pix1 |= (planes[p] & 0x8000) != 0 ? val : 0;
                            pix1 >>= 1;
                            planes[p] <<= 1;
                        }

                        for (p = 0 ; p < nbP ; p++)
                        {
                            pix2 |= (planes[p] & 0x8000) != 0 ? val : 0;
                            pix2 >>= 1;
                            planes[p] <<= 1;
                        }

                        {
                            u32 v1 = ST2RVBcolor( HW_COLOR_LUT[pix1] );                        
                            u32 v2 = ST2RVBcolor( HW_COLOR_LUT[pix2] );

                            if (nbP == 4)
                            {
                                *d++ = v1;
                                *d++ = v1;
                                *d++ = v2;
                                *d++ = v2;
                            }
                            else
                            {
                                *d++ = v1;
                                *d++ = v2;
                            }
                        }
                    }
                }
                else
                {
                    u32 v = ST2RVBcolor( HW_COLOR_LUT[0] );   
                    
                    for (i = 0 ; i < 8 ; i++)
                    {
                        *d++ = v;
                        if (nbP == 4)
                        {
                            *d++ = v;
                        }
                    }
                }
            }

            currentcycle++;
        }

        if ((y >= starty) && (y <= endy))
        {
            if (nbP != 1)
            {
                memset(dest + emul_WIDTH, 0, emul_WIDTH * sizeof(u32));
                dest += emul_WIDTH;
            }
            source += pitch;
        }

        dest += emul_WIDTH;
    }
}


extern "C" void EMULwait (u32 _ms)
{
    Sleep(_ms);
}

extern "C" void EMULrender ()
{
    void* adr       = (void*) SYSreadVideoBase();
    u8    videomode = *HW_VIDEO_MODE;
    u16   steaddpixels = 0;


    ST2PCLut(HW_COLOR_LUT, g_miniEmul.imagelut);

    if ((*HW_VIDEO_PIXOFFSET != 0) && (*HW_VIDEO_PIXOFFSET_HIDDEN == 0))
    {
        steaddpixels = 16;
    }

    if (g_miniEmul.fbext.enable)
    {
        emulfromChunkToRVB (adr, videomode);
        WINdrawImage (g_miniEmul.w, g_miniEmul.image, emul_WIDTH, emul_HEIGHT, 32, NULL, 0, 0);
    }
    else
    {
        u16  width  = 0;
        u16  height = 0;
        u16  pitch  = 0;

        switch (videomode)
        {
        case HW_VIDEO_MODE_1P:
            width  = 640 + steaddpixels;
            height = 400;
            pitch  = width / 8;
            BITfromChunk1PTo8b (adr, pitch, NULL, g_miniEmul.image, width, height, width);
            break;

        case HW_VIDEO_MODE_2P:
            width  = 640 + steaddpixels;
            height = 200;
            pitch  = width / 4;
            BITfromChunk2PTo8b (adr, pitch, NULL, g_miniEmul.image, width, height, width);
            break;

        case HW_VIDEO_MODE_4P:
            width  = 320 + steaddpixels;
            height = 200;
            pitch  = width / 2;
            BITfromChunk4PTo8b (adr, pitch, NULL, g_miniEmul.image, width, height, width);
            break;
        }

        WINdrawImage (g_miniEmul.w, g_miniEmul.image, width, height, 8, (u8*) g_miniEmul.imagelut, (g_miniEmul.width - width) / 2, (g_miniEmul.height - height) / 2);
    }

    WINrender (g_miniEmul.w);

    {
        u32 extrakey = WINgetKeyExtraState (g_miniEmul.w);

        if ( extrakey != g_miniEmul.lastExtraKey )
        {
            g_miniEmul.lastExtraKey = extrakey;

            switch ( extrakey )
            {
            case VK_PAUSE:
                g_miniEmul.captureSound ^= 1;
                break;
            case VK_END:
                TRACsaveLog("logPC.txt");
                break;
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
            case HW_BLITTER_OP_NOT_S:               dw = ~sw;            break;
            case HW_BLITTER_OP_NOT_S_2:             dw = ~sw;            break;
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

        htone++;

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

extern "C" void EMUL_ROR_W (EMULregister* _reg, u8 _shift)
{
    if (_shift > 0)
    {
	    u16 bits = _reg->w;
	
        _shift &= 15;

        bits <<= 16 - _shift;

	    _reg->w >>= _shift;	
    	_reg->w |= bits;
    }
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
