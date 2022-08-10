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

#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\ALLOC.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"
#include "DEMOSDK\BITMAP.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\BLSSND.H"

#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"

#include "FX\SNDSHADE\SNDSHADE.H"
#include "FX\VREGANIM\VREGANIM.H"

#include "BLITZIK\SRC\SCREENS.H"
#include "BLITZIK\SRC\SSHADE\SSHADE.H"

#include "EXTERN\ARJDEP.H"

#include "BLITZIK\BLITZWAV.H"


#define SSHADE_RASTERIZE_ENABLE 0

#if SSHADE_RASTERIZE_ENABLE
#   define SSHADE_RASTERIZE(COLOR) *HW_COLOR_LUT = PCENDIANSWAP16(COLOR);
#else
#   define SSHADE_RASTERIZE(COLOR)
#endif

#define VIS_HEIGHT              44
#define VIS_LINES               (VIS_HEIGHT << 2)
#define VIS_NBSTARTCOLORS       64
#define VIS_FIRST_SCANLINE      ((50 - VIS_HEIGHT) << 1)
#define VIS_ROT                 5
#define VIS_EMPTY_H             (VIS_FIRST_SCANLINE + 8)

#define VIS_SCREEN_PITCH        160

#define VIS_PRECOMP_SIZE        (((VIS_WIDTH * VIS_HEIGHT) * 4UL + 2UL) / 3UL)
#define VIS_PRECOMP_BUFFERSIZE  (VIS_PRECOMP_SIZE * ARRAYSIZE(g_screens.sshade->routines))

#define VIS_CODEBUFFER_SIZE     ((VIS_WIDTH*VIS_HEIGHT*8UL+8UL)*3UL)      /* 8 is the size of opcode per pixel */

enum VisualizerState_
{
    VIS_VERTICAL,
    VIS_CROSS,
    VIS_ROTATOR,
    VIS_RATIO,
    VIS_EMPTY
};
typedef enum VisualizerState_ VisualizerState;

typedef void (*SShadeFunction)(void* _src, void* _table, u32 _dest);


static void samScrollMaskColors(u16* _p, u16 _nb)
{
    u16 i;

    for (i = 0 ; i < _nb ; i++)
    {
        *_p++ = PCENDIANSWAP16(*_p) & 0xFFE;
    }
}


static void VISinitMulTable(SShade* this)
{
    u16 t;
    u16 offset = 0;
    u16* p = this->table;


    for (t = 0 ; t < 256 ; t++)
    {
        *p++ = offset;
        offset += VIS_SCREEN_PITCH;
    }
}

static void VISinitFlashColor(SShade* this)
{
    u16 i;
    u16* p = this->startcolflash;

    for (i = 0; i < 2; i++)
    {
        static u16 st[2] = { 16, 68 };
        u16* s = this->startcolors + st[i];
        u16 t;

        this->startflash[i] = p;

        STDmcpy2(p, s, 32);              /* copy forward */
        p += 16;
        s += 16;

        for (t = 16; t > 0; t--)      /* copy reverse */
        {
            *p++ = *s--;
        }

        STDmcpy2(p, this->startflash[i], 48);
        p += 24;
    }
}

#ifdef __TOS__
void SShadeColorTableInit(void* colors, void* startcolors, u32 opAdd, u32 count);
#endif

static void VISinitColorTable(SShade* this)
{
    u8* count;
    u16* p;
    u16 i;


    /* init color tables */
    count = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, 4096 );
    STDfastmset (count, 0UL, 4096);

    {
        u16 c;

        /* elaborate a set of nearly white differenciated colors (that branch to different nodes of color graph) */
        STATIC_ASSERT(VIS_WIDTH <= 48);

        p = this->startcolors;
        
        /* VIS_NBSTARTCOLORS */
        for (i = 0, c = 0; i < 16; i++, c += 4096)
        {
            p[16] = c | 0x7FE;
            p[32] = c | 0xFFE;
            p[48] = c | 0xF7E;
            
            *p++  = c | 0x77E;
        }
    }

    /*for (      ; i < VIS_NBSTARTCOLORS ; i++)
        *p++ = (i << 12) | 0xFF6;*/

        /* make a cycled buffer with start colors */
    STDmcpy2(&this->startcolors[VIS_NBSTARTCOLORS], this->startcolors, VIS_WIDTH << U16_SIZEOF_SHIFT);

