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
#include "DEMOSDK\LOAD.H"
#include "DEMOSDK\BITMAP.H"
#include "DEMOSDK\TRACE.H"

#include "DEMOSDK\DATA\SYSTFNT.H"

#include "DEMOSDK\PC\WINDOW.H"

#include "BLITZIK\SRC\SCREENS.H"
#include "BLITZIK\BLITZWAV.H"

#include "DEMOSDK\PC\EMUL.H"


#if BLZ_DEVMODE()
#   define BLZ_WAITFOR_SCREENSELECT() 0
#else
#   define BLZ_WAITFOR_SCREENSELECT() 0
#endif

#define BLITZ_MENU_RASTERIZE_ENABLE 0

#if BLITZ_MENU_RASTERIZE_ENABLE
#   define BLITZ_MENU_RASTERIZE(COLOR) *HW_COLOR_LUT = PCENDIANSWAP16(COLOR);
#else
#   define BLITZ_MENU_RASTERIZE(COLOR)
#endif


#define BLITZ_MENUPLASMA_WIDTH          48
#define BLITZ_MENUPLASMA_HEIGHT         197
#define BLITZ_MENUPLASMA_BUFFERHEIGHT   99
#define BLITZ_MENUPLASMA_PITCH          (512 << 1)
#define BLITZ_MENUPLASMA_LOOP           256

#define BLITZ_MENU_XSPACING      128
#define BLITZ_MENU_YSPACING      24

#define BLITZ_MENU_PITCH         288    /* ((((BLITZ_MENU_WIDTH - 1) *BLITZ_MENU_XSPACING) / 4) + 160) */
#define BLITZ_MENU_SCROLL1_H     48
#define BLITZ_MENU_ICON_W        64
#define BLITZ_MENU_H             32
#define BLITZ_MENU_TIPS_H        10
#define BLITZ_MENU_TIPS_PITCH    (160 + 160)

#define BLITZ_MENU_EMPTY1_Y      BLITZ_MENU_SCROLL1_H
#define BLITZ_MENU_POS_Y         (BLITZ_MENU_EMPTY1_Y + 4)
#define BLITZ_MENU_EMPTY2_Y      (BLITZ_MENU_POS_Y + BLITZ_MENU_H)
#define BLITZ_MENU_TIPS_Y        (BLITZ_MENU_EMPTY2_Y + 5)
#define BLITZ_MENU_EMPTY3_Y      (BLITZ_MENU_TIPS_Y + BLITZ_MENU_TIPS_H)
#define BLITZ_MENU_SCROLL2_Y     (BLITZ_MENU_EMPTY3_Y + 1)
#define BLITZ_MENU_SCROLL2_H     (BLITZ_MENUPLASMA_HEIGHT - BLITZ_MENU_SCROLL2_Y)
                                                                
#define BLITZ_MENU_TEXT_XPAD     8  /* in bytes */
#define BLITZ_MENU_TEXT_YPAD     2

#define BLITZ_MENU_MAXSHIFTX    (BLITZ_MENU_XSPACING*BLITZ_MENU_WIDTH)

#define BLITZ_MENU_WAIT_FRAMES  60
#define BLITZ_MENU_BASSTREBLE_COUNT 30

#define BLITZ_GENCODE_BUFFERSIZE    0x2100




#ifdef __TOS__

typedef BlitZMenuASMimport* (*FuncBMpImport)(void);

static BlitZMenuASMimport* BMpImport(void)
{
    return ((FuncBMpImport)g_screens.menu->code)();
}

static void BlitZMenuDisplay(BlitZMenu* this, u16* _buffer, u16 _x1, u16 _x2, u16 _y1, u16 _y2)
{
    BlitZMenuASMimport* asmimport = this->asmimport;

    asmimport->p1plasma     = _buffer + _x1 + (this->plasmasin[_y1] >> 1);
    asmimport->p2plasma     = _buffer + _x2 + (this->plasmasin[_y2] >> 1);
    asmimport->plasmacurve1 = &this->plasmasindelta[_y1];
    asmimport->plasmacurve2 = &this->plasmasindelta[_y2];
    asmimport->rasters      = &this->flashcolors[_x1 & 31];

    if (asmimport->pixelOffsetIcons)
        asmimport->pitchIcons = (BLITZ_MENU_PITCH - 160 - 4) >> 1;
    else
        asmimport->pitchIcons = (BLITZ_MENU_PITCH - 160) / 2;

    if (asmimport->pixelOffsetTips)
        asmimport->pitchTips = (BLITZ_MENU_TIPS_PITCH - 160 - 4) >> 1;
    else
        asmimport->pitchTips = (BLITZ_MENU_TIPS_PITCH - 160) / 2;
}

#define BlitZMenuPCInit()

#else

/*---------------------------------------------------------------------------------
    PC routines
---------------------------------------------------------------------------------*/
static void* blitzMenuCopyIcons(void* _src, void* _dst, u16 _pitchdst)
{
    u16* s = (u16*) _src;
    u16* d = (u16*) _dst;
    u16 nbwords = *s++;
    u16 h = *s++;
    u16 t, i;


    nbwords = PCENDIANSWAP16(nbwords);
    h       = PCENDIANSWAP16(h);

    _pitchdst >>= 1;    
    _pitchdst -= nbwords << 1;

    for (t = 0; t < h; t++)
    {
        for (i = 0 ; i < nbwords ; i++)
        {
            *d++ = *s++;
            *d++ = *s++;
        }

        d += _pitchdst;
    }

    return s;
}

static void blitzMenuXor (void* _adr, u16 _nbwords, u16 _h, u16 _pitch)
{
    u16* p = (u16*) _adr;
    u16 t,i;

    _pitch >>= 1;
    _pitch -= _nbwords << 1;

    for (t = 0 ; t < _h ; t++)  
    {
        for (i = 0; i < _nbwords ; i++)
        {
            /**p = 0xffff;*/
            *p ^= p[1];
            p += 2;
        }
        p += _pitch;
    }
}

static void blitzMenuClear(void* _adr, u16 _nbwords, u16 _h, u16 _pitch)
{
    u16* p = (u16*)_adr;
    u16 t, i;

    _pitch >>= 1;
    _pitch -= _nbwords << 1;

    for (t = 0; t < _h; t++)
    {
        for (i = 0; i < _nbwords; i++)
        {
            *p = 0;
            p += 2;
        }
        p += _pitch;
    }
}


static BlitZMenuASMimport g_ASMimport;

static BlitZMenuASMimport* BMpImport(void)
{
    g_ASMimport.blitzMenuCopyIcons = blitzMenuCopyIcons;
    g_ASMimport.blitzMenuXor       = blitzMenuXor;
    g_ASMimport.blitzMenuClear     = blitzMenuClear;

    return &g_ASMimport;
}

static void BlitZMenuPCInit(void)
{   
    STDmset (g_ASMimport.opcodes, 0, sizeof(g_ASMimport.opcodes));

    g_ASMimport.opcodes[BMpOp_Begin                   ].cycles_div2 = 0  ;
    g_ASMimport.opcodes[BMpOp_End                     ].cycles_div2 = 0  ;
    g_ASMimport.opcodes[BMpOp_PreBlit1                ].cycles_div2 = 22 ;
    g_ASMimport.opcodes[BMpOp_PreBlit2                ].cycles_div2 = 22 ;
    g_ASMimport.opcodes[BMpOp_Blit                    ].cycles_div2 = 208;
    g_ASMimport.opcodes[BMpOp_C0_back                 ].cycles_div2 = 6  ;
    g_ASMimport.opcodes[BMpOp_C2_000                  ].cycles_div2 = 6  ;
    g_ASMimport.opcodes[BMpOp_C1_555                  ].cycles_div2 = 8  ;
    g_ASMimport.opcodes[BMpOp_C1_666                  ].cycles_div2 = 8  ;
    g_ASMimport.opcodes[BMpOp_C1_FFF                  ].cycles_div2 = 8  ;
    g_ASMimport.opcodes[BMpOp_C3_777                  ].cycles_div2 = 8  ;
    g_ASMimport.opcodes[BMpOp_C3_R                    ].cycles_div2 = 8  ;
    g_ASMimport.opcodes[BMpOp_Pitch160                ].cycles_div2 = 6  ;
    g_ASMimport.opcodes[BMpOp_PitchIcons              ].cycles_div2 = 10 ;
    g_ASMimport.opcodes[BMpOp_PitchTips               ].cycles_div2 = 10 ;
    g_ASMimport.opcodes[BMpOp_PixOffset0              ].cycles_div2 = 6  ;
    g_ASMimport.opcodes[BMpOp_PixOffsetIcons          ].cycles_div2 = 10 ;
    g_ASMimport.opcodes[BMpOp_PixOffsetTips           ].cycles_div2 = 10 ;
    g_ASMimport.opcodes[BMpOp_AdrEmpty                ].cycles_div2 = 12 ;
    g_ASMimport.opcodes[BMpOp_AdrScroller1            ].cycles_div2 = 12 ;
    g_ASMimport.opcodes[BMpOp_AdrScroller2            ].cycles_div2 = 12 ;
    g_ASMimport.opcodes[BMpOp_AdrScroller2Shift       ].cycles_div2 = 12 ;
    g_ASMimport.opcodes[BMpOp_AdrScrollerIcons        ].cycles_div2 = 12 ;
    g_ASMimport.opcodes[BMpOp_AdrScrollerTips         ].cycles_div2 = 12 ;
       
}

