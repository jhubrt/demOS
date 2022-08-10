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


/* BINARIZE tool transforms your raw data stored into DATA folder to baked data into DATABIN folder

   Typical workflow is :

   DATA => (BINARIZE) => DATABIN => (ARJ batchs) => DATABIN => (IMAGER) => Floppy images          
*/

extern "C"
{
#   include "DEMOSDK\BASTYPES.H"
#   include "DEMOSDK\SYSTEM.H"
#   include "DEMOSDK\STANDARD.H"
#   include "DEMOSDK\COLORS.H"
#   include "DEMOSDK\BITMAP.H"

#   include "DEMOSDK\DATA\SYSTFNT.H"

#   include "DEMOSDK\PC\WINDOW.H"
#   include "DEMOSDK\PC\BMPLOADER.H"
#   include "DEMOSDK\PC\BITCONVR.H"
#   include "BLITZIK\SRC\COMMANDS.H"
}

#include <math.h>
#include <vector>
#include <string>

#include "TOOLS\BINARIZE\HELPERS.H"
#include "TOOLS\BINARIZE\HELPERSW.H"
#include "TOOLS\FONT\GLYPH.H"

#include "FX\VECTOR\TOOLS\B_VECTOR.H"
#include "BINARIZE.h"


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

static void scangradientFromPalette(char* _filename, BITsurface &surface, u16 _index1, u16 _index2)
{
    FILE* file = fopen (_filename, "wb");
    ASSERT(file != NULL);

    printf("index %d to %d - %s\n", _index1, _index2, _filename);

    for (u32 t = _index1; t < _index2;  t++)
    {
        u32 color = surface.lut.data.p32[t];

        u8  r = (u8)(color >> 16);
        u8  g = (u8)(color >> 8);
        u8  b = (u8) color;

        printf("R=%3u G=%3u B=%3u => ", r, g, b);

        r >>= 4;
        g >>= 4;
        b >>= 4;

        printf("R=%u G=%u B=%u\n", r, g, b);

        u16 stcolor = (COL4b2ST[r] << 8) | (COL4b2ST[g] << 4) | COL4b2ST[b];
        stcolor = PCENDIANSWAP16(stcolor);
        TOOLwritecheck(&stcolor, sizeof(stcolor), file);
    }

    fclose(file);
}


static void binarize_0_MenuColors( WINdow* _window, char* _sourcefilename, char* _destfilename)
{
    char filename[256];
    char drive[256];
    char subdir[256];
    char name[256];
    char ext[256];

    BITsurface surface;
    
    BITsurfaceConstruct(&surface);

    auto result = BITbmpLoad(&surface, &stdAllocator, _sourcefilename);
    ASSERT(result == BITloadResult_OK);

    _splitpath (_destfilename, drive, subdir, name, ext);
    sprintf (filename, "%s%s%s", drive, subdir, name);

    char* c = &filename[strlen(filename) - 1];

    if (*c != '0')
        printf ("#ERROR: dest filename should end with '0' : %s\n", filename);

    strcat (filename, ext);

    scangradientFromPalette(filename, surface, 0, 2);
    *c = '1';
    scangradientFromPalette(filename, surface, 2, 43);

    if (_window != NULL)
    {
        WINdrawImage(_window, surface.buffer, surface.width, surface.height, 8, surface.lut.data.p, (WINDOW_WIDTH - surface.width) / 2, (WINDOW_HEIGHT - surface.height) / 2);
        WINrender(_window);
        WINwaitForGUI(_window);
    }

    BITsurfaceDestroy(&surface);
}

static void outputIcons (void* _buffer, u16 _nbsrcplanes, u16 _srcpitch, u16 _nbdstplanes, u16 _nbwords, u16 _h, FILE* _output)
{
    u16* p = (u16*) _buffer;
    u16 y;


    ASSERT(_nbsrcplanes >= _nbdstplanes);

    TOOLwritecheck(_nbwords, _output); 
    TOOLwritecheck(_h      , _output); 

    _srcpitch >>= 1;

    for (y = 0; y < _h; y++)
    {
        u16 x;

        for (x = 0; x < _nbwords; x++)
        {
            u16 n;

            for (n = 0; n < _nbdstplanes; n++)
            {
                TOOLwritecheck(p, sizeof(*p), _output); 
                p++;
            }

            p += _nbsrcplanes - _nbdstplanes;
        }

        p += _srcpitch - (_nbwords * _nbsrcplanes);
    }
}

static void binarizeMenuBlueSurface(BITsurface* _surface)
{
    u32* p = (u32*)_surface->buffer;

    for (u32 t = 0; t < (_surface->size >> 2); t++)
    {
        p[t] = 255;
    }
}


