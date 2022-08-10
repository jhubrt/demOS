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
#define WAVHERO_RASTERIZE() 0
#else
#define WAVHERO_RASTERIZE() 0
#endif

#if WAVHERO_RASTERIZE()
#   define WAVHERO_RASTERIZE_COLOR(COLOR) *HW_COLOR_LUT=COLOR
#else
#   define WAVHERO_RASTERIZE_COLOR(COLOR)
#endif

#ifdef __TOS__
#   define WAVHERO_DEBUG_RAS_SYNC  0
#endif
#define WAVHERO_DEBUG_RAS_H     0 || WAVHERO_DEBUG_RAS_SYNC
#define WAVHERO_DEBUG_NOBARS    0
#define WAVHERO_DEBUG_CYCLING   0

#define WAVHERO_PITCH       168
#define WAVHERO_NBSTARS     258
#define WAVHERO_INC         20
#define WAVHERO_ZMAX        (126-STAR_INCMAX)
#define WAVHERO_RASCOLORS_W 8
#define WAVHERO_VOLSAMPLES  250
#define WAVHERO_CODESIZE    90000UL
#define WAVHERO_FIRSTLINE   12
#define WAVHERO_FIRSTLINE2  139

/* #define WAVHERO_TRACERASTERS */


ASMIMPORT WavHeroASMimport WHimportTable;

void WHvbl (void) PCSTUB;

static void wavHeroResetStarPositions (STARparam* _param)
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


static void wavHeroManageCommands(WavHero* this, bool allownavigation_)
{
    while (BLZ_COMMAND_AVAILABLE)
    {
        u8 cmd = BLZ_CURRENT_COMMAND;
        u8 category = cmd & BLZ_CMD_CATEGORY_MASK;
        /* u8 category = cmd & BLZ_CMD_CATEGORY_MASK; */

        BLZ_ITERATE_COMMAND;

        if (category == BLZ_CMD_VOICE1_CATEGORY)
        {
            if (cmd <= BLZ_CMD_VOICE1_2)
            {
                this->feed = cmd == BLZ_CMD_VOICE1_1;
            }
        }
        else if (category == BLZ_CMD_LINE1_CATEGORY)
        {
            if (cmd <= BLZ_CMD_E)
            {
                BLZ_TRAC_COMMAND_NUM("WAHspeed", cmd - BLZ_CMD_Q + 1);
                this->stars.inc = WHimportTable.starinc = (cmd - BLZ_CMD_Q + 1) << 1;
            }
        }            
        else if (category == BLZ_CMD_LINE2_CATEGORY)
        {
            if (cmd <= BLZ_CMD_S)
            {
                BLZ_TRAC_COMMAND_NUM("WAH3Dmode", cmd & BLZ_CMD_COMMAND_MASK);
                this->colormode = cmd & BLZ_CMD_COMMAND_MASK;
            }
        }
        else if (allownavigation_)
        {
            if (ScreensManageScreenChoice(BLZ_EP_WAVHERO, cmd))
                return;
        }
    }
}



static u8* wavHeroGenerateCodeSub(s16 filler1, s16 filler2, bool filler2isEndFiller, u16 nbmaxfiller1, u16 nbmaxfiller2, u16* nbFiller1, u16* nbFiller2, u32 targetcycle, u8* output, bool displayraster)
{
    u32 cycles = 0;
    u32 nextcycles = 0;

    
    if (displayraster)
    {
#       if WAVHERO_DEBUG_RAS_H || WAVHERO_DEBUG_RAS_SYNC
        u16* p = (u16*) output;
        u16 t;
#       endif    

        CGENgenerate(&WHimportTable.opcodes[WHrOp_Raster], cycles, output);

#       if WAVHERO_DEBUG_RAS_H || WAVHERO_DEBUG_RAS_SYNC
        for (t = WHimportTable.opcodes[WHrOp_Raster].opcodelen_div2 ; t > 0 ; t--)
            if (p[t] == 0x8250)
                p[t] = 0x8240;
#       endif    

#       if WAVHERO_DEBUG_RAS_SYNC
        for (t = WHimportTable.opcodes[WHrOp_Raster].opcodelen_div2 ; t > 0 ; t--)
            if (p[t] == 0x8242)
                p[t] = 0x8240;
#       endif

        targetcycle -= WHimportTable.opcodes[WHrOp_Raster].cycles_div2 << 1;
        cycles = 0;
    }

    if (filler1 != -1)  
    {   
        do
        {
            if (*nbFiller1 < nbmaxfiller1)
            {
                nextcycles += WHimportTable.opcodes[filler1].cycles_div2 << 1;

                if (nextcycles <= targetcycle)
                {
                    CGENgenerate(&WHimportTable.opcodes[filler1], cycles, output);
                    (*nbFiller1)++;
                }
            }
            else
                break;
        } 
        while (nextcycles <= targetcycle);
    }

    if (filler2 != -1)
    {
        if ((filler2isEndFiller == false) || (*nbFiller1 >= nbmaxfiller1))
        {
            nextcycles = cycles;

            do
            {
                if (*nbFiller2 < nbmaxfiller2)
                {
                    nextcycles += WHimportTable.opcodes[filler2].cycles_div2 << 1;

                    if (nextcycles <= targetcycle)
                    {
                        CGENgenerate(&WHimportTable.opcodes[filler2], cycles, output);
                        (*nbFiller2)++;
                    }
                }
                else
                    break;
            } 
            while (nextcycles <= targetcycle);
        }
    }

    CGENaddNops(cycles, targetcycle, output);
    
    return output;
}