static void BlitZMenuDisplay(BlitZMenu* this, u16* _buffer, u16 _x1, u16 _x2, u16 _y1, u16 _y2)
{
    u16  x,y;
    u16* p  = _buffer + _x1 + (this->plasmasin[_y1] >> 1);
    u16* p2 = _buffer + _x2 + (this->plasmasin[_y2] >> 1);
    u16  flashindex = _x1 & 31;

    u16* sn  = &this->plasmasindelta[_y1];
    u16* sn2 = &this->plasmasindelta[_y2];


    ASSERT(_y1 < 512);
    ASSERT(_y2 < 512);
    
    EMULfbExStart(HW_VIDEO_MODE_2P, 64, 63, 64 + 319, 63 + BLITZ_MENUPLASMA_HEIGHT - 1 + 2, 160, 0);

    EMULfbExSetColor(EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, 63), 0, PCENDIANSWAP16(this->asmimport->color1));
    EMULfbExSetColor(EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, 65), 0, 0);

    for (y = 0 ; y < BLITZ_MENUPLASMA_HEIGHT ; y++)
    {
        u16* pp;
        u16* pp2;
        u32 cycle = EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0,  63 + y + 2);


        /* Change display address code */
        if ((y >= BLITZ_MENU_TIPS_Y) && (y < BLITZ_MENU_SCROLL2_Y))
        {
            EMULfbExSetColor(cycle, 2, 0);
            EMULfbExSetColor(cycle, 3, PCENDIANSWAP16(0x777));
        }
        else
        {
            u16 color = this->flashcolors[flashindex++];
            EMULfbExSetColor(cycle, 3, PCENDIANSWAP16(color));
        }

        if (y == 0)
        {
            EMULfbExSetWidth     (cycle, 64, 64 + 320, 160);
            EMULfbExSetPixOffset (cycle, 0);
            EMULfbExSetAdr       (cycle, (u32)(this->asmimport->scroller1));
            EMULfbExSetColor     (cycle, 1, PCENDIANSWAP16(0x555));
        }
        else if (y == 24)
        {
            EMULfbExSetColor     (cycle, 1, PCENDIANSWAP16(0x666));
        }
        else if ((y == BLITZ_MENU_EMPTY1_Y) || (y == BLITZ_MENU_EMPTY2_Y))
        {
            EMULfbExSetAdr       (cycle, (u32)this->empty);
        }
        else if (y == BLITZ_MENU_POS_Y)
        {            
            EMULfbExSetWidth     (cycle, 64, 64 + 320, BLITZ_MENU_PITCH);
            EMULfbExSetPixOffset (cycle, this->asmimport->pixelOffsetIcons);
            EMULfbExSetAdr       (cycle, (u32)this->asmimport->iconscroller);
            EMULfbExSetColor     (cycle, 1, PCENDIANSWAP16(0xFFF));
        }
        else if (y == BLITZ_MENU_TIPS_Y)
        {
            EMULfbExSetWidth     (cycle, 64, 64 + 320, BLITZ_MENU_TIPS_PITCH);
            EMULfbExSetPixOffset (cycle, this->asmimport->pixelOffsetTips);
            EMULfbExSetAdr       (cycle, (u32)this->asmimport->tipscroller);
        }
        else if (y == BLITZ_MENU_EMPTY3_Y)
        {
            EMULfbExSetWidth     (cycle, 64, 64 + 320, 160);
            EMULfbExSetPixOffset (cycle, 0);
            EMULfbExSetAdr       (cycle, (u32)this->empty);
        }
        else if (y == BLITZ_MENU_SCROLL2_Y)
        {
            EMULfbExSetAdr       (cycle, (u32)this->asmimport->scroller2shift);
        }
        else if (y == (BLITZ_MENU_SCROLL2_Y + BLITZ_MENU_YSPACING))
        {
            EMULfbExSetAdr       (cycle, (u32)this->asmimport->scroller2);
        }

        /* Plasma code */
        cycle += 32;

        for (x = 0; x < BLITZ_MENUPLASMA_WIDTH;  x++)
        {
            if (y & 1)
            {
                pp = p;
                EMULfbExSetColor(cycle, 0, PCENDIANSWAP16(*pp));
                p++;
            }
            else
            {
                pp2 = p2;
                EMULfbExSetColor(cycle, 0, PCENDIANSWAP16(*pp2));
                p2++;
            }

            cycle += 8;
        }

        if (y & 1)
        {
            p -= BLITZ_MENUPLASMA_WIDTH;
            p += *sn >> 1;
        }
        else
        {
            p2 -= BLITZ_MENUPLASMA_WIDTH;
            p2 += *sn2 >> 1;
        }

        sn++;
        sn2++;

        EMULfbExSetColor(cycle, 0, 0);

    }

    EMULfbExSetColor(EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, y + 2 + 63), 0, PCENDIANSWAP16(this->asmimport->color1));
    EMULfbExSetColor(EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, y + 2 + 65), 0, 0);

    EMULfbExEnd();
}

static blitztestsinus(u16 data1)
{
    s16 dx = 0;

    do
    {
        s16 lastX[2], lastY[2];
        s16 x;

        WINsetColor(EMULgetWindow(), 0, 0, 0);
        WINclear(EMULgetWindow());

        for (x = 0; x < 640; x++)
        {
            s16 snx[2]; 
            s16 sny[2];

            s16 y[2];
            s16 offx[2];


            snx[0] = g_screens.base.sin [(x + dx     + 228   ) & 511];
            snx[1] = g_screens.base.sin [(x + dx * 2 + 220   ) & 511];

            snx[0] = PCENDIANSWAP16(snx[0]);
            snx[1] = PCENDIANSWAP16(snx[1]);

            y[0] = (STDmuls(snx[0], 140 - data1) >> 16);
            y[0] += 256;

            y[1] = (STDmuls(snx[1], 140 - data1) >> 16);
            y[1] += 256;

            sny[0] = g_screens.base.sin [(y[0] + dx * 2) & 511];
            sny[1] = g_screens.base.sin [(y[1] + dx    ) & 511];

            sny[0] = PCENDIANSWAP16(sny[0]);
            sny[1] = PCENDIANSWAP16(sny[1]);

            offx[0] = (STDmuls(sny[0],  160) >> 16) + 160;
            offx[1] = (STDmuls(sny[1],  160) >> 16) + 160;

            if (x > 0)
            {
                WINsetColor(EMULgetWindow(), 0, 255, 0);
                WINline(EMULgetWindow(), lastX[0], lastY[0], x+offx[0], y[0]);

                WINsetColor(EMULgetWindow(), 255, 0, 0);
                WINline(EMULgetWindow(), lastX[1], lastY[1], x+offx[1], y[1]);
            }

            lastX[0] = x + offx[0];
            lastY[0] = y[0];

            lastX[1] = x + offx[1];
            lastY[1] = y[1];
        }

        dx = (dx + 1) & 511;

        WINsetColor  (EMULgetWindow(), 255, 255, 255);
        WINrectangle (EMULgetWindow(), 320, 256 - 100, 320 + 384, 256 + 100);

        WINrender ( EMULgetWindow() );

        EMULwait(20);
    }
    while(1);
}

#endif

/*---------------------------------------------------------------------------------
    Sub routines
---------------------------------------------------------------------------------*/

static void blitzMenuShadowPrint(BlitZMenu* this, char* _s, void* _screenprintadr, u16 _screenPitch, u16 _bitplanPitch)
{
    u8* adr = (u8*)_screenprintadr;

    _bitplanPitch--;

    while (*_s)
    {
        u8  c = *_s++;
        u8* d = adr;
        u16 index = SYSfont.charsmap[c];

        if (index != SYS_FNT_SPACECHAR) /* not space char */
        {
            u8* bitmap = this->charshadows + (index << SYS_FNT_OFFSETSHIFT);

            *d = *bitmap++;	d += _screenPitch;
            *d = *bitmap++;	d += _screenPitch;
            *d = *bitmap++;	d += _screenPitch;

            *d = 0xFF;	    d += _screenPitch;
            *d = 0xFF;	    d += _screenPitch;
            *d = 0xFF;	    d += _screenPitch;

            *d = *bitmap++;	d += _screenPitch;
            *d = *bitmap++;	d += _screenPitch;
            *d = *bitmap++;	d += _screenPitch;
            *d = *bitmap++;	d += _screenPitch;
        }
        else /* special mask for space char */
        {
            *d = 0;	    d += _screenPitch;
            *d = 0;	    d += _screenPitch;
            *d = 0;	    d += _screenPitch;
            *d = 0xFF;	d += _screenPitch;
            *d = 0xFF;	d += _screenPitch;
            *d = 0xFF;	d += _screenPitch;
            *d = 0;	    d += _screenPitch;
            *d = 0;	    d += _screenPitch;
            *d = 0;	    d += _screenPitch;
            *d = 0;	    d += _screenPitch;
        }

        if (1 & (u32)adr)
        {
            adr += _bitplanPitch;
        }
        else
        {
            adr++;
        }
    }
}

static s8 blitZMenuSetSelectYFromValidation(BlitZMenu* this, s16 _x)
{
    BlitZMenuCell* cell = &this->menubarstate.grid[_x][1];
    s8 i;

    if (_x != 0)
    {
        u8 h = this->gridheight[_x];

        for (i = 0 ; i < h ; i++, cell++)
        {            
            if (cell->validated)    /*if (this->menubarstate.grid[_x][i + 1].validated)*/
            {
                return i;
            }
        }
    }

    return 0;
}

/*---------------------------------------------------------------------------------
    Menu init 
---------------------------------------------------------------------------------*/
#define PLASMA_MOVEOFFSET   6

