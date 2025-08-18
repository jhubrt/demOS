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
    #include "DEMOSDK\BASTYPES.H"
    #include "DEMOSDK\STANDARD.H"
    #include "DEMOSDK\COLORS.H"
    #include "DEMOSDK\BITMAP.H"
    #include "DEMOSDK\PC\WINDOW.H"
    #include "DEMOSDK\PC\SURFACE.H"
    #include "DEMOSDK\PC\BMPLOADER.H"
}

#include "TOOLS\BINARIZE\HELPERS.H"

#include <assert.h>


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



static void convert4P320To4B (BITsurface& surfaceSource_, BITsurface& surfaceDest_)
{
/*BITsurfaceInit(&stdAllocator, &surfaceDest_, , 320, (u16)(size / 160), 160);
BITlutSetExternal(&surface.lut, BITlutFormat_STe, pal, 16);

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
}*/
}

static void convert2P640To2B(WINdow* _window, char* _sourcefilename, char* _destfilename)
{
    FILE* file = fopen(_sourcefilename, "rb");
    ASSERT(file != NULL);

    {
        u16 t, pal[16];
        BITsurface surface, surface2;
        u32 size = TOOLfileSize(file);

        u8* buffer = (u8*)malloc(size);

        u32 read = fread(buffer, 1, size, file);
        ASSERT(read == size);

        fclose(file);

        BITsurfaceConstruct(&surface);
        BITsurfaceConstruct(&surface2);

        for (t = 0; t < 16; t++)
        {
            pal[t] = t == 0 ? -1 : 0; /*PCENDIANSWAP16(COL4b2ST[t]);*/
        }

        BITsurfaceSetExternal(&surface, buffer, BITformat_Chunk2P, 640, (u16)(size / 160), 160);
        BITlutSetExternal(&surface.lut, BITlutFormat_STe, pal, 4);

        if (_window != NULL)
        {
            BITsurfaceConvert(&stdAllocator, &surface, &surface2, BITformat_8bits);
            WINdrawImage(_window, surface2.buffer, surface2.width, surface2.height, 8, surface2.lut.data.p, 0, 0);
            WINrender(_window);
            WINwaitForGUI(_window);
        }

        BITsurfaceDestroy(&surface);
        BITsurfaceDestroy(&surface2);

        free(buffer);
    }
}


int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("#ERROR: wrong parameters (%d)\n"
            "v1.0.0\n"
            "  Usage:\n"
            "    P2C source dest\n", argc);

        return 1;
    }

    bool nopal = false;
    int base = 1;
    int size = -1;
    int shift = 0;

    for (; base < (argc-2); base++)
    {
        if (_strcmpi(argv[base], "-nopal") == 0)
        {
            nopal = true;
        }
        else if (_strnicmp(argv[base], "-size", strlen("-size")) == 0)
        {
            size = atoi(&argv[base][strlen("-size")]);
        }
        else if (_strnicmp(argv[base], "-shift", strlen("-shift")) == 0)
        {
            shift = atoi(&argv[base][strlen("-shift")]);
        }
        else
        {
            break;
        }
    }

    const char* sourceFilename = argv[base];
    const char* destFilename   = argv[base+1];

    char sourceext[512];
    char destext[512];

    _splitpath(sourceFilename, NULL, NULL, NULL, sourceext);
    _splitpath(destFilename  , NULL, NULL, NULL, destext);

    _strupr(sourceext);
    _strupr(destext);

    BITsurface surfaceSource, surfaceDest;

    BITsurfaceConstruct(&surfaceSource);
    BITsurfaceConstruct(&surfaceDest);

    if (_strcmpi(sourceext, ".PI1") == 0)
    {
        if (BITdegasLoad(&surfaceSource, &stdAllocator, sourceFilename) != BITloadResult_OK)
            goto readerror;
    }