static void wavHeroGenerateCode(WavHero* this)
{
    CGENdesc* code = WHimportTable.opcodes;
    u16  cycles = 0;
    u16  nbscanlines = 0;
    u8*  output;
    s16  t;
    u16  y = WAVHERO_FIRSTLINE + 2;
   

#   ifndef __TOS__
    code[WHrOp_Raster       ].cycles_div2 = 296 / 2;
    code[WHrOp_LowBorder    ].cycles_div2 = 56  / 2;
    code[WHrOp_Volume       ].cycles_div2 = 104 / 2;
    code[WHrOp_VolumeEnd    ].cycles_div2 = 128 / 2;
    code[WHrOp_StarErase    ].cycles_div2 = 20  / 2;
    code[WHrOp_StarDrawBegin].cycles_div2 = 100 / 2;
    code[WHrOp_StarDraw     ].cycles_div2 = 156 / 2;
    code[WHrOp_StarDrawEnd  ].cycles_div2 = 40  / 2;
#   endif

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "wavHeroGenerateCode", '\n');

    output = this->code;

    TRAClogNumber10S(TRAC_LOG_FLOW, "code", (output - this->code), 5, '\n');
    CGENgenerate (&code[WHrOp_Begin], cycles, output);
    TRAClogNumber10S(TRAC_LOG_FLOW, "code", (output - this->code), 5, '\n');

    {
        u16 nbvolumes       = 0;
        u16 nbVolEnd        = 0;
        u16 nbStarErase     = 0;
        u16 nbStarDraw      = 0;
        u16 nbStarDrawBegin = 0;
        u16 nbStarDrawEnd   = 0;

#       ifndef __TOS__
        u16 nbRas = 1;
#       endif
        
        output = aCGENaddnops(output, 32);

        for (t = 0 ; t < 3 ; t++)
        {
            CGENgenerateSimple(&WHimportTable.opcodes[WHrOp_Volume], output); 
            nbvolumes++;
        }

        t = this->rasnb - 2;

        do
        {
            nbscanlines = this->rasheight[t];
            t--;
            y += nbscanlines;
            output = wavHeroGenerateCodeSub(WHrOp_Volume, WHrOp_VolumeEnd, true, WAVHERO_VOLSAMPLES, 1, &nbvolumes, &nbVolEnd, nbscanlines << 9, output, true);

#           ifndef __TOS__
            nbRas++;
#           endif
        } 
        while (nbVolEnd == 0);

        for (; t >= 0; t--)
        {
            nbscanlines = this->rasheight[t];
            y += nbscanlines;
            output = wavHeroGenerateCodeSub(WHrOp_StarErase, -1, true, WAVHERO_NBSTARS, 0, &nbStarErase, NULL, nbscanlines << 9, output, true);

#           ifndef __TOS__
            nbRas++;
#           endif
        }
        ASSERT(nbStarErase > 0);

        nbscanlines = 1;
        y += nbscanlines;
        output = wavHeroGenerateCodeSub(WHrOp_StarErase, WHrOp_StarDrawBegin, true, WAVHERO_NBSTARS, 1, &nbStarErase, &nbStarDrawBegin, nbscanlines << 9, output, false);
        ASSERT(nbStarErase == WAVHERO_NBSTARS);
        ASSERT(nbStarDrawBegin == 1);

        nbscanlines = WAVHERO_FIRSTLINE2 - y;
        y += nbscanlines;
        output = wavHeroGenerateCodeSub(WHrOp_StarDraw, -1, false, WAVHERO_NBSTARS, 0, &nbStarDraw, NULL, (nbscanlines << 9), output, false);

        {
            u16 lasty = y;

            for (t = 0; t < this->rasnb - 1; )
            {
                nbscanlines = this->rasheight[t++];
                y += nbscanlines;

                if ((lasty <= 200) && (y > 200))
                {
#                   define WAVHERO_RASTER_CYCLESSHIFT 24
#                   define WAVHERO_RASTER_TO_LOWBORDER_CYCLES 272

                    /*u16 lowbordercycles = WHimportTable.opcodes[WHrOp_LowBorder].cycles_div2 << 1; */
                    u32 targetcycle = ((200 - lasty) << 9) + WAVHERO_RASTER_TO_LOWBORDER_CYCLES; 

                    output = wavHeroGenerateCodeSub(WHrOp_StarDraw, -1, false, WAVHERO_NBSTARS, 0, &nbStarDraw, NULL, targetcycle + WAVHERO_RASTER_CYCLESSHIFT, output, true);

                    CGENgenerateSimple(&WHimportTable.opcodes[WHrOp_LowBorder], output);

                    targetcycle = 512 - WAVHERO_RASTER_TO_LOWBORDER_CYCLES - (WHimportTable.opcodes[WHrOp_LowBorder].cycles_div2 << 1);
                    targetcycle += (y - 201) << 9;

                    /*output = wavHeroGenerateCodeSub(WHrOp_StarDraw, -1, false, WAVHERO_NBSTARS, 0, &nbStarDraw, NULL, targetcycle, output, false);*/

                    /*targetcycle = (y - 200) << 9;
                    targetcycle -= WAVHERO_RASTER_TO_LOWBORDER_CYCLES + lowbordercycles;*/

                    output = wavHeroGenerateCodeSub(WHrOp_StarDraw, WHrOp_StarDrawEnd, true, WAVHERO_NBSTARS, 1, &nbStarDraw, &nbStarDrawEnd, targetcycle, output, false);
                }
                else
                {
                    u32 targetcycle = nbscanlines << 9;

                    if (y == 198)
                        targetcycle -= WAVHERO_RASTER_CYCLESSHIFT;

                    output = wavHeroGenerateCodeSub(WHrOp_StarDraw, WHrOp_StarDrawEnd, true, WAVHERO_NBSTARS, 1, &nbStarDraw, &nbStarDrawEnd, targetcycle, output, true);
                }
                lasty = y;

#               ifndef __TOS__
                nbRas++;

                if (nbStarDrawEnd > 0)
                    TRAClogNumber10S(TRAC_LOG_SPECIFIC, "rasnb:", t, 3, '\n');
#               endif
            }
            ASSERT(nbStarDrawEnd > 0);
            
            CGENgenerateSimple(&WHimportTable.opcodes[WHrOp_Raster], output);
        }

        /*printf("%d\n", nbRas);*/
    }

    ASSERT(y < 249);

    CGENgenerateSimple (&code[WHrOp_End], output);

    ASSERT((output - this->code) < WAVHERO_CODESIZE);
}