static void blitzMenuInitGenerateCode(BlitZMenu* this)
{
    CGENdesc* code = this->asmimport->opcodes;
    u8*  output;
    u16  cycles = 0;
    u16  targetcycles = sys.isMegaSTe ? 508 : 512; /* compensate blitter slowdown */
    u16  t;
    u16  nblines = 0;
    u16* patchoffset;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "blitzMenuInitGenerateCode", '\n');

    BlitZMenuPCInit();

    this->asmimport->waitloop  = 4;
    this->asmimport->waitloop2 = 70;

    output = (u8*)this->plasmacode = MEM_ALLOC(&sys.allocatorMem, BLITZ_GENCODE_BUFFERSIZE);

    CGENgenerateSimple(&code[BMpOp_Begin], output);
    /*CGENaddNops(0, 4, output);*/

    /* line 0 */
    CGENgenerate(&code[BMpOp_AdrScroller1], cycles, output);

    /*CGENgenerate(&code[BMpOp_C3_R]        , cycles, output); moved to begin */
    /*CGENgenerate(&code[BMpOp_C1_000]      , cycles, output); moved to begin */
    /*CGENgenerate(&code[BMpOp_C2_555]      , cycles, output); moved to begin */

    CGENgenerate(&code[BMpOp_C0_back], cycles, output); 

    patchoffset = (u16*) output;
    CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)], cycles, output);
    patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
    nblines++;

    CGENgenerate(&code[BMpOp_Blit], cycles, output);

    /* line 1 to 24 */
    for (t = 1; t < 24; t++)
    {
        cycles = code[BMpOp_Blit].cycles_div2 * 2;
        
        CGENgenerate(&code[BMpOp_C3_R], cycles, output);

        patchoffset = (u16*) output;
        CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)], cycles, output);
        patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
        nblines++;

        CGENaddNops(cycles, targetcycles, output);
        CGENgenerate(&code[BMpOp_Blit], cycles, output);
    }

    /* line 24 */
    cycles = code[BMpOp_Blit].cycles_div2 * 2;
    
    CGENgenerate(&code[BMpOp_C1_666], cycles, output);
    CGENgenerate(&code[BMpOp_C3_R]  , cycles, output);
    
    patchoffset = (u16*) output;
    CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)], cycles, output);
    patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
    nblines++;
    
    CGENaddNops(cycles, targetcycles, output);
    CGENgenerate(&code[BMpOp_Blit], cycles, output);

    /* line 25 -> BLITZ_MENU_EMPTY1_Y - 1 */
    for (t = 25; t < BLITZ_MENU_EMPTY1_Y; t++)
    {
        cycles = code[BMpOp_Blit].cycles_div2 * 2;

        CGENgenerate(&code[BMpOp_C3_R], cycles, output);
        
        patchoffset = (u16*) output;
        CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)], cycles, output);
        patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
        nblines++;

        CGENaddNops(cycles, targetcycles, output);
        CGENgenerate(&code[BMpOp_Blit], cycles, output);
    }

    /* line BLITZ_MENU_EMPTY1_Y (48) */
    cycles = code[BMpOp_Blit].cycles_div2 * 2;

    CGENgenerate(&code[BMpOp_AdrEmpty], cycles, output);

    patchoffset = (u16*) output;
    CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)]    , cycles, output);
    patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
    nblines++;
    
    CGENaddNops(cycles, targetcycles, output);

    CGENgenerate(&code[BMpOp_Blit], cycles, output);

    /* line BLITZ_MENU_EMPTY1_Y -> BLITZ_MENU_POS_Y - 2 (50) */
    for (t = BLITZ_MENU_EMPTY1_Y + 1; t < (BLITZ_MENU_POS_Y - 1); t++)
    {
        cycles = code[BMpOp_Blit].cycles_div2 * 2;

        patchoffset = (u16*) output;
        CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)], cycles, output);
        patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
        nblines++;

        CGENaddNops(cycles, targetcycles, output);
        CGENgenerate(&code[BMpOp_Blit], cycles, output);
    }

    /* BLITZ_MENU_POS_Y - 1 (51) => push pixoffset + pitch this earlier to minimize performance peak on next step */
    cycles = code[BMpOp_Blit].cycles_div2 * 2;

    CGENgenerate(&code[BMpOp_PixOffsetIcons]  , cycles, output);

    patchoffset = (u16*) output;
    CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)], cycles, output);
    patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
    nblines++;

    CGENgenerate(&code[BMpOp_PitchIcons], cycles, output);    

    CGENaddNops(cycles, targetcycles, output);
    CGENgenerate(&code[BMpOp_Blit], cycles, output);

    /* line BLITZ_MENU_POS_Y (52) */
    cycles = code[BMpOp_Blit].cycles_div2 * 2;

    CGENgenerate(&code[BMpOp_AdrScrollerIcons], cycles, output);
    CGENgenerate(&code[BMpOp_C3_R], cycles, output);

    patchoffset = (u16*) output;
    CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)]            , cycles, output);
    patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
    nblines++;

    CGENaddNops(cycles, targetcycles, output);
    CGENgenerate(&code[BMpOp_Blit], cycles, output);

    /* line BLITZ_MENU_POS_Y + 1 (53) */
    cycles = code[BMpOp_Blit].cycles_div2 * 2;
    CGENgenerate(&code[BMpOp_C3_R]            , cycles, output);
    CGENgenerate(&code[BMpOp_C1_FFF]          , cycles, output);

    patchoffset = (u16*) output;
    CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)], cycles, output);
    patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
    nblines++;

    CGENaddNops(cycles, targetcycles, output);
    CGENgenerate(&code[BMpOp_Blit], cycles, output);

    /* line BLITZ_MENU_POS_Y + 2 -> BLITZ_MENU_EMPTY2_Y - 1 */
    for (t = BLITZ_MENU_POS_Y + 2; t < BLITZ_MENU_EMPTY2_Y; t++)
    {
        cycles = code[BMpOp_Blit].cycles_div2 * 2;

        CGENgenerate(&code[BMpOp_C3_R], cycles, output);

        patchoffset = (u16*) output;
        CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)], cycles, output);
        patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
        nblines++;

        CGENaddNops(cycles, targetcycles, output);
        CGENgenerate(&code[BMpOp_Blit], cycles, output);
    }

    /* line BLITZ_MENU_EMPTY2_Y */
    cycles = code[BMpOp_Blit].cycles_div2 * 2;
    CGENgenerate(&code[BMpOp_AdrEmpty], cycles, output);

    patchoffset = (u16*) output;
    CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)]    , cycles, output);
    patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
    nblines++;

    CGENaddNops(cycles, targetcycles, output);

    CGENgenerate(&code[BMpOp_Blit], cycles, output);

    /* line BLITZ_MENU_EMPTY2_Y + 1 -> BLITZ_MENU_TIPS_Y - 2 */
    for (t = BLITZ_MENU_EMPTY2_Y + 1; t < (BLITZ_MENU_TIPS_Y - 1); t++)
    {
        cycles = code[BMpOp_Blit].cycles_div2 * 2;

        patchoffset = (u16*) output;
        CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)], cycles, output);
        patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
        nblines++;

        CGENaddNops(cycles, targetcycles, output);
        CGENgenerate(&code[BMpOp_Blit], cycles, output);
    }

    /* BLITZ_MENU_TIPS_Y - 2 */
    cycles = code[BMpOp_Blit].cycles_div2 * 2;

    CGENgenerate(&code[BMpOp_PixOffsetTips], cycles, output);

    patchoffset = (u16*) output;
    CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)], cycles, output);
    patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
    nblines++;

    CGENgenerate(&code[BMpOp_PitchTips], cycles, output);

    CGENaddNops(cycles, targetcycles, output);
    CGENgenerate(&code[BMpOp_Blit], cycles, output);

    /* line BLITZ_MENU_TIPS_Y */
    cycles = code[BMpOp_Blit].cycles_div2 * 2;

    CGENgenerate(&code[BMpOp_AdrScrollerTips], cycles, output);
    CGENgenerate(&code[BMpOp_C2_000]         , cycles, output);

    patchoffset = (u16*) output;
    CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)]           , cycles, output);
    patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
    nblines++;

    CGENaddNops(cycles , targetcycles, output);

    CGENgenerate(&code[BMpOp_Blit], cycles, output);

    /* line BLITZ_MENU_TIPS_Y + 1 */
    cycles = code[BMpOp_Blit].cycles_div2 * 2;

    CGENgenerate(&code[BMpOp_C3_777]         , cycles, output);

    patchoffset = (u16*) output;
    CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)]           , cycles, output);
    patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
    nblines++;

    CGENaddNops(cycles, targetcycles, output);
    CGENgenerate(&code[BMpOp_Blit], cycles, output);

    /* line BLITZ_MENU_TIPS_Y + 2 -> BLITZ_MENU_EMPTY3_Y - 1 */
    for (t = BLITZ_MENU_TIPS_Y + 2; t < BLITZ_MENU_EMPTY3_Y; t++)
    {
        cycles = code[BMpOp_Blit].cycles_div2 * 2;
        
        patchoffset = (u16*) output;
        CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)], cycles, output);
        patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
        nblines++;

        CGENaddNops(cycles, targetcycles, output);
        CGENgenerate(&code[BMpOp_Blit], cycles, output);
    }

    /* line BLITZ_MENU_EMPTY3_Y */
    cycles = code[BMpOp_Blit].cycles_div2 * 2;
    
    CGENgenerate(&code[BMpOp_PixOffset0]       , cycles, output);

    patchoffset = (u16*) output;
    CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)]             , cycles, output);
    patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
    nblines++;

    CGENgenerate(&code[BMpOp_AdrEmpty]         , cycles, output);     /* add this empty line to change pixoffset and pitch before next step to avoid performance peak */
    CGENgenerate(&code[BMpOp_Pitch160]         , cycles, output);

    CGENaddNops(cycles, targetcycles, output);   

    CGENgenerate(&code[BMpOp_Blit]             , cycles, output);

    /* line BLITZ_MENU_SCROLL2_Y */
    cycles = code[BMpOp_Blit].cycles_div2 * 2;
    
    CGENgenerate(&code[BMpOp_AdrScroller2Shift], cycles, output);
    CGENgenerate(&code[BMpOp_C3_R]             , cycles, output);
    
    patchoffset = (u16*) output;
    CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)] , cycles, output);
    patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
    nblines++;

    CGENaddNops(cycles, targetcycles, output);

    CGENgenerate(&code[BMpOp_Blit], cycles, output);

    /* line BLITZ_MENU_SCROLL2_Y + 1 -> line BLITZ_MENU_SCROLL2_Y + BLITZ_MENU_YSPACING - 1 */
    for (t = BLITZ_MENU_SCROLL2_Y + 1; t < (BLITZ_MENU_SCROLL2_Y + BLITZ_MENU_YSPACING); t++)
    {
        cycles = code[BMpOp_Blit].cycles_div2 * 2;

        CGENgenerate(&code[BMpOp_C3_R], cycles, output);

        patchoffset = (u16*) output;
        CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)], cycles, output);
        patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
        nblines++;

        CGENaddNops(cycles, targetcycles, output);
        CGENgenerate(&code[BMpOp_Blit], cycles, output);
    }

    /* line BLITZ_MENU_SCROLL2_Y + BLITZ_MENU_YSPACING */
    cycles = code[BMpOp_Blit].cycles_div2 * 2;
    
    CGENgenerate(&code[BMpOp_AdrScroller2], cycles, output);
    CGENgenerate(&code[BMpOp_C3_R], cycles, output);

    patchoffset = (u16*) output;
    CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)], cycles, output);
    patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
    nblines++;

    CGENaddNops(cycles, targetcycles, output);

    CGENgenerate(&code[BMpOp_Blit], cycles, output);

    /* line BLITZ_MENU_SCROLL2_Y + BLITZ_MENU_YSPACING + 1 -> BLITZ_MENUPLASMA_HEIGHT + 1 */
    for (t = BLITZ_MENU_SCROLL2_Y + BLITZ_MENU_YSPACING + 1; t < BLITZ_MENUPLASMA_HEIGHT ; t++)
    {
        cycles = code[BMpOp_Blit].cycles_div2 * 2;
        
        CGENgenerate(&code[BMpOp_C3_R], cycles, output);
        
        patchoffset = (u16*) output;
        CGENgenerate(&code[BMpOp_PreBlit1 + (nblines & 1)], cycles, output);
        patchoffset[PLASMA_MOVEOFFSET] = nblines << 1;
        nblines++;

        CGENaddNops(cycles, targetcycles, output);
        CGENgenerate(&code[BMpOp_Blit], cycles, output);
    }

    CGENgenerateSimple(&code[BMpOp_End], output);

    TRAClogNumber(TRAC_LOG_FLOW, "gencodelen", (u8*)output-(u8*)this->plasmacode, 6);
    ASSERT((u8*)output <= (u8*)this->plasmacode+BLITZ_GENCODE_BUFFERSIZE);
}