#   ifdef __TOS__

    SShadeColorTableInit(g_screens.sshadeStatic.colors, this->startcolors, (u32)this->opAdd, (u32) count);

#   else

    /* make a gradient with colors coming from data */
    for (i = 0; i < VIS_NBSTARTCOLORS; i++)
    {
        s16 lastColor = g_screens.sshadeStatic.colors[i];
        u16 t;

        u16 cr = COLST24b[(lastColor >> 8) & 0xF];
        u16 cg = COLST24b[(lastColor >> 4) & 0xF];
        u16 cb = COLST24b[lastColor & 0xF];

        ASSERT((lastColor & 1) == 0);

        /* branch start color on gradient 1 st color */
        *(u16*)(this->opAdd + (s16)this->startcolors[i]) = lastColor;

        for (t = 16; t > 0;)
        {
            t--;

            {
                u16 dr = (cr * t) >> 4;
                u16 dg = (cg * t) >> 4;
                u16 db = (cb * t) >> 4;

                u16 nextColor = ((u16)COL4b2ST[dr] << 8) | ((u16)COL4b2ST[dg] << 4) | (u16)COL4b2ST[db];

                nextColor &= 0xFFE;

                if (nextColor != 0)
                {
                    u16 c = count[nextColor];
                    /* ASSERT(c < 24); */
                    count[nextColor]++;
                    nextColor |= c << 12;
                }

                *(u16*)(this->opAdd + lastColor) = nextColor;
                lastColor = nextColor;
            }
        }

        *(u16*)(this->opAdd + lastColor) = 0;
    }

    this->opAdd[0] = 0;     /* block at 0 */

                            /*    {
                            int max = 0; 
                            int t;
                            for (t = 0; t < 4096 ; t++)
                            if (count[t] >= 8)
                            {
                            printf ("%x ", t);
                            max++;
                            }

                            printf ("\n%d\n", max);
                            }*/

#   endif


    MEM_FREE ( &sys.allocatorMem, count );
}


void SShadeSet(u16* p, u16* p2, u16 len, s16 inc, u16 mask)
#ifdef __TOS__
;
#else
{
    u16 t;
    u16 value = 0xAAAA;

    inc >>= 1;

    for (t = 0; t < len; t++)
    {
        *p = value;
        p += inc;
        *p2 = value;
        p2 += inc;
    }

    p -= inc;
    p2 -= inc;

    *p &= PCENDIANSWAP16(mask);
    *p2 &= PCENDIANSWAP16(mask);
}
#endif

static void SShadeInitCurve (SShade* this)
{
    u16* p = (u16*) this->bitmaps;
    u16* p2;
    u16 mask1, mask2;
    u16 len;
    u16 t, r;


    p  += 16;
    p2 = p + (256UL * (u32)VIS_SCREEN_PITCH / 2UL);

    for (t = 1 ; t < 128 ; t++)
    {       
        p  += VIS_SCREEN_PITCH / 2;
        p2 -= VIS_SCREEN_PITCH / 2;
       
        len = ((t >> 4) * 4) + 4;
        len >>= 2;

        r = t & 15; 

        mask1 = ~((1UL << (15 - r)) - 1);
        mask2 =  ((1UL <<       r)  - 1);

        SShadeSet(p+1, p+48,     len,  4, mask1);
        SShadeSet(p-2, p-2+49,   len, -4, mask2);

        SShadeSet(p2+1, p2+48  , len,  4, mask1);
        SShadeSet(p2-2, p2-2+49, len, -4, mask2);
    }

    p  += VIS_SCREEN_PITCH / 2;

    SShadeSet(p+1, p+48,   len,  4, mask1);
    SShadeSet(p-2, p-2+49, len, -4, mask2);

    /*p = (u16*) this->bitmaps;
    for (t = 0 ; t < 256 ; t++)
    {
        p[40] = 0x8000;
        p += 80;
    }*/
}


/*
    u32 --------

    10 bits dx
    10 bits dy
    10 bits dx
    
    u32 --------

    10 bits dy
    10 bits dx
    10 bits dy

*/

/* #define SSHADE_TESTMAXSAMPLE */

#ifdef SSHADE_TESTMAXSAMPLE
static u8 sampletest[2000];
#endif