void WavHeroInitStatic (FSM* _fsm)
{
    u16  t, i, ts;
    u8*  p;
    u16* k = SNDYM_g_keys.w;


    p = g_screens.wavheroStatic.ymheight;

    STDfastmset(p, 0, sizeof(g_screens.wavheroStatic.ymheight));

    for (t = 0, ts = 0 ; t < SNDYM_NBKEYS ; t++, ts += 256)
    {
        u16 index1 = *k++;
        u16 index2 = *k;


        index1 = PCENDIANSWAP16(index1);
        index2 = PCENDIANSWAP16(index2);

        if (index1 == index2)
            p[index1] = (ts >> 5) % 96;
        else
        {
            for (i = index2 ; i <= index1 ; i++)
            {
                u16 y = ((index1 - i) << 8) / (index1 - index2); 

                y += ts;
                p[i]  = (u8)(y >> 5) % 96;                
            }
        }
    }

    g_screens.wavheroStatic.starfielddiv = STARstaticIniField1P (&sys.allocatorCoreMem);

    FSMgotoNextState(_fsm);
}


static void wavHeroInitColors(WavHero* this)
{
    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "wavHeroInitColors", '\n');

    this->keyscolors[0]  = 0xF00;  
    this->keyscolors[16] = 0xFF0;  
    this->keyscolors[32] = 0x0F0;  
    this->keyscolors[48] = 0x0FF;  
    this->keyscolors[64] = 0x00F;  
    this->keyscolors[80] = 0xF0F;  
    this->keyscolors[96] = 0xF00;  

    COLcomputeGradient4b4b (&this->keyscolors[0] , &this->keyscolors[16], 1, 16, &this->keyscolors[0] );
    COLcomputeGradient4b4b (&this->keyscolors[16], &this->keyscolors[32], 1, 16, &this->keyscolors[16]);
    COLcomputeGradient4b4b (&this->keyscolors[32], &this->keyscolors[48], 1, 16, &this->keyscolors[32]);
    COLcomputeGradient4b4b (&this->keyscolors[48], &this->keyscolors[64], 1, 16, &this->keyscolors[48]);
    COLcomputeGradient4b4b (&this->keyscolors[64], &this->keyscolors[80], 1, 16, &this->keyscolors[64]);
    COLcomputeGradient4b4b (&this->keyscolors[80], &this->keyscolors[96], 1, 16, &this->keyscolors[80]);
}