static void blitzMenuInitPlasmaCurve(BlitZMenu* this)
{
    u16 size = 512 + BLITZ_MENUPLASMA_HEIGHT + 1;
    u16 y;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "blitzMenuInitPlasmaCurve", '\n');


    /*int min = 65536;
    int max = -65536;*/
    
    this->plasmasin       = (u16*) MEM_ALLOC(&sys.allocatorMem, size << 1);
    this->plasmasindelta  = (u16*) MEM_ALLOC(&sys.allocatorMem, size << 1);

    for (y = 0; y < size; y++)
    {
        s16 sn = g_screens.base.sin[(y + 228) & 511];   /* 228 to phase with x sine the best way */
        u16 offset;

        sn = PCENDIANSWAP16(sn);
        offset = ((STDmuls(sn, 160) >> 16) + 80) << 1; /* in bytes */
        this->plasmasin[y] = offset;

        /*if (offset < min)
            min = offset;

        if (offset > max)
            max = offset;*/
    }

    for (y = 2 ; y < size; y++)
    {
        this->plasmasindelta[y-2] = this->plasmasin[y] - this->plasmasin[y-2] + BLITZ_MENUPLASMA_PITCH;
    }

    /* printf("min = %d ; max = %d\n", min, max); */
}

static void blitzMenuInitFlashColors(BlitZMenu* this)
{
    u16 nbcolors = (BLITZ_MENUPLASMA_HEIGHT - BLITZ_MENU_TIPS_H) + 31;
    u16 i;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "blitzMenuInitFlashColors", '\n');

    this->flashcolors = (u16*) MEM_ALLOC (&sys.allocatorMem, nbcolors * sizeof(u16));

    for (i = 0; i < nbcolors; i++)
    {
        u16 v = i & 31;
        u8  stval;

        if (v > 16)
        {
            v = 31 - v;
        }

        stval = COL4b2ST[v];
        this->flashcolors[i] = (stval << 8) | (stval << 4);
    }
}

static char blitzBassTreble[] = " Bass     Treble   ";


static void blitzUpdateBassTreble(BlitZMenu* this)
{
    s8 level = g_screens.bass - 6;

    if (level < 0)
    {
        blitzBassTreble[6] = '-';
        blitzBassTreble[7] = '0' - level;
    }
    else
    {
        blitzBassTreble[6] = '+';
        blitzBassTreble[7] = '0' + level;
    }

    level = g_screens.treble - 6;

    if (level < 0)
    {
        blitzBassTreble[17] = '-';
        blitzBassTreble[18] = '0' - level;
    }
    else
    {
        blitzBassTreble[17] = '+';
        blitzBassTreble[18] = '0' + level;
    }

    SYSfastPrint(blitzBassTreble, this->basstreblebuffer + BLITZ_MENU_TIPS_PITCH * 2 - 40 , BLITZ_MENU_TIPS_PITCH, 4, (u32)&SYSfont);
}

static void blitzMenuInitTips(BlitZMenu* this)
{
    u16 size;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "blitzMenuInitTips", '\n');

    this->tipscrollerbuffersize = size = (BLITZ_MENU_TIPS_H * BLITZ_MENU_TIPS_PITCH) + 160;
    this->tipscrollerbuffer = (u8*)MEM_ALLOC(&sys.allocatorMem, size);
    STDfastmset(this->tipscrollerbuffer, 0, size);

    this->basstreblebuffer  = (u8*)MEM_ALLOC(&sys.allocatorMem, size);
    STDfastmset(this->basstreblebuffer , 0, size);
    
    blitzUpdateBassTreble(this);

    {
        u16* d = (u16*)(this->basstreblebuffer + BLITZ_MENU_TIPS_PITCH - 40 + 2);
        u16 t, i;
        
        for (t = 0 ; t < 9 ; t++)
        {
            for (i = 0 ; i < 10 ; i++)
            {
                *d = 0xFFFF;
                d += 2;
            }

            d += (BLITZ_MENU_TIPS_PITCH - 40) / 2;
        }
    }

    {
        u16 c;
        u8* s = SYSfont.data;
        u8* d;
        u16 nbchars = (SYS_FNT_UNDEFINED / 2) + 1;

        d = this->charshadows = (u8*)MEM_ALLOC(&sys.allocatorMem, nbchars * 8);

        STDfastmset(d, 0, nbchars * 8);

        for (c = 0; c < nbchars; c++)
        {
            *d++ = (s[0] != 0) ? 0xFF : 0;
            *d++ = ((s[0] != 0) || (s[1] != 0)) ? 0xFF : 0;
            *d++ = ((s[0] != 0) || (s[1] != 0) || (s[2] != 0)) ? 0xFF : 0;

            *d++ = ((s[4] != 0) || (s[5] != 0) || (s[6] != 0)) ? 0xFF : 0;
            *d++ = ((s[5] != 0) || (s[6] != 0) || (s[7] != 0)) ? 0xFF : 0;
            *d++ = ((s[6] != 0) || (s[7] != 0)) ? 0xFF : 0;
            *d++ = (s[7] != 0) ? 0xFF : 0;

            s += 8;
            d++;
        }
    }
}

