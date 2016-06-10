/*------------------------------------------------------------------------------  -----------------
  The MIT License (MIT)

  Copyright (c) 2015-2016 J.Hubert

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
------------------------------------------------------------------------------------------------- */

#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\ALLOC.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"
#include "DEMOSDK\SOUND.H"
#include "DEMOSDK\BITMAP.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\COLORS.H"

#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"

#include "REBIRTH\SRC\SCREENS.H"
#include "REBIRTH\SRC\SLIDES.H"
#include "REBIRTH\SRC\MORPH.H"
#include "REBIRTH\SRC\SNDTRACK.H"

#include "EXTERN\ARJDEP.H"

#include "REBIRTH\DISK1.H"
#include "REBIRTH\DISK2.H"


static u8 SlidePhotoIndexes[] =
{
    RSC_DISK2_SLIDES__PHOTF_HC,
    RSC_DISK2_SLIDES__PHOT5_HC,
    RSC_DISK2_SLIDES__PHOT8_HC,
    RSC_DISK2_SLIDES__PHOT6_HC
};

void SLItc	          (void) PCSTUB;
void SLIinit          (void* _tcbuf, u16 _tcHeight) PCSTUB;
void SLIfadetogrey    (u16* _src, u16* _dest, u32 _converter, u16 _h) PCSTUB;
void SLIconvertToGrey (u16* _srcbit, u16* _srcpal, u32 _dest, u16 _h) PCSTUB;

ASMIMPORT u16   SLItunX1;
ASMIMPORT u16   SLItunX2;
ASMIMPORT u16*  SLItcBuf;

#define SLI_COLORS      54
#define SLI_WIDTH	    336
#define SLI_HEIGHT	    198
#define SLI_FB_SIZE     (200UL * 168UL)
#define SLI_PICT_HEIGHT	199
#define SLI_HCDISPLAY_NBFRAMES  150

static void SlidePrecomputeGradient (u16* _dest)
{
    u16 c;
    s16 coef;


    for (coef = 1 ; coef < 17 ; coef++)
    {
        s8* mul = &g_screens.slides->multable[coef][16];

        for (c = 0 ; c < 4096 ; c++)
        {
            s16 sr = COLST24b[(c & 0xF00) >> 8];
            s16 sg = COLST24b[(c & 0xF0 ) >> 4];
            s16 sb = COLST24b[(c & 0xF  )     ];

            {
                s16 grey = g_screens.slides->greyscaler[c];

                u16 r = mul[grey - sr] + sr;
                u16 g = mul[grey - sg] + sg;
                u16 b = mul[grey - sb] + sb;

                /*u16 r = (((grey - sr) * coef) >> 4) + sr;
                u16 g = (((grey - sg) * coef) >> 4) + sg;
                u16 b = (((grey - sb) * coef) >> 4) + sb;*/

                r = COL4b2ST[r];
                g = COL4b2ST[g];
                b = COL4b2ST[b];

                *_dest++ = (r << 8) | (g << 4) | b;
            }
        }
    }
}


static void SLIinitDeployer(void)
{
    u16 i;

    for (i = 0 ; i < 0x1000 ; i++)
    {
        g_screens.slides->deployer[0][i] = i;
        g_screens.slides->deployer[1][i] = i;
    }

    for (i = 0x1000 ; i < 0x8000 ; i++)
    {
        u16 lowbits = i >> 12;
        u16 r = COLST24b[(i & 0xF00) >> 8];
        u16 g = COLST24b[(i & 0xF0 ) >> 4];
        u16 b = COLST24b[ i & 0xF        ];

        u16 r2 = r;
        u16 g2 = g;
        u16 b2 = b;

        switch (lowbits)
        {
        case 0: 
            ASSERT(0);
            break;
        case 1:         /*   B */
            b2++;            
            break;
        case 2:         /*  G  */
            g++;
            break;
        case 3:         /*  GB */
            g2++;
            b++;
            break;
        case 4:         /* R   */
            r2++;            
            break;
        case 5:         /* R B */
            r++;
            b2++;
            break;
        case 6:         /* RG  */
            r2++;
            g++;
            break;
        case 7:         /* RGB */
            r++;
            g2++;
            b++;
            break;
        }

        g_screens.slides->deployer[0][i] = ((u16)COL4b2ST[r ] << 8) | ((u16)COL4b2ST[g ] << 4) | (u16)COL4b2ST[b ];
        g_screens.slides->deployer[1][i] = ((u16)COL4b2ST[r2] << 8) | ((u16)COL4b2ST[g2] << 4) | (u16)COL4b2ST[b2];
    }
}


static void SlideMaskColors (u16* _src, u16* _dest, u16 _h)
{
    u16 y, i;


    for (y = 0 ; y < _h ; y++)
    {
        for (i = 0 ; i < 56 ; i++)
        {   
            *_dest++ = *_src++ & 0xFFF;
        }
    }
}