static void binarize_1_MenuIcons( WINdow* _window, char* _sourcefilename, char* _destfilename)
{
    BITsurface surface;

    BITsurfaceConstruct(&surface);

    auto result = BITneoLoad(&surface, &stdAllocator, _sourcefilename);
    ASSERT(result == BITloadResult_OK);

    if (_window != NULL)
    {
        BITsurface surface2;

        BITsurfaceConstruct(&surface2);

        BITsurfaceConvert (&stdAllocator, &surface, &surface2, BITformat_8bits);

        WINdrawImage (_window, surface2.buffer, surface2.width, surface2.height, 8, surface2.lut.data.p, (WINDOW_WIDTH - surface2.width) / 2, (WINDOW_HEIGHT - surface2.height) / 2);
        WINrender (_window);
        WINwaitForGUI(_window);

        BITsurfaceDestroy(&surface2);
    }

/*    for (u32 t = 0 ; t < (surface.size >> 1) ; t++)
    {
        u16 plane = ((u16*) surface.buffer) [t];
        ((u16*) surface.buffer) [t] = PCENDIANSWAP16(plane);
    }*/

    FILE* file = fopen(_destfilename, "wb");

    outputIcons (surface.buffer      , 4, 160, 2, 4, 32, file);
    outputIcons (surface.buffer + 32 , 4, 160, 2, 4, 32, file);
    outputIcons (surface.buffer + 64 , 4, 160, 2, 4, 32, file);
    outputIcons (surface.buffer + 96 , 4, 160, 2, 4, 32, file);
    /*outputIcons (surface.buffer + 128, 4, 160, 2, 4, 32, file);*/

    fclose(file);
}


static void binarize_3_MenuInfos(WINdow* _window, char* _sourcefilename, char* _destfilename)
{
    BITsurface surface, surface2, surfacetocompose;

    BITsurfaceConstruct(&surface);
    BITsurfaceConstruct(&surface2);
    BITsurfaceConstruct(&surfacetocompose);

    auto result = BITneoLoad(&surface, &stdAllocator, _sourcefilename);
    ASSERT(result == BITloadResult_OK);

    BITsurfaceConvert(&stdAllocator, &surface, &surface2, BITformat_8bits);

    surface2.lut.data.p32[0] = 0xFF;
    surface2.lut.data.p32[1] = 0xFFFFFF;

    BITsurfaceConvert(&stdAllocator, &surface2, &surface, BITformat_x888);

    // set up for palette future conversion
    u32 pal[256];

    memset(pal, -1, sizeof(pal));
    pal[0] = 0xFF;
    pal[1] = 0xFFFFFF;
    pal[2] = 0;
    pal[3] = 0x808080;

    BITfont font  = BITglyphCreateFont("LEDfont", 20, 20, 1000);
    BITfont font2 = BITglyphCreateFont("LEDfont", 12, 12, 500);

    BITsurfaceInit(&stdAllocator, &surfacetocompose, BITformat_x888, 640, 200, BIT_DEFAULT_PITCH);
    BITlutSetExternal(&surfacetocompose.lut, BITlutFormat_x888, pal, 256);

    binarizeMenuBlueSurface(&surfacetocompose);

    BITsurfaceRectCopy(&surface, 0, 0, 320, 200, & surfacetocompose, 0, 0);

    BITglyphDC dc = BITglyphCreateDC(&surfacetocompose);
    
    BITglyphSetFont(dc, font);
    BITglyphText(dc, 482, 15, "BlitZWav" , true, false);

    BITglyphSetFont(dc, font2);
    BITglyphText(dc, 590, 56, "DOK" , true, false);
    BITglyphText(dc, 491, 68, "METAL AGES" , true, false);
    BITglyphText(dc, 400, 85, "CYBERNETICS 2022" , true, false);

    BITglyphUpdate(dc);

    if (_window != NULL)
    {
        WINdrawImage(_window, surfacetocompose.buffer, surfacetocompose.width, surfacetocompose.height, 32, surfacetocompose.lut.data.p, (WINDOW_WIDTH - surfacetocompose.width) / 2, (WINDOW_HEIGHT - surfacetocompose.height) / 2);
        WINrender(_window);
        WINwaitForGUI(_window);
    }

    BITsurfaceConvert(&stdAllocator, &surfacetocompose, &surface2, BITformat_8bits);
    BITsurfaceConvert(&stdAllocator, &surface2, &surface, BITformat_Chunk2P);

    FILE* file = fopen(_destfilename, "wb");

    outputIcons(surface.buffer, 2, 160, 2, 40, 99, file);

    fclose(file);
}


static void binarize_4_Pictures(WINdow* _window, char* _sourcefilename, char* _destfilename)
{
    BITsurface surface, surface2;
    char opcode[16384];
    char wdata[16384];
    u32 opcodeSize = 0, wdataSize = 0;


    BITsurfaceConstruct(&surface);
    BITsurfaceConstruct(&surface2);

    BITbmpLoad(&surface, &stdAllocator, _sourcefilename);
    printf("load %s\n", _sourcefilename);
    BITsurfaceConvert(&stdAllocator, &surface, &surface2, BITformat_Plane1P);

    BITcompress1P(surface2.buffer, opcode, sizeof(opcode), wdata, sizeof(wdata), surface2.pitch, surface2.height, &opcodeSize, &wdataSize);
    printf("Compress method 1 size = %d (%d + %d)\n", opcodeSize + wdataSize, opcodeSize, wdataSize);
    FILE* file = fopen(_destfilename, "wb");
    ASSERT(file != nullptr);
    printf("#BUILD %s\n", _destfilename);

    if (opcodeSize & 1)
        opcodeSize++;

    u16 offset = (u16)opcodeSize;
    offset = PCENDIANSWAP16(offset);
    fwrite(&offset, 1, sizeof(offset), file);
    fwrite(opcode, 1, opcodeSize, file);
    fwrite(wdata, 1, wdataSize, file);
    fclose(file);

    u32 wdataSize2 = 0;
    BITcompressRLE1P(surface2.buffer, wdata, sizeof(wdata), surface2.pitch, surface2.height, &wdataSize2);

    if (wdataSize2 <= wdataSize)
        printf("#WARNING: RLE method gives better result\n");

    if (_window != NULL)
    {
        WINdrawImage(_window, surface.buffer, surface.width, surface.height, 8, surface.lut.data.p, (WINDOW_WIDTH - surface.width) / 2, (WINDOW_HEIGHT - surface.height) / 2);
        WINrender(_window);
        WINwaitForGUI(_window);
    }

    BITsurfaceDestroy(&surface2);
    BITsurfaceDestroy(&surface);
}