static void wavHeroInitPolygon(WavHero* this) 
{
    u16 t, i;
    u8* img = this->framebuffer[0] + (124 * WAVHERO_PITCH);
    u16 plane = 0;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "wavHeroInitPolygon", '\n');       

    for (i = 1; i < 8; i <<= 1, plane += 2)
    {
        s16 x = -168;

        if ((plane & 2) == 0)
        {
            SYSvsync;           /* vsync to avoid colliding between hline and sound routine in VBL */
            VECclrpass();
        }

        for (t = 1; t < 8; t++)
        {
            if (t & i)
            {
                s16 x11 = x;
                s16 x12 = x + 40;
                s16 x21 = (x * STAR_PERSPEC / 256);
                s16 x22 = (x12 * STAR_PERSPEC / 256);
                s16 y2 = 123;
                s16 y1 = (y2 * STAR_PERSPEC / 256);

                x11 += 168;
                x12 += 168;
                x21 += 168;
                x22 += 168;

                VECdisplayBar(x11, x12, x22, x21, y2, y1, this->dlist);
                VECloop(img + plane, this->dlist + 4, 1);
            }

            x += 49;
        }
    }

    SYSvsync;   /* need to vsync to avoid colliding VBL and sound routine */

    VECclrpass();

    *HW_BLITTER_ADDR_SOURCE = (u32)img;
    *HW_BLITTER_ADDR_DEST   = (u32)img + WAVHERO_PITCH;
    *HW_BLITTER_XSIZE       = WAVHERO_PITCH / 2;
    *HW_BLITTER_YSIZE       = 124;
    *HW_BLITTER_ENDMASK1    = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1;
    *HW_BLITTER_HOP         = HW_BLITTER_HOP_SOURCE;
    *HW_BLITTER_OP          = HW_BLITTER_OP_S_XOR_D;
    *HW_BLITTER_XINC_SOURCE = 2;
    *HW_BLITTER_YINC_SOURCE = 2;
    *HW_BLITTER_XINC_DEST   = 2;
    *HW_BLITTER_YINC_DEST   = 2;
    *HW_BLITTER_CTRL2       = 0;
    *HW_BLITTER_CTRL1       = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;

    EMULblit();

    {
        u8* p1 = this->framebuffer[0] + 123 * WAVHERO_PITCH;
        u8* p2 = p1 + WAVHERO_PITCH;

        for (t = 0; t < 124; t++)
        {
            STDmcpy2(p1, p2, WAVHERO_PITCH);
            p1 -= WAVHERO_PITCH;
            p2 += WAVHERO_PITCH;
        }
    }

#   if WAVHERO_DEBUG_NOBARS
    STDmset(this->framebuffer[0], 0UL, (u32) WAVHERO_HEIGHT * (u32) WAVHERO_PITCH);
#   endif

    STDmcpy2(this->framebuffer[1], this->framebuffer[0], (u32) WAVHERO_HEIGHT * (u32) WAVHERO_PITCH);
}


static void wavHeroInitRasters(WavHero* this)
{
    WavHeroRaster* rasters0 = &this->rasters[0];
    WavHeroRaster* rasters1 = &this->rasters[1];

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "wavHeroInitRasters", '\n');

    rasters0->vbl.scanLinesTo1stInterupt = rasters1->vbl.scanLinesTo1stInterupt = WAVHERO_FIRSTLINE;
    rasters0->vbl.nextRasterRoutine      = rasters1->vbl.nextRasterRoutine      = (RASinterupt) this->code;

    {
        u16 max = 124;
        s16 ir = max + 16;
        s16 last;
        u16 nb = 0;
        u16 t,i;
        u16 inc = 1;
        u16 rasindexmax = 0;
        u16 h = 0;


        t = max * 16;
        last = t / ir;

        for (i = 1; i < max; i++)
        {
            u16 current;

            ir = (max - i) + 16;
            current = t / ir;

            if (current != last)
            {
                u16 starcolor;
                u16 starindex = 15 - (nb >> 2);
                ASSERT(starindex < 16);

                starcolor = COL4b2ST[starindex];
                starcolor |= (starcolor << 8) | (starcolor << 4);

                this->rasstarcolors[nb] = starcolor;
                this->rasheight[nb] = current - last;
                this->rasindex[nb] = inc;
                h += this->rasheight[nb];
                
                rasindexmax += inc;

                last = current;
                inc = 1;

                nb++;
            }
            else
            {
                inc++;
            }
        }

        this->rasnb = nb;
        this->rasindexmax = rasindexmax + 1;
        
        for (i = 0; i < nb; i++)
        {
            rasters0->rasters[i     ].inctable   = (WAVHERO_RASCOLORS_W                                + (WAVHERO_RASCOLORS_W - 1)) * -2;
            rasters0->rasters[i + nb].inctable   = 2;

            rasters1->rasters[i     ].inctable   = ((this->rasindex[nb - 1 - i] * WAVHERO_RASCOLORS_W) + (WAVHERO_RASCOLORS_W - 1)) * -2;
            rasters1->rasters[i + nb].inctable   = ((this->rasindex[i + 1     ] * WAVHERO_RASCOLORS_W) - (WAVHERO_RASCOLORS_W - 1)) * 2;

            rasters0->rasters[i     ].starscolor = rasters1->rasters[i     ].starscolor = this->rasstarcolors[i         ];
            rasters0->rasters[i + nb].starscolor = rasters1->rasters[i + nb].starscolor = this->rasstarcolors[nb - 1 - i];
        }

        rasters0->rasters[57     ].inctable = rasters1->rasters[57     ].inctable = (WAVHERO_RASCOLORS_W - 1) * -2;
        rasters0->rasters[57 + nb].inctable = rasters1->rasters[57 + nb].inctable = 2;
        
        rasters0->rasters[58].starscolor = rasters1->rasters[58].starscolor = 0x888;
        
#       if WAVHERO_DEBUG_RAS_H
        {
            u16 backgroundcolor = 7;

            for (i = 0; i < ARRAYSIZE(this->rasters.rasters); i++)
            {
                rasters0->rasters[i].starscolor = rasters1->rasters[i].starscolor = backgroundcolor;
                backgroundcolor ^= 3;
            }
        }
#       endif
       
        TRAClogNumber10S(TRAC_LOG_SPECIFIC, "rash"       , h          , 3, '\n'); 
        TRAClogNumber10S(TRAC_LOG_SPECIFIC, "rasnb"      , this->rasnb, 3, '\n'); 
        TRAClogNumber10S(TRAC_LOG_SPECIFIC, "rasindexmax", rasindexmax, 3, '\n'); 
        /*printf("nb = %d\n", nb);*/

        ASSERT(nb <= ARRAYSIZE(this->rasheight));
    }              
}