static void SlideDeployPaletteFlipAndMask (u16* _src, u16* _dest, u16 _h)
{
    u16 y, i, c;
    u16* dest2 = _dest + (_h * 64);
    u16* deploy0 = g_screens.slides->deployer[0];
    u16* deploy1 = g_screens.slides->deployer[1];


    for (y = 0 ; y < _h ; y++)
    {
        *_dest++ = 0;
        *dest2++ = 0;

        for (i = 0 ; i < 47 ; i++)
        {   
            c = *_src;
            *_src++ &= 0xFFF;
            *_dest++ = deploy0[c];
            *dest2++ = deploy1[c];
        }

        *_dest++ = 0;
        *dest2++ = 0;

        for (i = 0 ; i < 9 ; i++)
        {
            c = *_src;
            *_src++ &= 0xFFF;
            *_dest++ = deploy0[c];
            *dest2++ = deploy1[c];
        }

        *_dest++ = 0;
        *_dest++ = 0;
        *_dest++ = 0;
        *_dest++ = 0;
        *_dest++ = 0;
        *_dest++ = 0;

        *dest2++ = 0;
        *dest2++ = 0;
        *dest2++ = 0;
        *dest2++ = 0;
        *dest2++ = 0;
        *dest2++ = 0;
    }
}


/*static u16 testcolor = 0;*/

#define SLI_ARROW_CORNER  40
#define SLI_ARROW_SIZE    32

static void SlidesDisplayArrow (u8* _p)
{
    u16* start = g_screens.slides->startpos;
    u16* end   = g_screens.slides->endpos;
    u16  t, nb;
    
    
    for (t = 0 ; t <= SLI_ARROW_CORNER ; t += 4)
    {
        *start++ = 0;                       *start++ = t;
        *start++ = t;                       *start++ = 0;
        *start++ = SLI_WIDTH - t - 1;       *start++ = 0;
        *start++ = SLI_WIDTH - 1;           *start++ = t;

        *start++ = 0;                       *start++ = t + 195 - SLI_ARROW_CORNER;
        *start++ = t;                       *start++ = 195;
        *start++ = SLI_WIDTH - t - 1;       *start++ = 195;
        *start++ = SLI_WIDTH - 1;           *start++ = t + 195 - SLI_ARROW_CORNER;

        *start++ = 4;                       *start++ = t + 4;
        *start++ = t + 4;                   *start++ = 4;
        *start++ = SLI_WIDTH - t - 1 - 4;   *start++ = 4;
        *start++ = SLI_WIDTH - 1 - 4;       *start++ = t + 4;

        *start++ = 4;                       *start++ = t + 195 - SLI_ARROW_CORNER - 4;
        *start++ = t + 4;                   *start++ = 195 - 4;
        *start++ = SLI_WIDTH - t - 1 - 4;   *start++ = 195 - 4;
        *start++ = SLI_WIDTH - 1 - 4;       *start++ = t + 195 - SLI_ARROW_CORNER - 4;
    }

    for (t = 0 ; t <= SLI_ARROW_SIZE ; t += 4)
    {
        *start++ = SLI_WIDTH >> 1;
        *start++ = (196 >> 1) - (SLI_ARROW_SIZE >> 1) + t;

        *start++ = (SLI_WIDTH >> 1) - (SLI_ARROW_SIZE >> 1) + t;
        *start++ = 196 >> 1;
    }    

    nb = ((u8*)start - (u8*)g_screens.slides->startpos) >> U16_SIZEOF_SHIFT;

    for (t = 0 ; t < nb ; t++)
    {
        *end++ = 0;
    }

    nb >>= 1;

    SLIstartMorph (g_screens.slides->startpos, g_screens.slides->endpos, (u32) g_screens.slides->morphcode, (u32) g_screens.slides->pos, nb);

    SLIdisplayMorph(
        (u16*)(_p + SLI_WIDTH), 
        g_screens.slides->pos, 
        (u32) g_screens.slides->verttable, 
        (u32) g_screens.slides->horitable, 
        nb);
}