static void binarize_2_MenuMenus(WINdow* _window, const char* _sourcefilename)
{
    const u32 TEXT_HEIGHT = 24;

    FILE* file = fopen(_sourcefilename, "rb");

    if (file == NULL)
    {
        printf("#ERROR: cannot open file %s\n", _sourcefilename);
    }
    else
    {
        BITsurface surface, surface2;

        BITsurfaceConstruct(&surface2);
        BITsurfaceConstruct(&surface);

        // set up for palette future conversion
        u32 pal[256];

        memset(pal, -1, sizeof(pal));
        pal[0] = 0xFF;
        pal[1] = 0xFFFFFF;
        pal[2] = 0;

        BITfont font = BITglyphCreateFont("LEDfont", 20, 20, 1000);
        //BITfont font = BITglyphCreateFont("Calibri", 32, 32);

        char line[1024];

        fgets(line, sizeof(line), file);
        u32 count = atoi(line);

        FILE* listfile = fopen("DATABIN\\MENUS\\MENULIST.BIN", "wb");

        for (u32 t = 0; t < count; t++)
        {
            std::vector<u32> widths;
            std::vector<std::string> tips;
            u8 nb;
            s32 x;

            BITsurfaceInit(&stdAllocator, &surface2, BITformat_x888, 640, 300, BIT_DEFAULT_PITCH);
            BITlutSetExternal(&surface2.lut, BITlutFormat_x888, pal, 256);

            binarizeMenuBlueSurface(&surface2);

            BITglyphDC dc = BITglyphCreateDC(&surface2);

            BITglyphSetFont(dc, font);

            fgets(line, sizeof(line), file);
            nb = (u8)atoi(line);

            TOOLwritecheck(nb, listfile);

            tips.resize(nb, "");

            printf("page %d: nb lines = %d\n", t, nb);

            for (u32 i = 0; i < nb; i++)
            {
                fgets(line, sizeof(line), file);
                strtok(line, "\r\n");
                BITglyphText(dc, 0, TEXT_HEIGHT * i, line, true, false);
                printf("'%s'", line);

                fgets(line, sizeof(line), file);
                if (strcmp(line, "\r\n") == 0)
                    line[0] = 0;
                else
                    strtok(line, "\r\n");
                tips[i] = line;
                printf(" '%s'\n", line);
            }

            BITglyphUpdate(dc);

            BITsurfaceConvert(&stdAllocator, &surface2, &surface, BITformat_8bits);

            for (u32 i = 0; i < nb; i++)
            {
                for (x = 639; x >= 0; x--)
                {
                    for (u32 y = i * TEXT_HEIGHT; y < (i + 1) * TEXT_HEIGHT; y++)
                    {
                        if (surface.buffer[x + y * surface.pitch] != 0)
                        {
                            widths.push_back(x + 2);
                            x = -1;
                            break;
                        }
                    }
                }
            }

            BITsurfaceConvert(&stdAllocator, &surface, &surface2, BITformat_Chunk2P);

            if (_window != NULL)
            {
                auto offsetx = (WINDOW_WIDTH - surface.width) / 2;
                auto offsety = (WINDOW_HEIGHT - surface.height) / 2;

                BITsurfaceConvert(&stdAllocator, &surface2, &surface, BITformat_8bits);

                WINdrawImage(_window, surface.buffer, surface.width, surface.height, 8, surface.lut.data.p, offsetx, offsety);

                WINsetColor(_window, 255, 0, 0);
                for (u32 i = 0; i < nb; i++)
                {
                    WINrectangle(_window, offsetx - 1, i * TEXT_HEIGHT + offsety, offsetx + widths[i], offsety + (i + 1) * TEXT_HEIGHT);
                }

                WINrender(_window);
                WINwaitForGUI(_window);
            }

            char filename[256];

            {
                sprintf(filename, "DATABIN\\MENUS\\MENUZIK%d.BIN", t);
                FILE* file = fopen(filename, "wb");

                for (u32 i = 0; i < widths.size(); i++)
                {
                    u16 nbwords = (u16)(((widths[i] + 15) & (~15)) >> 4);
                    outputIcons(surface2.buffer + i * TEXT_HEIGHT * 160, 2, 160, 2, nbwords, TEXT_HEIGHT, file);
                }

                fclose(file);
            }

            {
                sprintf(filename, "DATABIN\\MENUS\\MENUZIK%d.TXT", t);
                FILE* file = fopen(filename, "wb");
                for (u32 i = 0; i < nb; i++)
                {
                    u8 len = 0;

                    if (tips[i].empty() == false)
                    {
                        len = (u8)tips[i].length();
                        ASSERT(len <= 79);
                        TOOLwritecheck(len, file);
                        TOOLwritecheck(tips[i].c_str(), len + 1, file);
                    }
                    else
                    {
                        TOOLwritecheck(len, file);
                    }
                }
                fclose(file);
            }

            BITglyphDestroyDC(dc);
        }

        fclose(listfile);

        BITglyphDestroyFont(font);
    }
}