static void sshadeManageCommands(SShade* this, bool allownavigation_)
{
    while (BLZ_COMMAND_AVAILABLE)
    {
        u8 cmd = BLZ_CURRENT_COMMAND;
        u8 category = cmd & BLZ_CMD_CATEGORY_MASK;

        
        BLZ_ITERATE_COMMAND;

        switch (category >> 4)
        {
        case BLZ_CMD_VOICE1_CATEGORY >> 4: 

            if (cmd <= BLZ_CMD_VOICE1_4)
            {
                BLZ_TRAC_COMMAND_NUM("SDSbackVoice", cmd - BLZ_CMD_VOICE1_1 + 1);
                this->voice = cmd - BLZ_CMD_VOICE1_1;
            }
            else if (cmd <= BLZ_CMD_VOICE1_6)
            {
                BLZ_TRAC_COMMAND_NUM("SDSbackEnabled", cmd - BLZ_CMD_VOICE1_5);
                this->displaysample = cmd - BLZ_CMD_VOICE1_5;
            }

            break;

        case BLZ_CMD_VOICE2_CATEGORY >> 4:

            if (cmd <= BLZ_CMD_VOICE2_4)
            {
                BLZ_TRAC_COMMAND_NUM("SDSfrontVoice", cmd - BLZ_CMD_VOICE2_1 + 1);
                SNSimportTable.voice2 = cmd - BLZ_CMD_VOICE2_1;
            }
            else if (cmd >= BLZ_CMD_VOICE2_7)
            {
                if (cmd <= BLZ_CMD_VOICE2_9)
                {
                    static u8 incs[] = { 8,10,11 };
                    BLZ_TRAC_COMMAND_NUM("SDSfrontPCMnointerlace_", incs[cmd - BLZ_CMD_VOICE2_7]);
                    SNSimportTable.sampleinc = incs[cmd - BLZ_CMD_VOICE2_7];
                }
            }
            break;

        case BLZ_CMD_LINE1_CATEGORY >> 4:

            if (cmd <= BLZ_CMD_T)
            {
                BLZ_TRAC_COMMAND_NUM("SDSbackFadeType_", cmd - BLZ_CMD_Q);
                this->fadetype = cmd - BLZ_CMD_Q;
                this->rotationcounter = 0;
            }
            else if (cmd <= BLZ_CMD_U)
            {
                BLZ_TRAC_COMMAND_NUM("SDSbackFlash", cmd - BLZ_CMD_Y);
                this->flash = cmd - BLZ_CMD_Y + 1;
            }
            else if (cmd == BLZ_CMD_I)
            {
                BLZ_TRAC_COMMAND("SDSbackInvertRotate");
                this->rotationcounter = (this->rotationcounter + 0x20) & 0x20;
            }
            break;
        
        case BLZ_CMD_LINE2_CATEGORY >> 4:

            if (cmd <= BLZ_CMD_F)
            {
                BLZ_TRAC_COMMAND_NUM("SDSfrontMode", cmd - BLZ_CMD_A);                
                this->displayverticalsample = cmd - BLZ_CMD_A;
            }
            break;

        case BLZ_CMD_LINE3_CATEGORY >> 4:

            if (cmd <= BLZ_CMD_M)
            {
                static u16 masks[] = { 0xFFF, 0xFF3, 0xFF0, 0xF0F, 0x0FF, 0xF, 0xF0, 0xF00 };
                BLZ_TRAC_COMMAND_NUM("SDSbackFilter_", cmd - BLZ_CMD_ANTISLASH);                
                SNSimportTable.mask = PCENDIANSWAP16(masks[cmd - BLZ_CMD_ANTISLASH]);
            }
            else
            {
                BLZ_TRAC_COMMAND_NUM("SDSbackInvertVideo", cmd == BLZ_CMD_DOT);                
                SNSimportTable.hopop = cmd == BLZ_CMD_DOT ? 0x303 : 0x30C;
            }
            break;

        default:
            if (allownavigation_)
                if (ScreensManageScreenChoice(BLZ_EP_SHADE_SOUND, cmd))
                    return;
        }
    }
}