void SlidesEntry (FSM* _fsm)
{
    u32 photoSize = LOADresourceRoundedSize(&RSC_DISK2, SlidePhotoIndexes[0]);
    LOADrequest* loadRequest;    
    LOADrequest* loadRequest2;
    u16 t;


    IGNORE_PARAM(_fsm);
    
    STDmset(HW_COLOR_LUT, 0, 32);

	g_screens.slides = MEM_ALLOC_STRUCT( &sys.allocatorMem, Slides );	
    DEFAULT_CONSTRUCT(g_screens.slides);

    g_screens.slides->bitmaps[0]  = (u8*) RINGallocatorAlloc ( &sys.mem, photoSize * 2UL );
    g_screens.slides->bitmaps[1]  = g_screens.slides->bitmaps[0] + photoSize;

    g_screens.slides->palettes[0] = (u8*) RINGallocatorAlloc ( &sys.mem, 64UL * sizeof(u16) * SLI_HEIGHT * 4UL);
    g_screens.slides->palettes[1] = g_screens.slides->palettes[0] + 64UL * sizeof(u16) * SLI_HEIGHT;
    g_screens.slides->palettes[2] = g_screens.slides->palettes[1] + 64UL * sizeof(u16) * SLI_HEIGHT;
    g_screens.slides->palettes[3] = g_screens.slides->palettes[2] + 64UL * sizeof(u16) * SLI_HEIGHT;

    STDmset (g_screens.slides->palettes[0], 0, 64UL * sizeof(u16) * SLI_HEIGHT * 4UL);

	g_screens.slides->framebuffers[0] = (u8*) RINGallocatorAlloc ( &sys.mem, SLI_FB_SIZE * 4UL);
	g_screens.slides->framebuffers[1] = g_screens.slides->framebuffers[0] + SLI_FB_SIZE;
	g_screens.slides->framebuffers[2] = g_screens.slides->framebuffers[1] + SLI_FB_SIZE;
	g_screens.slides->framebuffers[3] = g_screens.slides->framebuffers[2] + SLI_FB_SIZE;

    g_screens.slides->morphcode   = (u8*)  RINGallocatorAlloc ( &sys.mem, SLIgetMorphCodeLen(SLI_MORPHPOINTS));
    g_screens.slides->horitable   = (u32*) RINGallocatorAlloc ( &sys.mem, SLI_WIDTH       * sizeof(u32));
    g_screens.slides->verttable   = (u16*) RINGallocatorAlloc ( &sys.mem, SLI_HEIGHT      * sizeof(u16));
    g_screens.slides->startpos    = (u16*) RINGallocatorAlloc ( &sys.mem, SLI_MORPHPOINTS * sizeof(u16) * 2);
    g_screens.slides->endpos      = (u16*) RINGallocatorAlloc ( &sys.mem, SLI_MORPHPOINTS * sizeof(u16) * 2);
    g_screens.slides->pos         = (u16*) RINGallocatorAlloc ( &sys.mem, SLI_MORPHPOINTS * sizeof(u16) * 3);
    g_screens.slides->points[0]   = (u16*) RINGallocatorAlloc ( &sys.mem, LOADresourceRoundedSize(&RSC_DISK2, RSC_DISK2_SLIDES__MASKS_PT) );

	loadRequest  = LOADdata (&RSC_DISK2, SlidePhotoIndexes[0], g_screens.slides->bitmaps[0], LOAD_PRIORITY_INORDER);
    loadRequest2 = LOADdata (&RSC_DISK2, RSC_DISK2_SLIDES__MASKS_PT, g_screens.slides->points[0], LOAD_PRIORITY_INORDER);

    SLIinitMorph (g_screens.slides->verttable, g_screens.slides->horitable, SLI_HEIGHT, SLI_WIDTH >> 1, SLI_WIDTH);

    {
        /* prepare and launch wait effect */
        STDmset(g_screens.slides->framebuffers[0], 0, SLI_FB_SIZE);
        g_screens.slides->pulseInc = 1;
        SlidesDisplayArrow(g_screens.slides->framebuffers[0]);
        SYSwriteVideoBase((u32) g_screens.slides->framebuffers[0]);
        FSMgotoNextState (&g_stateMachine);
    }

    g_screens.slides->deployer[0] = (u16*) RINGallocatorAlloc ( &sys.mem, 65536UL * 2UL );
    g_screens.slides->deployer[1] = g_screens.slides->deployer[0] + 32768UL;

    g_screens.slides->greyscaler  = (u8*)  RINGallocatorAlloc ( &sys.mem, 4096);
    g_screens.slides->gradients   = (u16*) RINGallocatorAlloc ( &sys.mem, 4096 * sizeof(u16) * 16);

    g_screens.slides->pointsgradient = (u8*) RINGallocatorAlloc ( &sys.mem, 8 * sizeof(u16) * 32);

    {           
        for (t = 0 ; t < 4 ; t++)
        {
            STDmset (g_screens.slides->framebuffers[t], 0, 336);
        }

        for (t = 1 ; t < ARRAYSIZE(SlidePhotoIndexes) ; t++)
        {
            u8* p = ((u8*) g_screens.slides->points[0]) + LOADmetadataOffset (&RSC_DISK2, RSC_DISK2_METADATA_SLIDES_PHOTFMSK_PT + t);
            g_screens.slides->points[t] = (u16*) p;
        }

        for (t = 0 ; t < ARRAYSIZE(SlidePhotoIndexes) ; t++)
        {
            u8* p = ((u8*) g_screens.slides->points[0]) + LOADmetadataOffset ( &RSC_DISK2, RSC_DISK2_METADATA_SLIDES_PHOTFMSK_PT + t + ARRAYSIZE(SlidePhotoIndexes) );
            g_screens.slides->pointspal[t+1] = (u16*) p;
        }
        
        g_screens.slides->pointspal[0] = g_screens.slides->pal;        
        g_screens.slides->pointspal[ARRAYSIZE(SlidePhotoIndexes) + 1] = &g_screens.slides->pal[8];

        STDmset (g_screens.slides->pointspal[0], 0x0FFF0FFFUL, 16);
        STDmset (g_screens.slides->pointspal[ARRAYSIZE(SlidePhotoIndexes) + 1], 0UL, 16);
    }

    SLIinitDeployer ();

    SLIgenerateMorphCode (g_screens.slides->morphcode, SLI_MORPHPOINTS);

    {
        for (t = 0 ; t < 4096 ; t++)
        {
            u16 r = COLST24b[(t & 0xF00) >> 8];
            u16 g = COLST24b[(t & 0xF0)  >> 4];
            u16 b = COLST24b[ t & 0xF ];

            /*u16 grey = (b * 10) + (r * 27) + (g * 91);*/
            u16 grey = (b * 14) + (r * 37) + (g * 77);
            grey >>= 7;

            g_screens.slides->greyscaler[t] = (u8) (grey & 0xE);
        }

        for (t = 0 ; t < 17 ; t++)
        {
            s16 i;

            for (i = -16 ; i < 16 ; i++)
            {
                g_screens.slides->multable[t][i + 16] = ((s16) t * i + 7) >> 4;
            }
        }

        SlidePrecomputeGradient(g_screens.slides->gradients);
    }

    SLItunX1 = 10;
    SLItunX2 = 10;

    if ( sys.isMegaSTe )
    {
        SLItunX2 -= 2;
    }

    g_screens.slides->rasterBootFunc = RASvbl1;
    g_screens.slides->rasterBootOp.scanLinesTo1stInterupt = 1;
    g_screens.slides->rasterBootOp.backgroundColor   = 0;
    g_screens.slides->rasterBootOp.nextRasterRoutine = SLItc;

    RASnextOpList = NULL;

    LOADwaitRequestCompleted ( loadRequest );	/* bitmap available ------------------------------------------ */
    LOADwaitRequestCompleted ( loadRequest2 );	/* points available ------------------------------------------ */
 
	snd.playerClientStep = STEP_SLIDES_STARTED;

    while (g_screens.slides->pulse != 0);

    SYSvsync;

    FSMgotoNextState (&g_stateMachineIdle);
    FSMgotoNextState (&g_stateMachine);

    HW_COLOR_LUT[1] = 0;
}


