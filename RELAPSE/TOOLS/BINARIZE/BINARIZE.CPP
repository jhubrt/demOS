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
#   include "DEMOSDK\SYSTEM.H"
#   include "DEMOSDK\STANDARD.H"
#   include "DEMOSDK\COLORS.H"
#   include "DEMOSDK\BITMAP.H"

#   include "DEMOSDK\PC\WINDOW.H"
#   include "DEMOSDK\PC\BMPLOADER.H"
#   include "DEMOSDK\PC\BITCONVR.H"
}

#include <math.h>
#include <vector>
#include <map>

#include "TOOLS\BINARIZE\HELPERS.H"
#include "TOOLS\BINARIZE\HELPERSW.H"

#include "RELAPSE\SRC\INTERLUD\INTERLUD.H"


/* imported from MPP */
int bmp2mpp(const char* filename, int _mode, int ste, int extra, int doubl, int optlevel, int randseed, const char* outfilename, int raw_palette);
void mpp2bmp(const char* _mppfilename, int _mode, int ste, int extra, int doubl, const char* bmpfilename, int raw_palette);

/* allocator */
static void* stdAlloc(void* _alloc, u32 _size)
{
    IGNORE_PARAM(_alloc);
    return malloc(_size);
}

static void stdFree(void* _alloc, void* _adr)
{
    IGNORE_PARAM(_alloc);
    free(_adr);
}

MEMallocator stdAllocator = { NULL, stdAlloc, stdAlloc, stdFree };

static void convert4P320To4B (WINdow* _window, char* _sourcefilename, char* _destfilename)
{
    FILE* file = fopen(_sourcefilename, "rb");
    ASSERT(file != NULL);

    {
        u16 t, pal[16];
        BITsurface surface, surface2;
        u32 size = TOOLfileSize (file);

        u8* buffer = (u8*) malloc( size );

        u32 read = fread (buffer, 1, size, file);
        ASSERT (read == size);

        fclose (file);

        BITsurfaceConstruct(&surface);
        BITsurfaceConstruct(&surface2);

        for (t = 0 ; t < 16 ; t++)
        {
            pal[t] = PCENDIANSWAP16(COL4b2ST[t]);
        }

        BITsurfaceSetExternal (&surface, buffer, BITformat_Chunk4P, 320, (u16)(size / 160), 160);
        BITlutSetExternal (&surface.lut, BITlutFormat_STe, pal, 16);

        BITsurfaceConvert (&stdAllocator, &surface, &surface2, BITformat_4bits);

        file = fopen(_destfilename, "wb");
        fwrite (surface2.buffer, 1, size, file);
        fclose (file);

        if (_window != NULL)
        {
            BITsurfaceConvert (&stdAllocator, &surface2, &surface, BITformat_8bits);
            WINdrawImage (_window, surface.buffer, surface.width, surface.height, 8, surface.lut.data.p, 0, 0);        
            WINrender (_window);
            WINwaitForGUI(_window);
        }       

        BITsurfaceDestroy (&surface);
        BITsurfaceDestroy (&surface2);

        free (buffer);
    }
}

static void convert2P640To2B (WINdow* _window, char* _sourcefilename, char* _destfilename)
{
    FILE* file = fopen(_sourcefilename, "rb");
    ASSERT(file != NULL);

    {
        u16 t, pal[16];
        BITsurface surface, surface2;
        u32 size = TOOLfileSize (file);

        u8* buffer = (u8*) malloc( size );

        u32 read = fread (buffer, 1, size, file);
        ASSERT (read == size);

        fclose (file);

        BITsurfaceConstruct(&surface);
        BITsurfaceConstruct(&surface2);

        for (t = 0 ; t < 16 ; t++)
        {
            pal[t] = t == 0 ? -1 : 0; /*PCENDIANSWAP16(COL4b2ST[t]);*/
        }

        BITsurfaceSetExternal (&surface, buffer, BITformat_Chunk2P, 640, (u16)(size / 160), 160);
        BITlutSetExternal (&surface.lut, BITlutFormat_STe, pal, 4);

        if (_window != NULL)
        {
            BITsurfaceConvert (&stdAllocator, &surface, &surface2, BITformat_8bits);
            WINdrawImage (_window, surface2.buffer, surface2.width, surface2.height, 8, surface2.lut.data.p, 0, 0);        
            WINrender (_window);
            WINwaitForGUI(_window);
        }

        BITsurfaceDestroy (&surface);
        BITsurfaceDestroy (&surface2);

        free (buffer);
    }
}



static void binarizeLiquidWater (WINdow* _window)
{
    convert4P320To4B (_window, "DATA\\LIQUID\\WATER.DAT", "DATABIN\\LIQUID\\WATER.4B");
}

// convert4P320To4B(_window, "DATA\\LIQUID\\FONT.BIN", "DATABIN\\LIQUID\\FONT.4B");

