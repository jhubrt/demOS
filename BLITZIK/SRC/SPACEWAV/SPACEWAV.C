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

#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\BITMAP.H"

#include "DEMOSDK\SYNTHYMD.H"

#include "DEMOSDK\PC\EMUL.H"

#include "BLITZIK\SRC\SCREENS.H"


#ifdef __TOS__
#define SPACEWAV_RASTERIZE() 0
#else
#define SPACEWAV_RASTERIZE() 0
#endif

#if SPACEWAV_RASTERIZE()
#   define SPACEWAV_RASTERIZE_COLOR(COLOR) *HW_COLOR_LUT=COLOR
#else
#   define SPACEWAV_RASTERIZE_COLOR(COLOR)
#endif

#define SPACEWAV_PITCH       168
#define SPACEWAV_NBSTARS     96
#define SPACEWAV_INC         20
#define SPACEWAV_ZMAX        (126-STAR_INCMAX)
#define SPACEWAV_HEIGHT      200


static s16 spaceWavPolygonImageInc[] = {0, 2, 4, 0, 2, 4, 0, 0};

#define pos3dx(this, x_, z) (this->stars.copyposx[((x_) << STAR_ZSHIFT) + (z)])
#define pos3dy(this, y_, z) (this->stars.copyposy[((y_) << STAR_ZSHIFT) + (z)])

static void spaceWavResetStarPositions (STARparam* _param)
{
    u32* z = _param->starz;
    u32 resetx = (u32) (&_param->prex[STAR_ZMAX]);
    u16 t;


    for (t = 0 ; t < _param->nbstars ; t++)
    {
        *z++ = resetx;
        *z++ = (u32) _param->prey;
    }
}


static void spaceWavManageCommands(SpaceWav* this, bool allownavigation_)
{
    while (BLZ_COMMAND_AVAILABLE)
    {
        u8 cmd = BLZ_CURRENT_COMMAND;
        u8 category = cmd & BLZ_CMD_CATEGORY_MASK;
        /* u8 category = cmd & BLZ_CMD_CATEGORY_MASK; */

        BLZ_ITERATE_COMMAND;

        if (category == BLZ_CMD_LINE1_CATEGORY)
        {
            if (cmd <= BLZ_CMD_E)
            {
                BLZ_TRAC_COMMAND_NUM("SPWspeed", cmd - BLZ_CMD_Q + 1);
                this->stars.inc = (cmd - BLZ_CMD_Q + 1) << 1;
            }
        }            
        else if (category == BLZ_CMD_LINE2_CATEGORY)
        {
            static u16 backgroundcolors[] = {0xF00, 0xF0, 0xF, 0xFFF};
            static u16 backgroundinc1  [] = {0x100, 0x10, 0x1, 0x111};
            static u16 backgroundinc2  [] = {0x800, 0x80, 0x8, 0x888};

            cmd &= BLZ_CMD_COMMAND_MASK;

            if (cmd < ARRAYSIZE(backgroundcolors))
            {
                BLZ_TRAC_COMMAND_NUM("SPWflashColor", cmd);
                
                this->backgroundcolor = backgroundcolors[cmd];
                this->backgroundinc1  = backgroundinc1  [cmd];
                this->backgroundinc2  = backgroundinc2  [cmd];
            }
        }
        else if (allownavigation_)
        {
            if (ScreensManageScreenChoice(BLZ_EP_SPACEWAV, cmd))
                return;
        }
    }
}

static void spaceWavFillYMtable (SpaceWav* this)
{
    u16 t;
    u8  last;
    u8* p;
    u16* k = SNDYM_g_keys.w;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "spaceWavFillYMtable", '\n');

    STDfastmset(this->ymheight, -1, sizeof(this->ymheight));

    p = this->ymheight;

    for (t = 0; t < SNDYM_NBKEYS; t++)
    {
        u16 index = *k++;
        p[PCENDIANSWAP16(index)] = (u8) t; 
    }

    last = SNDYM_NBKEYS - 1;

    p = this->ymheight;

    for (t = 0; t < ARRAYSIZE(this->ymheight); t++)
    {
        if (*p != 0xFF)
            last = *p++;
        else
            *p++ = last; 
    }
}