static void SlideGreyFade (u16* _src, u16* _dest, u32 _coef, u16 _h)
{
    ASSERT((_coef > 0) && (_coef < 17));

/*    if ( _coef == 0 )
    {
        u16 y;


        for (y = 0 ; y < _h ; y++)
        {
            *_dest++ = 0;

            STDmcpy (_dest, _src, 47 * sizeof(u16));
            _src += 47;
            _dest += 47;

            *_dest++ = 0;

            STDmcpy (_dest, _src, 9 * sizeof(u16));
            _src += 9;
            _dest += 9;

            *_dest++ = 0;
            *_dest++ = 0;
            *_dest++ = 0;
            *_dest++ = 0;
            *_dest++ = 0;
            *_dest++ = 0;
        }
    }
    else*/
    {
        u16* converter = g_screens.slides->gradients + ((_coef - 1) << 12);

#   ifdef __TOS__

        SLIfadetogrey (_src, _dest, (u32) converter, _h);

#   else

        u16 x, y;

        for (y = 0 ; y < _h ; y++)
        {
            _dest++;

            for (x = 1 ; x < 58 ; x++)
            {
                if (x == 48)
                {
                    _dest++;
                    x++;
                }

                *_dest++ = converter[*_src++];
            }

            _dest += 6;
        }

#   endif

    }
}


static void SlideConvertToGrey (u16* _srcbit, u16* _srcpal, u16* _dest, u16 _h)
{
#   ifdef __TOS__

    SLIconvertToGrey (_srcbit, _srcpal, (u32) _dest, _h);

#   else

    u16 srcbit [4];
    u16 destbit [4] = {0, 0, 0, 0};
    u16 pal [16];
    u16 x, y, b;
    u16 ip;

    
    for (y = 0 ; y < _h ; y++)
    {
        ip = 0;
        STDmcpy(pal, _srcpal, 16 * sizeof(u16));
        _srcpal += 16;

        for (x = 0 ; x < 336 ; x += 16)
        {
            srcbit[0] = *_srcbit++;
            srcbit[1] = *_srcbit++;
            srcbit[2] = *_srcbit++;
            srcbit[3] = *_srcbit++;

            for (b = 0 ; b < 16 ; b++)
            {
                u16 c = 0;

                if (srcbit[0] & 0x8000)
                    c |= 1;
                if (srcbit[1] & 0x8000)
                    c |= 2;
                if (srcbit[2] & 0x8000)
                    c |= 4;
                if (srcbit[3] & 0x8000)
                    c |= 8;

                srcbit[0] <<= 1;
                srcbit[1] <<= 1;
                srcbit[2] <<= 1;
                srcbit[3] <<= 1;

                c = pal[c];

                destbit[0] <<= 1;
                destbit[1] <<= 1;
                destbit[2] <<= 1;
                destbit[3] <<= 1;

                if (c & 0x8)
                    destbit[0] |= 1;
                if (c & 0x1)
                    destbit[1] |= 1;
                if (c & 0x2)
                    destbit[2] |= 1;
                if (c & 0x4)
                    destbit[3] |= 1;

                if ((b & 7) == 4)
                {
                    pal[ip++] = *_srcpal++;
                    ip &= 15;
                }
            }

            *_dest++ = destbit[0];
            *_dest++ = destbit[1];
            *_dest++ = destbit[2];
            *_dest++ = destbit[3];
        }

        _srcpal += 6;
    }

#   endif
}