static void convertOldPZPolygons (WINdow* _window, const char* _inputDir, s16 _centerx, s16 _centery, char* _outputFile)
{
    const char* polygonFiles[] = 
    {
        "C.DAT",
        "Y_1.DAT",
        "Y_2.DAT",
        "B_1.DAT",
        "B_2.DAT",
        "E_1.DAT",
        "E_2.DAT",
        "R_1.DAT",
        "R_2.DAT",
        "N.DAT",
        "E_1.DAT",
        "E_2.DAT",
        "T_1.DAT",
        "T_2.DAT",
        "I_1.DAT",
        "I_2.DAT",
        "C.DAT",
        "S_1.DAT",
        "S_2.DAT"
    };

    BITsurface surface;

    BITsurfaceConstruct (&surface);
    BITsurfaceInit (&stdAllocator, &surface, BITformat_8bits, 1024, 768, BIT_DEFAULT_PITCH);
    BITlutInit (&stdAllocator, &surface.lut, BITlutFormat_x888, 256);
    BITsurfaceClear(&surface);

    surface.lut.data.p32[0]   = 0; 
    surface.lut.data.p32[1]   = 0xFFFFFF; 
    surface.lut.data.p32[254] = (109 << 16) | (109 << 8) | 109; 
    surface.lut.data.p32[255] = (170 << 16) | (170 << 8) | 170; 

    {
        s16 offset = 0;
        u16 colorindex = 2;

        for (u16 t = 0 ; t < ARRAYSIZE(polygonFiles) ; t++)
        {
            char temp[256];

            sprintf (temp, "%s%s", _inputDir, polygonFiles[t]);

            FILE* inFile = fopen (temp, "rb");
            assert(inFile != NULL);

            size_t size = TOOLfileSize(inFile);
            assert(size <= sizeof(temp));
            fread(temp, 1, size, inFile);

            fclose(inFile);
            
            int oldx = 0, oldy = 0, firstx = 0, firsty = 0;

            WINsetColor(_window, 64,64,64);
            for (u16 i = 2 ; i < size ; i += 4)
            {
                u16 val;

                val = *(u16*)&temp[i];
                int x = offset + PCENDIANSWAP16(val) + 10;

                val = *(u16*)&temp[i + 2];
                int y = PCENDIANSWAP16(val) + 10;

                surface.buffer[x + y * surface.pitch] = (u8)colorindex;

                if (i > 2)
                {
                    WINline(_window, x, y, oldx, oldy);
                }
                else
                {
                    firstx = x;
                    firsty = y;
                }
                
                oldx = x;
                oldy = y;

                surface.lut.data.p32[colorindex++] = ((rand() & 0xFF)  << 16) | ((rand() & 0xFF) << 8) | colorindex; 
            }

            colorindex += 4;

            WINline(_window, firstx, firsty, oldx, oldy);

            switch (t)
            {
            case 0:  offset += 50;   break; /* C */
            case 2:  offset += 50;   break; /* Y */
            case 4:  offset += 50;   break; /* B */
            case 6:  offset += 50; 	 break; /* E */			
            case 8:  offset += 50;   break; /* R */			    
            case 9:  offset += 50;	 break; /* N */			    
            case 11: offset += 50;   break; /* E */		     
            case 13: offset += 50;   break;	/* T */			    
            case 15: offset += 20;   break; /* I */			    
            case 16: offset += 50;   break; /* C */		     
            }
        }
    }

    BITbmpSave (&surface, _outputFile);

    WINrender(_window);
    WINwaitForGUI(_window);

    BITsurfaceDestroy (&surface);
}



static void rect (BITsurface* _surface, s16 _x1, s16 _y1, s16 _x2, s16 _y2, u8 _pix)
{
    u16 x, y;

    for (y = _y1 ; y <= _y2 ; y++)
    {
        u8* p = _surface->buffer + y * _surface->width + _x1;

        for (x = _x1 ; x <= _x2 ; x++)
        {
            *p++ = _pix;
        }
    }
}


static void binarize_7_Keyboard(WINdow* _window)
{
    BITsurface surface;
    u16 t;

    u8 keyscolor1[] = {1,3,5,6,8,10,12};


    BITsurfaceConstruct (&surface);

    BITsurfaceInit (&stdAllocator, &surface, BITformat_8bits, 368, 35, BIT_DEFAULT_PITCH);
    
    BITlutInit(&stdAllocator, &surface.lut, BITlutFormat_x888, 256);

    for (t = 0 ; t < 16 ; t++)
    {
        surface.lut.data.p32[t] = (t * 8 + 128) << (8 * (t % 3));
    }

    for (t = 0 ; t < ARRAYSIZE(keyscolor1) ; t++)
    {
        rect(&surface, t * 26, 0, t * 26 + 22, 32, keyscolor1[t % 7]);
    }

    t = 0; rect(&surface, t * 26 + 15, 0, t * 26 + 20 + 13, 24, 0);
    t = 1; rect(&surface, t * 26 + 15, 0, t * 26 + 20 + 13, 24, 0);
    t = 3; rect(&surface, t * 26 + 15, 0, t * 26 + 20 + 13, 24, 0);
    t = 4; rect(&surface, t * 26 + 15, 0, t * 26 + 20 + 13, 24, 0);
    t = 5; rect(&surface, t * 26 + 15, 0, t * 26 + 20 + 13, 24, 0);

    t = 0; rect(&surface, t * 26 + 17, 0, t * 26 + 18 + 13, 22, 2);
    t = 1; rect(&surface, t * 26 + 17, 0, t * 26 + 18 + 13, 22, 4);
    t = 3; rect(&surface, t * 26 + 17, 0, t * 26 + 18 + 13, 22, 7);
    t = 4; rect(&surface, t * 26 + 17, 0, t * 26 + 18 + 13, 22, 9);
    t = 5; rect(&surface, t * 26 + 17, 0, t * 26 + 18 + 13, 22, 11);

    if (_window != nullptr)
    {
        WINdrawImage(_window, surface.buffer, surface.width, surface.height, 8, surface.lut.data.p32, 0, 0);
        WINrender(_window);
        WINwaitForGUI(_window);
    }

    BITsurfaceDestroy (&surface);    
}