static void spaceWavInitRasters(SpaceWav* this)
{
    u16* p = this->rasters.ras[14].colors;   
    u16 t;
         

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "spaceWavInitRasters", '\n');

    p[0]  = 0x100;
    p[1]  = 0x010;
    p[2]  = 0x110;
    p[3]  = 0x001;
    p[4]  = 0x101;
    p[5]  = 0x011;
    p[6]  = 0x111;
          
    p[7]  = 0x888;
    p[8]  = 0x100;
    p[9]  = 0x010;
    p[10] = 0x110;
    p[11] = 0x001;
    p[12] = 0x101;
    p[13] = 0x011;
    p[14] = 0x111;

    this->rasters.vbl.nextRasterRoutine      = (RASinterupt)RASmid15;
    this->rasters.vbl.scanLinesTo1stInterupt = 18;
    
    p = this->rasters.vbl.colors;

    p[0]  = 0xF00;
    p[1]  = 0x0F0;
    p[2]  = 0xFF0;
    p[3]  = 0x00F;
    p[4]  = 0xF0F;
    p[5]  = 0x0FF;
    p[6]  = 0x555;
         
    p[7]  = 0xFFF;
    p[8]  = 0xF00;
    p[9]  = 0x0F0;
    p[10] = 0xFF0;
    p[11] = 0x00F;
    p[12] = 0xF0F;
    p[13] = 0x0FF;
    p[14] = 0x555;

    COLcomputeGradient16Steps(this->rasters.vbl.colors, this->rasters.ras[14].colors, 15, 1, this->rasters.rasbegin.colors);
    STDmcpy2(this->rasters.ras[29].colors, this->rasters.rasbegin.colors, 30);

    this->rasters.rasbegin.colors[14]            |= RASstopMask;
    this->rasters.rasbegin.nextRasterRoutine      = (RASinterupt)RASmid15;
    this->rasters.rasbegin.scanLineToNextInterupt = 5;

    for (t = 0 ; t < 14 ; t++)
    {
        COLcomputeGradient16Steps(this->rasters.vbl.colors, this->rasters.ras[14].colors, 15, t + 2, this->rasters.ras[t].colors);
        STDmcpy2(this->rasters.ras[29 - t].colors, this->rasters.ras[t].colors, 30);
    }

    STDmcpy2(this->rasters.rasend.colors, this->rasters.vbl.colors, 30);

    this->rasters.rasend.colors[14]            |= RASstopMask;
    this->rasters.rasend.nextRasterRoutine      = NULL;
    this->rasters.rasend.scanLineToNextInterupt = 200;
}

static void spaceWavInitKeysInfo (SpaceWav* this)
{
    u16 t;
    u8 keymin = 255;
    u8 keymax = 0;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "spaceWavInitKeysInfo", '\n');

    this->keysnoteinfo = (u8*)MEM_ALLOC(&sys.allocatorMem, g_screens.player.sndtrack->nbKeys);
    this->keyh = (u8*)MEM_ALLOC(&sys.allocatorMem, g_screens.player.sndtrack->nbKeys);

    for (t = 0; t < g_screens.player.sndtrack->nbKeys; t++)
    {
        u8 key = g_screens.player.sndtrack->keysnoteinfo[t];

        key = (key >> 4) * 12 + (key & 0xF);

        if (key > keymax)
            keymax = key;

        if (key < keymin)
            keymin = key;

        this->keysnoteinfo[t] = key;
    }

    for (t = 0; t < g_screens.player.sndtrack->nbKeys; t++)
    {
        this->keyh[t] = 128 + (u16) STDdivu((this->keysnoteinfo[t] - keymin) * 71, keymax - keymin);
    }
}



