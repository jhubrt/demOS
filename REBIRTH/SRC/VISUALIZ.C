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

#include "FX\VREGANIM\VREGANIM.H"

#include "REBIRTH\SRC\SCREENS.H"
#include "REBIRTH\SRC\SNDSHADE.H"
#include "REBIRTH\SRC\VISUALIZ.H"
#include "REBIRTH\SRC\SNDTRACK.H"

#include "EXTERN\ARJDEP.H"

#include "REBIRTH\REBIRTH1.H"

#define VIS_HEIGHT              46
#define VIS_LINES               (VIS_HEIGHT << 2)
#define VIS_NBSTARTCOLORS       80
#define VIS_FIRST_SCANLINE      ((50 - VIS_HEIGHT) << 1)
#define VIS_BITMAP_BASE         (VIS_FIRST_SCANLINE + 2)
#define VIS_ROT                 7
#define VIS_VOLUME_THRESHOLD    0x400
#define VIS_ANIMBUFFER_SIZE     160000UL

#define VIS_LOGO0_Y             0
#define VIS_LOGO0_H             45
#define VIS_LOGO1_Y             45
#define VIS_LOGO1_H             70
#define VIS_LOGO0_GLITCH_Y      115
#define VIS_LOGO1_GLITCH_Y      160

#define VIS_SCREEN_PITCH        160

/* 
SNSireg:	move.l	d0,a2
SNSload:	move.w	$AA(a0),d0
SNSstore:	move.w	(a1,d0.w),(a2)+ 
*/

ENUM(VisualizerState)
{
    VIS_EMPTY,
    VIS_VERTICAL,
    VIS_CROSS,
    VIS_RATIO,
    VIS_ROTATOR
};

ENUM(VisualizerFlashType)
{
    VIS_FLASH_OFF       = 0,
    VIS_FLASH_ON        = 1
};

#define VIS_NOSYNC          false
#define VIS_SYNC_STEP       true

#define VIS_NOANIM          -1

#define VIS_TRACK_LOOP      false
#define VIS_TRACK_BEAT      true

STRUCT(VisualizerOp)
{
    u8      fadetype;    
    u8      flashfx;
    bool    syncstep;
};

STRUCT(VisualizerTrack)
{
    VisualizerOp*  track;
    u8             len;
    u8             loopstart;
    s8             animateat;
    bool           trackonbeat;
};

static VisualizerOp visTrack0[] = 
{ 
    VIS_EMPTY   , VIS_FLASH_OFF, VIS_NOSYNC, 
    VIS_EMPTY   , VIS_FLASH_OFF, VIS_NOSYNC, 
    VIS_EMPTY   , VIS_FLASH_OFF, VIS_NOSYNC, 
    VIS_EMPTY   , VIS_FLASH_OFF, VIS_SYNC_STEP, 
    VIS_VERTICAL, VIS_FLASH_OFF, VIS_NOSYNC,
    VIS_VERTICAL, VIS_FLASH_OFF, VIS_NOSYNC,
    VIS_VERTICAL, VIS_FLASH_OFF, VIS_NOSYNC,
    VIS_CROSS   , VIS_FLASH_OFF, VIS_NOSYNC,
    VIS_CROSS   , VIS_FLASH_OFF, VIS_NOSYNC,
    VIS_VERTICAL, VIS_FLASH_OFF, VIS_NOSYNC,
    VIS_CROSS   , VIS_FLASH_OFF, VIS_NOSYNC,
    VIS_VERTICAL, VIS_FLASH_OFF, VIS_NOSYNC
};

static VisualizerOp visTrack1[] = 
{
    VIS_RATIO   , VIS_FLASH_OFF, VIS_SYNC_STEP,
    VIS_RATIO   , VIS_FLASH_ON , VIS_NOSYNC,
    VIS_ROTATOR , VIS_FLASH_OFF, VIS_NOSYNC,
    VIS_RATIO   , VIS_FLASH_ON , VIS_NOSYNC,
    VIS_ROTATOR , VIS_FLASH_ON,  VIS_NOSYNC
};

static VisualizerOp visTrack2[] = 
{
    VIS_VERTICAL, VIS_FLASH_OFF, VIS_SYNC_STEP,  
    VIS_CROSS   , VIS_FLASH_OFF, VIS_NOSYNC, 
    VIS_ROTATOR , VIS_FLASH_ON,  VIS_NOSYNC, 
    VIS_RATIO   , VIS_FLASH_ON,  VIS_NOSYNC 
};

static VisualizerOp visTrack3[] = 
{
    VIS_VERTICAL, VIS_FLASH_ON, VIS_SYNC_STEP,  
    VIS_CROSS   , VIS_FLASH_ON, VIS_NOSYNC, 
    VIS_ROTATOR , VIS_FLASH_ON, VIS_NOSYNC, 
    VIS_RATIO   , VIS_FLASH_ON, VIS_NOSYNC
};

static VisualizerTrack visTracksList[] = 
{
    {visTrack0, ARRAYSIZE(visTrack0), 8, 5         , VIS_TRACK_LOOP },
    {visTrack1, ARRAYSIZE(visTrack1), 1, VIS_NOANIM, VIS_TRACK_LOOP },
    {visTrack2, ARRAYSIZE(visTrack2), 0, VIS_NOANIM, VIS_TRACK_LOOP }, 
    {visTrack3, ARRAYSIZE(visTrack3), 0, VIS_NOANIM, VIS_TRACK_BEAT }
};


typedef void (*VisualizerFunction)(void* _src, void* _table, u32 _dest);