static void binarize_8_Sprite(WINdow* _window, char* _sourcefilename, char* _destfilename)
{
    BITsurface surface;
    u16 t, i;

    BITlutConstruct(&surface);

    BITneoLoad (&surface, &stdAllocator, _sourcefilename);

    FILE* file = fopen(_destfilename, "wb");

    i = 0;

    for (t = 0; t < 32; t++)
    {
        fwrite (&surface.buffer[i], 2, 1, file); 
        i += 8;
        fwrite (&surface.buffer[i], 2, 1, file); 
        i += 152;
    }

    fclose(file);

    BITsurfaceDestroy(&surface);
}


static void binarize_9_InfoText(WINdow* _window, char* _sourcefilename, char* _destfilename)
{
    char line[4096];
    u16  nblines = 0;

    FILE* filesrc = fopen(_sourcefilename, "rt");
    FILE* filedst = fopen(_destfilename  , "wb");

    ASSERT(filesrc != NULL);
    ASSERT(filedst != NULL);

    while (fgets(line, sizeof(line), filesrc) != nullptr)
    {
        u32 len = strlen(line);
        
        if (len > 0)
            if ((line[len-1] == '\r') || (line[len-1] == '\n'))
                len--;

        if (len > 0)
            if ((line[len-1] == '\r') || (line[len-1] == '\n'))
                len--;

        for (u32 t = len ; t < 42 ; t++)
            line[t] = ' ';
        line[42] = 0;

        if (len > 42)
            printf("#WARNING line (%d) '%s' too long will be truncated\n", nblines, line);

        fwrite (line, 43, 1, filedst);

        nblines++;
    }

    if ((nblines + 1) & 15)
    {
        printf("#WARNING file '%s' does not have a 16 multiple number of lines (%d) \n", _sourcefilename, nblines + 1);
        /*    u16 t;

        memset (line, ' ', 42);
        line[43] = 0;

        for (t = nblines & 15 ; t < 16 ; t++)
        {
            fwrite (line, 43, 1, filedst);
        }*/
    }

    fclose (filesrc);
    fclose (filedst);
}


static void binarize_10_InfoVolumePatterns(WINdow* _window, char*_sourcefilename, char* _destfilename)
{
    const u16 INFO_VOLUME_MAX = 32;
    u16 volumemasks[INFO_VOLUME_MAX+1][3];
    u16 t, i;
    s32 k = 0;

    BITsurface surface;


    BITsurfaceConstruct(&surface);

    BITdegasLoad(&surface, &stdAllocator, _sourcefilename);

    u16* p = (u16*) surface.buffer; 

    for (i = 0; i <= INFO_VOLUME_MAX; i++)
    {
        volumemasks[i][0] = p[0];
        volumemasks[i][1] = p[4];
        volumemasks[i][2] = p[8];

        p += surface.pitch / 2;
    }

    BITsurfaceDestroy(&surface);

    {
        FILE* file = fopen(_destfilename, "wb");
        ASSERT(file != NULL);

        fwrite (volumemasks, 1, sizeof(volumemasks), file);

        fclose(file);
    }

    if (_window != NULL)
    {
        for (i = 0; i <= INFO_VOLUME_MAX; i++)
        {
            u16 m;
            u16 x = 0;

            for (m = 0; m < 3; m++)
            {
                u16 mask = PCENDIANSWAP16(volumemasks[i][m]);

                for (t = 0; t < 16; t++)
                {
                    if (mask & 0x8000)
                        WINsetColor(_window, 255, 255, 255);
                    else
                        WINsetColor(_window, 128, 128, 128);

                    mask <<= 1;

                    WINfilledRectangle(_window, x, i * 10, x + 10, i * 10 + 10);
                    x += 10;
                }
            }
        }

        WINrender(_window);
        WINwaitForGUI(_window);
    }
}