static void binarizeIntroLogo (const char* sourceFilename_, const char* destFilename_, WINdow* window_)
{
    BITsurface surfaceSource, surfaceDest;
    u16 x,y;

    FILE* file = fopen(destFilename_, "wb");
    ASSERT(file != NULL);

    BITsurfaceConstruct(&surfaceSource);
    BITsurfaceConstruct(&surfaceDest);

    auto result = BITbmpLoad(&surfaceSource, &stdAllocator, sourceFilename_);
    ASSERT (result == BITloadResult_OK);

    BITsurfaceConvert(&stdAllocator, &surfaceSource, &surfaceDest, BITformat_Plane1P);

    u16 zero = 0;
    u8* sprite = surfaceDest.buffer;

    for (u16 s = 0; s < 7; s++, sprite += 6)
    {
        u8* src = sprite;

        for (y = 0; y < surfaceDest.height; y++)
        {
            for (x = 0; x < 6; x += 2)
            {
                u16* p = (u16*)(src + x);

                fwrite(p, 2, 1, file);
            }

            fwrite(&zero, 2, 1, file);

            src += surfaceDest.pitch;
        }
    }

    fclose(file);

    BITsurfaceDestroy(&surfaceSource);
    BITsurfaceDestroy(&surfaceDest);
}

static void binarizeLiquidFont (const char* sourceFilename_, const char* destFilename, WINdow* window_)
{
    u32 destsize = 8*8*20*2;
    u16* dest = (u16*) malloc(destsize);
    u16* final = (u16*) malloc(destsize);
    u16* p = dest;  

    BITsurface surface;

    BITsurfaceConstruct(&surface);

    BITdegasLoad(&surface, &stdAllocator, sourceFilename_);
    
    for (u16 t = 0; t < 40; t++)
    {
        u16 offset = (t / 20) * 8 * 160;
        offset += (t % 20) * 8;

        u16* s = (u16*) &surface.buffer[offset];

        for (u16 i = 0 ; i < 8 ; i++)
        {            
            *p++ = *s++;
            *p++ = *s++;
            *p++ = *s++;
            *p++ = *s++;
            s += 80-4;
        }
    }

    BITfromChunk4PTo4b (dest, 160, nullptr, final, 320, 16, 160);

    FILE* file = fopen(destFilename, "wb");
    ASSERT(file != nullptr);

    u32 written = fwrite (final, 1, destsize, file);
    ASSERT(written == destsize);

    fclose (file);

    BITsurfaceDestroy(&surface);
    free (dest);
    free (final);
}

u8& getPixel(BITsurface& surface_, u32 x_, u32 y_)
{
    return surface_.buffer[y_ * surface_.pitch + x_];
}

static void binarizeWaterFall (const char* sourceFilename_, const char* destFilename, WINdow* window_)
{   
    BITsurface surface, surfaces[2], surfaced[2];


    BITsurfaceConstruct(&surface);
    BITsurfaceConstruct(&surfaces[0]);
    BITsurfaceConstruct(&surfaces[1]);
    BITsurfaceConstruct(&surfaced[0]);
    BITsurfaceConstruct(&surfaced[1]);

    BITbmpLoad(&surface, &stdAllocator, sourceFilename_);

    u32 spectrum[32];
    u32 x, y;

    memset (spectrum, 0UL, sizeof(spectrum));

    for (y = 0 ; y < surface.height ; y++)
    {
        for (x = 0 ; x < surface.width ; x++)
        {
            u8 value = surface.buffer[y * surface.pitch + x];
            ASSERT(value < ARRAYSIZE(spectrum));
            spectrum[value]++;
        }
    }

    for (x = 0; x < ARRAYSIZE(spectrum); x++)
    {
        printf ("%d: %u\n", x, spectrum[x]);
    }

    BITsurfaceInit(&stdAllocator, &surfaces[0], BITformat_8bits, surface.width, surface.height, surface.pitch);
    BITsurfaceInit(&stdAllocator, &surfaces[1], BITformat_8bits, surface.width, surface.height, surface.pitch);

    for (y = 0 ; y < surface.height ; y++)
    {
        for (x = 0 ; x < surface.width ; x++)
        {
            u8 value = getPixel(surface, x, y);
            u8 invert = (x & 1) == (y & 1);

            switch (value)
            {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                getPixel(surfaces[0], x, y) = value;
                getPixel(surfaces[1], x, y) = value;
                break;

            case 6:
                getPixel(surfaces[invert], x, y) = 5;
                getPixel(surfaces[!invert], x, y) = 6;
                break;

            case 7:
                getPixel(surfaces[0], x, y) = 6;
                getPixel(surfaces[1], x, y) = 6;
                break;

            case 8:
                getPixel(surfaces[0], x, y) = 7;
                getPixel(surfaces[1], x, y) = 7;
                break;

            case 9:
                getPixel(surfaces[0], x, y) = 8;
                getPixel(surfaces[1], x, y) = 8;
                break;

            case 10:
                getPixel(surfaces[0], x, y) = 9;
                getPixel(surfaces[1], x, y) = 9;
                break;

            case 11:
                getPixel(surfaces[invert], x, y) = 9;
                getPixel(surfaces[!invert], x, y) = 14;
                break;

            case 15:
                getPixel(surfaces[invert], x, y) = 10;
                getPixel(surfaces[!invert], x, y) = 1;
                break;

            case 16:
            case 18:
            case 20:
            case 22:
            case 24:
            case 26:
                getPixel(surfaces[0], x, y) = ((value - 16) >> 1) + 10;
                getPixel(surfaces[1], x, y) = ((value - 16) >> 1) + 10;
                break;

            case 17:
            case 21:
                getPixel(surfaces[invert], x, y) = ((value - 16) >> 1) + 10;
                getPixel(surfaces[!invert], x, y) = ((value - 16) >> 1) + 11;
                break;

            case 19:
            case 23:
            case 25:
                getPixel(surfaces[invert], x, y) = ((value - 16) >> 1) + 11;
                getPixel(surfaces[!invert], x, y) = ((value - 16) >> 1) + 10;
                break;

            default: 
                ASSERT(0);
            }
        }
    }

    if (window_ != NULL)
    {
        BITlut lut;

        BITlutConstruct(&lut);

        u16 colors[16];

        FILE* file = fopen ("RELAPSE\\DATA\\CASCADE\\DEST.PAL", "rb");
        ASSERT(file != nullptr);
        fread(colors, 16, 2, file);
        fclose(file);

        BITlutSetExternal(&lut, BITlutFormat_STe, colors, 16);
        BITlutConvert(&stdAllocator, &lut, &surfaces[0].lut, BITlutFormat_x888);
        BITlutConvert(&stdAllocator, &lut, &surfaces[1].lut, BITlutFormat_x888);

        while (!WINisKeyHit(window_))
        {
            WINdrawImage (window_, surfaces[0].buffer, surfaces[0].width, surfaces[0].height, 8, surfaces[0].lut.data.p, 0, 0);
            WINrender (window_);
            WINdrawImage (window_, surfaces[1].buffer, surfaces[1].width, surfaces[1].height, 8, surfaces[1].lut.data.p, 0, 0);
            WINrender (window_);
        }

        WINwaitForGUI(window_);
    }

    BITsurfaceInit(&stdAllocator, &surfaced[0], BITformat_4bits, surface.width, surface.height, surface.pitch / 2);
    BITsurfaceInit(&stdAllocator, &surfaced[1], BITformat_4bits, surface.width, surface.height, surface.pitch / 2);

    BITsurfaceConvert(&stdAllocator, &surfaces[0], &surfaced[0], BITformat_4bits);
    BITsurfaceConvert(&stdAllocator, &surfaces[1], &surfaced[1], BITformat_4bits);

    char destfile2[256];

    strcpy(destfile2, destFilename);
    strcat(destfile2, "_");

    bool success;

    success = BITrawbufferSave(&surfaced[0], destFilename); ASSERT(success);
    success = BITrawbufferSave(&surfaced[1], destfile2); ASSERT(success);

    BITsurfaceDestroy(&surface);
    BITsurfaceDestroy(&surfaces[0]);
    BITsurfaceDestroy(&surfaces[1]);
    BITsurfaceDestroy(&surfaced[0]);
    BITsurfaceDestroy(&surfaced[1]);
}