/*static void VISdisplayTestImage()
{        
    u16 t = 0,i;

    for (i = 0 ; i < VIS_HEIGHT ; i++)
    {
        g_screens.visualizer->tcbuffers[0][t] = (i << 4) & 0xF0;
        t += VIS_WIDTH;
    }

    for (i = 0 ; i < VIS_WIDTH ; i++)
    {
        if ( i == (VIS_WIDTH - 1) )
        {
            g_screens.visualizer->tcbuffers[0][i] = 0xF;
        }
        else
        {
            g_screens.visualizer->tcbuffers[0][i] = 0xF0;
        }
    }
}*/

static void VISinitMulTable(void)
{
    u16 t;
    
    for (t = 0 ; t < ARRAYSIZE(g_screens.visualizer->multable) ; t++)
    {
        g_screens.visualizer->multable[t] = (u16) STDmulu(t, VIS_SCREEN_PITCH);
    }
}

static void VISinitColorTable(void)
{
    u8* count;
    u16* p;
    u16 i;


    /* init color tables */
    count = (u8*) RINGallocatorAlloc ( &sys.mem, 4096 );
    STDmset (count, 0, 4096);

    {
        p = g_screens.visualizer->colors;

        for (i = 0 ; i < g_screens.visualizer->nbcolors ; i++)
        {
            *p++ = PCENDIANSWAP16(*p) & 0xFFE;
        }
    }

    {
        /* elaborate a set of nearly white differenciated colors (that branch to different nodes of color graph) */
        STATIC_ASSERT(VIS_WIDTH <= 48);

        p = g_screens.visualizer->startcolors;

        for (i = 0 ; i < VIS_NBSTARTCOLORS ; i++)
        {
            if ( i < 16 )
            {
                *p++ = (i << 12) | 0x77E;
            }
            else if ( i < 32 )
            {
                *p++ = (i << 12) | 0x7FE;
            }
            else if ( i < 48 )
            {
                *p++ = (i << 12) | 0xFFE;
            }                        
            else if ( i < 64 )
            {
                *p++ = (i << 12) | 0xF7E;
            }               
            else
            {
                *p++ = (i << 12) | 0xFF6;
            }
        }

        /* make a cycled buffer with start colors */
        STDmcpy (&g_screens.visualizer->startcolors[VIS_NBSTARTCOLORS], g_screens.visualizer->startcolors, VIS_WIDTH << 1); 

        /* make a gradient with colors coming from data */
        for (i = 0 ; i < VIS_NBSTARTCOLORS ; i++)
        {
            s16 lastColor = g_screens.visualizer->colors[i];
            u16 t;

            u16 cr = COLST24b[ (lastColor >> 8) & 0xF ];
            u16 cg = COLST24b[ (lastColor >> 4) & 0xF ];
            u16 cb = COLST24b[  lastColor       & 0xF ];
            
            /* branch start color on gradient 1 st color */
            *(u16*)(g_screens.visualizer->opAdd + (s16) g_screens.visualizer->startcolors[i]) = lastColor;

            for (t = 16 ; t > 0 ;)
            {
                t--;

                {
                    u16 dr = (cr * t) >> 4;
                    u16 dg = (cg * t) >> 4;
                    u16 db = (cb * t) >> 4;

                    u16 nextColor = ((u16)COL4b2ST[dr] << 8) | ((u16)COL4b2ST[dg] << 4) | (u16)COL4b2ST[db];

                    nextColor &= 0xFFE;

                    if ( nextColor != 0 )
                    {
                        u16 c = count[ nextColor ];
                        ASSERT(c < 24);
                        count[ nextColor ]++;
                        nextColor |= c << 12;
                    }

                    *(u16*)(g_screens.visualizer->opAdd + lastColor) = nextColor;
                    lastColor = nextColor;
                }
            }

            *(u16*)(g_screens.visualizer->opAdd + lastColor) = 0;
        }

        /* color flash data */
        {
            p = g_screens.visualizer->startcolflash;

            for (i = 0 ; i < 2 ; i++)
            {
                static u16 st [2] = {16, 68};
                u16* s = g_screens.visualizer->startcolors + st[i];
                u16 t;

                g_screens.visualizer->startflash[i] = p; 

                STDmcpy(p, s, 32);              /* copy forward */
                p += 16;
                s += 16;

                for (t = 16 ; t > 0 ; t--)      /* copy reverse */
                {
                    *p++ = *s--;
                }

                STDmcpy(p, g_screens.visualizer->startflash[i], 48);    
                p += 24;
            }
        }

        g_screens.visualizer->opAdd[0] = 0;     /* block at 0 */
    }

    RINGallocatorFree ( &sys.mem, count );
}