#define VIS_EMPTYSIZE        ((u32)(200UL * VIS_SCREEN_PITCH))
#define VIS_BITMAPBUFFERSIZE (256UL * (u32)VIS_SCREEN_PITCH)
#define VIS_TCBUFFERSIZE     ((u32)((VIS_WIDTH * VIS_HEIGHT) << U16_SIZEOF_SHIFT) * 2UL)
#define VIS_DATABUFFERSIZE   (VIS_BITMAPBUFFERSIZE + VIS_EMPTYSIZE + VIS_TCBUFFERSIZE)
#define VIS_BUFFERSIZE       (VIS_DATABUFFERSIZE + 65536UL)


void SShadeEnter (FSM* _fsm)
{
    SShade* this;
    

    /*STATIC_ASSERT(VIS_DATABUFFERSIZE <= 65536UL);*/
    IGNORE_PARAM(_fsm);

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "SShadeEnter", '\n');

    STDmset(HW_COLOR_LUT, 0, 32);

    this = g_screens.sshade = MEM_ALLOC_STRUCT( &sys.allocatorMem, SShade );	
    DEFAULT_CONSTRUCT(this);

#   ifdef SSHADE_TESTMAXSAMPLE
    STDmset(sampletest, 0x7F7F7F7FUL, 2000);
#   endif

    this->colorscroll = 0; /*16 << 4;*/

    this->startcolors   = (u16*) MEM_ALLOC ( &sys.allocatorMem, (VIS_NBSTARTCOLORS + VIS_WIDTH) << U16_SIZEOF_SHIFT );
    this->startcolflash = (u16*) MEM_ALLOC ( &sys.allocatorMem, ((8 + VIS_WIDTH) * 2) << U16_SIZEOF_SHIFT );
    this->bitbuf        = (u8*)  MEM_ALLOC ( &sys.allocatorMem, VIS_BUFFERSIZE);
    /*this->routines[0]   = (u16*) MEM_ALLOC ( &sys.allocatorMem, VIS_CODEBUFFER_SIZE ); moved to static pre init */ 
    
    STDmcpy2 (this->routines, g_screens.sshadeStatic.routines, ARRAYSIZE(this->routines) * sizeof(u16*));

    this->opAddBuffer   = (u8*)  MEM_ALLOC ( &sys.allocatorMem, 65536UL );

    SNSimportTable.table = this->table;

    {
        u32 adr = (u32) this->bitbuf;

        adr += 65535UL;
        adr &= 0xFFFF0000UL;

        this->bitmaps = (u8*) adr;
        this->empty   = this->bitmaps + VIS_BITMAPBUFFERSIZE;

        this->tcbuffers[0]  = (u16*)(this->empty + VIS_EMPTYSIZE);
        this->tcbuffers[1]  = this->tcbuffers[0] + (VIS_TCBUFFERSIZE >> 2);
        
        /*TRAClogNumberS("buffer ", (u32) this->bitbuf      , 8, '\n');
        TRAClogNumberS("bitmaps", (u32) this->bitmaps     , 8, '\n');
        TRAClogNumberS("cbuffer", (u32) this->tcbuffers[0], 8, '\n');
        TRAClogNumberS("empty  ", (u32) this->empty       , 8, '\n');
        TRAClogNumberS("size   ", VIS_DATABUFFERSIZE, 8, '\n');*/
    }
    
    STDfastmset (this->bitbuf, 0UL, VIS_BUFFERSIZE);
    SYSwriteVideoBase ((u32) this->empty);

    /*STDfastmset (this->bitmaps, 0UL, VIS_DATABUFFERSIZE);*/
    /*STDfastmset (this->opAddBuffer, 0UL, 65536UL);*/
    this->opAdd = this->opAddBuffer + 32768UL;

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "VISinitMulTable", '\n');

    VISinitMulTable(this);

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "VISinitColorTable", '\n');
    VISinitColorTable(this);

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "VISinitFlashColor", '\n');
    VISinitFlashColor(this);

    /* moved to static preinit
    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "VISgenerateCode", '\n');

    p = this->routines[1] = SNSimportTable.generateCodeCall(g_screens.sshadeStatic.precompcoords[0], this->routines[0], VIS_HEIGHT);
    ASSERT( ((u32)p - ((u32)this->routines[0])) <= VIS_CODEBUFFER_SIZE);

    p = this->routines[2] = SNSimportTable.generateCodeCall(g_screens.sshadeStatic.precompcoords[1], this->routines[1], VIS_HEIGHT);
    ASSERT( ((u32)p - ((u32)this->routines[0])) <= VIS_CODEBUFFER_SIZE);

    p = SNSimportTable.generateCodeCall(g_screens.sshadeStatic.precompcoords[2], this->routines[2], VIS_HEIGHT);
    ASSERT( ((u32)p - ((u32)this->routines[0])) <= VIS_CODEBUFFER_SIZE);
    */ 

    SNSimportTable.tunX3 = 2;

    if ( sys.isMegaSTe )
    {
        SNSimportTable.tunX3 -= 2;
    }

    SNSimportTable.srcAdr = this->tcbuffers[0];

    /* VISdisplayTestImage(); */

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "SShadeInitCurve", '\n');
    SShadeInitCurve(this);

    TRAClogFrameNum(TRAC_LOG_FLOW);
    SNSimportTable.initCall (this->table, VIS_WIDTH, VIS_HEIGHT);

    /* init rasters system */
    this->rasterBootFunc = RASvbl1;
    this->rasterBootOp.scanLinesTo1stInterupt = VIS_FIRST_SCANLINE;
    this->rasterBootOp.backgroundColor   = 0;
    this->rasterBootOp.nextRasterRoutine = SNSimportTable.displayColorsInterupt;
  
    SNSimportTable.mask  = PCENDIANSWAP16(0xFFF);
    SNSimportTable.hopop = 0x303;

    SNSimportTable.lines     = (void**) &this->empty;
    SNSimportTable.sampleinc = 8;
    SNSimportTable.empty     = this->empty;
    SNSimportTable.voice2    = 0;

    this->displaysample = true;    
    this->flip = 0;

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "SShadeEnter_return", '\n');