static void wavHeroInitKeysInfo (WavHero* this)
{
    u16 keydelta = g_screens.sndtrackKeyMax - g_screens.sndtrackKeyMin;
    u16 t;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "wavHeroInitKeysInfo", '\n');

    this->keysnoteinfo = (u8*)MEM_ALLOC(&sys.allocatorMem, g_screens.player.sndtrack->nbKeys);
    this->keyh = (u8*)MEM_ALLOC(&sys.allocatorMem, g_screens.player.sndtrack->nbKeys);

    for (t = 0; t < g_screens.player.sndtrack->nbKeys; t++)
    {
        u8 key = g_screens.player.sndtrack->keysnoteinfo[t];

        key = (key >> 4) * 12 + (key & 0xF);

        this->keysnoteinfo[t] = key;
    }

    for (t = 0; t < g_screens.player.sndtrack->nbKeys; t++)
    {
        this->keyh[t] = 128 + (u16) STDdivu((this->keysnoteinfo[t] - g_screens.sndtrackKeyMin) * 71, keydelta);
    }
}



void WavHeroEnter (FSM* _fsm)
{
    WavHero* this;
    u32  framebuffersize;
    u16  t;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "WavHeroEnter", '\n');

    STDmset(HW_COLOR_LUT,0,32);

    this = g_screens.wavhero = MEM_ALLOC_STRUCT(&sys.allocatorMem, WavHero);
    DEFAULT_CONSTRUCT(this);

    framebuffersize = (u32)WAVHERO_PITCH * (u32)WAVHERO_HEIGHT;

    this->framebuffer[0] = (u8*) MEM_ALLOC(&sys.allocatorMem, framebuffersize * 2UL);
    STDfastmset(this->framebuffer[0], 0, framebuffersize * 2UL);
    this->framebuffer[1] = this->framebuffer[0] + framebuffersize;

    SYSwriteVideoBase ((u32) this->framebuffer[0]);

    this->code = (u8*) MEM_ALLOC(&sys.allocatorMem, WAVHERO_CODESIZE);

    {
        s16* p = this->pitchmul;
        s16  offset = -(WAVHERO_HEIGHT / 2) * WAVHERO_PITCH;
    
        for (t = 0; t < WAVHERO_HEIGHT; t++)
        {
            *p++ = offset;
            offset += WAVHERO_PITCH;
        }
    }
    this->stars.nbstars  = WAVHERO_NBSTARS;
    this->stars.pitch    = WAVHERO_PITCH;
    this->stars.height   = WAVHERO_HEIGHT;
    this->stars.pitchmul = this->pitchmul;
    this->stars.inc      = 2;
    this->nbstarsdiv6minusone = (u16) STDdivu(this->stars.nbstars, 6) - 1;
    this->feed = true;
      
    STDmset(WHimportTable.volumes, 0, sizeof(WHimportTable.volumes));

    wavHeroInitColors(this);

    wavHeroInitPolygon(this);

    wavHeroInitKeysInfo(this);

    wavHeroInitRasters(this);

    wavHeroGenerateCode(this);

    this->stars.copyposx = (s16*) MEM_ALLOC(&sys.allocatorMem, sizeof(s16) * ((u32)STAR_WIDTH * (u32)(STAR_ZMAX + 1)));
    this->stars.copyposy = (u8*)  MEM_ALLOC(&sys.allocatorMem, WAVHERO_HEIGHT * (STAR_ZMAX + 1));

    STARinit1P(&sys.allocatorMem, &this->stars, g_screens.wavheroStatic.starfielddiv);

    {
        void** eb1 = (void**)this->stars.erasebuffers[0];
        void** eb2 = (void**)this->stars.erasebuffers[1];
        void*  val = this->framebuffer[0] + 6;

        for (t = 0 ; t < WAVHERO_NBSTARS ; t++)
        {
            *eb1++ = val;
            *eb2++ = val;
        }
    }

    this->colormode = 1;

    BlitZsetVideoMode(HW_VIDEO_MODE_4P, 0, BLITZ_VIDEO_16XTRA_PIXELS);

    WHimportTable.samplebuffer     = (void*) g_screens.player.dmabufstart;
    WHimportTable.starinc          = this->stars.inc;
    WHimportTable.rndx             = this->stars.rndx;
    WHimportTable.rndy             = this->stars.rndy;
    WHimportTable.currentrndx      = this->stars.rndx;
    WHimportTable.currentrndy      = this->stars.rndy;
    WHimportTable.starz            = this->stars.starz;
    WHimportTable.currentstarerase = this->stars.erasebuffers[0];
    WHimportTable.framebuffer      = this->framebuffer[0] + (6 + (WAVHERO_HEIGHT >> 1) * WAVHERO_PITCH);
    WHimportTable.colorstable      = this->rascolors[0] + ((this->rasindexmax - 1) * WAVHERO_RASCOLORS_W);

    wavHeroManageCommands(this, false);  /* hack => do this here in sync to avoid complexifying to put it on main thread like it should be */

    RASnextOpList = &this->rasters[this->colormode];
    SYSvblroutines[1] = (SYSinterupt)WHvbl;

    TRACsetVideoMode (168);