static void VISgenerateCode( s16* sin )
{
    u16 j;
    s16* cos = sin + 128;


    STDmcpy (sin + 512, sin, 256);

    {
        s16 x, y;
        s16 halfw = VIS_WIDTH >> 1;
        s16 halfh = VIS_HEIGHT >> 1;

        u16* p = g_screens.visualizer->routines[0];

        *p++ = SNSireg;

        for (y = -halfh ; y < halfh ; y++)
        {
            s16 srcy = y * 60;

            srcy /= 64;
            srcy += halfh;

            for (x = -halfw ; x < halfw ; x++)
            {
                s16 srcx = x * 61;

                srcx /= 64;
                srcx += halfw;

                *p++ = SNSload;
                *p++ = (srcx << 1) + (srcy * (VIS_WIDTH << 1));
                *((u32*)p) = SNSstore;
                p += 2;
            }
        }

        *p++ = HW_68KOP_RTS;
    }

    for (j = 0 ; j < 2 ; j++)
    {
        s16 cs;
        s16 sn;
        s16 x, y;

        s16 halfw = VIS_WIDTH >> 1;
        s16 halfh = VIS_HEIGHT >> 1;
        u16* p = g_screens.visualizer->routines[j+1];

        if (j == 0)
        {
            cs = cos[VIS_ROT];
            sn = sin[VIS_ROT];
        }
        else
        {
            cs = cos[512 - VIS_ROT];
            sn = sin[512 - VIS_ROT];
        }

        cs = PCENDIANSWAP16 ( cs );
        sn = PCENDIANSWAP16 ( sn );

        *p++ = SNSireg;

        for (y = -halfh ; y < halfh ; y++)
        {
            for (x = -halfw ; x < halfw ; x++)
            {
                s32 x2, y2;
                u16 srcx, srcy;


                x2  = STDmuls(x , cs);
                x2 -= STDmuls(y , sn);
                y2  = STDmuls(x , sn);
                y2 += STDmuls(y , cs);

                x2 += 16384;
                y2 += 16384;

                x2 >>= 6;
                y2 >>= 6;

                x2 *= 60;
                y2 *= 60;

                x2 >>= 15;
                y2 >>= 15;

                x2 += halfw;
                y2 += halfh;

                if ( x2 < 0 )
                    x2 = 0;

                if ( x2 >= VIS_WIDTH )
                    x2 = VIS_WIDTH - 1;

                if ( y2 < 0 )
                    y2 = 0;

                if ( y2 >= VIS_HEIGHT )
                    y2 = VIS_HEIGHT - 1;

                srcx = (u16) x2;
                srcy = (u16) y2;

                *p++ = SNSload;
                *p++ = (srcx << 1) + (srcy * (VIS_WIDTH << 1));
                *((u32*)p) = SNSstore;
                p += 2;

#               ifndef __TOS__
                WINsetColor  ( EMULgetWindow(), 128 + (rand() & 127), 128 + (rand() & 127), 128 + (rand() & 127));
                WINrectangle ( EMULgetWindow(), srcx * 10 - 1, srcy * 10 - 1, srcx * 10 + 1, srcy * 10 + 1);
                WINline ( EMULgetWindow(), srcx * 10, srcy * 10, (x + halfw) * 10, (y + halfh) * 10);
                WINrender ( EMULgetWindow() );
#               endif
            }
        }

        *p++ = HW_68KOP_RTS;
    }
}


static void VISinitGlitchedBitmap (u16* p)
{
    u16 x, y;

    STDmcpy (p, g_screens.visualizer->bitmaps + VIS_FIRST_SCANLINE * VIS_SCREEN_PITCH, VIS_LOGO0_GLITCH_Y * VIS_SCREEN_PITCH);

    for (y = 0 ; y < VIS_LOGO0_GLITCH_Y ; y++)
    {
        u16 x1, x2;

        x1 = (y >> 3) & 31;
        x2 = x1 + 12;

        for (x = 0 ; x < 40 ; x++)
        {
            if (( x >= x1 ) && ( x <= x2 ))
            {
                if (p[1])
                {
                    if (p[1] < 256)
                    {
                        p[0] = 0xFFFF; 
                        p[1] = 0xFF; 
                    }
                    else
                    {
                        p[0] = 0xFFFF;
                        p[1] = 0xFF00;
                    }
                }
            }

            p += 2;
        }
    }
}