#   if DEMOS_MEMDEBUG
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif

    RASnextOpList     = &this->rasterBootOp;
    SYSvblroutines[1] = this->rasterBootFunc;
    
    BlitZsetVideoMode(HW_VIDEO_MODE_2P, 0, BLITZ_VIDEO_NOXTRA_PIXEL);

    HW_COLOR_LUT[1] = PCENDIANSWAP16(0x234);
    HW_COLOR_LUT[2] = PCENDIANSWAP16(0x356);
    HW_COLOR_LUT[3] = PCENDIANSWAP16(0x444);

    /*SYSvsync;*/ 
    sshadeManageCommands (this, false); /* hack => do this here in sync to avoid complexifying to put it on main thread like it should be */

#   ifndef __TOS__
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRAClog(TRAC_LOG_MEMORY, "SoundShade memallocator dump", '\n');
        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif

    FSMgotoNextState (_fsm);
    FSMgotoNextState (&g_stateMachine);
}   


static void VISdisplayFlash (SShade* this, void* _buffer, u16 _index)
{
    u16 t;
    u16* d = _buffer;
    u16* startc2 = this->startflash[_index]; /* PATCH (SNDdmaLoopCount & 2) != 0]; */

    for (t = 0 ; t < VIS_HEIGHT ; t++)
    {
        STDmcpy2 (d, startc2 + (STDmfrnd() & 7), VIS_WIDTH << 1);
        d += VIS_WIDTH;
    }

    this->flash = 0;
}

#ifdef __TOS__
#   define SSHADE_WAITFOR_COLORSDISPLAY() while(*HW_VECTOR_TIMERB != 0)
#else

void SShadePCdisplay(SShade* this, u16 _height)
{
    u16* p = (u16*) SNSimportTable.srcAdr;
    u16 x,y;

    u8* s = *(u8**) SNSimportTable.lines;


    s += SNSimportTable.voice2;

    EMULfbExStart(HW_VIDEO_MODE_2P, 64, 63, 64 + 320, 63 + 199, 160, 0);

    for (y = 0 ; y < _height ; y++)
    {
        u16 j;

        for (j = 0 ; j < 4 ; j++)
        {
            u32 cycle = EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, 63 + y * 4 + j + VIS_FIRST_SCANLINE);
            u32 adr = (u32) (this->bitmaps + this->table[*s]);

            s += SNSimportTable.sampleinc;
            
            EMULfbExSetAdr   (cycle, adr);

            cycle += 32;

            for (x = 0 ; x < VIS_WIDTH ; x++)
            {
                u16 c = PCENDIANSWAP16(p[x]) & SNSimportTable.mask;
                if ( SNSimportTable.hopop & 4 )
                    c = ~c;
                EMULfbExSetColor( cycle, 0, c);
                cycle += 8;
            } 

            EMULfbExSetColor( cycle, 0, 0);
        }

        p += VIS_WIDTH;
    }

    EMULfbExSetAdr ( EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 32, 63 + y * 4 + VIS_FIRST_SCANLINE), (u32) this->empty );

    EMULfbExEnd();
}


