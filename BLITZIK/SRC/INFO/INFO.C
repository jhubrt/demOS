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

#include "EXTERN\ARJDEP.H"
#include "EXTERN\RELOCATE.H"

#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\LOAD.H"
#include "DEMOSDK\BITMAP.H"
#include "DEMOSDK\DATA\SYSTFNT.H"

#include "DEMOSDK\PC\EMUL.H"
#include "DEMOSDK\PC\WINDOW.H"

#include "FX\SMPLCURV\SMPLCURV.H"

#include "BLITZIK\BLITZWAV.H"

#include "BLITZIK\SRC\SCREENS.H"


#define INFO_FORCE_COMPOMODE() 0

#define INFO_COMPOMODE_PACE 20

#ifdef __TOS__
#define INFO_RASTERIZE() 0
#else
#define INFO_RASTERIZE() 0
#endif

#if INFO_RASTERIZE()
#   define INFO_RASTERIZE_COLOR(COLOR)  *HW_COLOR_LUT = COLOR;
#else
#   define INFO_RASTERIZE_COLOR(COLOR)
#endif

#define INFO_VECTOR_COLOR        0x4
#define INFO_TEXT_COLOR_STE3     0x04F
#define INFO_TEXT_COLOR_STE2     0x07F
#define INFO_TEXT_COLOR_STE      0x0C7

#define NBMAXEDGES_PERPOLY  32
#define SIZEMAX_PERPOLY     ((NBMAXEDGES_PERPOLY * 4 + 4) * sizeof(u16))
#define POLY_DISPLISTSIZE   (50UL * 1024UL)

#define INFO_LINE_SIZE      43

#define INFO_LINE_H             9
#define INFO_CURSOR_OFFSET1     2
#define INFO_FIRST_STEP_COMPO   19
#define INFO_FIRST_STEP         29
#define INFO_SCROLL_STEP        4
#define INFO_SCROLL_STEP2       16
#define INFO_VECTOR_YOFFSET     22
#define INFO_DISPLAY_H          245
#define INFO_BITMAP_Y1          55
#define INFO_BITMAP_Y2          145


#define INFO_FRAMEBUFFER_SIZE ((u32)VEC_PITCH * (u32)INFO_LINE_H * (32UL + 32UL + (u32)INFO_CURSOR_OFFSET1))


static bool IntroAnimation_Scene0 (VECanimationState* _state)
{
    PZanimationState* state = (PZanimationState*) _state;

    state->animState.angle = (state->animState.angle + 4) & 511;

    if (state->animState.angle == 0)
    {
        return true;
    }

    return false;
}

static void info_pzcheckList (u16* _dlist)
{
#   ifdef DEMOS_ASSERT
    {
        u32 displistsize = ((u8*)_dlist - (u8*)g_screens.info->displist);
        ASSERT (displistsize <= POLY_DISPLISTSIZE);
    }
#   endif
}


#ifdef __TOS__

typedef InfoASMimport* (*FuncInfoASMimport)(void);

static void InfoImport(Info* this)
{
    this->asmimport = ((FuncInfoASMimport)this->code)();
}

#else

static void infoDrawVolumes(u16* maxvols_, u8* image, u32 volumesmasks_)
{
    Info* this = NULL;
    u16* volumesmasks = (u16*)volumesmasks_;
    u16 t;


    *HW_BLITTER_HOP = HW_BLITTER_HOP_HTONE;
    *HW_BLITTER_OP = HW_BLITTER_OP_S;
    *HW_BLITTER_XSIZE = 1;
    *HW_BLITTER_XINC_DEST = 8;
    *HW_BLITTER_YINC_DEST = VEC_PITCH;
    *HW_BLITTER_ENDMASK1 = -1;
    *HW_BLITTER_ENDMASK2 = -1;
    *HW_BLITTER_ENDMASK3 = -1;
    *HW_BLITTER_CTRL2 = 0;

    for (t = 0; t < ARRAYSIZE(this->maxvols); t++)
    {
        u16* p = &volumesmasks[maxvols_[t] << 2];
        u16 i;
        u16 mask;


        mask = *p++;

        for (i = 0; i < 16; i++)
            HW_BLITTER_HTONE[i] = mask;

        *HW_BLITTER_ADDR_DEST = (u32)image;
        image += 8;

        *HW_BLITTER_YSIZE = INFO_DISPLAY_H;
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();

        mask = *p++;

        for (i = 0; i < 16; i++)
            HW_BLITTER_HTONE[i] = mask;

        *HW_BLITTER_ADDR_DEST = (u32)image;
        image += 8;

        *HW_BLITTER_YSIZE = INFO_DISPLAY_H;
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();

        mask = *p++;

        for (i = 0; i < 16; i++)
            HW_BLITTER_HTONE[i] = mask;

        *HW_BLITTER_ADDR_DEST = (u32)image;
        image += 8;

        *HW_BLITTER_YSIZE = INFO_DISPLAY_H;
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();
    }
}