static void blitzMenuInitScroller(BlitZMenu* this)
{
    /* -------------------------------          -------------------------------     <-- scroll buffer 1
       48 - MENU BAR YPOS                       48 - SCROLLER1 H                                                
       -------------------------------          -------------------------------
       4  - EMPTY1 Y                            TEXT 1
       -------------------------------          TEXT 2
       32 - MENU BAR Y                          TEXT 3       
       -------------------------------          -------------------------------
       4  - EMPTY2 Y                            199 - 50 - 32                67
       -------------------------------                                       50     <-- scroll buffer 2
       8  - SCROLLER TIP Y                      -------------------------------
       -------------------------------          TEXT 1
       4  - EMPTY3 Y                            TEXT 2
       -------------------------------          TEXT 3
       199 - 48 - 4 - 32 - 4 - 8 - 4            -------------------------------
            SCROLLER                            199 - 50 - 32                67
                                                                             50     <-- scroll buffer 3
       -------------------------------          -------------------------------
                                                TEXT 1
                                                TEXT 2
                                                TEXT 3
                                                -------------------------------
                                                199 - 50 - 32                67     <-- scroll buffer end
                                                                             50
                                                ------------------------------- */

    u16 t;
    u32 size;
    u8* p;
    u8  totalheight = 0;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "blitzMenuInitScroller", '\n');

    size = LOADmetadataSize(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_MENUS_MENULIST_BIN);

    p = this->plasma + LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_MENUS_MENULIST_BIN);

    for (t = 0 ; t < size ; t++)
    {
        ASSERT(*p < BLITZ_MENU_HEIGHT);
        totalheight += *p;
        this->gridheight[t+1] = *p++;
    }

    size  = BLITZ_MENU_SCROLL1_H;
    size += BLITZ_MENU_SCROLL2_H * 3;
    size += BLITZ_MENU_YSPACING * totalheight;
    size ++; /* patch because we need 200 lines on pc */
    size += BLITZ_MENU_SCROLL2_H; /* info screen */
    size  = (size << 7) + (size << 5); /* *160 */

    this->gridscrollerbuffersize = size;
    this->gridscrollerbuffer = (u8*) MEM_ALLOC(&sys.allocatorMem, size);
    ASSERT(this->gridscrollerbuffer != NULL);
    STDfastmset(this->gridscrollerbuffer, 0, size);

    size -= BLITZ_MENU_SCROLL2_H * 160;
    this->gridscrollers[0] = this->gridscrollerbuffer + size;

    size -= BLITZ_MENU_SCROLL2_H * 160;
    this->empty = this->gridscrollerbuffer + size;

    this->empty += 160;
    this->empty = (u8*)(((u32)this->empty + 255) & 0xFFFFFF00UL);
    this->empty -= 160;

    p = this->gridscrollerbuffer;

    {
        for (t = 0; t < BLITZ_MENU_WIDTH; t++)
        {
            if (this->gridheight[t] == 0)
            {
                if (t > 0)
                {
                    this->gridscrollers[t] = this->empty;
                }
            }
            else
            {
                u8* packeddata = this->plasma + LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_MENUS_MENUZIK0_ARJX + t - 1);
                u8* src = this->tempunpacked;
                u8* dst = p + (BLITZ_MENU_SCROLL1_H + BLITZ_MENU_TEXT_YPAD) * 160 + BLITZ_MENU_TEXT_XPAD;
                u16 i;

                
                ARJdepack(src, packeddata);

                this->gridscrollers[t] = p;

                size = BLITZ_MENU_SCROLL1_H;
                size += (this->gridheight[t] * BLITZ_MENU_YSPACING);
                size += BLITZ_MENU_SCROLL2_H - BLITZ_MENU_SCROLL1_H;
                size = (size << 7) + (size << 5); /* *160 */

                STDfastmset(p, 0, size);

                for (i = 0; i < this->gridheight[t]; i++)
                {
                    BlitZMenuCell* cell = &this->menubarstate.grid[t][i+1];

                    cell->nbwords    = (u8) PCENDIANSWAP16(*(u16*)src);
                    cell->h          = BLITZ_MENU_YSPACING;
                    cell->adr        = dst;
                    cell->pitch_div2 = 160 / 2;

                    src = this->asmimport->blitzMenuCopyIcons(src, dst, 160);
                    dst += BLITZ_MENU_YSPACING * 160;
                }

                p += size;

                /*STDmset(p - 160, -1, 160);*/
            }
        }
    }

    {
        u8* packeddata = this->plasma + LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_MENUS_INFOS_ARJX);
        u8* src        = this->tempunpacked;

        ARJdepack (src, packeddata);
        this->asmimport->blitzMenuCopyIcons(src, this->gridscrollers[0], 160);
    }

    this->asmimport->empty =
        this->asmimport->scroller1 = 
        this->asmimport->scroller2 = 
        this->asmimport->scroller2shift = 
        this->empty;
}

static void blitzMenuInitIcons(BlitZMenu* this)
{
    u16   t;
    void* packeddata = this->plasma + LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_MENUS_ICONS_ARJX);
    void* src        = this->tempunpacked;
    u8*   dst        = this->iconscrollerbuffer;

    
    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "blitzMenuInitIcons", '\n');

    ARJdepack(src, packeddata);

    dst += 8;

    for (t = 0; t < (BLITZ_MENU_WIDTH - 1); t++)
    {
        BlitZMenuCell* cell = &this->menubarstate.grid[t][0];
        
        u16 nbwords = ((u16*)src)[0];
        u16 h       = ((u16*)src)[1];

        cell->nbwords   = (u8) PCENDIANSWAP16(nbwords);
        cell->h         = (u8) PCENDIANSWAP16(h);
        cell->pitch_div2= BLITZ_MENU_PITCH / 2;
        cell->adr       = dst;

        src = this->asmimport->blitzMenuCopyIcons(src, dst, BLITZ_MENU_PITCH); 
        dst += BLITZ_MENU_XSPACING / 4;
    }

    this->asmimport->iconscroller = this->iconscrollerbuffer + BLITZ_MENU_PITCH - 160;
}


static void blitzMenuInitTipsText(BlitZMenu* this)
{
    u8 i, t, j = 0;
    u8* src;
    u16 tipsstart = (u16) LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_MENUS_MENUZIK0_TXT);
    u16 tipssize  = (u16) LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_MENUS_AUTORUN_BIN) - tipsstart;

    
    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "blitzMenuInitTipsText", '\n');

    this->tipstext = (u8*) MEM_ALLOC(&sys.allocatorMem, tipssize);
    STDmcpy(this->tipstext, this->plasma + tipsstart, tipssize);

    for (t = 0; t < BLITZ_MENU_WIDTH; t++)
    {
        if (this->gridheight[t] > 0)
        {
            src = (u8*)this->tipstext;
            src += LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_MENUS_MENUZIK0_TXT + j) - tipsstart;
            j++;

            for (i = 0; i < this->gridheight[t]; i++)
            {
                BlitZMenuCell* cell = &this->menubarstate.grid[t][i + 1];

                cell->tipslen = *src++;

                if (cell->tipslen > 0)
                {
                    cell->tips = src;
                    src += cell->tipslen + 1;
                }
            }
        }
    }
    
    this->asmimport->tipscroller = this->tipscrollerbuffer;
    this->asmimport->pixelOffsetTips = 0;
}

void blitzMenuInitAutomaton(BlitZMenu* this)
{
    this->automatonsize = (u16) LOADmetadataSize(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_MENUS_AUTORUN_BIN + g_screens.compomode);
    this->automaton     = MEM_ALLOC(&sys.allocatorMem, this->automatonsize);   

    STDmcpy2(this->automaton, this->plasma + LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_MENUS_AUTORUN_BIN + g_screens.compomode), this->automatonsize);
}


static void blitzMenuInitGrid(BlitZMenu* this)
{
    BlitZMenuCell* cell;
    u8 t;


    /* default state */
    this->menubarstate.grid[g_screens.persistent.menu.selectx][0].validated = true;

    this->menubarstate.grid[0][1].action = BZMA_SELECT_INFO;

    cell = &this->menubarstate.grid[1][1];

    for (t = 0 ; t < this->gridheight[1] ; t++, cell++)
    {
        cell->action    = BZMA_SELECT_MUSIK;
        cell->param     = t;
        cell->validated = t == g_screens.persistent.menu.currentmodule;       
    }

    cell = &this->menubarstate.grid[2][1];

    for (t = 0 ; t < this->gridheight[2] ; t++, cell++)
    {
        cell->action    = BZMA_SELECT_PLAYMODE;
        cell->param     = t;
        cell->validated = t == g_screens.persistent.menu.playmode;
    }

    cell = &this->menubarstate.grid[3][1];
    
    for (t = 0 ; t < this->gridheight[3] ; t++, cell++)
    {
        cell->action    = BZMA_SELECT_COLORS;
        cell->param     = t;
        cell->validated = t == g_screens.persistent.menu.colorchoice;
    }
}