/*    else if (_strcmpi(ext, ".PI2") == 0)
    {
        if (BITdegasLoad (&surfaceSource, &stdAllocator, argv[1]) != BITloadResult_OK)
        {
            printf ("#ERROR: cannot open file %s for read", argv[1]);
            return 1;
        }
        BITsurfaceConvert(&stdAllocator, &surfaceSource, &surfaceDest, BITformat_2bits);
        fwrite (surfaceSource.lut.data.p, 2, 4, file);
        fwrite (surfaceDest.buffer, 1, surfaceDest.size, file);
    }*/
    else if (_strcmpi(sourceext, ".NEO") == 0)
    {
        if (BITneoLoad(&surfaceSource, &stdAllocator, sourceFilename) != BITloadResult_OK)
            goto readerror;
    }
    else if (_strcmpi(sourceext, ".KID") == 0)
    {
        if (BITkidLoad(&surfaceSource, &stdAllocator, sourceFilename) != BITloadResult_OK)
            goto readerror;
    }
    else if (_strcmpi(sourceext, ".DAT") == 0)
    {
        if (BITcompressedLoad(&surfaceSource, &stdAllocator, sourceFilename) != BITloadResult_OK)
            goto readerror;
    }
    else if (_strcmpi(sourceext, ".BMP") == 0)
    {
        if (BITbmpLoad(&surfaceSource, &stdAllocator, sourceFilename) != BITloadResult_OK)
            goto readerror;

        BITlutConvert(&stdAllocator, &surfaceSource.lut, &surfaceSource.lut, BITlutFormat_STe);
    }
    else if (_strcmpi(sourceext, ".BIN") == 0)
    {
        FILE* sourceFile = fopen (sourceFilename, "rb");
        if (sourceFile == NULL)
            goto readerror;

        fseek (sourceFile, 0, SEEK_END);
        u32 size = ftell (sourceFile);
        void* buffer = malloc(size);
        fseek (sourceFile, 0, SEEK_SET);
        fread( buffer, 1, size, sourceFile);
        fclose(sourceFile);
        BITsurfaceSetExternal(&surfaceSource, buffer, BITformat_Chunk4P, 32, (u16)(size / 16), 16);
        nopal = true;
    }   
    else
    {
        printf ("#ERROR: unknown source format\n");
        return 1;
    }

    {
        FILE* destFile = fopen(destFilename, "wb");
        if (destFile == NULL)
        {
            printf("#ERROR: cannot open file %s for write", destFilename);
            return 1;
        }

        if (_strcmpi(destext, ".8B") == 0)
        {
            BITsurfaceConvert(&stdAllocator, &surfaceSource, &surfaceDest, BITformat_8bits);
            if (nopal == false)
                fwrite(surfaceSource.lut.data.p, 2, 16, destFile);
            if (shift > 0)
            {
                u32 t;

                for (t = 0 ; t < surfaceDest.size ; t++)
                    surfaceDest.buffer[t] <<= shift;
            }
        }
        else if (_strcmpi(destext, ".4B") == 0)
        {
            BITsurfaceConvert(&stdAllocator, &surfaceSource, &surfaceDest, BITformat_4bits);
            if (nopal == false)
                fwrite(surfaceSource.lut.data.p, 2, 16, destFile);
        }
        else if (_strcmpi(destext, ".3P") == 0)
        {
            BITsurfaceConvert(&stdAllocator, &surfaceSource, &surfaceDest, BITformat_Chunk3P);
            if (nopal == false)
                fwrite(surfaceSource.lut.data.p, 2, 8, destFile);
        }
        else if (_strcmpi(destext, ".1P") == 0)
        {
            BITsurfaceConvert(&stdAllocator, &surfaceSource, &surfaceDest, BITformat_Plane1P);
        }
        else if (_strcmpi(destext, ".PI1") == 0)
        {
            u16 res = 0;

            fwrite(&res, sizeof(res), 1, destFile);
            fwrite(surfaceSource.lut.data.p, 2, 16, destFile);

            surfaceDest = surfaceSource;
            
            BITsurfaceConstruct(&surfaceSource);    // Reset source descriptor
        }
        else if (_strcmpi(destext, ".C4V") == 0)
        {
            u16 countlen = 0, datalen = 0;

            ASSERT(surfaceSource.format == BITformat_Chunk4P);

            if (nopal == false)
                fwrite(surfaceSource.lut.data.p, 2, 16, destFile);

            BITrleHeader* header = (BITrleHeader*) BITcompressRLE4v (&stdAllocator, surfaceSource.buffer, surfaceSource.pitch, surfaceSource.height);

            TOOLwritecheck(header, PCENDIANSWAP16(header->repeatLen) + PCENDIANSWAP16(header->dataLen) + BIT_RLE_HEADERSIZE, destFile);

            MEM_FREE(&stdAllocator, header);

            size = 0;
        }
        else
        {
            printf("#ERROR: unknown destination format\n");
            return 1;
        }

        if (size == -1)
            size = surfaceDest.size;
        
        if (size > 0)
            fwrite(surfaceDest.buffer, 1, size, destFile);

        fclose(destFile);

        BITsurfaceDestroy(&surfaceDest);
        BITsurfaceDestroy(&surfaceSource);
    }

    return 0;

readerror:
    printf ("#ERROR: cannot open file %s for read\n", sourceFilename);
    return 1;
}