void VisualizerEntry (FSM* _fsm)
{
    u16 tcsize = VIS_WIDTH * VIS_HEIGHT;
    LOADrequest* loadRequest;
    LOADrequest* loadRequest2;
    s16* sin;
    u16 j;


    IGNORE_PARAM(_fsm);

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    sin  = (s16*) RINGallocatorAlloc ( &sys.mem, 1024 + 256);

    g_screens.visualizer = MEM_ALLOC_STRUCT( &sys.allocatorMem, Visualizer );	
    DEFAULT_CONSTRUCT(g_screens.visualizer);

    g_screens.visualizer->colorscroll = 16 << 4;
    g_screens.visualizer->timmax = -1;

    g_screens.visualizer->nbcolors      = (u16) (LOADmetadataOffset(&RSC_REBIRTH1, RSC_REBIRTH1_METADATA_VISUALIZ_TEXTOS_PAL) >> 1);
    g_screens.visualizer->colors        = (u16*) RINGallocatorAlloc ( &sys.mem, LOADresourceRoundedSize(&RSC_REBIRTH1, RSC_REBIRTH1_VISUALIZ_PAL_BIN) );

    loadRequest  = LOADdata (&RSC_REBIRTH1, RSC_REBIRTH1_VISUALIZ_PAL_BIN, g_screens.visualizer->colors, LOAD_PRIORITY_INORDER);
    loadRequest2 = LOADdata (&RSC_REBIRTH1, RSC_REBIRTH1_POLYZOOM_SIN_BIN, sin, LOAD_PRIORITY_INORDER);
    
    g_screens.visualizer->startcolors   = (u16*) RINGallocatorAlloc ( &sys.mem, (VIS_NBSTARTCOLORS + VIS_WIDTH) << U16_SIZEOF_SHIFT );
    g_screens.visualizer->startcolflash = (u16*) RINGallocatorAlloc ( &sys.mem, ((8 + VIS_WIDTH) * 2) << U16_SIZEOF_SHIFT );
    g_screens.visualizer->animations    = (u16*) RINGallocatorAlloc ( &sys.mem, VIS_ANIMBUFFER_SIZE);
    g_screens.visualizer->bitbuf        = RINGallocatorAlloc ( &sys.mem, (32000UL * 2UL) + 65536UL );

    {
        u32 adr = (u32) g_screens.visualizer->bitbuf;

        adr += 65535UL;
        adr &= 0xFFFF0000UL;

        g_screens.visualizer->bitmaps = (u8*) adr;
    }

    g_screens.visualizer->opAddBuffer   = (u8*) RINGallocatorAlloc ( &sys.mem, 65536UL );
    STDmset (g_screens.visualizer->opAddBuffer, 0, 65536UL);
    g_screens.visualizer->opAdd = g_screens.visualizer->opAddBuffer + 32768UL;

    g_screens.visualizer->tcbuffers[0] = (u16*)RINGallocatorAlloc ( &sys.mem, tcsize << 2 );
    g_screens.visualizer->tcbuffers[1] = g_screens.visualizer->tcbuffers[0] + tcsize;

    {
        u16 codeBufferSize = VIS_WIDTH * VIS_HEIGHT * 8 + 4;  /* 8 is the size of opcode per pixel */

        for (j = 0 ; j < 3 ; j++)
        {
            g_screens.visualizer->routines[j] = (u16*) RINGallocatorAlloc ( &sys.mem, codeBufferSize );
        }
    }

    SNScolor = 0;

    LOADwaitRequestCompleted ( loadRequest );   /* colors available ---------------------------------------------- */

    if (visTracksList[g_screens.visualizerIndex].animateat != VIS_NOANIM)
    {
        loadRequest = LOADdata(&RSC_REBIRTH1, RSC_REBIRTH1_VISUALIZ_TEXTOS_ARJX, g_screens.visualizer->animations, LOAD_PRIORITY_INORDER);
    }
    else
    {
        loadRequest = NULL;
    }

    VISinitMulTable();
    VISinitColorTable();

    LOADwaitRequestCompleted ( loadRequest2 );	/* sin table available ------------------------------------------ */

    VISgenerateCode(sin);
    RINGallocatorFree ( &sys.mem, sin );

    STDmset (g_screens.visualizer->bitmaps, 0, 64000UL);

    if (visTracksList[g_screens.visualizerIndex].animateat != VIS_NOANIM)
    {
        LOADwaitRequestCompleted ( loadRequest ); /* bitmaps available ---------------------------------------------- */
        ARJdepack(g_screens.visualizer->bitmaps + (VIS_BITMAP_BASE * VIS_SCREEN_PITCH), g_screens.visualizer->animations);

        VISinitGlitchedBitmap ((u16*)(g_screens.visualizer->bitmaps + (VIS_LOGO0_GLITCH_Y + VIS_FIRST_SCANLINE) * VIS_SCREEN_PITCH));

        /* *HW_VIDEO_MODE = HW_VIDEO_MODE_2P;
        SYSwriteVideoBase((u32)(g_screens.visualizer->bitmaps + (VIS_LOGO0_GLITCH_Y + VIS_FIRST_SCANLINE) * VIS_SCREEN_PITCH));
        STDmcpy(HW_COLOR_LUT + 1, g_screens.visualizer->colors + g_screens.visualizer->nbcolors + 1, 6);
        while(1); */
    }
  
    SNStunX1 = 4;
    SNStunX2 = 5;
    SNStunX3 = 10;
    SNStunX4 = 1;

    if ( sys.isMegaSTe )
    {
        SNStunX3--;
        SNStunX4--;
    }

    /* VISdisplayTestImage(); */

    FSMgotoNextState (&g_stateMachineIdle);
}   



void VisualizerEntryFast (FSM* _fsm)
{
    u16 tcsize = VIS_WIDTH * VIS_HEIGHT;


    IGNORE_PARAM(_fsm);

    SNSinit (g_screens.visualizer->tcbuffers[1], g_screens.visualizer->animations, VIS_WIDTH, VIS_HEIGHT);

    STDmset(HW_COLOR_LUT + 1, 0, 6);
    SYSwriteVideoBase ((u32) g_screens.visualizer->bitmaps);

    STDmset (g_screens.visualizer->tcbuffers[0], 0, tcsize << U16_SIZEOF_SHIFT << 1);

    /* init animation system */
    STDmset (g_screens.visualizer->animations, 0, VIS_LINES << U16_SIZEOF_SHIFT);
    g_screens.visualizer->animateto = g_screens.visualizer->currentanimation = g_screens.visualizer->animations + VIS_LINES;

    /* init rasters system */
    g_screens.visualizer->rasterBootFunc = RASvbl1;
    g_screens.visualizer->rasterBootOp.scanLinesTo1stInterupt = VIS_FIRST_SCANLINE;
    g_screens.visualizer->rasterBootOp.backgroundColor   = 0;
    g_screens.visualizer->rasterBootOp.nextRasterRoutine = SNStc;

    g_screens.visualizer->trackcounter = 0;

    LOADwaitFDCIdle();  /* random bug fix on real ste*/

  	SNDwaitDMALoop();

    HW_COLOR_LUT[1] = 0;
    HW_COLOR_LUT[2] = 0x444;
    HW_COLOR_LUT[3] = 0xFFF;
   
    RASnextOpList     = &g_screens.visualizer->rasterBootOp;
    SYSvblroutines[0] = g_screens.visualizer->rasterBootFunc;

    g_screens.visualizer->entryClientStep = snd.playerClientStep;
    g_screens.visualizer->displaysample = true;    
    /* VISdisplayTestImage(); */

    FSMgotoNextState (&g_stateMachineIdle);
    FSMgotoNextState (&g_stateMachine);
}   

extern volatile u16		SNDdmaLoopCount;

static void VISdisplayFlash (u16* _buffer)
{
    u16 t;
    u16* d = _buffer;
    u16* startc2 = g_screens.visualizer->startflash[(SNDdmaLoopCount & 2) != 0];

    for (t = 0 ; t < VIS_HEIGHT ; t++)
    {
        STDmcpy (d, startc2 + (STDmfrnd() & 7), VIS_WIDTH << 1);
        d += VIS_WIDTH;
    }
}

static void VisualizerStepTrack (VisualizerTrack* _track)
{
    g_screens.visualizer->trackcounter++;

    if (g_screens.visualizer->trackcounter == _track->len)
    {
        g_screens.visualizer->trackcounter = _track->loopstart;
    }
}