static BITloadResult iffLoad (BITsurface* _surface, MEMallocator* _allocator, char* _filename)
{
    BITloadResult returnCode = BITloadResult_READERROR;
    FILE* file = fopen (_filename, "rb");

    if ( file == NULL )
        goto Error;

    BITsurfaceInit (_allocator, _surface, BITformat_8bits, 320, 256, BIT_DEFAULT_PITCH);

    BITlutConstruct(&_surface->lut);
    BITlutInit(_allocator, &_surface->lut, BITlutFormat_x888, 256);

    fseek (file, 228, SEEK_SET);

    // TODO: retrieve size from file...
    {
        u16 y;
        u8* p = _surface->buffer;

        for (y = 0 ; y < 256 ; y++)
        {
            u16 w;

            _surface->lut.data.p32[y] = y << 11;

            for (w = 0 ; w < 20 ; w++)
            {
                u16 plane[5];
                u16 i;

                for (i = 0 ; i < ARRAYSIZE(plane) ; i++)
                {
                    fread (&plane[i], sizeof(plane[0]), 1, file);
                    plane[i] = PCENDIANSWAP16(plane[i]);
                    fseek(file, 38, SEEK_CUR);
                }

                for (i = 0 ; i < 16 ; i++)
                {
                    u8  c = 0;
                    u16 j;

                    for (j = 0 ; j < 5 ; j++)
                    {
                        if ( plane[j] & 0x8000 )
                        {
                            c |= 1 << j;
                        }

                        plane[j] <<= 1;
                    }

                    *p++ = c;
                }

                fseek(file, -40*5+2, SEEK_CUR);
            }

            fseek(file,228 + y*5*40, SEEK_SET);
        }
    }

    fclose(file);
    file = NULL;

    return BITloadResult_OK;

Error:
    if ( file != NULL )
        fclose (file);

    return returnCode;
}



static void extractWaterFall( WINdow* window )
{
    if ( window != NULL )
    {
        BITsurface surface, surface2;

        BITsurfaceConstruct(&surface);
        BITsurfaceConstruct(&surface2);

        iffLoad (&surface, &stdAllocator, "DATA\\CASCADE\\WATER.IFF");
        BITsurfaceConvert (&stdAllocator, &surface, &surface2, BITformat_888);
        BITbmpSave (&surface2, "DATA\\CASCADE\\WATER.BMP");

        WINdrawImage (window, surface.buffer, surface.width, surface.height, 8, surface.lut.data.p, 0, 0);
        WINrender (window);
        WINwaitForGUI(window);

        BITsurfaceDestroy(&surface2);
        BITsurfaceDestroy(&surface);
    }
}