#   define SSHADE_WAITFOR_COLORSDISPLAY() SShadePCdisplay(this, VIS_HEIGHT)
#endif


void SShadeActivity (FSM* _fsm)
{
    SShade* this     = g_screens.sshade;
    s8*  sampled     = (s8*) g_screens.player.dmabufstart + this->voice;   /* (s8*) SNDlastDMAposition; */
    bool flip        = this->flip;
    u16* backbuffer  = this->tcbuffers[flip];
    u16* frontbuffer = this->tcbuffers[!flip];
    u16* startc      = this->startcolors;
    u16* m           = backbuffer + ((VIS_HEIGHT >> 1) * VIS_WIDTH);


    if (sampled == NULL)
    {
        return;
    }

    if (this->displayverticalsample)
    {
        s16 color;
        s16 color2;

        this->colorscroll2 += 3;

        color = this->startcolors[(this->colorscroll2 >> 4) & (VIS_NBSTARTCOLORS - 1)];

        switch (this->displayverticalsample)
        { 
        case 1:
            color  = *(u16*)(this->opAdd + color);
            color2 = *(u16*)(this->opAdd + color);
            color2 = *(u16*)(this->opAdd + color2);
            break;
        case 2:
            color2 = *(u16*)(this->opAdd + color);
            color  = 0xFFF;
            break;
        case 3:
            color2 = 0x555;
            color  = 0xFFF;
            break;
        }
        HW_COLOR_LUT[1] = PCENDIANSWAP16(color);
        HW_COLOR_LUT[2] = PCENDIANSWAP16(color2);

        SNSimportTable.lines = (void**)&g_screens.player.dmabufstart;
    }
    else
    {
        SNSimportTable.lines = (void**)&this->empty;
    }

    startc += (this->colorscroll >> 4);
        
    IGNORE_PARAM(_fsm);

    /* copy true color pixels back to front buffer */
    switch (this->fadetype)
    {
    case VIS_VERTICAL:
        SSHADE_RASTERIZE(0x444);

        SNSimportTable.srcAdr = backbuffer;

        if (this->flash)
            VISdisplayFlash(this, backbuffer, this->flash - 1);
        else
        {
            if (this->displaysample)
                SNSimportTable.fillsamplCall (sampled, startc , (u32) m, VIS_WIDTH, 20);

            SSHADE_RASTERIZE(0x0);

            SSHADE_WAITFOR_COLORSDISPLAY();

            SSHADE_RASTERIZE(0x50);

            if ( this->colorscroll & 1 )
            {
                SNSimportTable.fadeCall (
                    backbuffer + VIS_WIDTH, 
                    this->opAdd, 
                    (u32) backbuffer, 
                    0, 
                    VIS_HEIGHT >> 1);
            }
            else
            {
                SNSimportTable.fadeCall (
                    backbuffer + ((VIS_HEIGHT - 2) * VIS_WIDTH), 
                    this->opAdd, 
                    (u32) (backbuffer + ((VIS_HEIGHT - 1) * VIS_WIDTH)),
                    -VIS_WIDTH * 4, 
                    VIS_HEIGHT >> 1);
            }
            
            SSHADE_RASTERIZE(0);
        }
        break;

    case VIS_CROSS:
        SNSimportTable.srcAdr = backbuffer;

        if (this->flash)
            VISdisplayFlash(this, backbuffer, this->flash - 1);
        else        
        {           
            if (this->displaysample)
                SNSimportTable.fillsamplCall (sampled, startc , (u32) m, VIS_WIDTH, 20);

            SSHADE_WAITFOR_COLORSDISPLAY();

            SSHADE_RASTERIZE(0x50);

            if ( this->colorscroll & 1 )
            {
                SNSimportTable.fade3Call (
                    backbuffer + VIS_WIDTH + 1, 
                    this->opAdd, 
                    (u32) (backbuffer), 
                    0, 
                    VIS_HEIGHT >> 1);

                SNSimportTable.fade3Call (
                    backbuffer + VIS_WIDTH + (VIS_WIDTH >> 1) - 1, 
                    this->opAdd, 
                    (u32) (backbuffer + (VIS_WIDTH >> 1) ), 
                    0, 
                    VIS_HEIGHT >> 1);
            }
            else
            {
                SNSimportTable.fade3Call (
                    backbuffer + 1 + ((VIS_HEIGHT - 2) * VIS_WIDTH), 
                    this->opAdd, 
                    (u32) (backbuffer + ((VIS_HEIGHT - 1) * VIS_WIDTH)),
                    -VIS_WIDTH * 4, 
                    VIS_HEIGHT >> 1);

                SNSimportTable.fade3Call (
                    backbuffer + (VIS_WIDTH >> 1) - 1 + ((VIS_HEIGHT - 2) * VIS_WIDTH), 
                    this->opAdd, 
                    (u32) (backbuffer + (VIS_WIDTH >> 1) + ((VIS_HEIGHT - 1) * VIS_WIDTH)),
                    -VIS_WIDTH * 4, 
                    VIS_HEIGHT >> 1);
            }
        }
        break;

    case VIS_RATIO:
        {
            {
#           ifndef __TOS__
            static int count = 0;

            if (count++ & 1)               
#           endif
            {
                SNSimportTable.srcAdr = frontbuffer;
                
                if (this->flash)
                    VISdisplayFlash(this, frontbuffer, this->flash - 1);
                else
                {
                    this->flip = !this->flip;

#                   ifdef __TOS__
                    ((SShadeFunction)this->routines[0]) (frontbuffer, this->opAdd, (u32)backbuffer);
#                   else
                    SNSfadeGCode(frontbuffer, this->opAdd, this->routines[0], VIS_HEIGHT, (u32)backbuffer);
#                   endif
                    
                    if (this->displaysample)
                       SNSimportTable.fillsamplCall(sampled, startc, (u32)m, VIS_WIDTH, 20);
                    
                    SSHADE_WAITFOR_COLORSDISPLAY();
                }
            }
        }
    }
        break;

    case VIS_ROTATOR:
        {
#           ifndef __TOS__
            static int count = 0;

            if (count++ & 1)
#           endif
            {
                SNSimportTable.srcAdr = frontbuffer;

                if (this->flash)
                    VISdisplayFlash(this, frontbuffer, this->flash - 1);
                else
                {
                    char flipo = (this->rotationcounter & 0x20) != 0;

                    this->rotationcounter++;
                    this->flip = !this->flip;

#                   ifdef __TOS__
                    ((SShadeFunction)this->routines[1 + flipo]) (frontbuffer, this->opAdd, (u32)backbuffer);
#                   else
                     SNSfadeGCode(frontbuffer, this->opAdd, this->routines[1 + flipo], VIS_HEIGHT, (u32)backbuffer);
#                   endif

                    if (this->displaysample && (this->flash == 0))
                        SNSimportTable.fillsamplCall(sampled, startc, (u32)m, VIS_WIDTH, 20);
                    
                    SSHADE_WAITFOR_COLORSDISPLAY();
                }
            }
        }
        break;

    case VIS_EMPTY:
        SNSimportTable.srcAdr = frontbuffer;
        this->flip = !this->flip;
        STDfastmset(backbuffer, 0UL, (VIS_WIDTH * VIS_HEIGHT) << U16_SIZEOF_SHIFT);
        if (this->displaysample)
            SNSimportTable.fillsamplCall(sampled, startc, (u32)m, VIS_WIDTH, 20);
        SSHADE_WAITFOR_COLORSDISPLAY();
        break;
    }

    this->colorscroll++;

    if (this->colorscroll >= (VIS_NBSTARTCOLORS << 4))
    {
        this->colorscroll = 0;
    }

    sshadeManageCommands(this, true);

    SSHADE_RASTERIZE(0x0);
}


