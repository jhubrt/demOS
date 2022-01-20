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


/*! @brief @ref BINARIZE @file */

/*! @defgroup BINARIZE

  BINARIZE tool transforms your raw data stored into DATA folder to baked data into DATABIN folder <br>

  Typical workflow is : <br>

  DATA => (BINARIZE) => DATABIN => (ARJ batchs) => DATABIN => (IMAGER) => Floppy images          
*/

extern "C"
{
#   include "DEMOSDK\BASTYPES.H"
#   include "DEMOSDK\SYSTEM.H"
#   include "DEMOSDK\STANDARD.H"

#   include "DEMOSDK\PC\WINDOW.H"
#   include "DEMOSDK\PC\BMPLOADER.H"
#   include "DEMOSDK\PC\BITCONVR.H"

    /* imported from MPP */
    int bmp2mpp(const char* filename, int _mode, int ste, int extra, int doubl, int optlevel, int randseed, const char* outfilename, int raw_palette);
    void mpp2bmp(const char* _mppfilename, int _mode, int ste, int extra, int doubl, const char* bmpfilename, int raw_palette);
}

#include "TOOLS\BINARIZE\HELPERS.H"

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

/* some services */
static void waitForGUI (WINdow* _window)
{
    s32 k = 0;

    do
    {
        WINgetMouse (_window, NULL, NULL, &k, NULL);
    }
    while (!WINisKeyHit(_window) && !WINisClosed(_window) && !k);

    if ( !WINisClosed(_window) )
    {
        while ( WINisKeyHit(_window) )
        {
            WINgetKey(_window);
        }

        do
        {
            WINgetMouse (_window, NULL, NULL, &k, NULL);
        }
        while (k);
    }
}

static void readcheck (void* _buf, u32 _size, FILE* _file)
{
    if ( fread (_buf, _size, 1, _file) != 1 )
    {
        printf ("#ERROR while reading %d bytes\n", _size);
    }
}

static void writecheck (void* _buf, u32 _size, FILE* _file)
{
    if ( fwrite (_buf, _size, 1, _file) != 1 )
    {
        printf ("#ERROR while writing %d bytes\n", _size);
    }
}


static void displayImage (WINdow* _window, BITsurface* _surface)
{
    if ( _window != NULL )
    {
        BITsurface temp;
        BITsurface* surface = _surface;


        if (( _surface->format != BITformat_8bits ) && ( _surface->format != BITformat_x888 ))
        {
            surface = &temp; 
            
            BITsurfaceConstruct (surface);
            BITsurfaceConvert (&stdAllocator, _surface, surface, BITformat_8bits);
        }

        WINdrawImage (
            _window, 
            surface->buffer, 
            surface->width, surface->height, 
            surface->format == BITformat_8bits ? 8 : 32, 
            surface->format == BITformat_8bits ? surface->lut.data.p : NULL,
            (768 - surface->width) / 2, (576 - surface->height) / 2);

        WINrender (_window);
        waitForGUI(_window);

        if ( surface == &temp )
        {
            BITsurfaceDestroy(surface);
        }
    }
}


static void truncateFile (const char* _source, const char* _dest, u32 _size)
{
    u8* buffer = NULL;
    u32 filesize;
    FILE* file = fopen(_source, "rb");
    ASSERT(file != NULL);

    fseek (file, 0, SEEK_END);
    filesize = ftell(file);
    ASSERT(filesize >= _size);

    buffer = (u8*) malloc(_size);

    fseek (file, 0, SEEK_SET);
    fread (buffer, 1, _size, file);
    fclose (file);

    file = fopen (_dest, "wb");
    ASSERT(file != NULL);
    fwrite(buffer, 1, _size, file);
    fclose (file);
}