void VisualizerActivity (FSM* _fsm)
{
    s8*  sampled     = (s8*) SNDlastDMAposition;
    u16* backbuffer  = g_screens.visualizer->tcbuffers[0];
    u16* frontbuffer = g_screens.visualizer->tcbuffers[1];
    
    VisualizerTrack* track   = &visTracksList[g_screens.visualizerIndex];
    VisualizerOp*    trackOp = &track->track[g_screens.visualizer->trackcounter];

    u8 fadetype = trackOp->fadetype;


    IGNORE_PARAM(_fsm);

    if ( g_screens.visualizer->animateto > g_screens.visualizer->currentanimation )
    {
        if ( g_screens.visualizer->animationframe < g_screens.visualizer->animationframescount )
        {
            /* *HW_COLOR_LUT = 0x30; */
            g_screens.visualizer->animationframe++;
        }
        else
        {
            /* *HW_COLOR_LUT = 0x70; */
            g_screens.visualizer->animationframe = 0;
            g_screens.visualizer->animationframescount = *g_screens.visualizer->currentanimation++;
            SNSlines = g_screens.visualizer->currentanimation;
            g_screens.visualizer->currentanimation += VIS_LINES;
        }
    }

    /* copy true color pixels back to front buffer */
    SNScopy (backbuffer, frontbuffer, VIS_WIDTH * VIS_HEIGHT);

    {
        /* clip display window to end or beginning of buffer (closest case) */
        s32 delta = ((s8*) snd.dmaBuffer + (snd.sampleLength << 1) ) - sampled;

        if ( delta < 1000 )
        {
            sampled = (s8*) snd.dmaBuffer;
        }

        /* use louder channel */
        if ( SNDleftVolume & 0x80 )
        {
            g_screens.visualizer->lvol = SNDleftVolume & 0x7F;
        }

        if ( SNDrightVolume & 0x80 )
        {
            g_screens.visualizer->rvol = SNDrightVolume & 0x7F;
        }

        if ( g_screens.visualizer->rvol > g_screens.visualizer->lvol )
        {
            sampled++;
        }
    }

    switch (fadetype)
    {
    case VIS_VERTICAL:
        {
            if ( g_screens.visualizer->colorscroll & 1 )
                SNSfade (
                    backbuffer + VIS_WIDTH, 
                    g_screens.visualizer->opAdd, 
                    (u32) backbuffer, 
                    0, 
                    VIS_HEIGHT >> 1);
            else
                SNSfade (
                    backbuffer + ((VIS_HEIGHT - 2) * VIS_WIDTH), 
                    g_screens.visualizer->opAdd, 
                    (u32) (backbuffer + ((VIS_HEIGHT - 1) * VIS_WIDTH)),
                    -VIS_WIDTH * 4, 
                    VIS_HEIGHT >> 1);
        }
        break;

    case VIS_CROSS:
        {
            if ( g_screens.visualizer->colorscroll & 1 )
            {
                SNSfade3 (
                    backbuffer + VIS_WIDTH + 1, 
                    g_screens.visualizer->opAdd, 
                    (u32) (backbuffer), 
                    0, 
                    VIS_HEIGHT >> 1);

                SNSfade3 (
                    backbuffer + VIS_WIDTH + (VIS_WIDTH >> 1) - 1, 
                    g_screens.visualizer->opAdd, 
                    (u32) (backbuffer + (VIS_WIDTH >> 1) ), 
                    0, 
                    VIS_HEIGHT >> 1);
            }
            else
            {
                SNSfade3 (
                    backbuffer + 1 + ((VIS_HEIGHT - 2) * VIS_WIDTH), 
                    g_screens.visualizer->opAdd, 
                    (u32) (backbuffer + ((VIS_HEIGHT - 1) * VIS_WIDTH)),
                    -VIS_WIDTH * 4, 
                    VIS_HEIGHT >> 1);

                SNSfade3 (
                    backbuffer + (VIS_WIDTH >> 1) - 1 + ((VIS_HEIGHT - 2) * VIS_WIDTH), 
                    g_screens.visualizer->opAdd, 
                    (u32) (backbuffer + (VIS_WIDTH >> 1) + ((VIS_HEIGHT - 1) * VIS_WIDTH)),
                    -VIS_WIDTH * 4, 
                    VIS_HEIGHT >> 1);
            }
        }
        break;

    case VIS_RATIO:
        ((VisualizerFunction) g_screens.visualizer->routines[0]) (frontbuffer, g_screens.visualizer->opAdd, (u32)backbuffer);
        break;

    case VIS_ROTATOR:
        {
            if ( g_screens.visualizer->beat )
            {
                ((VisualizerFunction) g_screens.visualizer->routines[1]) (frontbuffer, g_screens.visualizer->opAdd, (u32)backbuffer);
            }
            else
            {
                ((VisualizerFunction) g_screens.visualizer->routines[2]) (frontbuffer, g_screens.visualizer->opAdd, (u32)backbuffer);
            }
        }
        break;

    case VIS_EMPTY:
        STDmset(backbuffer, 0, VIS_WIDTH * VIS_HEIGHT * sizeof(u16));
        break;
    }

    if (( fadetype == VIS_RATIO ) || ( fadetype == VIS_ROTATOR ))
    {
        u16 currentvblcount = SYSvblLcount;
   
        /* hack to synchronize following code => else I have some desynchros with the colors change routine */ 
        while ((*HW_VECTOR_TIMERB) != 0)     
        {
            if (SYSvblLcount != currentvblcount)
                break;
        }
    }

    {
        u16* startc = g_screens.visualizer->startcolors + (g_screens.visualizer->colorscroll >> 4);
        u16* m = backbuffer + ((VIS_HEIGHT >> 1) * VIS_WIDTH);

        u16 vol;
  
        if ( g_screens.visualizer->displaysample )
        {
            vol = SNSfilsampl (sampled, startc , (u32) m, VIS_WIDTH, 20);
        }

        if ( g_screens.visualizer->lastcount != SNDdmaLoopCount ) 
        {		
            g_screens.visualizer->lastcount = SNDdmaLoopCount;

            if (trackOp->flashfx)
            {
                VISdisplayFlash(backbuffer);
            }

            if ( track->trackonbeat == false )
            {
                VisualizerStepTrack(track);
            }

            if ( snd.playerClientStep == (g_screens.visualizer->entryClientStep + 2) )
            {
                snd.playerClientStep++;
            }
        }

        /* beat detector */
        if ( vol > VIS_VOLUME_THRESHOLD )
        {
            if ( g_screens.visualizer->timmax < 0 )
            {
                if (trackOp->fadetype == VIS_ROTATOR)
                {
                    g_screens.visualizer->beat = !g_screens.visualizer->beat;
                }

                if ( track->trackonbeat )
                {
                    VisualizerStepTrack(track);
                }
            }

            g_screens.visualizer->timmax = 5;
        }

        g_screens.visualizer->timmax--;

        /* display volume in order to tune beat effects
        {
        char* temp = "         ";
        STDuxtoa (temp, vol, 4);
        STDuxtoa (&temp[5], max, 4);
        SYSdebugPrint (g_screens.visualizer->bitmaps, VIS_SCREEN_PITCH, SYS_2P_BITSHIFT, 0, 0, temp);
        } */
    }

    g_screens.visualizer->colorscroll++;

    if (g_screens.visualizer->colorscroll >= (VIS_NBSTARTCOLORS << 4))
    {
        g_screens.visualizer->colorscroll = 0;
    }

    if ( trackOp->syncstep )
    {
        if ( g_screens.visualizer->entryClientStep == snd.playerClientStep )
        {
            snd.playerClientStep++;
        }
    }

    /* display sample sum to tune beat threshold...
    STD_uxtoa (temp, SNStrace, 8);
    SYS_debugPrint (g_screens.visualizer->framebuffers[0], VIS_SCREEN_PITCH, 8, 0, 100, temp);*/
}