#   ifndef __TOS__
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRAClog(TRAC_LOG_MEMORY,"WavHero memallocator dump", '\n');
        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif

    /*HW_COLOR_LUT[8] = PCENDIANSWAP16(0xFFF);*/

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "WavHeroEnterEnd", '\n');

    FSMgotoNextState(_fsm);
    FSMgotoNextState(&g_stateMachine);
}


#ifdef __TOS__

#define wavHeroDisplayStarfield(this, erasebuffer, image) {\
    WHimportTable.currentstarerase = erasebuffer;\
    WHimportTable.framebuffer      = ((u8*)(image)) + (6 + (WAVHERO_HEIGHT >> 1) * WAVHERO_PITCH); }

/*static void wavHeroDisplayStarfield(WavHero* this, void* erasebuffer, u8* image)
{
    WHimportTable.currentstarerase = erasebuffer;
    WHimportTable.framebuffer      = ((u8*)(image)) + (6 + (WAVHERO_HEIGHT >> 1) * WAVHERO_PITCH); 
}*/

static void wavHeroDisplayColors(WavHero* this, u16* rasterscolors)
{
    while (*HW_VECTOR_TIMERB != 0);

    WHimportTable.colorstable      = rasterscolors + ((this->rasindexmax - 1) * WAVHERO_RASCOLORS_W);
    WHimportTable.samplebuffer     = (void*) g_screens.player.dmabufstart; 
    RASnextOpList                  = &this->rasters[this->colormode];
}

#else

static void wavHeroDisplayStarfield(WavHero* this, void* erasebuffer, u8* image)
{
    WAVHERO_RASTERIZE_COLOR(0x300);
    STARerase1P(erasebuffer, this->nbstarsdiv6minusone);

    WAVHERO_RASTERIZE_COLOR(0x700);
    STARdraw1P(image + (6 + (WAVHERO_HEIGHT >> 1) * WAVHERO_PITCH), this->stars.starz, this->stars.nbstars - 1, (u32)erasebuffer, (u32)&this->stars);
}

static void wavHeroComputeVolumes (u16* vol)
{
    s8* p = (s8*) g_screens.player.dmabufstart;
    u16 t;


    vol[0] = vol[1] = vol[2] = vol[3] = 0;

    for (t = 0 ; t < WAVHERO_VOLSAMPLES ; t++)
    {
        s8 s;

        s = *p++;   
        vol[0] += s > 0 ? s : -s;

        s = *p++;   
        vol[1] += s > 0 ? s : -s;

        s = *p++;   
        vol[3] += s > 0 ? s : -s;

        s = *p++;   
        vol[2] += s > 0 ? s : -s;

        p += 4;
    }

    vol[0] >>= 8;
    vol[1] >>= 8;
    vol[2] >>= 8;
    vol[3] >>= 8;
}

static void wavHeroDisplayColors(WavHero* this, u16* rastercolors_)
{
    static u16* lastrastercolors = NULL;
    u16  backgroundcolor = 7;
    u16* rastercolors = lastrastercolors;
    u16* pl;
    const int BASEY = 40;

    lastrastercolors = rastercolors_;   // delay one frame

    wavHeroComputeVolumes(WHimportTable.volumes);

    if (rastercolors == NULL)
        return;

    EMULfbExStart(HW_VIDEO_MODE_4P, 80, BASEY, 80 + WAVHERO_PITCH * 2 - 1, BASEY + WAVHERO_HEIGHT - 1, WAVHERO_PITCH, 0);
    {
        u16 t, j;
        s16 i;
        u16 y = BASEY;
        u32 cycles;
        u16* p = rastercolors + (this->rasindexmax - 1) * WAVHERO_RASCOLORS_W;
        u16  rasterindex = 0;

        WavHeroRaster* rasters = &this->rasters[this->colormode];

#       ifdef WAVHERO_TRACERASTERS
        static bool firsttime = true;
#       endif

        {
            u16 colorstars = rasters->rasters[rasterindex].starscolor;

            cycles = EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, y);

            EMULfbExSetColor(cycles, 8, PCENDIANSWAP16(colorstars));

#           if WAVHERO_DEBUG_RAS_H
            EMULfbExSetColor(cycles, 0, PCENDIANSWAP16(backgroundcolor));
            backgroundcolor ^= 3;
#           endif

            /*printf ("%d: %p %d\n", rasterindex, p, this->rasheight[0]);*/

            for (j = 0; j < 7; j++)
            {
                u16 color = p[j];

                EMULfbExSetColor(cycles, j + 1, PCENDIANSWAP16(color));

                if (color != 0)
                    EMULfbExSetColor(cycles, j + 9, PCENDIANSWAP16(color));
                else
                    EMULfbExSetColor(cycles, j + 9, PCENDIANSWAP16(colorstars));
            }

            pl = p;

            p += (rasters->rasters[rasterindex].inctable / 2) + (WAVHERO_RASCOLORS_W - 1);

#           ifdef WAVHERO_TRACERASTERS
            if (firsttime)
                printf("%d;%d;%d\n", rasterindex, y - BASEY, p - pl);
#           endif

            rasterindex++;
        }

        y += WAVHERO_FIRSTLINE + 2;

        cycles = EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, y);
        
        EMULfbExSetColor(cycles, 8, PCENDIANSWAP16(0xFFF));

        for (i = this->rasnb - 2; i >= 0 ; i--)
        {
            u16 colorstars = rasters->rasters[rasterindex].starscolor;


            cycles = EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, y);

            EMULfbExSetColor(cycles, 8, PCENDIANSWAP16(colorstars));