void BlitZMenuEnter (FSM* _fsm)
{
    BlitZMenu* this;
    LOADrequest* loadRequest, *loadRequest2;
    u32 plasmabuffersize;
    u16 colorssize = (u16) LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_MENUS_MENUZIK0_TXT);
    u8* temppacked2 = NULL;

    /*blitztestsinus(nbcolors[1]);*/

    STDmset(HW_COLOR_LUT, 0, 32);

    BlitZsetVideoMode(HW_VIDEO_MODE_2P, 0, BLITZ_VIDEO_NOXTRA_PIXEL);
    SYSvsync; /* be sure load interupt will not postpone this request at next VBL */

    this = g_screens.menu = MEM_ALLOC_STRUCT (&sys.allocatorMem, BlitZMenu);
    DEFAULT_CONSTRUCT(this);
      
    this->displayedcolor = -1;
    this->iconscrollerbuffersize = BLITZ_MENU_PITCH * BLITZ_MENU_H + 160;
    this->iconscrollerbuffer = (u8*) MEM_ALLOC(&sys.allocatorMem, this->iconscrollerbuffersize);
    this->colors = (u16*) MEM_ALLOC(&sys.allocatorMem, colorssize);

    plasmabuffersize = (u32)BLITZ_MENUPLASMA_PITCH * (u32)BLITZ_MENUPLASMA_BUFFERHEIGHT;
    this->plasma = (u8*) MEM_ALLOC(&sys.allocatorMem, plasmabuffersize);

    /* use plasma buffer as temporary buffer for compressed data */
    temppacked2 = this->plasma + 100000UL;
    loadRequest = LOADdata (&RSC_BLITZWAV, RSC_BLITZWAV_______OUTPUT_BLITZIK_MENU_ARJX, temppacked2, LOAD_PRIORITY_INORDER);
    ASSERT((LOADresourceRoundedSize(&RSC_BLITZWAV, RSC_BLITZWAV_______OUTPUT_BLITZIK_MENU_ARJX) + 100000UL) <= plasmabuffersize);

    ASSERT(LOADresourceRoundedSize(&RSC_BLITZWAV, RSC_BLITZWAV_MENUS_MENUS_BIN) <= 100000UL);
    loadRequest2 = LOADdata(&RSC_BLITZWAV, RSC_BLITZWAV_MENUS_MENUS_BIN, this->plasma, LOAD_PRIORITY_INORDER);
    this->tempunpacked = this->plasma + LOADresourceRoundedSize(&RSC_BLITZWAV, RSC_BLITZWAV_MENUS_MENUS_BIN);
    ASSERT((this->tempunpacked + 50000UL) < temppacked2);
     
    STDfastmset(this->iconscrollerbuffer, 0, BLITZ_MENU_PITCH * BLITZ_MENU_H + 160);

    blitzMenuInitFlashColors(this);

    blitzMenuInitTips(this);

    blitzMenuInitPlasmaCurve(this);

    LOADwaitRequestCompleted(loadRequest);

    this->code = MEM_ALLOC(&sys.allocatorMem, LOADmetadataOriginalSize(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_______OUTPUT_BLITZIK_MENU_ARJX));
    ARJdepack(this->code, temppacked2);
    SYSrelocate(this->code);    
    this->asmimport = BMpImport();

    LOADwaitRequestCompleted(loadRequest2);

    STDmcpy2(this->colors, this->plasma, colorssize);

    blitzMenuInitAutomaton(this);

    blitzMenuInitGenerateCode(this);

    blitzMenuInitIcons(this);

    blitzMenuInitScroller(this);

    blitzMenuInitGrid(this);

    blitzMenuInitTipsText(this);

    /*blitzMenuInitPlasmaBuffer(g_screens.menucolorchoice);*/
    
    /* Default state init */
    this->interactive = true;

    this->menubarstate.selecty = blitZMenuSetSelectYFromValidation(this, g_screens.persistent.menu.selectx);

    this->menubarstate.backtaskGridx = BLITZ_MENU_WIDTH - 1;
    this->menubarstate.bardisplayy = -BLITZ_MENU_SCROLL1_H;

    this->menubarstate.bardisplayx = (BLITZ_MENU_WIDTH - 1) * BLITZ_MENU_XSPACING; /* last icon does not exist */

    SYSwriteVideoBase((u32)this->empty);

    STDfastmset(this->plasma, 0, plasmabuffersize);

#   if DEMOS_MEMDEBUG
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif

    /* init rasters system */
    this->rasterBootFunc                      = RASvbl1;
    this->rasterBootOp.scanLinesTo1stInterupt = 1;
    this->rasterBootOp.backgroundColor        = 0;
    this->rasterBootOp.nextRasterRoutine      = this->plasmacode;

    BlitZMenuDisplay(this, (u16*)this->plasma, 0, 0, 0, 0);

    SYSvsync;
   
    *HW_VIDEO_PIXOFFSET = 0;
    *HW_VIDEO_OFFSET = 0;

    RASnextOpList     = &this->rasterBootOp;
    SYSvblroutines[1] = this->rasterBootFunc;

    g_screens.runningphase = BLZ_PHASE_MENU;

#   ifndef __TOS__
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRAClog(TRAC_LOG_MEMORY, "Menu memallocator dump", '\n');
        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif
        
    BLZ_FLUSH_COMMANDS;

    FSMgotoNextState (_fsm);
    FSMgotoNextState (&g_stateMachine);
}

/*---------------------------------------------------------------------------------
    Menu run
---------------------------------------------------------------------------------*/

static u8 movecurve1[] = {1, 2, 3, 5, 8, 14};
static u8 movecurve2[] = {1, 2, 3, 5, 8};
static u8 movecurve3[] = {1, 2, 3, 5, 8, 14, 24, 40};