/* Logo base class -------------------------------- */ 

typedef bool (*VisualizerAnimateFunc)(VANIanimation*, u16* _p);

static u16* VisualizeLogoAnimate(VANIanimation** _logos, u16 _nbLogos, u16* _p)
{
    u16 t;
    bool run;
    bool changed;
    u16* lastnbframes = _p;

    do
    {
        run = false;
        changed = false;

        *_p = 0; /* nb frames */

        STDmset(_p, 0, VIS_LINES*sizeof(u16));

        for (t = 0 ; t < _nbLogos ; t++)
        {
            /*char temp[16];
            
            STDuxtoa (temp, _logos[t]->nbFrames, 10);
            SYSdebugPrint ( g_screens.visualizer->bitmaps, VIS_SCREEN_PITCH, SYS_2P_BITSHIFT, _logos[t]->ysrc >> 3, 0, temp );*/

            changed |= _logos[t]->animateFunc(_logos[t], _p + 1);
            run     |= (_logos[t]->nbFrames > 0);
        }

        if ( changed )
        {
            lastnbframes = _p;
            _p[VIS_LINES] = 0;
            _p += VIS_LINES + 1;
        }
        else
        {
            ASSERT(lastnbframes != _p);
            (*lastnbframes)++;
        }

        ASSERT ((u8*)_p < ((((u8*)g_screens.visualizer->animations) + VIS_ANIMBUFFER_SIZE)));

        /* *HW_COLOR_LUT ^= 0x30; */
    }
    while (run);

    return _p;
}

/* Logo move -------------------------------- */

STRUCT(VisualizerLogoMove)
{
    VANIanimation logo;
    s16 speedy;
};

static bool VisualizerLogoMoveUpdate(VANIanimation* _this, u16* _p)
{
    VisualizerLogoMove* me = (VisualizerLogoMove*) _this;

    u16 h = _this->height;
    s16 current = _this->y;
    u16 line = g_screens.visualizer->multable[_this->ysrc];
    u16 t;


    for (t = 0 ; t < h ; t++, current++)
    {
        if (( current >= 0 ) && ( current < VIS_LINES))
        {
            _p[current] = line;
        }

        line += VIS_SCREEN_PITCH;
    }

    if ( _this->nbFrames > 0 )
    {
        _this->y += me->speedy;
        _this->nbFrames--;
        return true;
    }
    else
    {
        return false;
    }
}

static void VisualizerLogoMoveConstruct(VisualizerLogoMove* _this, u16 _ysrc, u16 _h, s16 _y, u16 _nbFrames, s16 _speedy)
{
    _this->logo.animateFunc = VisualizerLogoMoveUpdate;
    _this->logo.ysrc        = _ysrc;
    _this->logo.height      = _h;
    _this->logo.y           = _y;
    _this->logo.nbFrames    = _nbFrames;
    _this->speedy           = _speedy;
}

/* Logo glitch -------------------------------- */

STRUCT(VisualizerLogoGlitch)
{
    VANIanimation logo;
    u16 ysrcglitch;
    s16 glitchnbframes;
    u16 y1glitch;
    u16 y2glitch;
};