#           if WAVHERO_DEBUG_RAS_H
            EMULfbExSetColor(cycles, 0, PCENDIANSWAP16(backgroundcolor));
            backgroundcolor ^= 3;
#           endif

            /*printf ("%d: %p %d\n", rasterindex, p, this->rasheight[i]);*/

            for (j = 0; j < 7; j++)
            {
                u16 color = p[j];

                EMULfbExSetColor(cycles, j + 1, PCENDIANSWAP16(color));

                if (color != 0)
                    EMULfbExSetColor(cycles, j + 9, PCENDIANSWAP16(color));
                else
                    EMULfbExSetColor(cycles, j + 9, PCENDIANSWAP16(colorstars));
            }

            pl = p;

            p += (rasters->rasters[rasterindex].inctable / 2) + (WAVHERO_RASCOLORS_W - 1);

#           ifdef WAVHERO_TRACERASTERS
            if (firsttime)
                printf("%d;%d;%d\n", rasterindex, y - BASEY, p - pl);
#           endif

            y += this->rasheight[i];

            rasterindex++;
        }

        /*printf ("--------------------------------\n");*/

        {
            u16 max = 192;
            s16 ir = max + 16;
            s16 last;
            /*static u16 base = 0;

            p += base;

            base++;
            if (base > 150)
                base = 0;*/

            t = max * 16;
            last = t / ir;

            y = WAVHERO_FIRSTLINE2 + BASEY;

            for (i = 0 ; i < this->rasnb ; i++)
            {
                u16 colorstars = rasters->rasters[rasterindex].starscolor;
                s16 j;


                cycles = EMULfbExComputeCycle(HW_VIDEO_SYNC_50HZ, 0, y);

                EMULfbExSetColor(cycles, 8, PCENDIANSWAP16(colorstars));

#               if WAVHERO_DEBUG_RAS_H
                {
                    static u16 lasty = 0;
                    EMULfbExSetColor(cycles, 0, PCENDIANSWAP16(backgroundcolor));
                    if ((lasty < 240) && (y >= 240))
                    {
                        EMULfbExSetColor(cycles, 0, PCENDIANSWAP16(0x700));
                    }
                    lasty = y;
                }
                backgroundcolor ^= 3;
#               endif

                /*printf ("%d: %p %d\n", rasterindex, p, this->rasheight[i]);*/

                for (j = 0; j < 7; j++)
                {
                    u16 color = p[j];

                    EMULfbExSetColor(cycles, j + 1, PCENDIANSWAP16(color));

                    if (color != 0)
                        EMULfbExSetColor(cycles, j + 9, PCENDIANSWAP16(color));
                    else
                        EMULfbExSetColor(cycles, j + 9, PCENDIANSWAP16(colorstars));
                }

                pl = p;

                p += (rasters->rasters[rasterindex].inctable / 2) + (WAVHERO_RASCOLORS_W - 1);

#               ifdef WAVHERO_TRACERASTERS
                if (firsttime)
                    printf("%d;%d;%d\n", rasterindex, y - BASEY, p - pl);
#               endif

                y += this->rasheight[i];

                rasterindex++;
            }
        }              

#       ifdef WAVHERO_TRACERASTERS
        firsttime = false;
#       endif
    }

    EMULfbExEnd();
}
#endif