static void binarize_11_CommandsSequence(WINdow* _window, char*_sourcefilename, char* _destfilename)
{
    std::vector<std::pair<u8,u8>> list;
    char temp[256];
    int line = 0;
    FILE* filesrc = fopen(_sourcefilename, "rt");
    ASSERT(filesrc != NULL);


    while (fgets(temp, sizeof(temp), filesrc))
    {
        char* p = strtok(temp, " \t\r\n");

        line++;

        if ((p != NULL) && (*p != 0))
        {
            if (*p != ';')
            {
                std::pair<u8,u8> command(0,0);                                                       
                auto cmd = atoi(p);

                if (cmd > 255)
                    printf ("#ERROR: %s(%d): command delay exceed 255 (%d)\n", _sourcefilename, line, cmd);

                command.first = (u8) cmd;

                p = strtok(NULL, " \t\r\n");

                if (p == NULL)
                    printf ("#ERROR: %s(%d): no command\n", _sourcefilename, line);
                else if (_stricmp(p, "UP") == 0)
                    command.second = BLZ_CMD_UP;
                else if (_stricmp(p, "DOWN") == 0)
                    command.second = BLZ_CMD_DOWN;
                else if (_stricmp(p, "LEFT") == 0)
                    command.second = BLZ_CMD_LEFT;
                else if (_stricmp(p, "RIGHT") == 0)
                    command.second = BLZ_CMD_RIGHT;
                else if (_stricmp(p, "SELECT") == 0)
                    command.second = BLZ_CMD_SELECT; 
                else
                    printf ("#ERROR: %s(%d): unknown command '%s'\n", _sourcefilename, line, p);

                list.push_back(command);
            }
        }
    }

    fclose (filesrc);

    FILE* filedst = fopen (_destfilename, "wb");
    ASSERT(filedst != NULL);

    fwrite(&list[0], sizeof(u8) * 2, list.size(), filedst);

    fclose(filedst);
}


static void binarize_12_PartyLogo(WINdow* _window, char*_sourcefilename, char* _destfilename)
{
    BITsurface surface;
    u16 line=0;
    u8  laststate = 0;


    BITsurfaceConstruct(&surface);

    BITbmpLoad(&surface, &stdAllocator, _sourcefilename);

    for (u16 y = 0; y < 200; y++)
    {
        u16 x1;

        for (u16 x = 0; x < 336; x++)
        {
            u8 state = surface.buffer[x+line] != 0;

            if (surface.buffer[x + line] > 7)
            {
                surface.buffer[x+line] = 0;
            }

            if (laststate == 0)
            {
                if (state == 1)
                    x1 = x;
            }
            else
            {
                if (state == 0)
                {
                    u16 x2 = x - 1;
                    u16 v = 0;
                    u16 w = x2-x1+1;
                    u8  value [] = { 1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2, 1 };
                    u8  value2[] = { 7, 6, 5, 4, 3, 2, 1, 2, 3, 4, 5, 6, 7 };

                    
                    for (u16 i = x1; i <= x2 ; i++, v++)
                    {
                        u8 pix;

                        pix = v * ARRAYSIZE(value) / w;
                        if (pix >= ARRAYSIZE(value))
                            pix =  ARRAYSIZE(value) - 1;

                        switch (y % 3)
                        {
                        case 1: pix = value2[pix]; break;
                        case 0: pix = value [pix]; break;
                        case 2: pix = 0;
                        }

                        surface.buffer[i+line] = pix;
                    }
                }
            }

            laststate = state;
        }

        line += surface.pitch;
    }

    if (_window != nullptr)
    {
        u32 seq[] =
        {
            0x20202020, 
            0x40404040,
            0x60606060,
            0x80808080,
            0xA0A0A0A0,
            0xC0C0C0C0,
            0xE0E0E0E0,

            0xA0A0A0A0,
            0x60606060,
            0x20202020,
            0x60606060,
            0xA0A0A0A0,
            0xC0C0C0C0,
            0xFFFFFFFF,

            0xA0A0A0A0,
            0x60606060,
            0x20202020,
            0x60606060,
            0xA0A0A0A0,
            0xC0C0C0C0,
            0xFFFFFFFF,
        };

        int i = 0;

        WINwaitForGUI(_window);
        
        s32 k = 0;

        do
        {
            WINgetMouse (_window, NULL, NULL, &k, NULL);
            memcpy(surface.lut.data.p32 + 1, &seq[i], 7 * 4);
            i ++;
            if (i >= 14)
                i = 7;
            WINdrawImage(_window, surface.buffer, surface.width, surface.height, 8, surface.lut.data.p32, 0, 0);
            WINrender(_window);
            WINwait(40);
        }
        while (!WINisKeyHit(_window) && (k == 0));
    }

    {
        BITsurface surface2;
        
        BITsurfaceConstruct(&surface2);

        BITsurfaceConvert(&stdAllocator, &surface, &surface2, BITformat_4bits);

        FILE* file = fopen(_destfilename, "wb");
        ASSERT(file != NULL);

        fwrite (surface2.buffer, 1, surface2.size, file);
        fclose(file);

        {
            BITsurface surface3;
            
            BITsurfaceConstruct(&surface3);

            BITsurfaceInit(&stdAllocator, &surface3, BITformat_8bits, 320, 200, 320 );
            BITsurfaceRectCopy(&surface, 0,0,320,200, &surface3, 0,0);

            BITsurfaceConvert(&stdAllocator, &surface3, &surface2, BITformat_Chunk4P);

            //BITdegasSave(&surface2, "BLITZIK\\TEST.PI1");

            BITsurfaceDestroy(&surface3);
        }


        BITsurfaceDestroy(&surface2);
    }

    BITsurfaceDestroy(&surface);
}



extern "C" void EMULnewFrame(void) {}
extern "C" bool EMULupdateRequested(void) { return true; }