static void infoComputeVolumes (s8* dmabuffer, u16* volumes)
{
    s16 sample;
    u16 i;


    volumes[0] = volumes[1] = volumes[2] = volumes[3] = 256;

    for (i = 0 ; i < 125 ; i++)
    {
        sample = *dmabuffer++;
        if (sample < 0)
            sample = -sample;
        volumes[0] += sample;

        sample = *dmabuffer++;
        if (sample < 0)
            sample = -sample;
        volumes[1] += sample;

        sample = *dmabuffer++;
        if (sample < 0)
            sample = -sample;
        volumes[2] += sample;

        sample = *dmabuffer++;
        if (sample < 0)
            sample = -sample;
        volumes[3] += sample;

        dmabuffer += 12;
    }

    volumes[0] >>= 9;
    volumes[1] >>= 9;
    volumes[2] >>= 9;
    volumes[3] >>= 9;
}


static void infoClearPlane (void* image, u16 nblines, u16 value)
{
    u16* p = (u16*) image;
    u16 t;

    nblines *= 21;

    for (t = 0 ; t < nblines ; t++)
    {
        *p = value;
        p += 4;
    }
}

static void infoCopyPlane (void* src_, void* dst_, u32 dst2_, u16 nblines)
{
    u16* s  = (u16*) src_;
    u16* d  = (u16*) dst_;
    u16* d2 = (u16*) dst2_;
    u16* d3 = d + INFO_FRAMEBUFFER_SIZE / 2UL;
    u16* d4 = d2 + INFO_FRAMEBUFFER_SIZE / 2UL;
    u16 t;

    nblines *= 21;

    for (t = 0 ; t < nblines ; t++)
    {
        *d  = *s;
        *d2 = *s;
        *d3 = *s;
        *d4 = *s;
        
        s += 4;
        d += 4;
        d2 += 4;
        d3 += 4;
        d4 += 4;
    }
}

InfoASMimport g_InfoASMimport;

static void InfoImport(Info* this)
{
    g_InfoASMimport.drawvolumes     = infoDrawVolumes;
    g_InfoASMimport.computevolumes  = infoComputeVolumes;
    g_InfoASMimport.clearplane      = infoClearPlane;
    g_InfoASMimport.copyplane       = infoCopyPlane;

    this->asmimport = &g_InfoASMimport;
}

#endif


static void infoInitVectorScene (Info* this, u8* tempvec)
{
    PZanimationState* state;
    u16* dlist;
    u16* polylinestmp;


    VECsceneConstruct(
        &this->scene,
        tempvec + LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_POLYZOOM_BLITZ_VEC),
        (u16)LOADmetadataSize(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_POLYZOOM_BLITZ_VEC));

    /* INIT SCENES */
    this->animationCallback = IntroAnimation_Scene0;
    this->scene.nbrepeat = 7;

    state = &this->animationState;
    state->ic = 0;
    state->icx = 0;
    state->animState.coef = 4900;


    polylinestmp = (u16*) MEM_ALLOCTEMP ( &sys.allocatorMem, SIZEMAX_PERPOLY );

    this->displist       = (u16*) MEM_ALLOC ( &sys.allocatorMem, POLY_DISPLISTSIZE );
    this->displistp      = this->displist;
    this->currentFrame   = 0;

    /* PRECOMPUTE VERTICES POSITIONS */	
    dlist = this->displist;

    {
        VECscene* scene = &this->scene;
        VECanimationCallback animate = this->animationCallback;

        if (animate != NULL)
        {
            VECanimationState* animationState = &(this->animationState.animState);
            /* CybervectorAnimationState* animationState = &(scene->animationState);  MAKES A FUCKING COMPILER ERROR :( */
            u16* scenedisplist = dlist;


            scene->displist = scenedisplist;

            dlist = VECscenePrecompute(scene, dlist, info_pzcheckList, g_screens.base.cos, g_screens.base.sin, animate, animationState, polylinestmp, SIZEMAX_PERPOLY);

#           ifdef DEMOS_DEBUG
            {
                u32 displistsize = ((u8*)dlist - (u8*)this->displist);
                TRAClogNumber10S(TRAC_LOG_SPECIFIC, "animsize", displistsize, 8, '\n');
            }
#           endif
        }
    }

    this->erase[0]     = this->erase[1]     = this->displist;
    this->erasebase[0] = this->erasebase[1] = this->framebuffer;

    MEM_FREE(&sys.allocatorMem, polylinestmp);
}