static void SlidesInitMorphPoints (u16* _source, u16* _dest)
{
    u16  nb = *_source++;
    u16* x = _source;
    u8*  y = (u8*)(x + nb);
    u16  t, i;


    for (t = 0, i = 0 ; t < SLI_MORPHPOINTS ; t++)
    {
        *_dest++ = x[i];
        *_dest++ = y[i];

        i++;

        if ( i >= nb )
        {
            i = 0;
        }
    }
}


static void SlidesInitMorphPointsRandom (u16* _dest)
{
    u16 t;

    for (t = 0 ; t < SLI_MORPHPOINTS ; t++)
    {
        u16 v = STDifrnd() & 0x7FFF;

        switch (STDifrnd() & 3)
        {
        case 0:
            *_dest++ = v % SLI_WIDTH;
            *_dest++ = 0;
            break;
        case 1:
            *_dest++ = v % SLI_WIDTH;
            *_dest++ = 195;
            break;
        case 2:
            *_dest++ = 0;
            *_dest++ = v % SLI_HEIGHT;
            break;
        case 3:
            *_dest++ = SLI_WIDTH - 1;
            *_dest++ = v % SLI_HEIGHT;
            break;
        }
    }
}


static void SlidesInitMorphPointsCross (u16* _dest)
{
    u16 t;


    for (t = 0 ; t < SLI_MORPHPOINTS ; t++)
    {
        switch (STDifrnd() & 7)
        {
        case 0:
            *_dest++ = 0;
            *_dest++ = 0;
            break;
        case 1:
            *_dest++ = SLI_WIDTH - 1;
            *_dest++ = 0;
            break;
        case 2:
            *_dest++ = SLI_WIDTH - 1;
            *_dest++ = 195;
            break;
        case 3:
            *_dest++ = 0;
            *_dest++ = 195;
            break;
        case 4:
            *_dest++ = 0;
            *_dest++ = 98;
            break;
        case 5:
            *_dest++ = SLI_WIDTH / 2;
            *_dest++ = 0;
            break;
        case 6:
            *_dest++ = SLI_WIDTH - 1;
            *_dest++ = 98;
            break;
        case 7:
            *_dest++ = SLI_WIDTH / 2;
            *_dest++ = 195;
            break;
        }
    }
}

void SlidesInitActivity (FSM* _fsm)
{
    u16 c = g_screens.slides->pulse;


    IGNORE_PARAM(_fsm);
   
    if ( c & 1 )
    {
        c >>= 1;
        HW_COLOR_LUT[1] = (COL4b2ST[c] << 8) | (COL4b2ST[c + 1] << 4) | COL4b2ST[c];
    }
    else
    {
        c >>= 1;
        HW_COLOR_LUT[1] = (COL4b2ST[c] << 8) | (COL4b2ST[c] << 4) | COL4b2ST[c];
    }

    g_screens.slides->pulse += g_screens.slides->pulseInc;

    if (( g_screens.slides->pulse == 0 ) || ( g_screens.slides->pulse == 31))
    {
        g_screens.slides->pulseInc = -g_screens.slides->pulseInc;
    }
}

void SlidesActivity (FSM* _fsm)
{
    IGNORE_PARAM(_fsm);
    
    if ( g_screens.slides->palettefliped != NULL )
    {         
        SLItcBuf = (u16*) (g_screens.slides->palettefliped + g_screens.slides->palettecurrentflip);
        g_screens.slides->palettecurrentflip ^= 64 * 2 * SLI_HEIGHT;
    }

    /*(*HW_COLOR_LUT) = testcolor;*/
}


static void SlidePointsGradientSequenceStartEnd(u16 _index, bool _end)
{
    u16  startColors[8];
    u16* endColors = g_screens.slides->pointspal[_index];
    u16* gradient = (u16*)g_screens.slides->pointsgradient;
    u16  t;

    for (t = 0 ; t < 8 ; t++)
    {
        startColors[t] = ((u16)t << 8) | (t << 4) | t;
    }

    COLcomputeGradient (startColors, endColors, 8, 16, gradient);

    if ( _end )
    {
        gradient += 15 * 8;
    }

    for (t = 0 ; t < 16 ; t++)
    {
        s16 i;
        u16* c = HW_COLOR_LUT + 1;

        SYSvsync;
        SYSvsync;

        for (i = 7 ; i != -1 ; i--)
        {
            *c = *gradient++;
            c += 2;
        }

        if ( _end )
        {
            gradient -= 16;
        }
    }
}

static void SlideInitPointsGradient(u16 _index)
{
    u16* startColors = g_screens.slides->pointspal[_index];
    u16* endColors   = g_screens.slides->pointspal[_index + 1];

    COLcomputeGradient (startColors, endColors, 8, 16, (u16*) g_screens.slides->pointsgradient);
}