void SpaceWavEnter (FSM* _fsm)
{
    SpaceWav* this;
    u32  framebuffersize;
    u16  t;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "SpaceWavEnter", '\n');

    STDmset(HW_COLOR_LUT,0,32);

    this = g_screens.spacewav = MEM_ALLOC_STRUCT(&sys.allocatorMem, SpaceWav);
    DEFAULT_CONSTRUCT(this);

    framebuffersize = (u32)SPACEWAV_PITCH * (u32)SPACEWAV_BUFFER_H;

    this->framebuffer[0] = (u8*) MEM_ALLOC(&sys.allocatorMem, framebuffersize * 2UL);
    STDfastmset(this->framebuffer[0], 0, framebuffersize * 2UL);
    this->framebuffer[1] = this->framebuffer[0] + framebuffersize;

    SYSwriteVideoBase ((u32) this->framebuffer[0]);

    {
        s16* p = this->pitchmul;
        s16  offset = -(SPACEWAV_BUFFER_H / 2) * SPACEWAV_PITCH;
    
        for (t = 0; t < SPACEWAV_BUFFER_H; t++)
        {
            *p++ = offset;
            offset += SPACEWAV_PITCH;
        }
    }
    this->stars.nbstars  = SPACEWAV_NBSTARS;
    this->stars.pitch    = SPACEWAV_PITCH;
    this->stars.height   = SPACEWAV_HEIGHT;
    this->stars.pitchmul = this->pitchmul;
    this->stars.inc      = 2;
    this->nbstarsdiv6minusone = (u16) STDdivu(this->stars.nbstars, 6) - 1;

    for (t = 0 ; t < ARRAYSIZE(this->states) ; t++)
    {
        this->states[t].mask    = 0xFFFF;
        this->states[t].lastkey = -1;
    }

    spaceWavInitKeysInfo(this);

    spaceWavFillYMtable(this);

    spaceWavInitRasters(this);

    this->stars.copyposx = (s16*) MEM_ALLOC(&sys.allocatorMem, sizeof(s16) * ((u32)STAR_WIDTH * (u32)(STAR_ZMAX + 1)));
    this->stars.copyposy = (u8*)  MEM_ALLOC(&sys.allocatorMem, SPACEWAV_HEIGHT * (STAR_ZMAX + 1));

    STARinit1P(&sys.allocatorMem, &this->stars, g_screens.wavheroStatic.starfielddiv);

    BlitZsetVideoMode(HW_VIDEO_MODE_4P, 0, BLITZ_VIDEO_16XTRA_PIXELS);

    RASnextOpList = &this->rasters;
    SYSvblroutines[1] = (SYSinterupt)RASvbl15;

    spaceWavManageCommands(this, false);  /* hack => do this here in sync to avoid complexifying to put it on main thread like it should be */

    TRACsetVideoMode (168);

#   ifndef __TOS__
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRAClog(TRAC_LOG_MEMORY,"SpaceWav memallocator dump", '\n');
        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif

    FSMgotoNextState(_fsm);
    FSMgotoNextState(&g_stateMachine);
}


#ifdef __TOS__
#define spaceWavPCDisplay(this)
#else
void spaceWavPCDisplay(SpaceWav* this)
{
    EMULfbExStart(HW_VIDEO_MODE_4P, 80, 40, 80 + SPACEWAV_PITCH * 2 - 1, 40 + SPACEWAV_BUFFER_H - 1, SPACEWAV_PITCH, 0);
    {
        u16 t, i;
        u16 y = 40;
        u32 cycles = EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, y);


        for (i = 0; i < 15; i++)
        {
            EMULfbExSetColor(cycles, i + 1, PCENDIANSWAP16(this->rasters.vbl.colors[i]));
        }
        
        y += 20;
        cycles = EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, y);
        for (i = 0; i < 15; i++)
        {
            EMULfbExSetColor(cycles, i + 1, PCENDIANSWAP16(this->rasters.rasbegin.colors[i]));
        }

        for (t = 0; t < ARRAYSIZE(this->rasters.ras); t++)
        {
            y += 5;
            cycles = EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, y);

            for (i = 0; i < 15; i++)
            {
                EMULfbExSetColor(cycles, i + 1, PCENDIANSWAP16(this->rasters.ras[t].colors[i]));
            }
        }

        y += 5;
        cycles = EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, y);
        for (i = 0; i < 15; i++)
        {
            EMULfbExSetColor(cycles, i + 1, PCENDIANSWAP16(this->rasters.rasend.colors[i]));
        }
    }
    EMULfbExEnd();
}
#endif