void SShadeExit (FSM* _fsm)
{
    SShade* this = g_screens.sshade;

    SYSvblroutines[1] = RASvbldonothing;
    BlitZturnOffDisplay();

    IGNORE_PARAM(_fsm);

    MEM_FREE ( &sys.allocatorMem, this->opAddBuffer     );
    /*MEM_FREE ( &sys.allocatorMem, this->routines[0]     ); moved to static pre init */
    MEM_FREE ( &sys.allocatorMem, this->bitbuf          );
    MEM_FREE ( &sys.allocatorMem, this->startcolflash   );
    MEM_FREE ( &sys.allocatorMem, this->startcolors     );

    MEM_FREE ( &sys.allocatorMem, g_screens.sshade );	
    this = g_screens.sshade = NULL;

    /*
#   if DEMOS_MEMDEBUG
    TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
    RINGallocatorDump(sys.allocatorMem.allocator, stdout);
#   endif
    */
    
    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    EMULcls();
    EMULfbExDisable();

    ScreensGotoScreen ();
}


void SShadeInitStatic (FSM* _fsm)
{
    LOADrequest* loadRequest;
    u8* temp;
    u8* precompcoords[3];


    ScreensLogFreeArea("SShadeInitStatic");

    {
        u16 offset = (u16) LOADmetadataOffset  (&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_SSHADE_PAL3_PAL);
        u16 size   = (u16) LOADmetadataSize    (&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_SSHADE_PAL3_PAL);

        g_screens.sshadeStatic.nbcolors  = (offset + size) >> 1; 
    }

    temp = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, LOADresourceRoundedSize(&RSC_BLITZWAV, RSC_BLITZWAV_SSHADE_PAL_BIN) );
    precompcoords[0] = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, VIS_PRECOMP_BUFFERSIZE );

    loadRequest  = LOADdata (&RSC_BLITZWAV, RSC_BLITZWAV_SSHADE_PAL_BIN, temp, LOAD_PRIORITY_INORDER);

    {
        u8* p;
        p = precompcoords[1] = SNSprecomputeCoordsZoom(precompcoords[0], 60, VIS_WIDTH, VIS_HEIGHT);
        ASSERT((p - precompcoords[0]) <= VIS_PRECOMP_BUFFERSIZE);
        p = precompcoords[2] = SNSprecomputeCoordsRotate(g_screens.base.sin, precompcoords[1], 0, 60, VIS_ROT, VIS_WIDTH, VIS_HEIGHT);
        ASSERT((p - precompcoords[0]) <= VIS_PRECOMP_BUFFERSIZE);
        p = SNSprecomputeCoordsRotate(g_screens.base.sin, precompcoords[2], 1, 60, VIS_ROT, VIS_WIDTH, VIS_HEIGHT);
        ASSERT((p - precompcoords[0]) <= VIS_PRECOMP_BUFFERSIZE);
    }

    {
        u16* p;
        u16** routines = g_screens.sshadeStatic.routines;

        routines[0] = (u16*)MEM_ALLOC(&sys.allocatorCoreMem, VIS_CODEBUFFER_SIZE);

        p = routines[1] = SNSimportTable.generateCodeCall(precompcoords[0], routines[0], VIS_HEIGHT);
        ASSERT(((u32)p - ((u32)routines[0])) <= VIS_CODEBUFFER_SIZE);

        p = routines[2] = SNSimportTable.generateCodeCall(precompcoords[1], routines[1], VIS_HEIGHT);
        ASSERT(((u32)p - ((u32)routines[0])) <= VIS_CODEBUFFER_SIZE);

        p = SNSimportTable.generateCodeCall(precompcoords[2], routines[2], VIS_HEIGHT);
        ASSERT(((u32)p - ((u32)routines[0])) <= VIS_CODEBUFFER_SIZE);
    }

    LOADwaitRequestCompleted ( loadRequest  );

    {
        u16 sizepal, sizesprite;

        sizepal = (u16) LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_LAYERZ_SPRITE1_BIN);
        g_screens.sshadeStatic.colors = (u16*)MEM_ALLOC(&sys.allocatorCoreMem, sizepal);
        STDmcpy2(g_screens.sshadeStatic.colors, temp, sizepal);

        sizesprite = (u16) LOADmetadataSize(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_LAYERZ_SPRITE1_BIN);
        g_screens.layerzStatic.blitzsprite = (u16*)MEM_ALLOC(&sys.allocatorCoreMem, sizesprite);
        STDmcpy(g_screens.layerzStatic.blitzsprite, temp + sizepal, sizesprite);
    }

    samScrollMaskColors(g_screens.sshadeStatic.colors, g_screens.sshadeStatic.nbcolors);

    MEM_FREE(&sys.allocatorMem, precompcoords[0]);
    MEM_FREE(&sys.allocatorMem, temp);

    EMULcls();

    FSMgotoNextState(_fsm);
}   