void WavHeroActivity(FSM* _fsm)
{
    WavHero* this = g_screens.wavhero;

    u8*       image = (u8*)this->framebuffer[this->flip];
    u16       *currentrascolor, *currentrascolor2, *colorp;
    u16*      vol = WHimportTable.volumes;
    u16       t;

#   if WAVHERO_DEBUG_CYCLING
    static u16 debugcolor = 0x10;
    debugcolor += 0x10;
    if (debugcolor > 0x70)
        debugcolor = 0x10;
#   endif

    WAVHERO_RASTERIZE_COLOR(0x7);

    IGNORE_PARAM(_fsm);
    
    colorp = currentrascolor  = this->rascolors[this->rascurrent];
    currentrascolor2 = this->rascolors[this->rascurrent + this->rasindexmax];

    wavHeroDisplayStarfield(this, this->stars.erasebuffers[this->flip], image);

    if (this->feed == false)
    {
        for (t = 0; t < 7; t++)
        {
            *currentrascolor++ = *currentrascolor2++ = 0;
        }
    }
    else        
    {
        BLSvoice* voice = g_screens.player.voices;
        u8        mixer;

        WAVHERO_RASTERIZE_COLOR(0x770);

        for (t = 0; t < 4; t++, voice++)
        {
            if (voice->keys[0] != NULL)
            {
                u16 index = ((u8*)voice->keys[0] - (u8*)g_screens.player.sndtrack->keys) / sizeof(BLSprecomputedKey);
                u16 key;
                u16 mul;

                ASSERT(index < g_screens.player.sndtrack->nbKeys);

                key = g_screens.player.sndtrack->keysnoteinfo[index];

                mul = ((u16)vol[t]) >> 2;
                if (mul > 16)
                    mul = 16;

                COLcomputeGradient16Steps4bSTe(&this->colorzero, &this->keyscolors[(key & 0xF) << 3], 1, mul, currentrascolor);
            }
            else
            {
                *currentrascolor = 0;
            }            

#           if WAVHERO_DEBUG_RAS_SYNC
            if (t == 0)
                *currentrascolor = 0;
#           endif

#           if WAVHERO_DEBUG_CYCLING
            *currentrascolor = debugcolor;
#           endif

            *currentrascolor2++ = *currentrascolor++;
        }

        WAVHERO_RASTERIZE_COLOR(0x707);

        {
            u8 val;
            u8 level;
            u16 freq;
            u16 y = 0;

            *HW_YM_REGSELECT = HW_YM_SEL_IO_AND_MIXER;
            mixer = HW_YM_GET_REG();

            for (t = 0; t < 3; t++)
            {
                bool noise = false;
                u16 mul;

                *HW_YM_REGSELECT = HW_YM_SEL_LEVELCHA + t;
                level = HW_YM_GET_REG();

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

                    y = g_screens.wavheroStatic.ymheight[freq];
                }
                else if ((val & 8) == 0)
                {
                    *HW_YM_REGSELECT = HW_YM_SEL_FREQNOISE;
                    freq = HW_YM_GET_REG();

                    noise = true;
                }

                mixer >>= 1;

                if (level > 0)
                {
                    u16 currentcolor;

                    if (noise)
                    {
                        currentcolor = COL4b2ST[15 - (freq >> 2) - (STDifrnd() & 7)];
                        currentcolor |= (currentcolor << 8) | (currentcolor << 4);

                        if (this->wavhero_noiseinc)
                            currentcolor = 0;

                        this->wavhero_noiseinc ^= 1;

                        mul = 16;
                    }
                    else
                    {
                        mul = ((level << 3) - level) >> 2; /* *7/4 */

                        /*if (this->colormode == 0)
                        {*/
                            currentcolor = this->keyscolors[y];

                            /*u16 key = y >> 8;
                            u16 semitone = key % 12;

                            if ((semitone == ((state->lastkey >> 8) % 12)) && ((key / 12) != (state->lastkey / (256 * 12))))
                                mul >>= 1;*/
                        /*}
                        else
                            currentcolor = this->keyscolors[(t + 4) << 4];*/
                    }

                    if (mul > 16)
                        mul = 16;

                    /*printf ("[%d] %5d %2d | ", t, y, (y >> 5) % 96);*/
                    COLcomputeGradient16Steps4bSTe(&this->colorzero, &currentcolor, 1, mul, currentrascolor);
                }
                else
                {
                    *currentrascolor = 0;
                }

#               if WAVHERO_DEBUG_CYCLING
                *currentrascolor = 0x70;
#               endif

                *currentrascolor2++ = *currentrascolor++;
            }
        }
    }

    WAVHERO_RASTERIZE_COLOR(0x0);

    this->flip ^= 1;

    this->rascurrent++;
    if (this->rascurrent >= this->rasindexmax)
        this->rascurrent = 0;

    wavHeroDisplayColors(this, colorp);
    
    SYSwriteVideoBase((u32)image);
    
    wavHeroManageCommands(this, true);

    /*{
    static int pop = 0;

    pop++;

    if ((pop & 7) == 0)
        currentrascolor2[7] = currentrascolor[7] = 0xFFFF;
    }

    printf ("\n"); */
}


void WavHeroBacktask(FSM* _fsm)
{
    IGNORE_PARAM(_fsm);

    /*STDstop2300();*/
}


void WavHeroExit(FSM* _fsm)
{
    WavHero* this = g_screens.wavhero;

    IGNORE_PARAM(_fsm);

    SYSvblroutines[1] = RASvbldonothing;

    BlitZturnOffDisplay();

    STARshutdown(&sys.allocatorMem, &this->stars);

    MEM_FREE(&sys.allocatorMem, this->framebuffer[0]);
    MEM_FREE(&sys.allocatorMem, this->stars.copyposx);
    MEM_FREE(&sys.allocatorMem, this->stars.copyposy);
    MEM_FREE(&sys.allocatorMem, this->keysnoteinfo);
    MEM_FREE(&sys.allocatorMem, this->keyh);
    MEM_FREE(&sys.allocatorMem, this->code);
    MEM_FREE(&sys.allocatorMem, this);

    this = g_screens.wavhero = NULL;

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