static void binarizeInfo(WINdow* _window)
{   
    BITsurface surface, surface2, surface3;

    BITsurfaceConstruct (&surface);
    BITsurfaceConstruct (&surface2);
    BITsurfaceConstruct (&surface3);
    
    BITsurfaceInit (&stdAllocator, &surface, BITformat_Chunk4P, 320, 200, BIT_DEFAULT_PITCH);
    BITlutInit(&stdAllocator, &surface.lut, BITlutFormat_STe, 16);

    surface.lut.data.p16[0] = 0; //0x70; 

    u16 startcolor = 8;
    u16 endcolor   = 0x85F;

    COLcomputeGradient(&startcolor, &endcolor, 1, 7, surface.lut.data.p16 + 1);

    //surface.lut.data.p16[7] = 0xFF; 

    for (u16 t = 0; t < 8; t++)
    {
        surface.lut.data.p16[t] = PCENDIANSWAP16(surface.lut.data.p16[t]);
    }
    
    memset (surface.buffer, 0, surface.size);

    u16 i = 0;

    for (u16 t = 0 ; t < 256 ; t++)
    {
        if (SYSfont.charsmap[t] != SYS_FNT_UNDEFINED)
        {    
            u8* adr = surface.buffer + 1 + (i % 19) * 8 + (i / 19) * 1600 + 320;
            char temp[2] = {0,0};
            
            temp[0] = (char)t;

            SYSfastPrint(temp, adr, 160, 8, (u32)&SYSfont);
            i++;
        }
    }
    
    BITsurfaceConvert(&stdAllocator, &surface, &surface2, BITformat_8bits);

    BITsurfaceInit (&stdAllocator, &surface3, BITformat_8bits, surface2.width, surface2.height, BIT_DEFAULT_PITCH);

    memset (surface3.buffer, 0, surface3.size);

    for (s32 y = 1; y < (surface2.height - 1) ; y++)
    {
        for (s32 x = 1; x < (surface2.width - 1) ; x++)
        {
            int destindex = y * surface2.pitch + x;
            u16 totalc = 0;

            for (s16 dy = -1; dy <= 1; dy++)
            {
                for (s16 dx = -1; dx <= 1; dx++)
                {
                    if (surface2.buffer[destindex + dx + dy * (s16)surface2.pitch])
                    {
                        if ((dx == 0) && (dy == 0))
                            totalc += 7 * 2;
                        else if ((dx == 0) || (dy == 0))
                            totalc += 7;
/*                        else
                            totalc += 7;*/
                    }
                }
            }

            totalc /= 5; //1*2 + 4*1; // + 4*1;

            if (totalc > 7)
                totalc = 7;

            surface3.buffer[destindex] = (u8) totalc;
        }
    }

    BITsurfaceConvert (&stdAllocator, &surface3, &surface, BITformat_Chunk4P);

    bool result = BITdegasSave (&surface, "DATA\\INFO\\FONT.PI1");
    ASSERT(result);

    if (_window != nullptr)
    {
        WINdrawImage(_window, surface3.buffer, surface3.width, surface3.height, 8, surface2.lut.data.p, 0, 0);
        WINrender(_window);
        WINwaitForGUI(_window);
    }

    BITsurfaceDestroy (&surface);    
    BITsurfaceDestroy (&surface2);
    BITsurfaceDestroy (&surface3);
}