/* binarization functions */
static void binarizeSystemFont (WINdow* _window)
{
    u8* buffer = (u8*) malloc(32000);
    {
        FILE* file = fopen("DATA\\SYSTFONT.NEO", "rb");
        ASSERT(file != NULL);

        fseek (file, 128, SEEK_SET);
        fread (buffer, 1, 32000, file);
        fclose (file);
    }

    {
        u16 t, i;

        FILE* file = fopen("DATABIN\\SYSTFONT.BIN", "wb");
        ASSERT(file != NULL);

        for (i = 0 ; i < 2 ; i++)
        {
            for (t = 0 ; t < 40 ; t++)
            {
                u16 j;

                for (j = 0 ; j < 8 ; j ++)
                {	
                    u32 off = (i * 8 + j) * 160;

                    if ( t & 1 )
                    {
                        off++;
                    }

                    off += (t >> 1) * 8;

                    fwrite (&buffer[off], 1, 1, file);
                }
            }
        }

        for (i = 0 ; i < 8 ; i++)
        {
            char c = 0;
            fwrite (&c, 1, 1, file);
        }

        fclose (file);
    }

    {
        BITsurface surface, surface2;
        u16 x,y,j;


        BITsurfaceConstruct(&surface);
        BITsurfaceConstruct(&surface2);

        BITsurfaceSetExternal (&surface, buffer, BITformat_Chunk4P, 320, 200, 160);
        BITsurfaceConvert (&stdAllocator, &surface, &surface2, BITformat_8bits);

        if (_window != NULL)
        {
            WINdrawImage (_window, surface2.buffer, 320, 200, 8, surface2.lut.data.p, 0, 0);        
            WINsetColor (_window, 255, 0, 0);

            for (y = 0 ; y < 16 ; y += 8)
            {
                for (x = 0 ; x < 320 ; x += 8)
                {
                    for (j = 0 ; j < 8 ; j ++)
                    {
                        if ( surface2.buffer[(y + j) * surface2.pitch + x + 7 ] != 0 )
                        {
                            WINfilledRectangle (_window, x, y, x + 4, y + 4);
                            break;
                        }

                        /*                    if ( surface2.buffer[(y + 7) * surface2.pitch + x + j ] != 0 )
                        {
                        WINfilledRectangle (_window, x, y, x + 4, y + 4);
                        break;
                        }*/
                    }
                }
            }

            WINrender (_window);
            waitForGUI(_window);
        }

        BITsurfaceDestroy(&surface2);
        BITsurfaceDestroy(&surface);
    }

    free (buffer);
    buffer = NULL;

    TOOLwriteStaticData("DATABIN\\SYSTFONT.BIN", "..\\DEMOSDK\\DATA\\SYSTFNT.H", "SYSfontdata");
}


static void binarizeSound (void)
{
    u32 samples [] = 
    {
        4190UL,			/* 1 */		44504UL,
        360222UL,		/* 2 */		44504UL,
        1606334UL,		/* 3 */		89008UL,
        1784382UL,		/* 4 */		89008UL,
        3475534UL,		/* 5 */		89008UL,
        3564542UL,		/* 6 */		89008UL,
        5967758UL,		/* 7 */		89008UL,
        6056766UL,		/* 8 */		89008UL,
        7391886UL,		/* 9 */		89008UL,
        7569902UL,		/* 10 */	89008UL,
        12465342UL,		/* 11 */	89008UL,
        14245502UL,		/* 12 */	44504UL,
        14646038UL,		/* 13 */	44504UL
    };

    u16 t;

    FILE* fileSource = fopen ("DATA\\ZIK\\DUALIS.RAW", "rb");
    ASSERT (fileSource != NULL);

    for (t = 0 ; t < ARRAYSIZE(samples) ; t += 2 )
    {
        char temp[256];		
        FILE* fileDest;
        void* buffer;
        u32 sz;

        sprintf (temp, "DATABIN\\ZIK\\%u.RAW", samples[t]);
        fileDest = fopen (temp, "wb");
        ASSERT (fileDest != NULL);

        fseek (fileSource, samples[t], SEEK_SET);

        buffer = malloc (samples[t + 1]);
        ASSERT (buffer != NULL);
        sz = fread (buffer, 1, samples[t + 1], fileSource);
        ASSERT (sz == samples[t + 1]);

        sz = fwrite (buffer, 1, samples[t + 1], fileDest);
        ASSERT (sz == samples[t + 1]);
        free (buffer);

        fclose (fileDest);
    }

    fclose (fileSource);
}