void SlidesBacktask (FSM* _fsm)
{
    s16 t;
    u16 pal = 0;
    u16 bitmapcurrent = 0;
    u16 currentpicture = 0;
    LOADrequest* loadRequest;

    u8* palette = g_screens.slides->bitmaps[bitmapcurrent];
    u8* bitmap  = palette + 56 * sizeof(u16) * SLI_PICT_HEIGHT;
    u8* paletteback = NULL;
    u8* bitmapback  = NULL;

    u16 gradient[16];
    

    loadRequest = LOADrequestLoad (&RSC_DISK2, SlidePhotoIndexes[currentpicture+1], g_screens.slides->bitmaps[!bitmapcurrent], LOAD_PRIORITY_INORDER);

    SlideDeployPaletteFlipAndMask ((u16*) g_screens.slides->bitmaps[0], (u16*) g_screens.slides->palettes[2], SLI_HEIGHT);

    SNDwaitDMALoop();

    SYSvsync;

    /* here we should have ST lut set to black */
    /* color change interuption should be off */
    SYSwriteVideoBase((u32) g_screens.slides->framebuffers[1]);

    /* create 4 bits grey picture using palettes[1] */
    /* 4 bits grey picture bits stored into frame[1] */
    SlideGreyFade ((u16*) palette, (u16*)g_screens.slides->palettes[1], 16, SLI_HEIGHT);
    SlideConvertToGrey ((u16*) bitmap, (u16*) g_screens.slides->palettes[1], (u16*)(g_screens.slides->framebuffers[1] + 336), SLI_HEIGHT);

    STDmcpy(g_screens.slides->framebuffers[0], g_screens.slides->framebuffers[1], SLI_FB_SIZE);

    /* fade from black to grey */
    SlidesInitMorphPointsCross (g_screens.slides->startpos);

    SlidesInitMorphPoints (g_screens.slides->points[0], g_screens.slides->endpos);

    SLIstartMorph (g_screens.slides->startpos, g_screens.slides->endpos, (u32) g_screens.slides->morphcode, (u32) g_screens.slides->pos, SLI_MORPHPOINTS);

    SLIdisplayMorph(
        (u16*)(g_screens.slides->framebuffers[1] + 168 * 2), 
        g_screens.slides->pos, 
        (u32) g_screens.slides->verttable, 
        (u32) g_screens.slides->horitable, 
        SLI_MORPHPOINTS );

    for (t = 0 ; t < 16 ; t++)
    {
        gradient[t] = (COL4b2ST[t] << 8) | (COL4b2ST[t] << 4) | COL4b2ST[t];
    }

    pal = 1;

    SlideInitPointsGradient(currentpicture);

    for (t = 0 ; t < SLI_MORPHFRAMES ; t++)
    {
        u16* c = HW_COLOR_LUT;

        SYSvsync;

        if ( t < 16 )
        {
            s8* mul = &g_screens.slides->multable[0][t + 16];

            c[0]  = gradient[*mul];  mul += 64;
            c[2]  = gradient[*mul];  mul += 64;
            c[4]  = gradient[*mul];  mul += 64;
            c[6]  = gradient[*mul];  mul += 64;
            c[8]  = gradient[*mul];  mul += 64;
            c[10] = gradient[*mul];  mul += 64;
            c[12] = gradient[*mul];  mul += 64;
            c[14] = gradient[*mul];
        }

        {
            u16* col = (u16*) (g_screens.slides->pointsgradient + ((t & 0xFFFC) << 2));

            c[1 ] = *col++;
            c[3 ] = *col++;
            c[5 ] = *col++;
            c[7 ] = *col++;
            c[9 ] = *col++;
            c[11] = *col++;
            c[13] = *col++;
            c[15] = *col;
        }

        pal = !pal;

        SYSwriteVideoBase((u32) g_screens.slides->framebuffers[pal]);

        SLImorphFunc (
            (u16*)(g_screens.slides->framebuffers[pal] + 336), 
            g_screens.slides->pos, 
            (u32) g_screens.slides->verttable, 
            (u32) g_screens.slides->horitable, 
            (u32) g_screens.slides->morphcode );
    }
  
    SlidePointsGradientSequenceStartEnd(currentpicture + 1, true);

    do
    {
        palette = g_screens.slides->bitmaps[bitmapcurrent];
        bitmap  = palette + 56 * sizeof(u16) * SLI_PICT_HEIGHT;

        paletteback = g_screens.slides->bitmaps[!bitmapcurrent];
        bitmapback  = paletteback + 56 * sizeof(u16) * SLI_PICT_HEIGHT;

        currentpicture++;

        STDmcpy (g_screens.slides->framebuffers[0] + 336, bitmap, 168UL * SLI_HEIGHT);
        SLIinit (g_screens.slides->palettes[1], SLI_HEIGHT);

        if ( loadRequest != NULL )
        {
            /* wait end of loading of next picture */
            LOADwaitRequestCompleted ( loadRequest );
        }

        SYSvsync;
        RASnextOpList = &g_screens.slides->rasterBootFunc;

        SYSwriteVideoBase((u32) g_screens.slides->framebuffers[0]);

        /* fade from grey HC to color HC */
        pal = 1;

        for (t = 16 ; t > 0 ; t--)
        {
            pal = !pal;
            SlideGreyFade ((u16*) palette, (u16*)g_screens.slides->palettes[pal], t, SLI_HEIGHT);
            SYSvsync;
            SLItcBuf = (u16*) g_screens.slides->palettes[pal];
        }

        /* display moment : flip palette => maybe should be done in activity to be able to precal stuffs here... */
        g_screens.slides->palettefliped = g_screens.slides->palettes[2];
        g_screens.slides->palettecurrentflip = 0;

        {
            u16 currentvblcount = SYSvblLcount; 

            /*testcolor = 0x70;*/
           
            SYSvsync;
            
            /*testcolor = 0x7;*/

            /* extract a 4b grey bitmap of next picture */
            /* use palette[0] as temporary */
            /* next picture grey palette into palette [1] */
            /* resulting 4b bitmap into framebuffers[2] and copied into framebuffers[3] */
            SlideMaskColors ((u16*) paletteback, (u16*) g_screens.slides->palettes[0], SLI_HEIGHT);
            SlideGreyFade ((u16*) g_screens.slides->palettes[0], (u16*)g_screens.slides->palettes[1], 16, SLI_HEIGHT);
            SlideConvertToGrey ((u16*) bitmapback, (u16*) g_screens.slides->palettes[1], (u16*)(g_screens.slides->framebuffers[2] + 336), SLI_HEIGHT);
            STDmset (g_screens.slides->palettes[0], 0, SLI_HEIGHT * 64 * 2);
            STDmcpy (g_screens.slides->framebuffers[3], g_screens.slides->framebuffers[2], SLI_FB_SIZE);

            /*testcolor = 0x30;*/

            while ((SYSvblLcount - currentvblcount) < SLI_HCDISPLAY_NBFRAMES);
            
            /*testcolor = 0x0;*/
        }

        g_screens.slides->palettefliped = NULL;

        /* fade from color HC to grey HC */
        pal = 3;

        for (t = 1 ; t < 17 ; t++)
        {
            pal ^= 1;
            SlideGreyFade ((u16*) palette, (u16*)g_screens.slides->palettes[pal], t, SLI_HEIGHT);
            SYSvsync;
            SLItcBuf = (u16*) g_screens.slides->palettes[pal];
        }

        /* display 4b grey picture stored into frame[1] */
        /* turn off interruptions */
        /* set colors to grey */
        SYSvsync;
        SYSwriteVideoBase((u32) g_screens.slides->framebuffers[1]);
        RASnextOpList = NULL;

        SYSvsync;
        
        for (t = 0 ; t < 8 ; t++)
        {
            HW_COLOR_LUT[(t<<1)+1] = HW_COLOR_LUT[t<<1] = (t << 8) | (t << 4) | t;
        }

        /* now palette[2] buffer is available => deploy colors into it */
        SlideDeployPaletteFlipAndMask ((u16*) paletteback, (u16*) g_screens.slides->palettes[2], SLI_HEIGHT);

        if ( currentpicture < ARRAYSIZE(SlidePhotoIndexes) ) 
        {
            /* here we have bitmap[bitmapcurrent] loaded => launch loading on bitmap[!bitmapcurrent] */
            loadRequest = LOADrequestLoad (&RSC_DISK2, SlidePhotoIndexes[currentpicture + 1], palette, LOAD_PRIORITY_INORDER);
        }
        else
        {
            loadRequest = NULL;
            snd.playerClientStep = STEP_SLIDES_NEAREND;
        }

        /* prepare points animation : */
        /* - copy 4b grey bits from frame[1] to frame[0] to double buffer animation */
        /* - prepare points lists */
        /* - initialize morph automodified routine */
        STDmcpy(g_screens.slides->framebuffers[0], g_screens.slides->framebuffers[1], SLI_FB_SIZE);

        SlidesInitMorphPoints (g_screens.slides->points[currentpicture-1], g_screens.slides->startpos);

        if ( loadRequest == NULL )
        {
            STDmset (g_screens.slides->framebuffers[2], 0, SLI_FB_SIZE);
            STDmset (g_screens.slides->framebuffers[3], 0, SLI_FB_SIZE);

            SlidesInitMorphPointsRandom (g_screens.slides->endpos);
        }
        else
        { 
           SlidesInitMorphPoints (g_screens.slides->points[currentpicture], g_screens.slides->endpos);
        }

        SLIstartMorph (g_screens.slides->startpos, g_screens.slides->endpos, (u32) g_screens.slides->morphcode, (u32) g_screens.slides->pos, SLI_MORPHPOINTS);

        SLIdisplayMorph(
                    (u16*)(g_screens.slides->framebuffers[1] + 168 * 2), 
                    g_screens.slides->pos, 
                    (u32) g_screens.slides->verttable, 
                    (u32) g_screens.slides->horitable, 
                    SLI_MORPHPOINTS );

        SlidePointsGradientSequenceStartEnd(currentpicture, false);

        SlideInitPointsGradient(currentpicture);

        {
            u16* c;
            u16* col;

            pal = 1;

            for (t = 0 ; t < (SLI_MORPHFRAMES / 2) ; t++)
            {
                SYSvsync;

                c = HW_COLOR_LUT;

                if ( t >= (SLI_MORPHFRAMES / 2 - 16 ) )
                {
                    s8* mul = &g_screens.slides->multable[0][16 + ((SLI_MORPHFRAMES / 2 - 1) - t)];

                    c[0]  = gradient[*mul]; mul += 64;
                    c[2]  = gradient[*mul]; mul += 64;
                    c[4]  = gradient[*mul]; mul += 64;
                    c[6]  = gradient[*mul]; mul += 64;
                    c[8]  = gradient[*mul]; mul += 64;
                    c[10] = gradient[*mul]; mul += 64;
                    c[12] = gradient[*mul]; mul += 64;
                    c[14] = gradient[*mul];
                }

                col = (u16*) (g_screens.slides->pointsgradient + ((t & 0xFFFC) << 2));

                c[1]  = *col++;
                c[3]  = *col++;
                c[5]  = *col++;
                c[7]  = *col++;
                c[9]  = *col++;
                c[11] = *col++;
                c[13] = *col++;
                c[15] = *col;

                pal = !pal;

                SYSwriteVideoBase((u32) g_screens.slides->framebuffers[pal]);

                SLImorphFunc (
                    (u16*)(g_screens.slides->framebuffers[pal] + 168 * 2), 
                    g_screens.slides->pos, 
                    (u32) g_screens.slides->verttable, 
                    (u32) g_screens.slides->horitable, 
                    (u32) g_screens.slides->morphcode );
            }

            pal = 3;

            for (t = SLI_MORPHFRAMES / 2 ; t < SLI_MORPHFRAMES ; t++)
            {
                SYSvsync;

                c = HW_COLOR_LUT;

                if ( t < (SLI_MORPHFRAMES / 2 + 16 ) )
                {
                    s8* mul = &g_screens.slides->multable[0][t - SLI_MORPHFRAMES / 2 + 16];

                    c[0]  = gradient[*mul];  mul += 64;
                    c[2]  = gradient[*mul];  mul += 64;
                    c[4]  = gradient[*mul];  mul += 64;
                    c[6]  = gradient[*mul];  mul += 64;
                    c[8]  = gradient[*mul];  mul += 64;
                    c[10] = gradient[*mul];  mul += 64;
                    c[12] = gradient[*mul];  mul += 64;
                    c[14] = gradient[*mul];
                }

                col = (u16*) (g_screens.slides->pointsgradient + ((t & 0xFFFC) << 2));

                c[1]  = *col++;
                c[3]  = *col++;
                c[5]  = *col++;
                c[7]  = *col++;
                c[9]  = *col++;
                c[11] = *col++;
                c[13] = *col++;
                c[15] = *col;

                pal ^= 1;

                SYSwriteVideoBase((u32) g_screens.slides->framebuffers[pal]);

                SLImorphFunc (
                    (u16*)(g_screens.slides->framebuffers[pal] + 168 * 2), 
                    g_screens.slides->pos, 
                    (u32) g_screens.slides->verttable, 
                    (u32) g_screens.slides->horitable, 
                    (u32) g_screens.slides->morphcode );
            }
        }

        SlidePointsGradientSequenceStartEnd(currentpicture+1, true);

        if ( loadRequest != NULL )
        {
            STDmcpy(g_screens.slides->framebuffers[1], g_screens.slides->framebuffers[2], SLI_FB_SIZE);        
            bitmapcurrent = !bitmapcurrent;
        }
    }
    while (loadRequest != NULL);

    if ( FSMisLastState(_fsm) )
    {
        STDmset(HW_COLOR_LUT,0,32);
	    while(1);
    }
    else
    {
        snd.playerClientStep = STEP_SLIDES_STOPPED;
        SNDwaitDMALoop();
        RASnextOpList = NULL;
        FSMgotoNextState (&g_stateMachine);
        ScreenWaitMainDonothing();
    }

    IGNORE_PARAM(_fsm);
}