void BlitZMenuActivity (FSM* _fsm)
{
    BlitZMenuPersistentData* thistatic = &g_screens.persistent.menu;
    BlitZMenu*          this         = g_screens.menu;
    BlitZMenuBarState*  menubarstate = &this->menubarstate;
    BlitZMenuASMimport* asmimport    = this->asmimport;

    s16 selectx = g_screens.persistent.menu.selectx;
    s8  selecty = menubarstate->selecty;
    u8  cmd     = 0xFF;


    IGNORE_PARAM(_fsm);

    BLITZ_MENU_RASTERIZE(0x400);

    /* Manage horizontal scroller */
    if (BLZ_COMMAND_AVAILABLE)
    {
        cmd = BLZ_CURRENT_COMMAND;
        BLZ_ITERATE_COMMAND;
    }

    if ((g_screens.persistent.menu.playmode == BLZ_PLAY_AUTORUN) || g_screens.compomode)
    {
        if ((g_screens.compomode == false) && (cmd == BLZ_CMD_SELECT))
        {
            g_screens.persistent.menu.playmode = BLZ_PLAY_SCOREDRIVEN;
            menubarstate->grid[2][1].validated = true;
            menubarstate->grid[2][4].validated = false;
            selectx = 2;
            selecty = 0;
            cmd = 0xFF;
        }
        else
        {
            cmd = 0xFF;
            g_screens.persistent.menu.autorunframes++;
            if (g_screens.persistent.menu.autorunframes >= this->automaton[g_screens.persistent.menu.autorunindex])
            {
                if (g_screens.persistent.menu.autorunindex >= this->automatonsize)
                {
                    g_screens.persistent.menu.playmode = BLZ_PLAY_SCOREDRIVEN;
                    menubarstate->grid[2][1].validated = true;
                    menubarstate->grid[2][4].validated = false;
                    selectx = 2;
                    selecty = 0;
                }
                else
                {
                    cmd = this->automaton[g_screens.persistent.menu.autorunindex + 1];
                    g_screens.persistent.menu.autorunindex += 2;
                    g_screens.persistent.menu.autorunframes = 0;
                }
            }
        }
    }

    if (cmd == BLZ_CMD_LEFT)
    {
        selectx--;
        if (selectx < 0)
        {
            selectx = 0;
        }
        else
        {
            menubarstate->grid[selectx + 1][0].validated = false;
            menubarstate->grid[selectx][0].validated = true;
            menubarstate->bardisplayy = -BLITZ_MENU_SCROLL1_H;
        }

        selecty = blitZMenuSetSelectYFromValidation(this, selectx);

        menubarstate->waitcounter = 0;
        menubarstate->tipid = 0x00FF;
    }

    else if (cmd == BLZ_CMD_RIGHT)
    {
        selectx++;
        if (selectx >= BLITZ_MENU_WIDTH)
            selectx = BLITZ_MENU_WIDTH - 1;
        else
        {
            menubarstate->grid[selectx - 1][0].validated = false;
            menubarstate->grid[selectx][0].validated = true;
            menubarstate->bardisplayy = -BLITZ_MENU_SCROLL1_H;
        }

        selecty = blitZMenuSetSelectYFromValidation(this, selectx);

        menubarstate->waitcounter = 0;
        menubarstate->tipid = 0x00FF;
    }

    {
        s16 targetx = selectx * BLITZ_MENU_XSPACING;

        if (targetx != menubarstate->bardisplayx)
        {
            s16 speed = targetx - menubarstate->bardisplayx;

            if (speed < 0)
            {
                speed = -speed;
                speed >>= 3;
                if (speed >= ARRAYSIZE(movecurve1))
                    speed = ARRAYSIZE(movecurve1) - 1;
                speed = movecurve1[speed];
                menubarstate->bardisplayx -= speed;
            }
            else
            {
                speed >>= 3;
                if (speed >= ARRAYSIZE(movecurve1))
                    speed = ARRAYSIZE(movecurve1) - 1;
                speed = movecurve1[speed];
                menubarstate->bardisplayx += speed;
            }
        }

        {
            s16 offsetx = menubarstate->bardisplayx;

            asmimport->iconscroller = this->iconscrollerbuffer + ((offsetx >> 2) & 0xFFFC);
            asmimport->pixelOffsetIcons = offsetx & 15;
        }
    }

    /* Manage vertical scroller */
    {
        s8 gridheight = this->gridheight[selectx];


        if (gridheight > 0)
        {
            if (cmd == BLZ_CMD_UP)
            {
                selecty--;
                if (selecty < 0)
                {
                    selecty = 0;
                }
                menubarstate->waitcounter = 0;
                menubarstate->tipid = 0x00FF;
            }

            else if (cmd == BLZ_CMD_DOWN)
            {
                selecty++;
                if (selecty >= gridheight)
                {
                    selecty = gridheight - 1;
                }
                menubarstate->waitcounter = 0;
                menubarstate->tipid = 0x00FF;
            }
        }

        {
            s16 targety = selecty * BLITZ_MENU_YSPACING;

            if (targety != menubarstate->bardisplayy)
            {
                s16 speed = targety - menubarstate->bardisplayy;

                if (speed < 0)
                {
                    speed = -speed;
                    speed >>= 3;
                    if (speed >= ARRAYSIZE(movecurve2))
                        speed = ARRAYSIZE(movecurve2) - 1;
                    speed = movecurve2[speed];
                    menubarstate->bardisplayy -= speed;
                }
                else
                {
                    speed >>= 3;
                    if (speed >= ARRAYSIZE(movecurve2))
                        speed = ARRAYSIZE(movecurve2) - 1;
                    speed = movecurve2[speed];
                    menubarstate->bardisplayy += speed;
                }
            }
        }

        if (gridheight > 0)
        {
            if (selecty < 0)
            {
                asmimport->scroller1 = this->empty;
                asmimport->scroller2 = this->empty;
                asmimport->scroller2shift = this->empty;
            }
            else
            {
                u32 offset = menubarstate->bardisplayy + BLITZ_MENU_SCROLL1_H;
                offset = (offset << 7) + (offset << 5); /* *160 */
                asmimport->scroller2 = this->gridscrollers[selectx] + offset;

                if (menubarstate->bardisplayy >= 0)
                    asmimport->scroller1 = asmimport->scroller2 - (BLITZ_MENU_SCROLL1_H * 160);
                else
                    asmimport->scroller1 = this->gridscrollers[selectx];

                asmimport->scroller2shift = asmimport->scroller2 + 8;
                asmimport->scroller2 += (BLITZ_MENU_YSPACING * 160);
            }

        }
        else
        {
            asmimport->scroller1 = this->empty;
            if (selectx == 0)
            {
                u32 offset = menubarstate->bardisplayy;
                offset = (offset << 7) + (offset << 5); /* *160 */

                asmimport->scroller2 = this->gridscrollers[0] + offset;
                asmimport->scroller2shift = asmimport->scroller2;
                asmimport->scroller2 += (BLITZ_MENU_YSPACING * 160);
            }
            else
            {
                asmimport->scroller2 = this->empty;
                asmimport->scroller2shift = this->empty;
            }
        }
    }

    menubarstate->waitcounter++;
    if (menubarstate->waitcounter >= BLITZ_MENU_WAIT_FRAMES)
    {
        menubarstate->tipid = (selectx << 8) | (u8)selecty;
    }

    {
        s16 tipsdisplayx = menubarstate->tipsdisplayx;

        if (menubarstate->tipstargetx != tipsdisplayx)
        {
            s16 speed = menubarstate->tipstargetx - tipsdisplayx;

            if (speed < 0)
            {
                speed = -speed;
                speed >>= 3;
                if (speed >= ARRAYSIZE(movecurve3))
                    speed = ARRAYSIZE(movecurve3) - 1;
                speed = movecurve3[speed];
                tipsdisplayx -= speed;
            }
            else
            {
                speed >>= 3;
                if (speed >= ARRAYSIZE(movecurve3))
                    speed = ARRAYSIZE(movecurve3) - 1;
                speed = movecurve3[speed];
                tipsdisplayx += speed;
            }
        }

        if (this->updatebasstreble > 0)
        {
            asmimport->tipscroller = this->basstreblebuffer + 160;
            asmimport->pixelOffsetTips = 0;

            if (this->updatebasstreble < BLITZ_MENU_BASSTREBLE_COUNT)
                this->updatebasstreble--;
        }
        else
        {
            asmimport->tipscroller = this->tipscrollerbuffer + ((tipsdisplayx >> 2) & 0xFFFC);
            asmimport->pixelOffsetTips = tipsdisplayx & 15;
        }

        menubarstate->tipsdisplayx = tipsdisplayx;
    }

    g_screens.persistent.menu.selectx = (s8)selectx;
    menubarstate->selecty = selecty;

    /*BlitZMenuDisplay((u16*)this->plasma, 0, 0, 0, 0);*/

    BlitZMenuDisplay(
        this,
        (u16*)this->plasma,
        thistatic->plasx,
        thistatic->plasmashiftmode ? thistatic->plasx2 : thistatic->plasx,
        thistatic->plasx << 1,
        thistatic->plasy2);

    /* Manage selection */
    if (cmd == BLZ_CMD_SELECT)
    {
        BlitZMenuCell* cell = &menubarstate->grid[selectx][1];
        s8 h = this->gridheight[selectx] - 1;
        s8 i;

        for (i = 0; i <= h; i++, cell++)
        {
            cell->validated = (i == selecty);
        }

        cell = &menubarstate->grid[selectx][selecty + 1];

        switch (cell->action)
        {
        case BZMA_SELECT_INFO:
            FSMgotoNextState(&g_stateMachineIdle);  /* goto exiting   */
            FSMgotoNextState(_fsm);                 /* goto donothing */
            break;

        case BZMA_SELECT_MUSIK:
            g_screens.persistent.menu.currentmodule = cell->param;
            this->interactive = false;
            selecty = this->gridheight[1]; /* for scroll leave animation */
            FSMgotoNextState(&g_stateMachineIdle);  /* goto exiting   */
            FSMgotoNextState(_fsm);                 /* goto donothing */
            break;

        case BZMA_SELECT_PLAYMODE:
            g_screens.persistent.menu.playmode = cell->param;
            if (g_screens.persistent.menu.playmode == BLZ_PLAY_AUTORUN)
            {
                g_screens.persistent.menu.autorunindex  = 0;
                g_screens.persistent.menu.autorunframes = 0;
            }
            break;

        case BZMA_SELECT_COLORS:
            g_screens.persistent.menu.colorchoice = cell->param;
            break;
        }
    }
    else if (cmd == BLZ_CMD_BASS_MINUS)
    {
        if (g_screens.bass > 0)
        {
            g_screens.bass--;
            *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME_BASS | g_screens.bass;
        }
        this->updatebasstreble = BLITZ_MENU_BASSTREBLE_COUNT;
    }
    else if (cmd == BLZ_CMD_BASS_PLUS)
    {
        if (g_screens.bass < 12)
        {
            g_screens.bass++;
            *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME_BASS | g_screens.bass;
        }
        this->updatebasstreble = BLITZ_MENU_BASSTREBLE_COUNT;
    }
    else if (cmd == BLZ_CMD_TREBLE_MINUS)
    {
        if (g_screens.treble > 0)
        {
            g_screens.treble--;
            *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME_TREBLE | g_screens.treble;
        }
        this->updatebasstreble = BLITZ_MENU_BASSTREBLE_COUNT;
    }
    else if (cmd == BLZ_CMD_TREBLE_PLUS)
    {
        if (g_screens.treble < 12)
        {
            g_screens.treble++;
            *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME_TREBLE | g_screens.treble;
        }
        this->updatebasstreble = BLITZ_MENU_BASSTREBLE_COUNT;
    }
#   if BLZ_DEVMODE()
    else if (cmd == BLZ_CMD_F9)
    {
        if (this->asmimport->waitloop)
            this->asmimport->waitloop--;
    }
    else if (cmd == BLZ_CMD_F10)
    {
        this->asmimport->waitloop++;
    }
    else if (cmd == BLZ_CMD_Q)
    {
        if (this->asmimport->waitloop2)
            this->asmimport->waitloop2--;
    }
    else if (cmd == BLZ_CMD_W)
    {
        this->asmimport->waitloop2++;
    }
#   endif

    /* Plasma variables update */
    {
        thistatic->plasx++;
        thistatic->plasx2 += 2;
       
        if (thistatic->plasx2 >= BLITZ_MENUPLASMA_LOOP)
        {
            thistatic->plasx2 = 0;
       
            if (thistatic->plasx >= BLITZ_MENUPLASMA_LOOP)
            {
                thistatic->plasx = 0;
            }
        }
       
        thistatic->plasy2 += thistatic->plasyinc;
        thistatic->plasy2 &= 511;
       
        thistatic->plasy1++;
        if (thistatic->plasy1 >= 512)
        {
            thistatic->plasy1 = 0;
            thistatic->plasyinc++;
       
            if (thistatic->plasyinc > 4)
            {
                thistatic->plasyinc = 1;
                thistatic->plasmashiftmode ^= 1;
            }
        }
    }

    BLITZ_MENU_RASTERIZE(0x40);
}


static void blitzMenuUpdateTips(BlitZMenu* this)
{
    u16 tipid = this->menubarstate.tipid;


    if (this->updatebasstreble == BLITZ_MENU_BASSTREBLE_COUNT)
    {
        blitzUpdateBassTreble(this);
        this->updatebasstreble--;
    }

    if (tipid != this->menubarstate.tipiddisplay)
    {
        s8  newx = (s8)(tipid >> 8);
        s8  newy = (s8)tipid & 0xFF;
        s8  oldx = (s8)(this->menubarstate.tipiddisplay >> 8);
        s8  oldy = (s8)this->menubarstate.tipiddisplay & 0xFF;

        this->menubarstate.tipiddisplay = tipid;

        if (this->menubarstate.grid[oldx][oldy + 1].tips != NULL)
        {
            this->menubarstate.tipstargetx = 0;
            this->menubarstate.tipiddisplay = 0;

            return;
        }

        if (this->menubarstate.grid[newx][newy + 1].tips != NULL)
        {
            u16  len;
            u16  t, i;
            u8*  pc;

            {
                u32* p = (u32*)(this->tipscrollerbuffer + 160);
                /*u32  mask = 0x55550000UL;*/
                u32  mask = 0;

                for (i = 0; i < BLITZ_MENU_TIPS_H; i++)
                {
                    for (t = 0; t < 40; t++)
                    {
                        *p++ = mask;
                    }

                    /*mask ^= 0xFFFF0000UL;*/
                    p += (BLITZ_MENU_TIPS_PITCH - 160) / 4;
                }

                /*p -= (BLITZ_MENU_TIPS_PITCH - 160) / 4;
                *p = 0x2AAA0000UL;*/
            }

            pc  = this->menubarstate.grid[newx][newy + 1].tips;
            len = this->menubarstate.grid[newx][newy + 1].tipslen;
                       
            SYSfastPrint(pc, this->tipscrollerbuffer + BLITZ_MENU_TIPS_PITCH + 160, BLITZ_MENU_TIPS_PITCH, 4, (u32)&SYSfont);

            ASSERT(len <= 79);
            this->menubarstate.tipstargetx = 640 - ((79 - (len | 1)) * 8); /* | 1 to enforce 16 pixels alignement to limit glitches on faulty STes */

            blitzMenuShadowPrint(this, pc, this->tipscrollerbuffer + 160 + 2, BLITZ_MENU_TIPS_PITCH, 4);
            
            /*            {
                u16* p = (u16*)(this->tipscrollerbuffer + BLITZ_MENU_TIPS_PITCH + 160);

                for (i = 0; i < 7; i++)
                {
                    for (t = 0; t < 40; t++)
                    {
                        p[-(BLITZ_MENU_TIPS_PITCH / 2) + 1] |= *p | (*p >> 1); // | (*p << 1);
                        p[                               1] |= *p | (*p >> 1); // | (*p << 1);
                        p[ (BLITZ_MENU_TIPS_PITCH / 2) + 1] |= *p | (*p >> 1); // | (*p << 1);
                        p += 2;
                    }

                    p += 80;
                }
            }*/
        }
    }
}