static void infoInitCode(Info* this, u8* temp)
{
    this->code = MEM_ALLOC(&sys.allocatorMem, LOADmetadataOriginalSize(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_______OUTPUT_BLITZIK_INFO_ARJX));
    ARJdepack(this->code, temp + LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_______OUTPUT_BLITZIK_INFO_ARJX));
    SYSrelocate(this->code);

    InfoImport(this);
}


static void infoInitText(Info* this, u8* temp) 
{
    u8 compomode = g_screens.compomode || INFO_FORCE_COMPOMODE();
    u32 textsize = LOADmetadataOriginalSize(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_INFO_INFO_ARJX + compomode);
    this->text = (char*) MEM_ALLOC(&sys.allocatorMem, textsize);
    ARJdepack(this->text, temp + LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_INFO_INFO_ARJX + compomode));
    this->textlines = (u16)STDdivu(textsize, INFO_LINE_SIZE);
}


static void infoInitVolumesMasks (Info* this, u8* temp)
{
    u16* p = (u16*)(temp + LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_INFO_VOLMASKS_BIN));
    u16 t;

    for (t = 0; t <= INFO_VOLUME_MAX; t++)
    {
        this->volumemasks[t][0] = *p++;
        this->volumemasks[t][1] = *p++;
        this->volumemasks[t][2] = *p++;
    }
}