/* Main */

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
   
    if (( command == -1 ) || ( command == 0 ))
    {
        printf ("binarize menu colors\n");
        
        if (command == -1)
        {
            binarize_0_MenuColors(window, "DATA\\MENUS\\TEST0.BMP", "DATABIN\\MENUS\\PINKBLU0.BIN");
            binarize_0_MenuColors(window, "DATA\\MENUS\\TEST1.BMP", "DATABIN\\MENUS\\GEOTECH0.BIN");
            binarize_0_MenuColors(window, "DATA\\MENUS\\TEST2.BMP", "DATABIN\\MENUS\\LASER0.BIN"  );
            binarize_0_MenuColors(window, "DATA\\MENUS\\TEST3.BMP", "DATABIN\\MENUS\\ECOCONC0.BIN");
            binarize_0_MenuColors(window, "DATA\\MENUS\\TEST4.BMP", "DATABIN\\MENUS\\GWARM0.BIN"  );
            binarize_0_MenuColors(window, "DATA\\MENUS\\TEST5.BMP", "DATABIN\\MENUS\\FRENCHT0.BIN");
        }
        else
            binarize_0_MenuColors(window, sourcefilename, destfilename);
    }
    
    if (( command == -1 ) || ( command == 1 ))
    {
        printf ("binarize menu icons\n");

        if (command == -1)
            binarize_1_MenuIcons(window, "DATA\\MENUS\\ICONS4.NEO", "DATABIN\\MENUS\\ICONS.BIN");
        else
            binarize_1_MenuIcons(window, sourcefilename, destfilename);
    }

    if (( command == -1 ) || ( command == 2 ))
    {
        printf ("binarize menu menus\n");
        
        if (command == -1)
            binarize_2_MenuMenus(window, "DATA\\MENUS\\MENU.TXT");
        else
            binarize_2_MenuMenus(window, sourcefilename);
    }

    if (( command == -1 ) || ( command == 3 ))
    {
        printf("binarize menu infos\n");
        
        if ( command == -1 )
            binarize_3_MenuInfos(window, "DATA\\MENUS\\INFOC.NEO", "DATABIN\\MENUS\\INFOS.BIN");
        else
            binarize_3_MenuInfos(window, sourcefilename, destfilename);
    }

    if (( command == -1 ) || ( command == 4 ))
    {
        printf("binarize pictures\n");

        if ( command == -1 )
        {
            binarize_4_Pictures(window, "DATA\\PICS\\BLITZ.BMP" , "DATABIN\\PICS\\BLITZ.BIN" );
            binarize_4_Pictures(window, "DATA\\PICS\\CYBER0.BMP", "DATABIN\\PICS\\CYBER0.BIN");
            binarize_4_Pictures(window, "DATA\\PICS\\CYBER1.BMP", "DATABIN\\PICS\\CYBER1.BIN");
            binarize_4_Pictures(window, "DATA\\PICS\\CYBER2.BMP", "DATABIN\\PICS\\CYBER2.BIN");
            binarize_4_Pictures(window, "DATA\\PICS\\CYBER3.BMP", "DATABIN\\PICS\\CYBER3.BIN");
            binarize_4_Pictures(window, "DATA\\PICS\\CYBER4.BMP", "DATABIN\\PICS\\CYBER4.BIN");
            binarize_4_Pictures(window, "DATA\\PICS\\CYBER5.BMP", "DATABIN\\PICS\\CYBER5.BIN");
            binarize_4_Pictures(window, "DATA\\PICS\\CYBER6.BMP", "DATABIN\\PICS\\CYBER6.BIN");
            binarize_4_Pictures(window, "DATA\\PICS\\CYBER7.BMP", "DATABIN\\PICS\\CYBER7.BIN");
            binarize_4_Pictures(window, "DATA\\PICS\\CYBER8.BMP", "DATABIN\\PICS\\CYBER8.BIN");
        }
        else
            binarize_4_Pictures(window, sourcefilename, destfilename);
    }

    if ((command == -1) || (command == 5))
    {
        printf ("binarize polyzoom polygons\n");

        VECbinarizeScene (window, "DATA\\POLYZOOM\\LOGOS.BMP",    253, 0, 0  , 420, 180, 212,    104,   2.0f, 2.0f, "DATABIN\\POLYZOOM\\PRESENT.VEC");
        VECbinarizeScene (window, "DATA\\POLYZOOM\\LOGOS.BMP",    253, 0, 260, 420, 420, 210,    346,   2.0f, 2.0f, "DATABIN\\POLYZOOM\\REFACTO.VEC");
        VECbinarizeScene (window, "DATA\\POLYZOOM\\LOGOS.BMP",    253, 0, 540, 460, 660, 230,    603,   2.0f, 2.0f, "DATABIN\\POLYZOOM\\BLITZWAV.VEC");
        VECbinarizeScene (window, "DATA\\POLYZOOM\\BLITZ.BMP",    253, 0, 0,   299, 299, 162,    140,   1.0f, 1.0f, "DATABIN\\POLYZOOM\\BLITZ.VEC");        
        VECbinarizeScene (window, "DATA\\POLYZOOM\\CYBERNET.BMP", 253, 0, 0,   600, 300, 250+10, 27+10, 1.0f, 1.0f, "DATABIN\\POLYZOOM\\CYBERNET.VEC");

        // convertOldPZPolygons (window, "DATA\\POLYZOOM\\OLD\\", 250, 27, "DATA\\POLYZOOM\\CYBERNET2.BMP");
    }

    if ((command == -1) || (command == 6))
    {
        binarizeInfo(window); // proto
    }

    if ((command == -1) || (command == 7))
    {
        printf ("binarize keyboard\n");

        binarize_7_Keyboard(window); // proto
    }

    if ((command == -1) || (command == 8))
    {
        printf ("binarize sprite\n");

        if (command == -1)
            binarize_8_Sprite(window, "DATA\\LAYERZ\\SPRITE.NEO", "DATABIN\\LAYERZ\\SPRITE1.BIN");
        else
            binarize_8_Sprite(window, sourcefilename, destfilename);
    }

    if ((command == -1) || (command == 9))
    {
        printf ("binarize info text\n");

        if (command == -1)
            binarize_9_InfoText(window, "DATA\\INFO\\INFO1.TXT", "DATABIN\\INFO\\INFO1.BIN");
        else
            binarize_9_InfoText(window, sourcefilename, destfilename);
    }

    if ((command == -1) || (command == 10))
    {
        printf ("binarize generate info volume masks\n");

        if (command == -1)
            binarize_10_InfoVolumePatterns(window, "DATA\\INFO\\VOLMASKS.PI1", "DATABIN\\INFO\\VOLMASKS.BIN");
        else
            binarize_10_InfoVolumePatterns(window, sourcefilename, destfilename);
    }

    if ((command == -1) || (command == 11))
    {
        printf ("binarize commands sequence\n");

        if (command == -1)
            binarize_11_CommandsSequence(window, "DATA\\MENUS\\AUTORUN.TXT", "DATABIN\\MENUS\\AUTORUN.BIN");
        else
            binarize_11_CommandsSequence(window, sourcefilename, destfilename);
    }

    if ((command == -1) || (command == 12))
    {
        printf ("binarize party logo\n");

        if (command == -1)
            binarize_12_PartyLogo(window, "DATA\\POLYZOOM\\PARTY.BMP", "DATABIN\\POLYZOOM\\PARTY.BIN");
        else
            binarize_12_PartyLogo(window, sourcefilename, destfilename);
    }

    if (window != NULL)
    {
        WINdestroy(window);
    }

    return 0;
}