static void slideCreatePointsList( char* _imageSource, char* _ptDest )
{
    BITsurface surface;

    BITsurfaceConstruct(&surface);

    BITbmpLoad (&surface, &stdAllocator, _imageSource);

    {
        u16* maskx = (u16*) malloc(surface.width * surface.height * 3);
        u8*  masky = (u8*) (maskx + surface.width * surface.height);
        u8*  p = surface.buffer;    
        u32  i = 0;
        u16  x,y;


        for (y = 0 ; y < surface.height ; y++)
        {
            for (x = 0 ; x < surface.width ; x++)
            {
                if ( p[x] != 0 )
                {
                    maskx[i] = PCENDIANSWAP16(x);
                    masky[i] = (u8) y;
                    i++;
                }
            }

            p += surface.pitch;
        }

        if ( i & 1 )
        {
            maskx[i] = maskx[0];
            masky[i] = masky[0];
            i++;
        }

        if ( i != 0 )
        {
            FILE* file = fopen (_ptDest, "wb");
            u16 nb = PCENDIANSWAP16((u16)i);

            fwrite (&nb  , sizeof(nb)   , 1, file);
            fwrite (maskx, sizeof(u16)  , i, file);
            fwrite (masky, sizeof(u8)   , i, file);

            fclose (file);
        }

        free (maskx);

        printf ("total points mask = %ld\n", i);
    }

    BITsurfaceDestroy(&surface);
}


static void binarizeRebirthLogo( WINdow* window )
{
    u8* image;
    u8* imagedest;

    image = (u8*) malloc (32034);
    imagedest = (u8*) malloc (32000);

    {
        FILE* file = fopen ("DATA\\POLYZOOM\\REBIRTH.PI1", "rb");
        readcheck (image, 32034, file);
        fclose (file);
    }

    if ( window != NULL )
    {
        BITsurface surface, surface2;


        BITsurfaceConstruct(&surface);
        BITsurfaceConstruct(&surface2);

        BITsurfaceSetExternal (&surface, image + 34, BITformat_Chunk4P, 320, 200, 160);
        BITlutSetExternal (&surface.lut, BITlutFormat_STe, image + 2, 16);
        BITsurfaceConvert (&stdAllocator, &surface, &surface2, BITformat_8bits);

        WINdrawImage (window, surface2.buffer, surface2.width, surface2.height, 8, surface2.lut.data.p, (768 - surface2.width) / 2, (576 - surface2.height) / 2);
        WINrender (window);

        waitForGUI(window);

        BITsurfaceDestroy(&surface2);
        BITsurfaceDestroy(&surface);			
    }

    {
        bool planes[4] = {1,1,1,0};

        BITignorePlanes (image + 34, planes, 20, 160, 200, imagedest);
    }

    {
        FILE* file = fopen ("DATABIN\\POLYZOOM\\REBIRTH.3CH", "wb");
        /* writecheck (image + 2, 32, file); */
        writecheck (imagedest, 7680, file);
        fclose (file);
    }

    free(image);
    free(imagedest);
}