static void binarizeSlidePicture (char* _src, char* _destdir, WINdow* _window)
{
    BITsurface surface, surface2;

    char dir[256];
    char filename[256];
    char temp[256];
    char temp2[256];

    bool testmode = _window != NULL;


    _splitpath (_src, dir, temp, filename, NULL);
    strcat (dir, temp);

    BITsurfaceConstruct(&surface);
    BITsurfaceConstruct(&surface2);

    BITbmpLoad (&surface, &stdAllocator, _src);
    BITsurfaceFSErrorDiffuse (&stdAllocator, &surface, 5, 5, 5);

    sprintf (temp, "%s%sFS.BMP", dir, filename);

    BITbmpSave (&surface, temp);

    sprintf (temp2, "%s%s.HC", _destdir, filename);

    bmp2mpp(temp, 2, true, true, false, 9, 42, temp2, 1);

    if ( testmode )
    {
        sprintf (temp, "%s%sOUT.BMP", _destdir, filename);

        mpp2bmp(temp2, 2, true, true, false, temp, 1);

        BITbmpLoad (&surface, &stdAllocator, temp);
        BITsurfaceConvert (&stdAllocator, &surface, &surface2, BITformat_x888);

        WINdrawImage (_window, surface2.buffer, surface2.width, surface2.height, 32, NULL, (WINDOW_WIDTH - surface2.width) / 2, (WINDOW_HEIGHT - surface2.height) / 2);
        WINrender (_window);

        WINwaitForGUI(_window);
    }

    BITsurfaceDestroy(&surface2);
    BITsurfaceDestroy(&surface);
}


static void binarizeCyberFont(char* _srcfile1, char* _destfile, WINdow* _window)
{
    BITsurface surface, surface2, surfaced, surfaced2;
    u8* d;
    const int fontw = 32;
    const int fonth = 54;

    BITsurfaceConstruct(&surface);
    BITsurfaceConstruct(&surface2);
    BITsurfaceConstruct(&surfaced);
    BITsurfaceConstruct(&surfaced2);

    char srcfile2[256];

    strcpy (srcfile2, _srcfile1);
    *(strrchr(srcfile2, '.') - 1) = '2';

    BITloadResult result;

    result = BITneoLoad(&surface , &stdAllocator, _srcfile1); 
    ASSERT(result == BITloadResult_OK);

    result = BITneoLoad(&surface2, &stdAllocator, srcfile2);
    ASSERT(result == BITloadResult_OK);

    BITsurfaceInit(&stdAllocator, &surfaced, BITformat_Chunk4P, fontw, 27 * fonth, fontw / 2);

    d = surfaced.buffer;

    for (u16 l = 0 ; l < 18 ; l++)
    {
        u16 x = l % 6;
        u16 y = l / 6;
        u8* s = surface.buffer + x * (fontw / 2) + y * fonth * surface.pitch;
        u16 t;

        for (t = 0; t < fonth; t++)
        {
            memcpy(d, s, fontw / 2);
            s += surface.pitch;
            d += surfaced.pitch;
        }
    }

    for (u16 l = 0 ; l < 9 ; l++)
    {
        u16 x = l % 6;
        u16 y = l / 6;
        u8* s = surface2.buffer + x * (fontw / 2) + y * fonth * surface2.pitch;
        u16 t;

        for (t = 0; t < fonth; t++)
        {
            memcpy(d, s, fontw / 2);
            s += surface2.pitch;
            d += surfaced.pitch;
        }
    }

    BITsurfaceConvert(&stdAllocator, &surfaced, &surfaced2, BITformat_4bits);

    bool success = BITrawbufferSave(&surfaced2, _destfile);
    ASSERT(success);

    BITsurfaceDestroy(&surface);
    BITsurfaceDestroy(&surface2);
    BITsurfaceDestroy(&surfaced);
    BITsurfaceDestroy(&surfaced2);
}