void InfoEnter (FSM* _fsm)
{
    Info* this;
    u8*   temp = NULL;


    IGNORE_PARAM(_fsm);

    BlitZsetVideoMode(HW_VIDEO_MODE_4P, 0, BLITZ_VIDEO_16XTRA_PIXELS);

    ScreensLogFreeArea("InfoEntry");

    this = g_screens.info = MEM_ALLOC_STRUCT( &sys.allocatorMem, Info );	
    DEFAULT_CONSTRUCT(this);

    {
        LOADrequest *loadRequestVec, *loadRequest2;
        u8  *tempvec;

        tempvec = MEM_ALLOCTEMP(&sys.allocatorMem, LOADresourceRoundedSize(&RSC_BLITZWAV, RSC_BLITZWAV_POLYZOOM_POLYZOOM_VEC));
        loadRequestVec = LOADdata(&RSC_BLITZWAV, RSC_BLITZWAV_POLYZOOM_POLYZOOM_VEC, tempvec, LOAD_PRIORITY_INORDER);

        temp = MEM_ALLOCTEMP(&sys.allocatorMem, LOADresourceRoundedSize(&RSC_BLITZWAV, RSC_BLITZWAV_______OUTPUT_BLITZIK_INFO_ARJX));
        loadRequest2 = LOADdata(&RSC_BLITZWAV, RSC_BLITZWAV_______OUTPUT_BLITZIK_INFO_ARJX, temp, LOAD_PRIORITY_INORDER);

        this->decompbuffer = (u8*) MEM_ALLOC ( &sys.allocatorMem, 200UL * (u32)VEC_PITCH );
        STDmset (this->decompbuffer, 0, 200UL * (u32)VEC_PITCH );

        this->framebuffer  = (u8*) MEM_ALLOC ( &sys.allocatorMem, INFO_FRAMEBUFFER_SIZE * 2UL );
        STDmset (this->framebuffer, 0, INFO_FRAMEBUFFER_SIZE * 2UL );

        LOADwaitRequestCompleted(loadRequestVec);

        infoInitVectorScene(this, tempvec); /* interlace loading in order to init vector scene while info file is loading */

        MEM_FREE(&sys.allocatorMem, tempvec);

        LOADwaitRequestCompleted(loadRequest2);
    }

    infoInitCode (this, temp);

    infoInitText (this, temp);

    if (g_screens.compomode || INFO_FORCE_COMPOMODE())
    {
        this->targetcursor = INFO_FIRST_STEP_COMPO;
        this->targetline   = INFO_FIRST_STEP_COMPO * INFO_LINE_H;
    }
    else
    {
        this->targetcursor = INFO_FIRST_STEP;
        this->targetline   = INFO_FIRST_STEP * INFO_LINE_H;
    }
    this->bmpindex       = 1;

    infoInitVolumesMasks (this, temp);

    /* plane 0: vector  */
    /* plane 1: volumes */
    /* plane 2: bitmap  */
    /* plane 3: text    */

#   ifndef __TOS__
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRAClog(TRAC_LOG_MEMORY, "Info memallocator dump", '\n');
        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif

    MEM_FREE(&sys.allocatorMem, temp); temp = NULL;

    SYSwriteVideoBase((u32)this->framebuffer);
    
    this->rasvbl.scanLinesTo1stInterupt = 199;
    this->rasvbl.nextRasterRoutine = RASlowB;
    this->raslowB.nextRasterRoutine = NULL;
    this->raslowB.scanLineToNextInterupt = 200;    

    RASnextOpList     = &this->rasvbl;
    SYSvblroutines[1] = (SYSinterupt)RASvbl;

    SYSvsync;

    {
        static u16 colors[] = 
        {
            INFO_VECTOR_COLOR,
            0,
            INFO_VECTOR_COLOR,
            0,
            INFO_VECTOR_COLOR,
            0,
            INFO_VECTOR_COLOR,

            INFO_TEXT_COLOR_STE,
            INFO_TEXT_COLOR_STE3,
            INFO_TEXT_COLOR_STE2,
            INFO_TEXT_COLOR_STE3,
            INFO_TEXT_COLOR_STE,
            INFO_TEXT_COLOR_STE3,
            INFO_TEXT_COLOR_STE2,
            INFO_TEXT_COLOR_STE3
        };

        STDmcpy2(HW_COLOR_LUT + 1, colors, 30);
        PCENDIANSWAPBUFFER16 (HW_COLOR_LUT + 1, 15);
    }

    BLZ_FLUSH_COMMANDS;

    FSMgotoNextState (_fsm);
    FSMgotoNextState(&g_stateMachine);
}



/*
0    1    2    3    4    5    6    7    8    9   10   11   12   13   14
ROL 

0     0    0    0    0    0    0    6    6    6    6    6    6    6    6   14
1          1    1    1    1    1    1    7    7    7    7    7    7    7    7
2   >           2    2    2    2  > 2    2    8    8    8    8    8    8  > 8
3   >    >           3    3    3  > 3  > 3    3    9    9    9    9    9  > 9
4   >    >    >           4    4  > 4  > 4  > 4    4   10   10   10   10  >10
5   >    >    >    >           5  > 5  > 5  > 5  > 5    5   11   11   11  >11
6        >    >    >    >              > 6  > 6  > 6  > 6    6   12   12   12
7             >    >    >    >      6    6  > 7  > 7  > 7  > 7    7   13   13
0    0    0  > 0  > 0  > 0    1    7    7  > 7  > 7  > 7  > 7    7    
1    1    1  > 1  > 1    1    1    8    8  > 8  > 8  > 8  > 8
2    2    2  > 2    2    2    9    9    9  > 9  > 9  > 9
3    3    3    3    3    3    3   10   10  >10  >10
4    4    4    4    4    4    4   11   11  >11
5    5    5    5    5    5    5   12   12
13
*/