static void binarizeVisualizerTexts( WINdow* _window )
{
    BITsurface surface, surface2, surface3;
    u16 nbbits;


    BITsurfaceConstruct(&surface);
    BITsurfaceConstruct(&surface2);
    BITsurfaceConstruct(&surface3);

    BITbmpLoad (&surface, &stdAllocator, "DATA\\VISUALIZ\\BITMAP_8.BMP");

    {
        u16 x,y;

        for (y = 0 ; y < surface.height ; y++)
        {
            for (x = 0 ; x < surface.width ; x++)
            {
                u8* p = &surface.buffer[y * surface.pitch + x];

                switch (*p)
                {
                case 0:
                    *p = 1;
                    break;
                case 1:
                    *p = 0;
                    break;
                }
            }
        }
    }

    BITsurfaceConvert (&stdAllocator, &surface, &surface3, BITformat_Chunk2P);

    if (_window != NULL)
    {
        BITsurfaceConvert (&stdAllocator, &surface3, &surface2, BITformat_x888);

        if ( surface2.format == BITformat_8bits )
            nbbits = 8;
        else
            nbbits = 32;

        WINdrawImage (_window, surface2.buffer, surface2.width, surface2.height, nbbits, surface2.lut.data.p, (768 - surface2.width) / 2, (576 - surface2.height) / 2);
        WINrender (_window);

        waitForGUI(_window);
    }

    {
        FILE* file = fopen ("DATABIN\\VISUALIZ\\TEXTOS.2CH", "wb");
        writecheck (surface3.buffer, 32000, file);
        fclose (file);

        file = fopen ("DATABIN\\VISUALIZ\\TEXTOS.PAL", "wb");
        writecheck (surface3.lut.data.p16, 8, file);
        fclose (file);
    }        

    BITsurfaceDestroy(&surface3);
    BITsurfaceDestroy(&surface2);
    BITsurfaceDestroy(&surface);
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

        WINdrawImage (_window, surface2.buffer, surface2.width, surface2.height, 32, NULL, (768 - surface2.width) / 2, (576 - surface2.height) / 2);
        WINrender (_window);

        waitForGUI(_window);
    }

    BITsurfaceDestroy(&surface2);
    BITsurfaceDestroy(&surface);
}


static void binarizeFugit( WINdow* _window )
{
    BITsurface surface, surface2;
    void* buffer = malloc(48000);
    u16 x,y,c;
    u16 cov = 0,uncov = 0;

    FILE* file = fopen ("DATA\\FUGIT\\LETTERS.BIN", "rb");
    ASSERT (file != NULL);
    fread (buffer, 48000, 1, file);
    fclose (file);

    BITsurfaceConstruct (&surface);
    BITsurfaceConstruct (&surface2);

    BITsurfaceSetExternal (&surface, buffer, BITformat_Chunk2P, 640, 300, 160);
    BITsurfaceConvert (&stdAllocator, &surface, &surface2, BITformat_8bits);

    if ( _window != NULL )
    {
        WINdrawImage (_window, surface2.buffer, surface2.width, surface2.height, 8, surface2.lut.data.p, (768 - surface2.width) / 2, (576 - surface2.height) / 2);
        WINrender (_window); 
        waitForGUI(_window);
    }

    file = fopen ("DATA\\FUGIT\\LETTERS2.BIN", "rb");
    fread (buffer, 24000, 1, file);
    fclose (file);

    BITsurfaceSetExternal (&surface, buffer, BITformat_Plane1P, 640, 300, 80);
    BITsurfaceConvert (&stdAllocator, &surface, &surface2, BITformat_8bits);

    //     if ( _window != NULL )
    //     {
    //         WINdrawImage (_window, surface2.buffer, surface2.width, surface2.height, 8, surface2.lut.data.p, (768 - surface2.width) / 2, (576 - surface2.height) / 2);
    //     }

#       define CHARLEN 64
#       define CHARH   100
#       define NBCHARS 26

    {
        FILE* filefont;
        FILE* fileoffset;

        fileoffset = fopen ("DATABIN\\FUGIT\\FONTOFFS.BIN", "wb");           
        ASSERT (fileoffset != NULL);

        filefont = fopen ("DATABIN\\FUGIT\\FONT.BIN", "wb");
        ASSERT (filefont != NULL);

        file = fopen ("DATABIN\\FUGIT\\FONTSCAN.BIN", "wb");
        ASSERT (file != NULL);

        for (c = 0 ; c < NBCHARS ; c++)
        {
            u16 xs = (c % 10) * CHARLEN;
            u16 ys = (c / 10) * CHARH;
            u16 yl = 0;
            u8  scans[CHARH];
            u16 nbscans = 0;
            u16 n;


            n = (u16) ftell(file);
            n = PCENDIANSWAP16(n);
            fwrite (&n, sizeof(n), 1 ,fileoffset);

            n = (u16) ftell(filefont);
            n = PCENDIANSWAP16(n);
            fwrite (&n, sizeof(n), 1 ,fileoffset);

            //WINsetColor(window, 0, 128, 0);
            //WINrectangle(window, (768 - surface2.width) / 2 + xs, (576 - surface2.height) / 2 + ys, (768 - surface2.width) / 2 + xs + CHARLEN - 1, ys + 5 - 1 + (576 - surface2.height) / 2);

            for (y = ys ; y < (ys + CHARH) ; y++)
            {
                bool found = false;

                for (x = xs ; x < (xs + CHARLEN) ; x ++)
                {        
                    if ( surface2.buffer[y * 640 + x] != 0 )
                    {
                        found = true;
                        break;
                    }
                }

                if ( found )
                {
                    scans[nbscans++] = (y - ys) - yl;
                    fwrite (&surface.buffer[y * surface.pitch + xs / 8], 8, 1, filefont);
                    yl = y - ys;
                }
                else
                {
                    surface.buffer[y * surface.pitch + xs / 8 + 7] = 0xFF;
                    //WINsetColor(window, 255, 0, 0);
                    //WINline(window, (768 - surface2.width) / 2 + xs, y + (576 - surface2.height) / 2, (768 - surface2.width) / 2 + xs + CHARLEN - 1, y + (576 - surface2.height) / 2);
                    uncov++;
                }

                cov++;
            }

            n = PCENDIANSWAP16(nbscans);
            fwrite (&n, sizeof(n), 1, file);

            if (nbscans > 0)
            {
                for (x = 0 ; x < nbscans ; x++)
                {
                    n = 80 * (u16) scans[x];
                    n = PCENDIANSWAP16(n);
                    fwrite (&n, sizeof(n), 1, file);
                }
            }
        }

        fclose(file);
        fclose(filefont);
        fclose(fileoffset);
    }

    printf ("cov=%d / uncov=%d\n", cov, uncov);

    if ( _window != NULL )
    {
        BITsurfaceConvert (&stdAllocator, &surface, &surface2, BITformat_8bits);

        WINdrawImage (_window, surface2.buffer, surface2.width, surface2.height, 8, surface2.lut.data.p, (768 - surface2.width) / 2, (576 - surface2.height) / 2);
        WINrender (_window);
        waitForGUI(_window);
    }

    free(buffer);
    buffer = NULL;
}