static void binarizeInterlude(char* _srcfile, char* _destfile, WINdow* _window)
{
    BITsurface surface, surface8;
    BITloadResult result;
    u16 x,y,row;

    char drive[256];
    char subdir[256];
    char filename[256];

    BITlut luts[3];
    
    _splitpath(_srcfile, drive, subdir, NULL, NULL);

    sprintf (filename, "%s\\%s\\RED.PAL", drive, subdir);
    result = BITpalLoadLUT(&luts[2], &stdAllocator, filename);
    ASSERT(result == BITloadResult_OK);

    sprintf (filename, "%s\\%s\\GREEN.PAL", drive, subdir);
    result = BITpalLoadLUT(&luts[0], &stdAllocator, filename);
    ASSERT(result == BITloadResult_OK);
    
    BITsurfaceConstruct(&surface);
    BITsurfaceConstruct(&surface8);

    result = BITneoLoad(&surface , &stdAllocator, _srcfile); 
    ASSERT(result == BITloadResult_OK);

    STDmcpy(&luts[1], &surface.lut, sizeof(luts[1]));

    FILE* file = fopen(_destfile, "wb");
    ASSERT(file != NULL);

    BITsurfaceConvert(&stdAllocator, &surface, &surface8, BITformat_8bits);

    const int   width     = INTERLUDE_BALLHEIGHT;
    const int   nbsteps   = 50;
    const float widthhalf = (float)width / 2.0f;
    
    s8  offsets[nbsteps][width];
    s16 step,i;

    const int phase1steps = 3;

    for (step = 0; step < nbsteps; step++)
    {
        if (step < phase1steps)
        {
            float amp = (float)step * 20.0f / (float)phase1steps;

            for (i = -width / 2; i < width / 2; i++)
            {
                float angle = ((float)i / float(width / 2) * 0.0f) * (float)M_PI;
                float amp2  = amp * cosf(i / widthhalf * (float)M_PI / 2.0f);

                offsets[step][i + width / 2] = s8(cos(angle) * amp2);
            }
        }
        else
        {
            float freq = (float)(step - phase1steps) / 10.0f;
            float amp = float(nbsteps - phase1steps - (step - phase1steps)) / (float(nbsteps - phase1steps) / 20.f);

            for (i = -width / 2; i < width / 2; i++)
            {
                float angle = ((float)i / float(width / 2) * freq) * (float)M_PI;
                float amp2  = amp * cosf(i / widthhalf * (float)M_PI / 2.0f);

                offsets[step][i + width / 2] = s8(cos(angle) * amp2);
            }
        }
    }

    _splitpath(_destfile, drive, subdir, NULL, NULL);

    sprintf (filename, "%s\\%s\\MOVE.BIN", drive, subdir);
    FILE* file2 = fopen(filename, "wb");
    ASSERT(file2 != nullptr);
    u8 nbstepsout = (u8) nbsteps;
    u8 widthout = (u8) width;
    fwrite (&nbstepsout, sizeof(nbstepsout), 1, file2);
    fwrite (&widthout, sizeof(widthout), 1, file2);
    fwrite (offsets, sizeof(offsets), 1, file2);
    fclose(file2);

    if (_window != NULL)
    {
        BITsurface surfacetest;

        BITsurfaceConstruct(&surfacetest);

        BITsurfaceInit(&stdAllocator, &surfacetest, BITformat_8bits, 640, 480, 640);

        for (u16 t = 0 ; t < 40 ; t++)
        {
            for (i = 0 ; i < 16 ; i++)
            {
                BITsurfaceRectCopy (&surface8, 0, 0, 8, 16, &surface8, t * 8, i * 16);
            }
        }

        do
        {
            for (step = 0; step < nbsteps ; step++)
            {
                WINsetColor(_window, 0, 0, 0);
                WINclear(_window);

                memset(surfacetest.buffer, 0, surfacetest.size);

                for (i = -width / 2; i < width / 2; i++)
                {
                    BITsurfaceRectCopy(&surface8, 0, i + width / 2, 320, 1, &surfacetest, offsets[step][i + width / 2], i + 50);
                }

                WINdrawImage(_window, surfacetest.buffer, 640, 480, 8, surface8.lut.data.p32, 100, 50);
                WINrender(_window);
                WINwait(20);
                //WINwaitForGUI(_window);
            }
            WINwaitForGUI(_window);
        } 
        while (WINisClosed(_window) == false);

        BITsurfaceDestroy(&surfacetest);
    }

    PCENDIANSWAPBUFFER16(luts[0].data.p16, 16);
    PCENDIANSWAPBUFFER16(luts[1].data.p16, 16);
    PCENDIANSWAPBUFFER16(luts[2].data.p16, 16);

    for (row = 0; row < 16; row++)
    {
        u16 gradient = row >> 3;

        BITlut* lut1 = &luts[gradient];
        BITlut* lut2 = &luts[gradient + 1];
        u16 lut[16];

        COLcomputeGradient16Steps(lut1->data.p16, lut2->data.p16, 16, (row & 7) * 2, lut);

        for (y = 0; y < 16; y++)
        {
            for (x = 0; x < 8; x++)
            {
                u8 pix = surface8.buffer[surface8.pitch * y + x];
                u16 color = lut[pix] << 2;
                u8* pcolor = (u8*)&color;

                fwrite(&pcolor[1], 1, 1, file);
                fwrite(&pcolor[0], 1, 1, file);
            }
        }
    }

    fclose(file);   

    BITsurfaceDestroy(&surface);
    BITsurfaceDestroy(&surface8);
}


static void binarizeGrafikS(/*char* _srcfile,*/ char* _destfile)
{
    FILE* file = fopen(_destfile, "wb");
    ASSERT(file != NULL);
    CONST u16 NUMBERS = 240;

    for (s16 t = -NUMBERS/2; t < NUMBERS/2; t++)
    {
        for (u16 i = 0 ; i < 32 ; i++)
        {
            float x =  sin(float(t) * 1.5f / 180.f * M_PI) * float(i + 48) +  78.0f + 1.0f;
            float y = -cos(float(t) * 1.5f / 180.f * M_PI) * float(i + 48) + 100.0f + 1.0f;

            u16 off = ((int)y & 0xFFFE) * 160 + (((int)x & 0xFFF0) >> 1);
            u16 pix = 3 << (0xE - ((int)x & 0xE));

            off = PCENDIANSWAP16(off);
            pix = PCENDIANSWAP16(pix);

            fwrite (&pix, sizeof(pix), 1, file);
            fwrite (&off, sizeof(off), 1, file);
        }
    }

    fclose(file);   
}