void SlidesExit (FSM* _fsm)
{
    IGNORE_PARAM(_fsm);

    RINGallocatorFree ( &sys.mem, g_screens.slides->bitmaps[0] );

    RINGallocatorFree ( &sys.mem, g_screens.slides->palettes[0] );
	RINGallocatorFree ( &sys.mem, g_screens.slides->framebuffers[0] );

    RINGallocatorFree ( &sys.mem, g_screens.slides->deployer[0] );

    RINGallocatorFree ( &sys.mem, g_screens.slides->greyscaler );
    RINGallocatorFree ( &sys.mem, g_screens.slides->gradients  );

    RINGallocatorFree ( &sys.mem, g_screens.slides->morphcode );
    RINGallocatorFree ( &sys.mem, g_screens.slides->horitable );
    RINGallocatorFree ( &sys.mem, g_screens.slides->verttable );
    RINGallocatorFree ( &sys.mem, g_screens.slides->startpos );
    RINGallocatorFree ( &sys.mem, g_screens.slides->endpos );
    RINGallocatorFree ( &sys.mem, g_screens.slides->pos );
    RINGallocatorFree ( &sys.mem, g_screens.slides->points[0] );
    RINGallocatorFree ( &sys.mem, g_screens.slides->pointsgradient );

	MEM_FREE( &sys.allocatorMem, g_screens.slides );	
    g_screens.slides = NULL;

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    FSMgotoNextState (&g_stateMachineIdle);
}