static void binarizeBootSplash(WINdow* _window)
{
    BITsurface surface, surface2;

    BITsurfaceConstruct (&surface);
    BITsurfaceConstruct (&surface2);

    BITbmpLoad (&surface, &stdAllocator, "DATA\\BOOT\\BOOT2P.BMP");
    ASSERT(surface.format == BITformat_8bits);

    {
        u32 t;
        u32 nb = surface.width * surface.height;

        for (t = 0 ; t < nb ; t++)
        {
            surface.buffer[t] = 3 - surface.buffer[t];
        }
    }

    BITsurfaceConvert (&stdAllocator, &surface, &surface2, BITformat_Chunk2P);

    {
        FILE* file = fopen ("DATABIN\\BOOT\\BOOT.2P", "wb");
        ASSERT (file != NULL);

        fwrite (surface2.buffer, 1, 32000, file);

        fclose (file);
    }

    displayImage (_window, &surface);

    BITsurfaceDestroy (&surface);
    BITsurfaceDestroy (&surface2);
}


/* Main */

void main (char argc, char** argv)
{
    WINinitParam init;
    WINdow* window = NULL;

    int command = (argc > 1) ? atoi(argv[1]) : -1;


    if ( command != -1 )
    {
        init.x = WINDOW_CENTER;
        init.y = WINDOW_CENTER;
        init.w = 768;
        init.h = 576;
        init.title = argv[1];
        init.hInstance = NULL;

        window = WINconstruct (&init);
    }

    if (( command == -1 ) || ( command == 1 ))
    {
        printf ("binarize system font\n");
        binarizeSystemFont (window);
    }

    if (( command == -1 ) || ( command == 2 ))
    {
        printf ("binarize sound\n");
        binarizeSound ();
    }

    if (( command == -1 ) || ( command == 3 ))
    {
        printf ("binarize Rebirth logo\n");
        binarizeRebirthLogo(window);
    }

    if (( command == -1 ) || ( command == 4 ))
    {
        printf ("binarize visualizer texts\n");
        binarizeVisualizerTexts(window);
    }

    if ((command == -1) || (command == 5))
    {
        printf ("binarize slides pictures\n");
        binarizeSlidePicture ("DATA\\SLIDES\\PHOT5.BMP", "DATABIN\\SLIDES\\", window);
        binarizeSlidePicture ("DATA\\SLIDES\\PHOT6.BMP", "DATABIN\\SLIDES\\", window);
        binarizeSlidePicture ("DATA\\SLIDES\\PHOT8.BMP", "DATABIN\\SLIDES\\", window);
        binarizeSlidePicture ("DATA\\SLIDES\\PHOTF.BMP", "DATABIN\\SLIDES\\", window);

        /* compression ratio test => not a large diff between 4p and chunk4 => 2%

        FILE* file = fopen("DATABIN\\SLIDES\\TEST.MPP", "rb");
        u8* buffer = malloc(55720);

        fread (buffer, 55720,1, file);
        fclose (file);

        file = fopen("DATABIN\\SLIDES\\TEST.PAL", "wb");
        fwrite (buffer, 199 * 56 * 2, 1, file);
        fclose (file);

        file = fopen("DATABIN\\SLIDES\\TEST.BIT", "wb");
        fwrite (buffer + 199 * 56 * 2, 199 * 168, 1, file);
        fclose (file);

        {
        BITsurface surface, surface2;

        BITsurfaceConstruct(&surface);
        BITsurfaceConstruct(&surface2);

        BITsurfaceSetExternal(&surface, buffer + 199 * 56 * 2, BITformat_Chunk4P, 336, 199, 168);

        BITsurfaceConvert(&stdAllocator, &surface, &surface2, BITformat_4bits);

        file = fopen("DATABIN\\SLIDES\\TEST.B4B", "wb");
        fwrite (surface2.buffer, 199 * 168, 1, file);
        fclose (file);
        } */
    }

    if ((command == -1) || (command == 6))
    {
        printf ("binarize slides morph points\n");

        slideCreatePointsList( "DATA\\SLIDES\\PHOT5MSK.BMP", "DATABIN\\SLIDES\\PHOT5MSK.PT" );
        slideCreatePointsList( "DATA\\SLIDES\\PHOT6MSK.BMP", "DATABIN\\SLIDES\\PHOT6MSK.PT" );
        slideCreatePointsList( "DATA\\SLIDES\\PHOT8MSK.BMP", "DATABIN\\SLIDES\\PHOT8MSK.PT" );
        slideCreatePointsList( "DATA\\SLIDES\\PHOTFMSK.BMP", "DATABIN\\SLIDES\\PHOTFMSK.PT" );

        truncateFile ( "DATA\\SLIDES\\PHOT5.PAL", "DATABIN\\SLIDES\\PHOT5.PA8", 16 );
        truncateFile ( "DATA\\SLIDES\\PHOT6.PAL", "DATABIN\\SLIDES\\PHOT6.PA8", 16 );
        truncateFile ( "DATA\\SLIDES\\PHOT8.PAL", "DATABIN\\SLIDES\\PHOT8.PA8", 16 );
        truncateFile ( "DATA\\SLIDES\\PHOTF.PAL", "DATABIN\\SLIDES\\PHOTF.PA8", 16 );
    }

    if ((command == -1) || (command == 7))
    {
        printf ("binarize fugit font\n");
        binarizeFugit(window);
    }

    if ((command == -1) || (command == 8))
    {
        printf ("binarize bootsector splash screen\n");
        binarizeBootSplash(window);
    }

    if (window != NULL)
    {
        WINdestroy(window);
    }
}