static bool VisualizerLogoGlitchUpdate(VANIanimation* _this, u16* _p)
{
    VisualizerLogoGlitch* me = (VisualizerLogoGlitch*) _this;

    u16 h = _this->height;
    u16 t;
    bool changed = false;


    if (_this->nbFrames == 1)
    {
        me->glitchnbframes = 0x7FFF;
        me->y1glitch = -1;
        me->y2glitch = -1;
        changed = true;
    }
    else if ( me->glitchnbframes <= 0 )
    {
        if ( me->glitchnbframes == 0 )
        {
            me->glitchnbframes = (STDifrnd() & 63);

            if ( STDifrnd() & 7 )
            {
                me->glitchnbframes >>= 4;
            }

            if (( STDifrnd() & 1 ) == 0 )
            {
                me->y1glitch = STDifrnd() & 63;
                if (me->y1glitch >= me->logo.height )
                {
                    me->y1glitch -= 15;
                }
                me->y2glitch = me->y1glitch + (STDifrnd() & 7);
            }
            else
            {
                me->glitchnbframes = -me->glitchnbframes;
                me->y1glitch = -1;
                me->y2glitch = -1;
            }
        }
        else
        {
            me->glitchnbframes = -me->glitchnbframes;
        }
        changed = true;
    }
    else
    {
        me->glitchnbframes--;
    }

    {
        u16 line = g_screens.visualizer->multable[_this->ysrc];
        u16 lineglitch = g_screens.visualizer->multable[me->ysrcglitch];

        ASSERT (( _this->y >= 0 ) && ( (_this->y + _this->height) < VIS_LINES));

        _p += _this->y;
        
        for (t = 0 ; t < h ; t++)
        {
            if ( (t >= me->y1glitch) && (t <= me->y2glitch) )
            {
                *_p++ = lineglitch;
            }
            else
            {
                *_p++ = line;
            }

            line += VIS_SCREEN_PITCH;
            lineglitch += VIS_SCREEN_PITCH;
        }
    }

    if ( _this->nbFrames > 0 )
    {
        _this->nbFrames--;
    }

    return changed;
}

static void VisualizerLogoGlitchConstruct (VisualizerLogoGlitch* _this, u16 _ysrc, u16 _h, s16 _y, u16 _nbFrames, s16 _glitchNbFrames, u16 _ysrcglitch)
{
    _this->logo.animateFunc = VisualizerLogoGlitchUpdate;
    _this->logo.ysrc        = _ysrc;
    _this->logo.height      = _h;
    _this->logo.y           = _y;
    _this->logo.nbFrames    = _nbFrames;
    _this->glitchnbframes   = _glitchNbFrames;
    _this->ysrcglitch       = _ysrcglitch;
    _this->y1glitch = _this->y2glitch = 0xFFFF;
}

/* Logo Spread FX -------------------------------- */

STRUCT(VisualizerLogoSpread)
{
    VANIanimation logo;
    u16 start;
    s16 acceleration;
    s16 *slicedy;
    s16 *slicespeed;
};

static bool VisualizerLogoSpreadUpdate(VANIanimation* _this, u16* _p)
{
    VisualizerLogoSpread* me = (VisualizerLogoSpread*) _this;

    u16  h = _this->height;
    s16  current;
    u16  line;
    s16  t;
    s16  y = _this->y;

    bool changed = (y + (me->slicedy[0] >> 5)) < VIS_LINES;

    if ( changed )
    {
        if ( me->start > 0 )
        {
            me->start--;
        }

        {
            u16  start        = me->start;
            s16  acceleration = me->acceleration;
            s16* speeds       = &me->slicespeed[start];
            s16* dys          = &me->slicedy[start];

            for (t = start ; t < h ; t++)
            {
                *speeds += acceleration;
                *dys++  += *speeds++;
            }
        }
    }

    line = g_screens.visualizer->multable[_this->ysrc];
    current = y;    
    
    for (t = 0 ; t < h ; t++, current++)    
    {
        s16 c = current + (me->slicedy[t] >> 5);

        if (( c >= 0 ) && ( c < (VIS_LINES - 1)))
        {
            _p[c] = line;
        }
    
        line += VIS_SCREEN_PITCH;
    }
        
    if ( _this->nbFrames > 0 )
    {
        _this->nbFrames--;
        return changed;
    }
    else
    {
        return false;
    }
}

static void VisualizerLogoSpreadConstruct (VisualizerLogoSpread* _this, u16 _ysrc, u16 _h, s16 _y, s16 _nbFrames, s16 _acceleration)
{
    u16 size;

    _this->logo.animateFunc   = VisualizerLogoSpreadUpdate;
    _this->logo.ysrc          = _ysrc;
    _this->logo.height        = _h;
    _this->logo.y             = _y;
    _this->logo.nbFrames      = _nbFrames;
    
    _this->start         = _h;
    _this->acceleration  = _acceleration;

    size = _h << U16_SIZEOF_SHIFT;
    
    _this->slicespeed = (s16*) RINGallocatorAlloc(&sys.mem, size);
    _this->slicedy    = (s16*) RINGallocatorAlloc(&sys.mem, size);
    
    STDmset (_this->slicespeed, 0, size);
    STDmset (_this->slicedy, 0, size);
}


/* Animations scripting -------------------------------- */

static u16* VisualizerAnimation1(u16* _p, s16* _ypos)
{
    VisualizerLogoMove movingLogos[2];
    VANIanimation*    logos[2];
    u16* anim;

    VisualizerLogoMoveConstruct (&movingLogos[0], VIS_BITMAP_BASE, VIS_LOGO0_H, -VIS_LOGO0_H, 40, 2);
    VisualizerLogoMoveConstruct (&movingLogos[1], VIS_LOGO1_Y + VIS_BITMAP_BASE, VIS_LOGO1_H, -VIS_LOGO1_H, 56, 3);

    logos[0] = &movingLogos[0].logo;
    logos[1] = &movingLogos[1].logo;
   
    anim = VisualizeLogoAnimate(logos, ARRAYSIZE(logos), _p);

    _ypos[0] = logos[0]->y;
    _ypos[1] = logos[1]->y;

    return anim;
}