void SpaceWavActivity(FSM* _fsm)
{
    SpaceWav* this = g_screens.spacewav;

    u8*       image = (u8*)this->framebuffer[this->flip];
    bool*     perasebase = this->verase[this->flip];
    bool*     perase;
    u16*      dlistbase = this->vlist[this->flip][0];
    u16*      dlist;
    u16       t;


    IGNORE_PARAM(_fsm);

    spaceWavPCDisplay(this);
    
    /* Erase polygons */
    SPACEWAV_RASTERIZE_COLOR(0x7);

    VECclrpass();

    for (t = 0, perase = perasebase, dlist = dlistbase; t < 7; t++)
    {
        u8* img = image + spaceWavPolygonImageInc[t];

        if (*perase++)
            VECclr(img, dlist);

        dlist += SPACEWAV_POLY_DRAWLIST_SIZE;
    }

    /* Background color */
    {
        u16 color = this->backgroundcolor;

        if (color != 0)
        {
            if (color & 0x888)
            {
                color &= 0x777;
            }
            else
            {
                color -= this->backgroundinc1;
                color |= this->backgroundinc2;
            }
        }

        this->backgroundcolor = color;
        aBLZbackground = PCENDIANSWAP16(color);
    }

    /* Starfield */
    {
        u32* erasebuffer = (u32*)this->stars.erasebuffers[this->flip];

        if (*erasebuffer != 0)
        {
            SPACEWAV_RASTERIZE_COLOR(0x300);
            STARerase1P(erasebuffer, this->nbstarsdiv6minusone);
        }

        SPACEWAV_RASTERIZE_COLOR(0x700);
        STARdraw1P(image + (6 + (SPACEWAV_BUFFER_H >> 1) * SPACEWAV_PITCH), this->stars.starz, this->stars.nbstars - 1, (u32)erasebuffer, (u32)&this->stars);
    }

    {
        SpaceWavState* state = this->states;

        BLSvoice* voice = g_screens.player.voices;
        u8        mixer;
        u16       t;


        SPACEWAV_RASTERIZE_COLOR(0x770);

        *HW_BLITTER_HOP = HW_BLITTER_HOP_BIT1;
        *HW_BLITTER_OP = HW_BLITTER_OP_BIT1;
        *HW_BLITTER_XINC_DEST = 8;

        for (t = 0; t < 4; t++, voice++, state++)
        {
            if (voice->keys[0] != NULL)
            {
                u16 index = ((u8*)voice->keys[0] - (u8*)g_screens.player.sndtrack->keys) / sizeof(BLSprecomputedKey);
                u16 y;
                u16 key;


                ASSERT(index < g_screens.player.sndtrack->nbKeys);

                key = this->keysnoteinfo[index];
                y = this->keyh[index];

                state->ywav   = y;
                state->levels = voice->volume * 2;

                state->back = 0;

                if (state->lastkey == -1)
                    state->front = 0;

                if (state->lastkey != key)
                    state->front = 0;

                state->front += SPACEWAV_INC;
                if (state->front > SPACEWAV_ZMAX)
                    state->front = SPACEWAV_ZMAX;

                state->lastkey = key;
            }
            else
            {
                if (state->lastkey != -1)
                {
                    state->back += SPACEWAV_INC;
                    if (state->back > SPACEWAV_ZMAX)
                    {
                        state->back    = SPACEWAV_ZMAX;
                        state->lastkey = -1;
                    }

                    state->front += SPACEWAV_INC;
                    if (state->front > SPACEWAV_ZMAX)
                        state->front = SPACEWAV_ZMAX;
                }
            }            
        }

        {
            u8 val;
            u8 level;
            u16 freq;
            u16 y;

            *HW_YM_REGSELECT = HW_YM_SEL_IO_AND_MIXER;
            mixer = HW_YM_GET_REG();

            for (t = 0; t < 3; t++, state++)
            {
                bool noise = false;

                *HW_YM_REGSELECT = HW_YM_SEL_LEVELCHA + t;
                level = HW_YM_GET_REG();

                state->mask = 0xFFFF;

                val = mixer & 9;
                if (val == 9)
                {
                    level = 0;
                }
                else if ((val & 1) == 0)
                {
                    *HW_YM_REGSELECT = HW_YM_SEL_FREQCHA_H + (t << 1);
                    freq = HW_YM_GET_REG() << 8;

                    *HW_YM_REGSELECT = HW_YM_SEL_FREQCHA_L + (t << 1);
                    freq |= HW_YM_GET_REG();

                    y = this->ymheight[freq];
                }
                else if ((val & 8) == 0)
                {
                    *HW_YM_REGSELECT = HW_YM_SEL_FREQNOISE;
                    y = HW_YM_GET_REG();
                    y <<= 1;

                    state->mask = this->flip ? 0xAAAA : 0x5555;
                    noise = true;
                }

                mixer >>= 1;

                if (level > 0)
                {
                    state->levels = (15 - level) - 4;
                    if (state->levels < 0)
                        state->levels = 0;

                    state->back = 0;

                    if (state->lastkey == -1)
                        state->front = 0;

                    if (state->lastkey == -1)
                    {
                        if (noise)
                            state->base = 0;
                        else
                            state->base = y - 36;
                    }
                    state->ywav = y - state->base;
                    if (state->ywav < 0)
                    {
                        state->base = y - 36;
                        state->ywav = y - state->base;
                        ASSERT(state->ywav >= 0);
                    }
                    else if (state->ywav > 71)
                    {
                        state->base = y - 36;
                        state->ywav = y - state->base;
                        ASSERT(state->ywav <= 71);
                    }

                    state->front += SPACEWAV_INC;
                    if (state->front > SPACEWAV_ZMAX)
                        state->front = SPACEWAV_ZMAX;

                    state->lastkey = y;
                }
                else
                {
                    if (state->lastkey != -1)
                    {
                        state->back += SPACEWAV_INC;
                        if (state->back > SPACEWAV_ZMAX)
                        {
                            state->back = SPACEWAV_ZMAX;
                            state->lastkey = -1;
                        }

                        state->front += SPACEWAV_INC;
                        if (state->front > SPACEWAV_ZMAX)
                            state->front = SPACEWAV_ZMAX;
                    }
                }
            }
        }

        {
            u16 x = 168 - 100;
            state = this->states;

            for (t = 0, perase = perasebase, dlist = dlistbase; t < 7; t++, perase++, state++)
            {
                u8* img = image + spaceWavPolygonImageInc[t];

                *perase = false;

                SPACEWAV_RASTERIZE_COLOR(0x770);

                if (state->lastkey != -1)
                {
                    u16 front = state->front;
                    u16 back = state->back;

                    if (front != back)
                    {
                        u16 x1 = x + state->levels;
                        u16 x2 = x + 40 - state->levels;
                        u16 y = state->ywav;

                        u16 x11 = pos3dx(this, x1, front);
                        u16 x21 = pos3dx(this, x1, back);

                        u16 x12 = pos3dx(this, x2, front);
                        u16 x22 = pos3dx(this, x2, back);

                        u16 y1 = pos3dy(this, y, front);
                        u16 y2 = pos3dy(this, y, back);

                        /*ASSERT((x11 >= 0) && (x11 < 320));
                        ASSERT((x12 >= 0) && (x12 < 320));
                        ASSERT((x21 >= 0) && (x21 < 320));
                        ASSERT((x22 >= 0) && (x22 < 320));
                        ASSERT((y1 >= 0) && (y1 < 200));
                        ASSERT((y2 >= 0) && (y2 < 200));*/

                        if (y1 != y2)
                        {
                            VECdisplayBar(x21, x22, x12, x11, y2, y1, dlist);

                            SPACEWAV_RASTERIZE_COLOR(0x550);
                            VECloop(img, dlist + 4, 1);

                            *perase = true;
                        }
                    }
                }

                dlist += SPACEWAV_POLY_DRAWLIST_SIZE;
                x += 50;

                if (t == 3)
                    x = 60;
            }
        }

        /* xor passes */
#       ifdef __TOS__
        SPACEWAV_RASTERIZE_COLOR(0);

        while (*HW_VECTOR_TIMERB != (u32) NULL);

        SPACEWAV_RASTERIZE_COLOR(0x50);
#       endif
        
        VECxorpass(0x206);

        perase = perasebase;
        dlist = dlistbase;
        state = this->states;

        for (t = 0; t < 7; t++, state++)
        {
            u8* img = image + spaceWavPolygonImageInc[t];

            if (state->mask != 0xFFFF)
            {
                u16* p = (u16*)HW_BLITTER_HTONE;
                u16  mask = state->mask;

                *HW_BLITTER_HOP = HW_BLITTER_HOP_SOURCE_AND_HTONE;

                *p++ = mask; *p++ = mask; *p++ = mask; *p++ = mask;
                *p++ = mask; *p++ = mask; *p++ = mask; *p++ = mask;
                *p++ = mask; *p++ = mask; *p++ = mask; *p++ = mask;
                *p++ = mask; *p++ = mask; *p++ = mask; *p++ = mask;
            }
            else
            {
                *HW_BLITTER_HOP = HW_BLITTER_HOP_SOURCE;
            }

            if (*perase++)
                VECxor(img, dlist);

            dlist += SPACEWAV_POLY_DRAWLIST_SIZE;
        }
    }

    SPACEWAV_RASTERIZE_COLOR(0x20);

    spaceWavManageCommands(this, true);

    SYSwriteVideoBase((u32)image);

    this->flip ^= 1;

    SPACEWAV_RASTERIZE_COLOR(0x0);
}


void SpaceWavBacktask(FSM* _fsm)
{
    IGNORE_PARAM(_fsm);

    /*STDstop2300();*/
}


void SpaceWavExit(FSM* _fsm)
{
    SpaceWav* this = g_screens.spacewav;

    IGNORE_PARAM(_fsm);

    SYSvblroutines[1] = RASvbldonothing;
    BlitZturnOffDisplay();

    STARshutdown(&sys.allocatorMem, &this->stars);

    MEM_FREE(&sys.allocatorMem, this->framebuffer[0]);
    MEM_FREE(&sys.allocatorMem, this->stars.copyposx);
    MEM_FREE(&sys.allocatorMem, this->stars.copyposy);
    MEM_FREE(&sys.allocatorMem, this->keysnoteinfo);
    MEM_FREE(&sys.allocatorMem, this->keyh);
    MEM_FREE(&sys.allocatorMem, this);

    this = g_screens.spacewav = NULL;

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