static void binarizeGrafikSImages(char* _srcfile, char* _destfile, WINdow* _window)
{
    BITsurface surface, surface2[2], surfaceST[2];

    BITsurfaceConstruct(&surface);

    BITloadResult result = BITbmpLoad(&surface , &stdAllocator, _srcfile); 
    ASSERT(result == BITloadResult_OK);

    BITsurfaceConstruct(&surface2[0]);
    BITsurfaceConstruct(&surface2[1]);

    BITsurfaceConstruct(&surfaceST[0]);
    BITsurfaceConstruct(&surfaceST[1]);

    BITsurfaceInit (&stdAllocator, &surface2[0], BITformat_8bits, surface.width, 200, surface.pitch);
    BITsurfaceInit (&stdAllocator, &surface2[1], BITformat_8bits, surface.width, 200, surface.pitch);
    
    memset (surface2[0].buffer, 8, surface2[0].size);
    memset (surface2[1].buffer, 8, surface2[1].size);

    BITlutInit(&stdAllocator, &surface2[0].lut, BITlutFormat_x888, 256);
    BITlutInit(&stdAllocator, &surface2[1].lut, BITlutFormat_x888, 256);

    surface2[0].lut.data.p32[0]  = surface.lut.data.p32[0];
    surface2[0].lut.data.p32[1]  = surface.lut.data.p32[3];
    surface2[0].lut.data.p32[2]  = surface.lut.data.p32[5];
    surface2[0].lut.data.p32[3]  = surface.lut.data.p32[7];
    surface2[0].lut.data.p32[4]  = surface.lut.data.p32[9];
    surface2[0].lut.data.p32[5]  = surface.lut.data.p32[11];
    surface2[0].lut.data.p32[6]  = surface.lut.data.p32[13];
    surface2[0].lut.data.p32[7]  = surface.lut.data.p32[15];
    surface2[0].lut.data.p32[8]  = surface.lut.data.p32[17];
    surface2[0].lut.data.p32[9]  = surface.lut.data.p32[19];
    surface2[0].lut.data.p32[10] = surface.lut.data.p32[21];
    surface2[0].lut.data.p32[11] = surface.lut.data.p32[23];

    surface2[0].lut.data.p32[12] = surface.lut.data.p32[24];
    surface2[0].lut.data.p32[13] = surface.lut.data.p32[25];
    surface2[0].lut.data.p32[14] = surface.lut.data.p32[26];
    surface2[0].lut.data.p32[15] = surface.lut.data.p32[27];

    memcpy (surface2[1].lut.data.p, surface2[0].lut.data.p, 256*4);

    for (u16 y = 0 ; y < surface.height ; y++)
    {
        for (u16 x = 0 ; x < surface.width ; x++)
        {
            u32 t = surface.pitch * y + x;
            u8 pix = surface.buffer[t];

            if (pix < 3)
            {
                surface2[0].buffer[t] = surface2[1].buffer[t] = 0;
            }
            else if (pix < 24)
            {
                if ((x & 1) != (y & 1))
                {
                    surface2[0].buffer[t] = (pix - 1) / 2;
                    surface2[1].buffer[t] = (pix - 0) / 2;
                }
                else
                {
                    surface2[1].buffer[t] = (pix - 1) / 2;
                    surface2[0].buffer[t] = (pix - 0) / 2;
                }
            }
            else
            {
                surface2[0].buffer[t] = pix - 12;
                surface2[1].buffer[t] = pix - 12;
            }

            if (surface2[0].buffer[t] == 0)
                surface2[0].buffer[t] = 8;
            else if (surface2[0].buffer[t] == 8)
                surface2[0].buffer[t] = 0;

            if (surface2[1].buffer[t] == 0)
                surface2[1].buffer[t] = 8;
            else if (surface2[1].buffer[t] == 8)
                surface2[1].buffer[t] = 0;
        }
    }

    const int TOPLINE = 31;
    const int BOTTOMLINE = 181;

    for (u16 y = TOPLINE; y < BOTTOMLINE; y++)
    {
        u32 t = surface.pitch * y;

        memmove(&surface2[0].buffer[t], &surface2[0].buffer[t] + 8, 440);
        memmove(&surface2[1].buffer[t], &surface2[1].buffer[t] + 8, 440);
    }

    BITsurfaceConvert(&stdAllocator, &surface2[0], &surfaceST[0], BITformat_Chunk4P);
    BITsurfaceConvert(&stdAllocator, &surface2[1], &surfaceST[1], BITformat_Chunk4P);
        
    FILE* file = fopen (_destfile, "wb");

    std::swap(surfaceST[0].lut.data.p16[0], surfaceST[0].lut.data.p16[8]);

    fwrite (surfaceST[0].lut.data.p, 2, 16, file);

    for (u16 i = 0; i < 2; i++)
    {
        for (u16 t = 0; t < TOPLINE; t++)
            fwrite(surfaceST[i].buffer + 32 + surfaceST[i].pitch * t, 160, 1, file);

        for (u16 t = TOPLINE; t < BOTTOMLINE; t++)
            fwrite(surfaceST[i].buffer + 8 + surfaceST[i].pitch * t, surfaceST[i].pitch, 1, file);

        for (u16 t = BOTTOMLINE; t < surfaceST[i].height; t++)
            fwrite(surfaceST[i].buffer + 32 + surfaceST[i].pitch * t, 160, 1, file);
    }

    fclose(file);    

    if (_window != NULL)
    {
        surface.lut.data.p32[255] = 0xFF0000;

        for (u16 t = 0 ; t < surface.height ; t++)
        {
            surface.buffer[surface.pitch*t + 64] = 255;
            surface.buffer[surface.pitch*t + surface.width - 65] = 255;
        }

        for (u16 y = 0 ; y < TOPLINE ; y++)
        {
            for (u16 x = 0 ; x < 64 ; x++)
                if (surface.buffer[surface.pitch*y + x] != 17)
                    surface.buffer[surface.pitch*y + x] = 255;

            for (u16 x = surface.width-64 ; x < surface.width ; x++)
                if (surface.buffer[surface.pitch*y + x] != 17)
                    surface.buffer[surface.pitch*y + x] = 255;
        }

        for (u16 y = BOTTOMLINE ; y < surface.height ; y++)
        {
            for (u16 x = 0 ; x < 64 ; x++)
                if (surface.buffer[surface.pitch*y + x] != 17)
                    surface.buffer[surface.pitch*y + x] = 255;

            for (u16 x = surface.width-64 ; x < surface.width ; x++)
                if (surface.buffer[surface.pitch*y + x] != 17)
                    surface.buffer[surface.pitch*y + x] = 255;
        }

        s32 k = 0;

        do
        {
            WINgetMouse(_window, nullptr, nullptr, &k, nullptr);

            WINdrawImage (_window, surface2[0].buffer, surface2[0].width, surface2[0].height, 8, surface2[0].lut.data.p, (WINDOW_WIDTH - surface2[0].width) / 2, (WINDOW_HEIGHT - surface2[0].height) / 2);
            WINrender (_window);

            WINdrawImage (_window, surface2[1].buffer, surface2[1].width, surface2[1].height, 8, surface2[1].lut.data.p, (WINDOW_WIDTH - surface2[1].width) / 2, (WINDOW_HEIGHT - surface2[1].height) / 2);
            WINrender (_window);
        }
        while(k == 0);

        do
        {
            WINgetMouse(_window, nullptr, nullptr, &k, nullptr);
        }
        while(k != 0);

        WINdrawImage (_window, surface.buffer, surface.width, surface.height, 8, surface.lut.data.p, (WINDOW_WIDTH - surface.width) / 2, (WINDOW_HEIGHT - surface.height) / 2);
        WINrender (_window);

        WINwaitForGUI(_window);
    }

    /*FILE* file = fopen (_destfile, "wb");

    fwrite (surface.buffer, 1, surface.size, file);

    fclose(file);*/

    BITsurfaceDestroy(&surface2[0]);
    BITsurfaceDestroy(&surface2[1]);

    BITsurfaceDestroy(&surface);
}