static void infoDisplayText(Info* this)
{  
    /*static u16 count = 0;*/
    /*STDutoa(currentstr, count++, 5);*/

    if (this->printcursor != this->targetcursor)
    {
        u8* pup1 = this->framebuffer + 6;
        u8* pup2, * pbot1, * pbot2;
        u16 realprintcursor, rollingprintcursor;


        if (this->targetcursor > this->printcursor)
        {
            s16 delta = this->printcursor * INFO_LINE_H;
            delta -= this->displayedline;
            if (delta >= INFO_LINE_H)
                return;

            rollingprintcursor = this->printcursor & 31;
            realprintcursor    = this->printcursor + 32;

            this->printcursor++;
        }
        else
        {
            s16 delta = this->displayedline;
            delta -= this->printcursor * INFO_LINE_H;
            if (delta >= INFO_LINE_H)
                return;

            rollingprintcursor = this->printcursor & 31;
            realprintcursor    = this->printcursor;
            this->printcursor--;
        }

        INFO_RASTERIZE_COLOR(0x770);

        pup1 += STDmulu(rollingprintcursor, INFO_LINE_H * VEC_PITCH);

        if (this->flip)
        {
            pup2  = pup1;
            pbot2 = pup1  + 32UL * (u32)INFO_LINE_H * (u32)VEC_PITCH;
            pbot1 = pbot2 + INFO_FRAMEBUFFER_SIZE;
            pup1  = pup2  + INFO_FRAMEBUFFER_SIZE;
        }
        else
        {
            pbot1 = pup1  + 32UL * (u32)INFO_LINE_H * (u32)VEC_PITCH;
            pbot2 = pbot1 + INFO_FRAMEBUFFER_SIZE;
            pup2  = pup1  + INFO_FRAMEBUFFER_SIZE;
        }

        SYSfastPrint(this->text + realprintcursor * INFO_LINE_SIZE, pup1, VEC_PITCH, 8, (u32)&SYSfont);

        INFO_RASTERIZE_COLOR(0x330);

        *HW_BLITTER_ENDMASK1 = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1;
        *HW_BLITTER_XSIZE = (VEC_PITCH / 8) * 8;
        *HW_BLITTER_HOP = HW_BLITTER_HOP_SOURCE;
        *HW_BLITTER_OP = HW_BLITTER_OP_S;
        *HW_BLITTER_XINC_SOURCE = 8;
        *HW_BLITTER_XINC_DEST = 8;
        *HW_BLITTER_YINC_SOURCE = 8;
        *HW_BLITTER_YINC_DEST = 8;
        *HW_BLITTER_CTRL2 = 0;

        *HW_BLITTER_ADDR_SOURCE = (u32)pup1;
        *HW_BLITTER_ADDR_DEST = (u32)pbot1;
        *HW_BLITTER_YSIZE = 1;
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();

        INFO_RASTERIZE_COLOR(0x440);

        *HW_BLITTER_ADDR_SOURCE = (u32)pup1;
        *HW_BLITTER_ADDR_DEST = (u32)pbot2;
        *HW_BLITTER_YSIZE = 1;
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();

        INFO_RASTERIZE_COLOR(0x330);

        *HW_BLITTER_ADDR_SOURCE = (u32)pup1;
        *HW_BLITTER_ADDR_DEST = (u32)pup2;
        *HW_BLITTER_YSIZE = 1;
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */
        EMULblit();
    }
}


static void infoDisplayVolumes(Info* this, u8* image)
{
    u16  t;
  
    INFO_RASTERIZE_COLOR(0x20);

    for (t = 0 ; t < 4 ; t++)
    {
        u16 level = this->volumes[t];
        if (level >= INFO_VOLUME_MAX)
            level = INFO_VOLUME_MAX;
        
        /*printf ("%d: %d %d ", t, level, this->maxvols[t]);*/

        if (level > this->maxvols[t])
            this->maxvols[t] = level;
        else if (this->maxvols[t] > 0)
            this->maxvols[t] --;            
    }

    for (t = 0 ; t < 3 ; t++)
    {
        u16 level = this->ymstates[t].level;

        level += level + level;
        
        if (level > this->maxvols[t+4])
            this->maxvols[t+4] = level;
        else if (this->maxvols[t+4] > 0)
            this->maxvols[t+4]--;            

        /*printf ("%d: %d %d ", t, level, this->maxvols[t]);*/
    }

    /*printf("\n");*/

    image += 2;

    INFO_RASTERIZE_COLOR(0x30);

    this->asmimport->drawvolumes (this->maxvols, image, (u32) this->volumemasks);
 
}