static void blitzMenuInitPlasmaBuffer(BlitZMenu* this, u8 _choice)
{
    u16* p = (u16*)this->plasma;
    u16  t;
    u16  nbcolors, offset;
    u16  color1, color2;
    u8   metadataindex = RSC_BLITZWAV_METADATA_MENUS_PINKBLU0_BIN + (_choice << 1);


    ASSERT(LOADmetadataSize(&RSC_BLITZWAV, metadataindex) == 2 * sizeof(u16));
    nbcolors = (u16)LOADmetadataSize(&RSC_BLITZWAV, metadataindex + 1) >> 1;
    offset   = (u16)LOADmetadataOffset(&RSC_BLITZWAV, metadataindex) >> 1;

    color1 = PCENDIANSWAP16(this->colors[offset    ]);
    color2 = PCENDIANSWAP16(this->colors[offset + 1]);

    /*color1 = 0x400;*/

#   ifdef __TOS__
    ((u16*)this->plasmacode)[1] = color1;   /* plasma code has been optimized with autogenerated move in order to stabilize plasma lsl synchro */
#   endif

    this->asmimport->color1 = color1;
    this->asmimport->color2 = color2;

    {
        u32 c1 = color1;
        u32 c2 = color2;

        c1 |= c1 << 16;
        c2 |= c2 << 16;

        /*STDfastmset(this->plasma                                                                , c1, BLITZ_MENUPLASMA_PITCH); now manage in interupt begining */
        STDfastmset(this->plasma /*+ BLITZ_MENUPLASMA_PITCH */                                      , c2, (u32)BLITZ_MENUPLASMA_PITCH * (u32)BLITZ_MENUPLASMA_BUFFERHEIGHT);
        /*STDfastmset(this->plasma + (u32)(BLITZ_MENUPLASMA_BUFFERHEIGHT - 1) * (u32)BLITZ_MENUPLASMA_PITCH , c1, BLITZ_MENUPLASMA_PITCH); now manage in interupt epilogue */
    }

    offset += 2; /* step over color 1 & 2 */

    for (t = 0; t < (BLITZ_MENUPLASMA_PITCH >> 1); t++, p++)
    {
        s16  sn = g_screens.base.sin[((t << 1) + 160) & 511]; /* + 160 to phase correctly with horizontal sine */
        u16  y;
        s16  i;
        u16* a;
        u16* c = this->colors + offset;


        sn = PCENDIANSWAP16(sn);

        y = (STDmuls(sn, 70 - (nbcolors >> 1)) >> 16);
        y += 48 - (nbcolors >> 1);

        a = p + STDmulu(y, BLITZ_MENUPLASMA_PITCH / 2);

        for (i = nbcolors ; i > 0 ; i--)
        {
#           ifdef __TOS__
            *a = *c++;
#           else
            *a = PCENDIANSWAP16(*c);
            c++;
#           endif
            a += BLITZ_MENUPLASMA_PITCH / 2;
        }
    }

    this->displayedcolor = _choice;
}



void BlitZMenuBacktask (FSM* _fsm)
{
    BlitZMenu* this = g_screens.menu;
    s8 t, inc = 1;
    s8 selectx = g_screens.persistent.menu.selectx; /* Acquire state */


    IGNORE_PARAM(_fsm);

#   ifndef __TOS__
    if ((STDifrnd() & 31) != 0)
        return;
#   endif

    /* Follow the grid state */
    if (selectx < this->menubarstate.backtaskGridx)
    {
        inc = -1;
    }

    blitzMenuUpdateTips(this);

    for (t = this->menubarstate.backtaskGridx; inc > 0 ? t <= selectx : t >= selectx; t += inc)
    {
        u16 i;

        for (i = 0; i <= this->gridheight[t] ; i++)
        {
            BlitZMenuCell* cell = &this->menubarstate.grid[t][i];

            bool running = true;

            while (running)
            {
                switch (cell->state)
                {
                case BZMS_Hidden:
                    if (cell->validated)
                        cell->state = BZMS_Drawing;
                    else
                        running = false;
                    break;

                case BZMS_Drawing:
                    if (cell->nbwords > 0)
                    {
                        this->asmimport->blitzMenuXor(cell->adr, cell->nbwords, cell->h, cell->pitch_div2 << 1);
                    }
                    cell->state = BZMS_Drawn;
                    /*printf("draw %d;%d  ", t, i);*/
                    break;

                case BZMS_Drawn:
                    if (cell->validated)
                        running = false;
                    else
                        cell->state = BZMS_Undrawing;
                    break;

                case BZMS_Undrawing:
                    if (cell->nbwords > 0)
                    {
                        this->asmimport->blitzMenuXor(cell->adr, cell->nbwords, cell->h, cell->pitch_div2 << 1);
                    }
                    cell->state = BZMS_Hidden;
                    /*printf("hide %d;%d  ", t, i);*/
                    break;
                }

                blitzMenuUpdateTips(this);
            }
        }
    }

    this->menubarstate.backtaskGridx = selectx;

    if (g_screens.persistent.menu.colorchoice != this->displayedcolor)
    {
        blitzMenuInitPlasmaBuffer(this, g_screens.persistent.menu.colorchoice);
    }
}

void BlitZMenuExiting(FSM* _fsm)
{   
    BlitZMenu* this = g_screens.menu;

    STDfastmset(this->plasma /*+ BLITZ_MENUPLASMA_PITCH */, 0, (u32)BLITZ_MENUPLASMA_PITCH * (u32)(BLITZ_MENUPLASMA_BUFFERHEIGHT - 1));
    this->asmimport->color1 = this->asmimport->color2 = 0;
    STDfastmset(this->tipscrollerbuffer , 0, this->tipscrollerbuffersize);
    STDfastmset(this->basstreblebuffer  , 0, this->tipscrollerbuffersize);
    STDfastmset(this->iconscrollerbuffer, 0, this->iconscrollerbuffersize);
    STDfastmset(this->gridscrollerbuffer, 0, this->gridscrollerbuffersize);
        
    FSMgotoNextState(_fsm);
}


/*---------------------------------------------------------------------------------
    Menu exit
---------------------------------------------------------------------------------*/

void BlitZMenuExit(FSM* _fsm)
{
    BlitZMenu* this = g_screens.menu;


    IGNORE_PARAM(_fsm);

    SYSvblroutines[1] = RASvbldonothing; 
    BlitZturnOffDisplay();

    MEM_FREE(&sys.allocatorMem, this->basstreblebuffer);
    MEM_FREE(&sys.allocatorMem, this->tipscrollerbuffer);
    MEM_FREE(&sys.allocatorMem, this->charshadows);
    MEM_FREE(&sys.allocatorMem, this->gridscrollerbuffer);
    MEM_FREE(&sys.allocatorMem, this->iconscrollerbuffer);
    MEM_FREE(&sys.allocatorMem, this->tipstext);
    MEM_FREE(&sys.allocatorMem, this->automaton);
    MEM_FREE(&sys.allocatorMem, this->colors);
    MEM_FREE(&sys.allocatorMem, this->plasma);
    MEM_FREE(&sys.allocatorMem, this->plasmacode);
    MEM_FREE(&sys.allocatorMem, this->plasmasin);
    MEM_FREE(&sys.allocatorMem, this->plasmasindelta);
    MEM_FREE(&sys.allocatorMem, this->flashcolors);
    MEM_FREE(&sys.allocatorMem, this->code);

    MEM_FREE(&sys.allocatorMem, g_screens.menu);
    this = g_screens.menu = NULL;

    /*
    #   if DEMOS_MEMDEBUG
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
    TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
    RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
    #   endif
    */

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    if (g_screens.persistent.menu.selectx == 0)
    {
        g_screens.runscreen = BLZ_EP_INFO;
        FSMgoto(&g_stateMachineIdle , g_screens.fsm.stateIdleEntryPoints[BLZ_EP_INFO]);
        FSMgoto(&g_stateMachine     , g_screens.fsm.stateEntryPoints    [BLZ_EP_INFO]);
    }
    else
    {
        switch (g_screens.persistent.menu.playmode)
        {
        case BLZ_PLAY_FROM_MENU:
            ScreensGotoLoader(BLZ_EP_MENU);
            break;
        case BLZ_PLAY_INTERACTIVE:
#           if BLZ_WAITFOR_SCREENSELECT()
            ScreensGotoLoader(BLZ_EP_WAIT_FOR_FX);
#           else
            ScreensGotoLoader(BLZ_EP_WAVHERO);
#           endif
            break;
        
        default:
            ASSERT(0);
        case BLZ_PLAY_AUTORUN:
        case BLZ_PLAY_SCOREDRIVEN:
            ScreensGotoLoader(BLZ_EP_WAIT_FOR_FX);
            break;
        }
    }
}


void BlitZMenuInitStatic (FSM* _fsm)
{
    g_screens.persistent.menu.plasyinc = 1;

    FSMgotoNextState(_fsm);
}