void ComputeGrafikSColortab(char* _destfile, WINdow* _window)
{
    const int WIDTH  = 64;
    const int HEIGHT = 72;

    u16* buffer = (u16*) malloc(WIDTH * HEIGHT * sizeof(u16));

    memset(buffer, 0, WIDTH * HEIGHT * sizeof(u16));

    u16 gradient[2][2];

    gradient[0][0] = 0xF0F;     
    gradient[0][1] = 0x0FF;     
                                
    gradient[1][0] = 0xFF0;     
    gradient[1][1] = 0x0F0;                        

    u16 gradientline[2][HEIGHT/2];

    COLcomputeGradient(&gradient[0][0], &gradient[0][1], 1, HEIGHT/2, gradientline[0] );
    COLcomputeGradient(&gradient[1][0], &gradient[1][1], 1, HEIGHT/2, gradientline[1] );

    //  P           Y
    //  o           o
    //  o           o
    //  o           o
    //  B           G

    u16 colors[2][64];
    u16 black[ARRAYSIZE(colors[0])];

    memset(black, 0, sizeof(black));

    for (u16 y = 0 ; y < HEIGHT/2 ; y++)
    {
        COLcomputeGradient(&gradientline[0][y], &gradientline[1][y], 1, 16, colors[0] );
        COLcomputeGradient(&gradientline[1][y], &gradientline[0][y], 1, 16, colors[0] + 16);
        COLcomputeGradient(&gradientline[0][y], &gradientline[1][y], 1, 16, colors[0] + 32);
        COLcomputeGradient(&gradientline[1][y], &gradientline[0][y], 1, 16, colors[0] + 48);

        COLcomputeGradient16Steps(black, colors[0], ARRAYSIZE(colors[0]), 10, colors[1]);

        for (u16 x = 0 ; x < ARRAYSIZE(colors[0]) ; x++)
        {
            buffer[y*2*WIDTH     + x] = colors[0][x];
            buffer[(y*2+1)*WIDTH + x] = colors[1][x];
        }
    }

    if (_window != nullptr)
    {
        BITsurface surface;

        BITsurfaceConstruct(&surface);
        BITsurfaceInit (&stdAllocator, &surface, BITformat_x888, WIDTH * 8, HEIGHT, -1);

        for (u16 y = 0 ; y < HEIGHT ; y++)
        {
            for (u16 x = 0 ; x < ARRAYSIZE(colors[0]) ; x++)
            {
                u16 stcolor = buffer[y * WIDTH + x];
                u8* p = &surface.buffer[y * surface.pitch + x * 8 * 4];

                p[3] = 255;
                p[0] = COLST24b [ stcolor       & 0xF] << 4;
                p[1] = COLST24b [(stcolor >> 4) & 0xF] << 4;
                p[2] = COLST24b [(stcolor >> 8) & 0xF] << 4;

                for (u16 j = 4 ; j < 28 ; j += 4)
                {
                    p[0 + j] = p[0];
                    p[1 + j] = p[1]; 
                    p[2 + j] = p[2];
                    p[3 + j] = p[3];
                }
            }
        }

        WINdrawImage (_window, surface.buffer, surface.width, surface.height, 32, nullptr, (WINDOW_WIDTH - surface.width) / 2, (WINDOW_HEIGHT - surface.height) / 2);
        WINrender (_window);

        WINwaitForGUI(_window);
    }

    FILE* destfile = fopen(_destfile, "wb");

    STDswapBuffer16(buffer, WIDTH * HEIGHT);

    fwrite (buffer, sizeof(u16), WIDTH * HEIGHT, destfile);

    fclose(destfile);
}