void InfoBacktask(FSM* _fsm)
{
    Info*   this   = g_screens.info;
    u8*     image  = this->framebuffer + 4 + (INFO_CURSOR_OFFSET1 * INFO_LINE_H) * VEC_PITCH;
    u8*     image2;
    u8*     bmp    = g_screens.layerzStatic.bmps[this->bmpindex].blitzbmp;
    u16     offset = g_screens.layerzStatic.bmps[this->bmpindex].blitzbmpoffset;
    
    static u16 startcolors[4] = { 0x0F0, 0x0F7, 0x060, 0x067 };
    static u16 endcolors  [4] = { 0, INFO_VECTOR_COLOR, 0, INFO_VECTOR_COLOR };


    EMUL_BEGIN_ASYNC_REGION

    this->bmpindex++;
    if (this->bmpindex >= ARRAYSIZE(g_screens.layerzStatic.bmps))
        this->bmpindex = 1;

    this->asmimport->clearplane(this->decompbuffer, 200, 0);

    if (this->exit) return;

    BIT1pUncompress(bmp, bmp + offset, (u32)this->decompbuffer);  

    if (this->exit) return;

    {
        EMUL_STATIC u8 trackindex;

        trackindex = g_screens.player.trackindex;

        EMUL_REENTER_POINT;
        do
        {
            if (this->exit) return;

            EMUL_EXIT_IF(trackindex == g_screens.player.trackindex);
        }
        while (trackindex == g_screens.player.trackindex);

        if (this->compocountdonotreset == false)
        {
            this->compocount = INFO_COMPOMODE_PACE;
            this->compocountdonotreset = true;
        }
    }

    {
        u16  line  = this->rollingline;
        bool part1 = true;

        PCONLY(printf("line 1: %d\n", line));

        line += (INFO_DISPLAY_H - 200) / 2;

        PCONLY(printf("line 2: %d\n", line));

        if ((line + INFO_BITMAP_Y2) >= (32 * INFO_LINE_H))
        {
            u16 line2, line3;
            u16 delta1, delta2 = 0xFFFF;

            line2 = (32 * INFO_LINE_H) - INFO_BITMAP_Y2;
            delta1 = line - line2;

            if ((line - ((32UL + INFO_CURSOR_OFFSET1) * INFO_LINE_H)) >= INFO_BITMAP_Y1)
            {
                line3 = line;

                if (line < ((32 + INFO_CURSOR_OFFSET1) * INFO_LINE_H - INFO_BITMAP_Y1))
                    line3 = ((32 + INFO_CURSOR_OFFSET1) * INFO_LINE_H - INFO_BITMAP_Y1);

                delta2 = line3 - line;            
            }

            if (delta2 < delta1)
            {
                line = line3;
                part1 = false;
            }
            else
            {
                line = line2;
            }
        }

        PCONLY(printf("line (result) %d: %d\n", part1, line));

        image += STDmulu(line, VEC_PITCH);

        if (part1)
            image2 = image + (32UL * (u32)INFO_LINE_H * (u32)VEC_PITCH);
        else
            image2 = image - (32UL * (u32)INFO_LINE_H * (u32)VEC_PITCH);

        ASSERT((image  + INFO_BITMAP_Y1 * VEC_PITCH) >= this->framebuffer);
        ASSERT((image2 + INFO_BITMAP_Y1 * VEC_PITCH) >= this->framebuffer);
    }

    this->asmimport->copyplane(
        this->decompbuffer + INFO_BITMAP_Y1 * VEC_PITCH, 
        image + INFO_BITMAP_Y1 * VEC_PITCH, 
        (u32)(image2 + INFO_BITMAP_Y1 * VEC_PITCH), 
        INFO_BITMAP_Y2 - INFO_BITMAP_Y1);

    /* plane 0: vector  */
    /* plane 1: volumes */
    /* plane 2: bitmap  */
    /* plane 3: text    */

    {
        EMUL_STATIC u16 t;
        u16 colors[4];

        for (t = 4; t <= 16; )
        {
            EMUL_REENTER_POINT;

            SYSvsync;

            if (this->exit) return;

            COLcomputeGradient16Steps(endcolors, startcolors, 4, t, colors);
            PCENDIANSWAPBUFFER16(colors, 4);
            STDmcpy2(HW_COLOR_LUT + 4, colors, 4 << 1);

            t += 4;
            EMUL_EXIT_IF(t <= 16);
        }

        for (t = 0; t <= 64; )
        {
            EMUL_REENTER_POINT;

            SYSvsync;

            if (this->exit) return;

            COLcomputeGradient16Steps(startcolors, endcolors, 4, t >> 2, colors);
            PCENDIANSWAPBUFFER16(colors, 4);
            STDmcpy2(HW_COLOR_LUT + 4, colors, 4 << 1);

            t++;
            EMUL_EXIT_IF(t <= 64);
        }
    }

    this->asmimport->clearplane(this->framebuffer + 4, (32 + 32 + INFO_CURSOR_OFFSET1) * 2 * INFO_LINE_H, 0);

    EMUL_END_ASYNC_REGION
}