static u16* VisualizerAnimation2(u16* _p, s16* _ypos)
{
    VisualizerLogoGlitch glitchLogos[2];
    VANIanimation*      logos[2];


    VisualizerLogoGlitchConstruct (&glitchLogos[0], VIS_BITMAP_BASE, VIS_LOGO0_H, _ypos[0], 284, -20, VIS_LOGO0_GLITCH_Y + VIS_BITMAP_BASE);
    VisualizerLogoGlitchConstruct (&glitchLogos[1], VIS_LOGO1_Y + VIS_BITMAP_BASE, VIS_LOGO1_H, _ypos[1], 284, -30, VIS_LOGO1_GLITCH_Y + VIS_BITMAP_BASE);

    logos[0] = &glitchLogos[0].logo;
    logos[1] = &glitchLogos[1].logo;

    return VisualizeLogoAnimate(logos, ARRAYSIZE(logos), _p);
}


static u16* VisualizerAnimation3(u16* _p, s16* _ypos)
{
    VANIanimation*       logos[2];
    VisualizerLogoMove   movingLogos;
    VisualizerLogoSpread spreadLogos;
    u16*                 anim;


    VisualizerLogoMoveConstruct (&movingLogos, VIS_BITMAP_BASE, VIS_LOGO0_H, _ypos[0], 0, 0);
    VisualizerLogoSpreadConstruct (&spreadLogos, VIS_LOGO1_Y + VIS_BITMAP_BASE, VIS_LOGO1_H, _ypos[1], 100, 5);

    logos[0] = &movingLogos.logo;
    logos[1] = &spreadLogos.logo;

    anim = VisualizeLogoAnimate(logos, ARRAYSIZE(logos), _p);

    RINGallocatorFree (&sys.mem, spreadLogos.slicedy);
    RINGallocatorFree (&sys.mem, spreadLogos.slicespeed);

    return anim;
}


static u16* VisualizerAnimation4(u16* _p, s16* _ypos)
{
    VisualizerLogoSpread spreadLogos;
    VANIanimation*       logos = &spreadLogos.logo;
    u16*                 anim;


    VisualizerLogoSpreadConstruct(&spreadLogos, VIS_BITMAP_BASE, VIS_LOGO0_H, _ypos[0], 90, 5);

    anim = VisualizeLogoAnimate(&logos, 1, _p);

    RINGallocatorFree (&sys.mem, spreadLogos.slicedy);
    RINGallocatorFree (&sys.mem, spreadLogos.slicespeed);

    return anim;
}


void VisualizerBacktask (FSM* _fsm)
{
    u16 i;

    if ( visTracksList[g_screens.visualizerIndex].animateat != VIS_NOANIM )
    {
        s16 ypos[2];
        
        /* compute animations for logos as background task and activate their display on main thread when ready... */    
        u16* anim1 = VisualizerAnimation1 (g_screens.visualizer->currentanimation, ypos);
        u16* anim2 = VisualizerAnimation2 (anim1, ypos);
        u16* anim3 = VisualizerAnimation3 (anim2, ypos);
        u16* anim4 = VisualizerAnimation4 (anim3, ypos);

        while (g_screens.visualizer->trackcounter != visTracksList[g_screens.visualizerIndex].animateat);

        /*for (i = 0 ; i < 2 ; i++)
        {
            g_screens.visualizer->animateto = anim2;
            while (g_screens.visualizer->currentanimation != anim2);
            g_screens.visualizer->currentanimation = anim1;
        }*/

        g_screens.visualizer->animateto = anim4;

        /*{ char temp[32];
        STDuxtoa (temp, ((u8*)anim4) - ((u8*)g_screens.visualizer->animations), 6);
        SYSdebugPrint(g_screens.visualizer->bitmaps, VIS_SCREEN_PITCH, SYS_2P_BITSHIFT, 0, 0, temp);}*/
    }

    if ( FSMisLastState(_fsm) )
    {
	    while(1);
    }
    else
    {
        SNDwaitClientStep(g_screens.visualizer->entryClientStep + 2);
        /* fade out */
        g_screens.visualizer->displaysample = false;
        for (i = 0 ; i < 50 ; i++)
        {
            SYSvsync;
        }
        SNDwaitClientStep(g_screens.visualizer->entryClientStep + 3);
        g_screens.visualizerIndex++;
        SYSvblroutines[0] = RASvbldonothing; 
        FSMgotoNextState (&g_stateMachine);
        ScreenWaitMainDonothing();
    }
    
    IGNORE_PARAM(_fsm);
}

void VisualizerExit (FSM* _fsm)
{
    u16 j;


    IGNORE_PARAM(_fsm);

    RINGallocatorFree ( &sys.mem, g_screens.visualizer->colors );
    RINGallocatorFree ( &sys.mem, g_screens.visualizer->startcolors );
    RINGallocatorFree ( &sys.mem, g_screens.visualizer->startcolflash );
    RINGallocatorFree ( &sys.mem, g_screens.visualizer->animations );
    RINGallocatorFree ( &sys.mem, g_screens.visualizer->bitbuf );
    RINGallocatorFree ( &sys.mem, g_screens.visualizer->opAddBuffer );
    RINGallocatorFree ( &sys.mem, g_screens.visualizer->tcbuffers[0] );

    for (j = 0 ; j < 3 ; j++)
    {
        RINGallocatorFree ( &sys.mem, g_screens.visualizer->routines[j] );
    }

    MEM_FREE( &sys.allocatorMem, g_screens.visualizer );	
    g_screens.visualizer = NULL;

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    FSMgotoNextState (&g_stateMachineIdle);
}