int main (char argc, char** argv)
{
    WINinitParam init;
    WINdow* window = NULL;

    bool  displaywindow  = false;
    char* sourcefilename = nullptr;
    char* destfilename   = nullptr;
    int   command;


    if (argc > 3)
    {
        command        = atoi(argv[1]);
        sourcefilename = argv[2];
        destfilename   = argv[3];
        init.title    = destfilename;

        if (argc > 4)
        {
            displaywindow = _strcmpi(argv[4], "-w") == 0;   
        }
    }
    else if (argc == 1)
    {
        init.title    = "testmode";
        displaywindow = true;
        command       = -1; // test mode
    }
    else
    {
        printf ("#ERROR : wrong parameters\n");
        return 1;
    }

    if ( displaywindow )
    {
        init.x         = 10;
        init.y         = 10;
        init.w         = WINDOW_WIDTH;
        init.h         = WINDOW_HEIGHT;
        init.hInstance = NULL;

        window = WINconstruct (&init);
    }

    /*{
        BITsurface surface;

        BITsurfaceConstruct(&surface);

        BITdegasLoad(&surface, &stdAllocator, "E:\\Emulator\\ST\\HDisk\\STE_D\\PROJECTS\\DEMOS\\RELAPSE\\SRC\\GRAFIK_S\\IMAGES\\GRAFIK3.PI1");

        surface.buffer[0] = 1;

        BITrleHeader* header = (BITrleHeader*) BITcompressRLE4v (&stdAllocator, surface.buffer, surface.pitch, surface.height);

        printf ("datalen: %d + %d = %d", header->repeatLen, header->dataLen, sizeof(u32) + sizeof(u16) * 3 + header->repeatLen + header->dataLen);

        STDmset(surface.buffer, 0UL, surface.size); 

        BITuncompressRLE4_C (header, surface.buffer);

        BITsurface surface2;

        BITsurfaceConstruct(&surface2);

        BITsurfaceConvert (&stdAllocator, &surface, &surface2, BITformat_x888);

        WINdrawImage (window, surface2.buffer, surface2.width, surface2.height, 32, NULL, (WINDOW_WIDTH - surface2.width) / 2, (WINDOW_HEIGHT - surface2.height) / 2);
        WINrender (window);

        WINwaitForGUI(window);
    }*/

    if (( command == -1 ) || ( command == 1 ))
    {
        printf ("binarize liquid water\n");
        binarizeLiquidWater (window);
    }

    if (( command == -1 ) || ( command == 2 ))
    {
        printf ("binarize liquid font\n");
        binarizeLiquidFont(sourcefilename, destfilename, window);
    }

    if (( command == -1 ) || ( command == 3 ))
    {
        printf ("binarize water fall\n");
        //extractWaterFall(window);
        binarizeWaterFall(sourcefilename, destfilename, window);
    }

    if (( command == -1 ) || ( command == 4 ))
    {
        printf ("binarize intro logo\n");
        binarizeIntroLogo(sourcefilename, destfilename, window);
    }

    if (( command == -1 ) || ( command == 5 ))
    {
        printf ("binarize cyberfont\n");
        binarizeCyberFont(sourcefilename, destfilename, window);
    }

    if (( command == -1 ) || ( command == 6 ))
    {
        printf ("binarize interlude\n");
        binarizeInterlude(sourcefilename, destfilename, window);
    }  

    if (( command == -1 ) || ( command == 7 ))
    {
        printf ("binarize grafik table\n");
        binarizeGrafikS(destfilename);
    }  

    if (( command == -1 ) || ( command == 8 ))
    {
        printf ("binarize grafik images\n");
        binarizeGrafikSImages(sourcefilename, destfilename, window);
    }  

    if (( command == -1 ) || ( command == 9 ))
    {
        printf ("invert grafik colortab\n");
        ComputeGrafikSColortab(destfilename, window);
    }  

    if (window != NULL)
    {
        WINdestroy(window);
    }

    return 0;
}