/*static s32 debugoffset = 0;*/

void InfoActivity (FSM* _fsm)
{
    /*s16 miny, maxy;*/
    Info*     this              = g_screens.info;
    u8*       image;
    u16*      currentlist       = this->displistp;
    VECscene* scene             = &this->scene;


    IGNORE_PARAM(_fsm);

    INFO_RASTERIZE_COLOR(0x200);

    SmplCurveYMgetStates (this->ymstates);

    VECclrpass();

    VECclr(this->erasebase[this->flip], this->erase[this->flip]);

#   ifdef __TOS__
    {
        u8 c = *HW_VIDEO_COUNT_L;
        while (c == *HW_VIDEO_COUNT_L);
    }
#   else
    EMULfbExStart(HW_VIDEO_MODE_4P, 64, 63, 64 + VEC_PITCH * 2 - 1, 63 + INFO_DISPLAY_H - 1, VEC_PITCH, 0);
    EMULfbExEnd();
#   endif

    infoDisplayText(this);

    this->rollingline = (u16)(STDdivu(this->displayedline, 32 * INFO_LINE_H) >> 16);
    this->offset = STDmulu(this->rollingline + INFO_CURSOR_OFFSET1 * INFO_LINE_H, VEC_PITCH);

    image = this->framebuffer + this->offset;

    if (this->flip)
        image += INFO_FRAMEBUFFER_SIZE;
    
    infoDisplayVolumes(this, image);

    /* vector */
    INFO_RASTERIZE_COLOR(0x400);
    this->displistp = VECloop(image + INFO_VECTOR_YOFFSET * VEC_PITCH, currentlist + 4, scene->nbPolygons);

    INFO_RASTERIZE_COLOR(0x200);
    VECcircle(image + INFO_VECTOR_YOFFSET * VEC_PITCH, 168, 100, 78);
    INFO_RASTERIZE_COLOR(0);

    INFO_RASTERIZE_COLOR(0x700);

    VECxorpass(0x206);

    currentlist[0] = 40;
    currentlist[1] = 11;
    currentlist[2] = 22;
    currentlist[3] = 115;

    VECxor(image + INFO_VECTOR_YOFFSET * VEC_PITCH, currentlist);   /* split xor pass around to avoid low border interrupt */

    INFO_RASTERIZE_COLOR(0x30);

    /* defer this CPU computation to allow low border interupt to happen there */
    this->asmimport->computevolumes ( (s8*) g_screens.player.dmabufstart, this->volumes ); 

    INFO_RASTERIZE_COLOR(0x0);

#   ifdef __TOS__
    while (*HW_VECTOR_TIMERB != NULL);
#   endif

    INFO_RASTERIZE_COLOR(0x700);

    currentlist[2] = 115;
    currentlist[3] = 178;

    VECxor(image + INFO_VECTOR_YOFFSET * VEC_PITCH, currentlist); /* split xor pass around to avoid low border interrupt */

    currentlist[2] = 22;

    /* the goal is to end frame to let time for background interupt */

    INFO_RASTERIZE_COLOR(0x2);

    this->erase[this->flip] = currentlist;
    this->erasebase[this->flip] = image + INFO_VECTOR_YOFFSET * VEC_PITCH;

    this->currentFrame++;

    if ( this->currentFrame >= scene->nbframes )
    {
        this->currentFrame = 0;
        this->displistp = (u16*)scene->displist;
    }

    if (this->targetline != this->displayedline)
    {
        static u8 movecurve2[] = {1, 2, 3, 4};
        s16 speed = this->targetline - this->displayedline;

        if (speed < 0)
        {
            speed = -speed;
            speed >>= 3;
            if (speed >= ARRAYSIZE(movecurve2))
                speed = ARRAYSIZE(movecurve2) - 1;
            speed = movecurve2[speed];
            this->displayedline -= speed;
            this->direction = -1;
        }
        else
        {
            speed >>= 3;
            if (speed >= ARRAYSIZE(movecurve2))
                speed = ARRAYSIZE(movecurve2) - 1;
            speed = movecurve2[speed];
            this->displayedline += speed;
            this->direction = 1;
        }

        TRAClogNumber10S(TRAC_LOG_SPECIFIC, "targetcursor"      , this->targetcursor,  3, ' ');
        TRAClogNumber10S(TRAC_LOG_SPECIFIC, "printcursor"       , this->printcursor,   3, ' ');
        TRAClogNumber10S(TRAC_LOG_SPECIFIC, "targetline"        , this->targetline,    3, ' ');
        TRAClogNumber10S(TRAC_LOG_SPECIFIC, "displayedline"     , this->displayedline, 3, ' ');
        TRAClogNumber10S(TRAC_LOG_SPECIFIC, "offset"            , this->offset,        5, '\n');
    }
    else
    {
        this->direction = 0;
    }

    SYSwriteVideoBase((u32)image);

    this->flip ^= 1;

    if (g_screens.compomode || INFO_FORCE_COMPOMODE())
    {
        this->compocount++;

        if (this->compocount > INFO_COMPOMODE_PACE)
        {
            if ((this->targetcursor + 31 + 1) < this->textlines)
            {
                this->targetcursor ++;
                this->targetline   += INFO_LINE_H;
            }
            
            this->compocount = 0;
        }
    }

    while(BLZ_COMMAND_AVAILABLE)
    {
        u8 cmd = BLZ_CURRENT_COMMAND;

        switch (cmd)
        {
        case BLZ_CMD_SELECT:
            if (g_screens.runscreen != BLZ_EP_MENU)
            {
                g_screens.runscreen = BLZ_EP_MENU;
                FSMgotoNextState(&g_stateMachineIdle);
                FSMgotoNextState(_fsm);
                this->exit = true;
            }
            break;

        case BLZ_CMD_UP:
            if (this->targetcursor >= INFO_SCROLL_STEP)
            {
                this->targetcursor -= INFO_SCROLL_STEP;
                this->targetline   -= INFO_LINE_H * INFO_SCROLL_STEP;
            }
            break;

        case BLZ_CMD_DOWN:
            if ((this->targetcursor + 31 + INFO_SCROLL_STEP) <= this->textlines)
            {
                this->targetcursor += INFO_SCROLL_STEP;
                this->targetline   += INFO_LINE_H * INFO_SCROLL_STEP;
            }
            break;
        
        case BLZ_CMD_LEFT:
            if (this->targetcursor >= INFO_SCROLL_STEP2)
            {
                this->targetcursor -= INFO_SCROLL_STEP2;
                this->targetline   -= INFO_LINE_H * INFO_SCROLL_STEP2;
            }
            break;

        case BLZ_CMD_RIGHT:
            if ((this->targetcursor + 31 + INFO_SCROLL_STEP2) <= this->textlines)
            {
                this->targetcursor += INFO_SCROLL_STEP2;
                this->targetline   += INFO_LINE_H * INFO_SCROLL_STEP2;
            }
            break;

      /*  case BLZ_CMD_Q:
            debugoffset -= VEC_PITCH;
            break;

        case BLZ_CMD_W:
            debugoffset += VEC_PITCH;
            break;

        case BLZ_CMD_E:
            debugoffset = 0;
            break;*/
        }

        BLZ_ITERATE_COMMAND;
    }

    INFO_RASTERIZE_COLOR(0);
}


void InfoExit(FSM* _fsm)
{
    Info* this= g_screens.info;

    IGNORE_PARAM(_fsm);

    SYSvblroutines[1] = RASvbldonothing;
    SYSvsync;
    STDmset (HW_COLOR_LUT, 0UL, 32);

    VECsceneDestroy(&this->scene);

    MEM_FREE(&sys.allocatorMem, this->text);
    MEM_FREE(&sys.allocatorMem, this->displist);
    MEM_FREE(&sys.allocatorMem, this->code);
    MEM_FREE(&sys.allocatorMem, this->framebuffer);
    MEM_FREE(&sys.allocatorMem, this->decompbuffer);
    MEM_FREE(&sys.allocatorMem, this);

    this = g_screens.info = NULL;

#   if DEMOS_MEMDEBUG
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    ScreensGotoScreen();
}
