/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2024 J.Hubert

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
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\BITMAP.H"
#include "DEMOSDK\MODX2MOD.H"
#include "DEMOSDK\CODEGEN.H"

#include "DEMOSDK\PC\EMUL.H"

#include "EXTERN\RELOCATE.H"
#include "EXTERN\WIZZCAT\PRTRKSTE.H"

#include "RELAPSE\SRC\SCREENS.H"
#include "RELAPSE\SRC\INTERLUD\INTERLUD.H"

#include "RELAPSE\RELAPSE1.H"


#define INTERLUDE_WIDTH  408
#define INTERLUDE_HEIGHT 256
#define INTERLUDE_PITCH  256

#define INTERLUDE_FRAMEBUFFER_SIZE ((u32)INTERLUDE_PITCH * (u32)INTERLUDE_HEIGHT * 2UL)
#define INTERLUDE_GENCODE_SIZE  70000UL

#define INTERLUDE_SOURCE_WIDTH      169
#define INTERLUDE_SOURCE_HEIGHT     16
#define INTERLUDE_SOURCE_PITCH      144
#define INTERLUDE_MOVE_MAXNBSTEPS   50
#define INTERLUDE_ROTATION_NBSTEPS  64

#define INTERLUDE_ACTIVATE_FLEXI() 1
#define INTERLUDE_TEST() 0

static char* Interlude_g_text = "INTERLUDE: LET'S HAVE A BREAK...";

typedef void(*InterludeGenCodeCall)(void* source, void* dest);


#ifdef __TOS__

static InterludeAsmImport* interludeAsmImport(void* _prxbuffer)
{
    return (InterludeAsmImport*)((DYNbootstrap)_prxbuffer)();
}

#else

#include "DEMOSDK\SYNTHYMZ.H"

void interludeInit (void* param_) {}

void interludeUpdate (void* param_) 
{
    Info* this = (Info*)param_;

    EMULfbExStart (HW_VIDEO_MODE_4P, 80, 20, 80 + 224 * 2 - 1, 20 + 274 - 1, 256, 0);
    EMULfbExEnd();
}

void interludeShutdown (void* param_) {}
void interludeVbl      (void)         {}

static InterludeAsmImport* interludeAsmImport(void* _prxbuffer)
{
    static InterludeAsmImport asmimport;

    asmimport.init     = (DYNevent)    interludeInit;
    asmimport.vbl      = (SYSinterupt) interludeVbl;
    asmimport.ymdecode = SNDblitzDecodeYM;

    return &asmimport;
}

#endif

#define INTERLUDE_EXIT(NEXTNBCYCLES,JUMPINDEX)  reenter##JUMPINDEX:  if ((cycles + (NEXTNBCYCLES)) >= maxcycles_) { this->gen.run = (JUMPINDEX); goto end; }
#define INTERLUDE_RENTER(JUMPINDEX)  case JUMPINDEX: goto reenter##JUMPINDEX

u16 gnbloads = 0;


static s8 interlud_bitmapping[8] = {0x14, 3, 2, 0x15, 0x16, -1, -1, 7};

static u16* interludeGenerateFlexiCode(Interlude* this, u16* d_, u16 maxcycles_, bool compensate_)
{
    const u16 lineoffset = INTERLUDE_SOURCE_PITCH - (((INTERLUDE_SOURCE_WIDTH + 15) & 0xFFF0) >> 3);
    u16* d = d_;
    u16  cycles = 0;
    s16* xs = &this->gen.xs;
    s16* ys = &this->gen.ys;
    s16* xd = &this->gen.xd;
    s16* yd = &this->gen.yd;

    /*
    d0   input pixel data
    d1   temp
    d2   $C002                  bit 2   start blitter and overscan
    d3   1                      bit 1   used to set blitter height
    d4	-2          move        bit 0          
    d5  -9          move        bit 3
    d6  -17         move        bit 4
    d7  -1          move clear  bit 7

    a0   source
    a1   dest
    */
   
    switch (this->gen.run)
    {
    INTERLUDE_RENTER(1);
    INTERLUDE_RENTER(2);
    INTERLUDE_RENTER(3);
    INTERLUDE_RENTER(4);
    INTERLUDE_RENTER(5);
    INTERLUDE_RENTER(6);
    INTERLUDE_RENTER(7);
    default: ASSERT(0);
    case 0: ;
    }

    do
    {
        {
            u16 angle = 511 - ((*xs + *xs + *xs) & 511);
            s16 sin = PCENDIANSWAP16(this->sin[angle]);
            s16 cos = PCENDIANSWAP16(this->sin[angle + 128]);
            s16 r   = ((62 - (*ys << 1)) << 1);

            *xd = (STDmuls(sin, r) >> 16) + 140 * 2 + 64; /* xs + 160; */
            *yd = (STDmuls(cos, r) >> 16) + 64;

            /*
            *xd = *xs + 272;
            *yd = *ys * 2 + 2;
            *yd = (STDmuls(cos, 45) >> 16) + 50 + *ys;*/

            /*printf ("xd=%d yd=%d sin=%f cos=%f r=%d angle=%d\n", xd, yd, sin / 65536.f, cos / 65536.f, r, xs << 1);*/
        }

        if ((*xs & 15) == 0)
        {
            INTERLUDE_EXIT(8,1);

            /*printf ("%d-%d: load word %p\n", xs, ys, d);*/
            *d++ = CGEN_OPCODE_MOVE_0Ax0_P_Dy(CGEN_MOVESIZE_W, 0, 0);       /* move.w  (a0)+,d0     8       */
             cycles += 8;
        }

        /*printf ("%d-%d: shift word %p\n", xs, ys, d);*/

        (*xs)++;

        if (*xs >= INTERLUDE_SOURCE_WIDTH)
        {
            if (lineoffset > 0)
            {
                INTERLUDE_EXIT(8,2);

                *d++ = CGEN_OPCODE_LEA_X_Ax_Ay(0, 0);
                *d++ = lineoffset;
                cycles += 8;
            }

            /*printf ("line feed\n");*/
            *xs = 0;
            (*ys)++;

            if ((*ys) >= INTERLUDE_SOURCE_HEIGHT)
                goto end;
        }

        if ((*xd < 0) || (*xd >= INTERLUDE_WIDTH) || (*yd < 0) || (*yd >= INTERLUDE_HEIGHT))
        {
            INTERLUDE_EXIT(4,3);

            *d++ = CGEN_OPCODE_ADD_W_Dx_Dy(0, 0);           /* add.w   d0,d0        4  */
            cycles += 4;
        }
        else
        {
            this->gen.bit = 7 - (*xd & 7);

            this->gen.offset = *yd * INTERLUDE_PITCH;
            this->gen.offset += (*xd & 0xFFF0) >> 1;
            this->gen.offset += (*xd & 0xF) >= 8 ? 1 : 0;

            ASSERT(this->gen.offset <= 32737);

            {
                if (this->framebuffer[this->gen.offset] == 0)
                {
                    this->framebuffer[this->gen.offset] = 1;

                    if (compensate_)
                    {
                        INTERLUDE_EXIT(36,4);

                        *d++ = CGEN_OPCODE_ADD_W_Dx_Dy(0, 0);
                        cycles += 4;

                        if ((interlud_bitmapping[this->gen.bit] & 0xF0) == 0x10)
                        {
                            /*
                            add.w   d0,d0                               4       4
                            bcs.s   +x                                  12      8
                            move.b  d7,offset(a1)                               12
                            bra.s   +y                                          12
                            move.b  dx,offset(a1)                       12
                            nop                                         4
                            nop                                         4
                            */

                            * d++ = CGEN_OPCODE_B_S(CGEN_CS, 6);
                            *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0_I(CGEN_MOVESIZE_B, 7, 1);
                            *d++ = this->gen.offset;

                            if (compensate_)
                                *d++ = CGEN_OPCODE_B_S(CGEN_T, 8);
                            else
                                *d++ = CGEN_OPCODE_B_S(CGEN_T, 4);

                            *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0_I(CGEN_MOVESIZE_B, interlud_bitmapping[this->gen.bit] & 7, 1);
                            *d++ = this->gen.offset;

                            if (compensate_)
                            {
                                *d++ = CGEN_OPCODE_NOP;
                                *d++ = CGEN_OPCODE_NOP;
                            }
                        }
                        else
                        {
                            /*
                            add.w   d0,d0                               4       4
                            bcs.s   +x                                  12      8
                            move.b  d7,offset(a1)                               12
                            bra.s   +y                                          12
                            move.b  #~(1 << bit),offset(a1)             16
                            nop                                         4
                            */

                            *d++ = CGEN_OPCODE_B_S(CGEN_CS, 6);
                            *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0_I(CGEN_MOVESIZE_B, 7, 1);
                            *d++ = this->gen.offset;

                            if (compensate_)
                                *d++ = CGEN_OPCODE_B_S(CGEN_T, 8);
                            else
                                *d++ = CGEN_OPCODE_B_S(CGEN_T, 6);

                            *d++ = CGEN_OPCODE_MOVE_IMMEDIATE_0Ax0_I(CGEN_MOVESIZE_B, 1);
                            *d++ = ~(1 << this->gen.bit);
                            *d++ = this->gen.offset;

                            if (compensate_)
                            {
                                *d++ = CGEN_OPCODE_NOP;
                            }
                        }

                        cycles += 32;
                    }
                    else
                    {
                        /*
                        add.w   d0,d0             4
                        scc.s   d1                4/8
                        or.b    dx,d1             4
                        move.b  d7,offset(a1)     12
                        */

                        INTERLUDE_EXIT(32,7);

                        *d++ = CGEN_OPCODE_ADD_W_Dx_Dy(0, 0);                       /* 4 */
                        cycles += 4;

                        *d++ = CGEN_OPCODE_S_Dx(CGEN_CC, 1);                        /* scc     d1         4/8  */
                        cycles += 8;

                        if ((interlud_bitmapping[this->gen.bit] & 0xF0) == 0x10)
                        {
                            *d++ = CGEN_OPCODE_OR_Dx_Dy(CGEN_OPSIZE_B, interlud_bitmapping[this->gen.bit] & 7, 1);        /* 4 */                        
                            cycles += 4;
                        }
                        else
                        {
                            *d++ = CGEN_OPCODE_OR_IMMEDIATE_Dy(CGEN_OPSIZE_B, 1);                                         /* 8 */
                            *d++ = ~(1 << this->gen.bit);

                            cycles += 8;
                        }

                        *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0_I(CGEN_MOVESIZE_B, 1, 1);   /* move.b  d1,X(a1)    12        */
                        *d++ = this->gen.offset;
                        cycles += 12;
                    }
                }
                else
                {
                    if (interlud_bitmapping[this->gen.bit] < 0)
                    {
                        INTERLUDE_EXIT(36, 5);

                        /* 
                        add.w   d0,d0                               4       4
                        bcs.s   +x                                  12      8
                        nop * 3                                             12 = 3 * 4
                        bra.s   +y                                          12
                        and.b   #~(1 << bit),offset(a1)             20
                        */

                        *d++ = CGEN_OPCODE_ADD_W_Dx_Dy(0, 0);
                        cycles += 4;

                        if (compensate_)
                        {
                            *d++ = CGEN_OPCODE_B_S(CGEN_CS, 8);
                            *d++ = CGEN_OPCODE_NOP;
                            *d++ = CGEN_OPCODE_NOP;
                            *d++ = CGEN_OPCODE_NOP;
                            *d++ = CGEN_OPCODE_B_S(CGEN_T, 6);
                        }
                        else
                        {
                            *d++ = CGEN_OPCODE_B_S(CGEN_CC, 6);
                        }

                        *d++ = CGEN_OPCODE_AND_IMMEDIATE_0Ay0_I(CGEN_OPSIZE_B, 1);
                        *d++ = ~(1 << this->gen.bit);
                        *d++ = this->gen.offset;

                        cycles += 32;
                    }
                    else
                    {
                        INTERLUDE_EXIT(32, 6);

                        /* 
                        add.w   d0,d0                               4       4
                        bcs.s   +x                                  12      8
                        nop * 2                                             8 = 2 * 4
                        bra.s   +y                                          12
                        and.b   dx,offset(a1)                       16
                        or
                        bclr.b  dx,offset(a1)                       16
                        */

                        *d++ = CGEN_OPCODE_ADD_W_Dx_Dy(0, 0);
                        cycles += 4;

                        if (compensate_)
                        {
                            *d++ = CGEN_OPCODE_B_S(CGEN_CS, 6);
                            *d++ = CGEN_OPCODE_NOP;
                            *d++ = CGEN_OPCODE_NOP;
                            *d++ = CGEN_OPCODE_B_S(CGEN_T, 4);
                        }
                        else
                        {
                            *d++ = CGEN_OPCODE_B_S(CGEN_CC, 4);
                        }

                        if (interlud_bitmapping[this->gen.bit] & 0x10)
                        {
                            *d++ = CGEN_OPCODE_AND_Dx_0Ay0_I(CGEN_OPSIZE_B, interlud_bitmapping[this->gen.bit] & 7, 1);
                        }
                        else
                        {
                            *d++ = CGEN_OPCODE_BCLR_Dx_0Ay0_I(interlud_bitmapping[this->gen.bit], 1);
                        }

                        *d++ = this->gen.offset;

                        cycles += 28;
                    }
                }
            }
        }
    } 
    while (1);
    
    /*printf("exit %p => %p - %d => %d\n", d, lastd, this->gen.xs, lastgen.xs);*/

end:

    if (compensate_)
    {
        u16 t;
        
        for (t = cycles ; t < maxcycles_ ; t += 4)
        {
            *d++ = CGEN_OPCODE_NOP;
            this->gen.lost++;
        }
    }

    return d;
}


static u16* interludeGenerateFullscreenScanlineType1(Interlude* this, u16* d, u16 nblines)
{
    u16 y;

    
    for (y = 0; y < nblines; y++)
    {
        *d++ = CGEN_OPCODE_MOVE_Dx_SHORTADR(CGEN_MOVESIZE_B, 2);   *d++ = 0x8260;
        *d++ = CGEN_OPCODE_MOVE_Dx_SHORTADR(CGEN_MOVESIZE_W, 2);   *d++ = 0x8260;
        *d++ = CGEN_OPCODE_MOVE_0Ax0_P_SHORTADR(CGEN_MOVESIZE_B, 5);  /* video counter           move.b  (a5)+,$ffff8207.w  16  */
        *d++ = 0x8207;

#       if INTERLUDE_ACTIVATE_FLEXI()
        d = interludeGenerateFlexiCode(this, d, (90 - 2 - 4) * 4, true);
#       else
        {
            u16 t;
            for (t = 0; t < 90 - 2 - 4; t++)
                *d++ = CGEN_OPCODE_NOP;
        }
#       endif

        *d++ = CGEN_OPCODE_MOVE_0Ax0_P_Dy(CGEN_MOVESIZE_B, 2, 1);  /* blitter source low      move.b  (a2)+,d1          8  */

        *d++ = CGEN_OPCODE_MOVE_Dx_SHORTADR(CGEN_MOVESIZE_W, 2);   *d++ = 0x820a;
        *d++ = CGEN_OPCODE_MOVE_Dx_SHORTADR(CGEN_MOVESIZE_B, 2);   *d++ = 0x820a;

        /*for (t = 0 ; t < 26 ; t++)
        *d++ = CGEN_OPCODE_NOP;*/

        if (sys.isMegaSTe == false)
            *d++ = CGEN_OPCODE_NOP;

        *d++ = CGEN_OPCODE_ADD_W_Dx_0Ay0(1, 3);                      /* blitter source low      add.w   d1,(a3)         12 */
        *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0(CGEN_MOVESIZE_W, 3, 4);      /* blitter vsize           move.w  d3,(a4)         8  */
        *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0(CGEN_MOVESIZE_W, 2, 6);      /* blitter start           move.w  d2,(a6)         8  preload $C002 in d2 instead of 2 to save 1 register (preshift color by 2 to compensate) */
    }
    
    return d;
}


static u16* interludeGenerateFullscreenScanlineType2(Interlude* this, u16* d)
{
#   if !INTERLUDE_ACTIVATE_FLEXI()
    u16 t;
#   endif
    
    *d++ = CGEN_OPCODE_MOVE_Dx_SHORTADR(CGEN_MOVESIZE_B, 2);   *d++ = 0x8260;
    *d++ = CGEN_OPCODE_MOVE_Dx_SHORTADR(CGEN_MOVESIZE_W, 2);   *d++ = 0x8260;
    *d++ = CGEN_OPCODE_MOVE_0Ax0_P_SHORTADR(CGEN_MOVESIZE_B, 5);  /* video counter           move.b  (a5)+,$ffff8207.w  16  */
    *d++ = 0x8207;

#   if INTERLUDE_ACTIVATE_FLEXI()
    d = interludeGenerateFlexiCode(this, d, (87 - 2 - 4) * 4, true);
#   else
    for (t = 0; t < 87 - 2 - 4; t++)
        *d++ = CGEN_OPCODE_NOP;
#   endif

    *d++ = CGEN_OPCODE_MOVE_0Ax0_P_Dy(CGEN_MOVESIZE_B, 2, 1);    /* blitter source low      move.b  (a2)+,d1        8  */
    *d++ = CGEN_OPCODE_ADD_W_Dx_0Ay0(1, 3);                      /* blitter source low      add.w   d1,(a3)         12 */

    *d++ = CGEN_OPCODE_MOVE_Dx_SHORTADR(CGEN_MOVESIZE_W, 2);   *d++ = 0x820a;
    *d++ = CGEN_OPCODE_MOVE_Dx_SHORTADR(CGEN_MOVESIZE_B, 2);   *d++ = 0x820a;

    /*    for (t = 0; t < 23; t++)
    *d++ = CGEN_OPCODE_NOP;*/

    if (sys.isMegaSTe == false)
        *d++ = CGEN_OPCODE_NOP;

    *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0(CGEN_MOVESIZE_W, 3, 4);      /* blitter vsize           move.w  d3,(a4)         8  */
    *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0(CGEN_MOVESIZE_W, 2, 6);      /* blitter start           move.w  d2,(a6)         8  preload $C002 in d2 instead of 4 to save 1 register (preshift color by 4 to compensate) */

    *d++ = CGEN_OPCODE_MOVE_Dx_SHORTADR(CGEN_MOVESIZE_W, 2);   *d++ = 0x820a;

    *d++ = CGEN_OPCODE_MOVE_Dx_SHORTADR(CGEN_MOVESIZE_B, 2);   *d++ = 0x8260;
    *d++ = CGEN_OPCODE_MOVE_Dx_SHORTADR(CGEN_MOVESIZE_W, 2);   *d++ = 0x8260;

    *d++ = CGEN_OPCODE_MOVE_0Ax0_P_SHORTADR(CGEN_MOVESIZE_B, 5);  /* video counter           move.b  (a5)+,$ffff8207.w  16  */
    *d++ = 0x8207;

    *d++ = CGEN_OPCODE_MOVE_Dx_SHORTADR(CGEN_MOVESIZE_B, 2);   *d++ = 0x820a;

#   if INTERLUDE_ACTIVATE_FLEXI()
    d = interludeGenerateFlexiCode(this, d, (87 - 2 - 4) * 4, true);
#   else
    for (t = 0; t < 87-2-4; t++)
        *d++ = CGEN_OPCODE_NOP;
#   endif

    *d++ = CGEN_OPCODE_MOVE_0Ax0_P_Dy(CGEN_MOVESIZE_B, 2, 1);  /* blitter source low      move.b  (a2)+,d1      8  */

    *d++ = CGEN_OPCODE_MOVE_Dx_SHORTADR(CGEN_MOVESIZE_W, 2);   *d++ = 0x820a;
    *d++ = CGEN_OPCODE_MOVE_Dx_SHORTADR(CGEN_MOVESIZE_B, 2);   *d++ = 0x820a;

    /*for (t = 0; t < 26; t++)
    *d++ = CGEN_OPCODE_NOP;*/

    if (sys.isMegaSTe == false)
        *d++ = CGEN_OPCODE_NOP;

    *d++ = CGEN_OPCODE_ADD_W_Dx_0Ay0(1, 3);                      /* blitter source low      add.w   d1,(a3)         12 */
    *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0(CGEN_MOVESIZE_W, 3, 4);      /* blitter vsize           move.w  d3,(a4)         8  */
    *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0(CGEN_MOVESIZE_W, 2, 6);      /* blitter start           move.w  d2,(a6)         8  preload $C002 in d2 instead of 2 to save 1 register (preshift color by 2 to compensate) */
    
    return d;
}

static void interludeGenerateFullscreenCode(Interlude* this, s32 size_)
{
    u16* d = this->gencode;
  
    /*
    d1  0           temp
    d2  $C002       overscan value + blitter start 

    a0              flexi scroll source
    a1              flexi scroll dest
    a2              briks colors list
    a3  $ffff8a26   blitter address LSB
    a4  $ffff8a38   blitter height
    a5              lines offset list
    a6  $ffff8a3C   blitter start register
    a7  $ffff8207
    */

    *d++ = CGEN_OPCODE_MOVEQ_L_X_Dy(0,1);

    *d++ = CGEN_OPCODE_LEA_SHORTADR_Ay(6);
    *d++ = 0x8250;

    *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0_M(CGEN_MOVESIZE_L,0,6);
    *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0_M(CGEN_MOVESIZE_L,0,6);
    *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0_M(CGEN_MOVESIZE_L,0,6);
    *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0_M(CGEN_MOVESIZE_L,0,6);

    *d++ = CGEN_OPCODE_LEA_SHORTADR_Ay(6);
    *d++ = 0x8A3C;

    d = interludeGenerateFullscreenScanlineType1(this, d, 227);
    d = interludeGenerateFullscreenScanlineType2(this, d);
    d = interludeGenerateFullscreenScanlineType1(this, d, 27);

    *d++ = CGEN_OPCODE_MOVEQ_L_X_Dy(0,1);
    *d++ = CGEN_OPCODE_LEA_SHORTADR_Ay(6);
    *d++ = 0x8240;

    *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0_P(CGEN_MOVESIZE_L,1,6);
    *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0_P(CGEN_MOVESIZE_L,1,6);
    *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0_P(CGEN_MOVESIZE_L,1,6);
    *d++ = CGEN_OPCODE_MOVE_Dx_0Ay0_P(CGEN_MOVESIZE_L,1,6);

#   if INTERLUDE_ACTIVATE_FLEXI()
    d = interludeGenerateFlexiCode(this, d, 512*27, false);
#   endif

    *d++ = CGEN_OPCODE_RTS;

    {
        s32 size = ((u8*)d - (u8*)this->gencode);
        ASSERT(size <= size_);
        TRAClogNumber10S(TRAC_LOG_FLOW, "codegensize", size, 6, '\n');
    }
}


static void interludInitBriks(u16* data_, u16* briks_)
{
    u16* s = data_;
    u16* d = briks_;
    u16  t;


    STDmset(d, 0UL, 24 * sizeof(u16));    d += 24;

    for (t = 1; t < INTERLUDE_HEIGHT; t++)
    {
        STDmcpy2(d, s, 8 * sizeof(u16));    d += 8;
        STDmcpy2(d, s, 8 * sizeof(u16));    d += 8;
        STDmcpy2(d, s, 8 * sizeof(u16));    d += 8;

        s += 8;
    }
}


static u8 interludInitMove(u8* data_, u8* briksadresses_, bool invert_)
{
    s8*  s = (s8*)data_ + 2;
    u8*  d = briksadresses_;
    u8   nbsteps, step;
    u16  i;


    nbsteps = data_[0];

    ASSERT(INTERLUDE_BALLHEIGHT == data_[1]);

    for (step = 0 ; step <= nbsteps ; step++)
    {
        for (i = 0 ; i < (INTERLUDE_HEIGHT * 2 - INTERLUDE_BALLHEIGHT) ; i++)
        {
            d[i] = 48;
        }

        d += INTERLUDE_HEIGHT - INTERLUDE_BALLHEIGHT;

        if (step > 0)
        {
            s8 o1 = 0, o2;

            for (i = 0 ; i < (INTERLUDE_BALLHEIGHT - 1) ; i++)
            {
                s8 data = s[i];
                if (invert_)
                    data = -data;
                o2 = (data + 24) & 7;
                d[i] = 48 + ((o2 - o1) << 1);
                o1 = o2;
            }

            d[i] = 48 + ((-o2) << 1);

/*            {
                s16 offset = 0;

                for (i = 0 ; i < INTERLUDE_BALLHEIGHT+2 ; i++)
                {
                    printf ("%d: acc=%d acc-lineadr=%d inc=%d data=%d data&7=%d\n", i, offset, offset-48*i, d[i], s[i], s[i]&7);
                    offset += d[i];
                }
            }*/

            s += INTERLUDE_BALLHEIGHT;
        }

        d += INTERLUDE_HEIGHT;
    }

    return nbsteps;
}


static void interludeInitBitmapLinesOffsets(Interlude* this)
{    
    u8* p = this->frame2ndwords;
    u16 i;

    STDmset(this->frame2ndwords, 0UL, (u32)(INTERLUDE_HEIGHT*2 - INTERLUDE_BALLHEIGHT) * (u32)INTERLUDE_ROTATION_NBSTEPS);

    for (i = 0 ; i < INTERLUDE_ROTATION_NBSTEPS ; i++)
    {
        u16 index = (INTERLUDE_ROTATION_NBSTEPS * INTERLUDE_ROTATION_NBSTEPS - (INTERLUDE_ROTATION_NBSTEPS - i) * (INTERLUDE_ROTATION_NBSTEPS - i)) >> 3;
        s16 cos = PCENDIANSWAP16(this->sin[index + 128]);
        s16 h;
        u16 offset;
        u16 t;
        u16 sline;
        u32 rest;
        s16 inc;
        s16 incr = 1;
        u16 acc = 0;


        /*printf ("%d \n", index);*/

        if (cos >= 0)
        {
            h = (s16)(STDmulu(cos+1, INTERLUDE_BALLHEIGHT) >> 15);
        }
        else
        {
            h = (s16)(STDmuls(cos, INTERLUDE_BALLHEIGHT) >> 15);
            h = -h;
        }

        if (h == 0)
        {
            cos = 0;
            h++;
        }

        rest = STDdivs(INTERLUDE_BALLHEIGHT, h);
        inc = (s16) rest;
        rest >>= 16;

        if (cos >= 0)
        {
            sline = 0;
        }
        else
        {
            sline = INTERLUDE_BALLHEIGHT - 1;
            inc = -inc;
            incr = -1;
        }

        offset = (INTERLUDE_BALLHEIGHT - h) >> 1;

        for (t = offset ; t < (INTERLUDE_BALLHEIGHT - offset) ; t++)
        {
            p[t + INTERLUDE_HEIGHT - INTERLUDE_BALLHEIGHT] = sline + 1;
            sline += inc;
            acc += (u16)rest;
            if (acc >= h)
            {
                acc -= h;
                sline += incr;
            }
        }

        p += INTERLUDE_HEIGHT*2 - INTERLUDE_BALLHEIGHT;
    }
}


static u16 initText(u8* font_, u8* text_)
{
    u16 len = STDstrLen(Interlude_g_text);
    u16 t;
    u16* d = (u16*) text_;

    ASSERT (INTERLUDE_SOURCE_PITCH >= len*2*2); /* two bytes by 16 x 16 char */
        
    STDmset(text_, 0UL, (u32)INTERLUDE_SOURCE_PITCH * (u32)INTERLUDE_SOURCE_HEIGHT * 16UL);

    for (t = 0; t < len*2; t++)
    {
        char c;
        u16 offset = 0;
        u16 i;
        u16* s;
        u16* d2 = d;

        if (t < len)
            c = Interlude_g_text[t];
        else
            c = Interlude_g_text[t - len];

        if ((c >= 'A') && ((c <= 'T')))
            offset = (c - 'A') << 1;
        else if ((c >= 'U') && ((c <= 'Z')))
            offset = ((c - 'U') << 1) + 17 * 40;
        else if ((c >= '0') && ((c <= '9')))
            offset = ((c - '0' + 6) << 1) + 17 * 40;
        else if (c == '.')
            offset = (16 << 1) + 17 * 40;
        else if (c == '\'')
            offset = (17 << 1) + 17 * 40;
        else if (c == ':')
            offset = (18 << 1) + 17 * 40;
        else if (c == ' ')
            offset = (19 << 1) + 17 * 40;

        s = (u16*)(font_ + offset);

        for (i = 0; i < 16; i++)
        {
            *d2 = *s;

            s  += 20;
            d2 += INTERLUDE_SOURCE_PITCH >> 1;
        }        

        d++;
    }

    for (t = 1; t < 16; t++)
    {
        u16 i;
        
        d = (u16*)text_;

        for (i = 1 ; i < ((INTERLUDE_SOURCE_PITCH >> 1) * INTERLUDE_SOURCE_HEIGHT) ; i++)
        {
            d[(INTERLUDE_SOURCE_PITCH >> 1) * INTERLUDE_SOURCE_HEIGHT]  = *d << 1;
            d[(INTERLUDE_SOURCE_PITCH >> 1) * INTERLUDE_SOURCE_HEIGHT] |= (d[1] & 0x8000) != 0;

            d++;
        }

        text_ += INTERLUDE_SOURCE_PITCH * INTERLUDE_SOURCE_HEIGHT;
    }

    return len * 16;
}

static void InterludeInitBackground(Interlude* this)
{
    u16* p = (u16*)this->framebuffer;
    u32 t;

    for (t = 0; t < (u32)(INTERLUDE_HEIGHT) * (u32)INTERLUDE_PITCH * 2UL; t += 8UL)
    {
        *p++ = 0x5555; /* 01010101 */
        *p++ = 0x3333; /* 00110011 */
        *p++ = 0x0F0F; /* 00001111 */
        *p++ = 0xFFFF;
    }
}


void InterludeEntry (FSM* _fsm)
{
    EMUL_STATIC Interlude* this;
    u8* temp;
    u8* temp2;
    u32 asmbufsize = LOADmetadataOriginalSize(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_______OUTPUT_RELAPSE_INTERLUD_ARJX) ;
    u32 samplesize = LOADmetadataOriginalSize(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTERLUD_ART_ARJX);
    CONST u32 alignedbuffersize = 
        (65536UL * 2UL)                         +   /* framebuffer size  */
        (u32)(3*8*INTERLUDE_HEIGHT*sizeof(u16)) +   /* briks buffer size */
        65536UL;                                    /* overhead to align */

    EMUL_BEGIN_ASYNC_REGION
        
    ASSERT(samplesize < 0x10000UL);

    g_screens.interlude = this = MEM_ALLOC_STRUCT( &sys.allocatorMem, Interlude );
    DEFAULT_CONSTRUCT(this);

    this->alignedbuffer  = (u8*)  MEM_ALLOC ( &sys.allocatorMem, alignedbuffersize);
    this->asmbuf         =        MEM_ALLOC ( &sys.allocatorMem, asmbufsize);
    this->sample         =        MEM_ALLOC ( &sys.allocatorMem, samplesize );
    this->gencode = (u16*) MEM_ALLOC ( &sys.allocatorMem, INTERLUDE_GENCODE_SIZE );
    this->briksmoveleft  = (u8*)  MEM_ALLOC ( &sys.allocatorMem, (INTERLUDE_HEIGHT*2-INTERLUDE_BALLHEIGHT)*(INTERLUDE_MOVE_MAXNBSTEPS+1));
    this->briksmoveright = (u8*)  MEM_ALLOC ( &sys.allocatorMem, (INTERLUDE_HEIGHT*2-INTERLUDE_BALLHEIGHT)*(INTERLUDE_MOVE_MAXNBSTEPS+1));
    this->frame2ndwords  = (u8*)  MEM_ALLOC ( &sys.allocatorMem, (u32)(INTERLUDE_HEIGHT*2-INTERLUDE_BALLHEIGHT)*(u32)INTERLUDE_ROTATION_NBSTEPS);
    this->text           = (u8*)  MEM_ALLOC ( &sys.allocatorMem, (u32)INTERLUDE_SOURCE_PITCH * (u32)INTERLUDE_SOURCE_HEIGHT * 16UL);

    this->framebuffer    = (u8*)(((u32)this->alignedbuffer+0xFFFFUL) & 0xFFFF0000UL);
    this->briks          = (u16*)(this->framebuffer + 65536UL * 2UL);

    temp  = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, LOADresourceRoundedSize (&RSC_RELAPSE1, RSC_RELAPSE1_INTERLUD_ART_ARJX) ); 
    temp2 = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, LOADresourceRoundedSize (&RSC_RELAPSE1, RSC_RELAPSE1_______OUTPUT_RELAPSE_INTERLUD_ARJX) ); 

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(3);

    g_screens.persistent.loadRequest[0] = LOADdata (&RSC_RELAPSE1, RSC_RELAPSE1_INTERLUD_ART_ARJX, temp, LOAD_PRIORITY_INORDER);
    g_screens.persistent.loadRequest[1] = LOADdata (&RSC_RELAPSE1, RSC_RELAPSE1_______OUTPUT_RELAPSE_INTERLUD_ARJX, temp2, LOAD_PRIORITY_INORDER);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[0]);
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(2);

    ASSERT(LOADmetadataSize(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTERLUD_SIN_BIN) == 1024);
    STDmcpy2 (this->sin, temp + LOADmetadataOffset(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTERLUD_SIN_BIN), 1024);
    STDmcpy2 (this->sin + (1024 / sizeof(s16)), this->sin, 1024 / 4);

    RELAPSE_UNPACK (this->sample, temp);
    this->samplesize = (u16) samplesize;
    aMX2MdpcmTOpcm(this->sample, this->samplesize);

    STDfastmset(this->alignedbuffer, 0UL, alignedbuffersize);

    ASSERT(INTERLUDE_GENCODE_SIZE >= LOADmetadataOriginalSize(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTERLUD_BRIK_ARJX));
    RELAPSE_UNPACK (this->framebuffer, temp + LOADmetadataOffset(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTERLUD_BRIK_ARJX));
    interludInitBriks((u16*)this->framebuffer, this->briks);

    ASSERT(INTERLUDE_GENCODE_SIZE >= LOADmetadataOriginalSize(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTERLUD_MOVE_ARJX));
    RELAPSE_UNPACK (this->framebuffer, temp + LOADmetadataOffset(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTERLUD_MOVE_ARJX));

    this->nbsteps = interludInitMove((u8*)this->framebuffer, this->briksmoveleft, true);
    ASSERT(this->nbsteps <= INTERLUDE_MOVE_MAXNBSTEPS);
    interludInitMove((u8*)this->framebuffer, this->briksmoveright, false);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[1]);
    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(1);

    STDmset (this->asmbuf, 0UL, asmbufsize);
    RELAPSE_UNPACK (this->asmbuf, temp2 + LOADmetadataOffset(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_______OUTPUT_RELAPSE_INTERLUD_ARJX));
    SYSrelocate (this->asmbuf);

    this->soundssize[0] = (u16) LOADmetadataSize(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTERLUD_YMSOUND0_YM);
    this->sounds[0] = (u8*) MEM_ALLOC (&sys.allocatorMem, this->soundssize[0]);
    STDmcpy(this->sounds[0], temp + LOADmetadataOffset(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTERLUD_YMSOUND0_YM), this->soundssize[0]);

    this->soundssize[1] = (u16) LOADmetadataSize(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTERLUD_YMSOUND1_YM);
    this->sounds[1] = (u8*) MEM_ALLOC (&sys.allocatorMem, this->soundssize[1]);
    STDmcpy(this->sounds[1], temp + LOADmetadataOffset(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTERLUD_YMSOUND1_YM), this->soundssize[1]);

    RELAPSE_UNPACK (temp, temp2 + LOADmetadataOffset(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_INTERLUD_FONT_ARJX));
    this->scrollen = initText(temp, this->text);

    STDfastmset(this->framebuffer, 0UL, INTERLUDE_FRAMEBUFFER_SIZE);
    interludeGenerateFullscreenCode(this, INTERLUDE_GENCODE_SIZE);
    
    InterludeInitBackground(this);

    interludeInitBitmapLinesOffsets(this);

    this->asmimport = interludeAsmImport(this->asmbuf);
    
    this->asmimport->gencode        = (DYNevent)this->gencode;
    this->asmimport->colorsbuffer   = this->briks;  
    this->asmimport->briksaddresses = this->briksmoveleft;
    this->asmimport->frame2nbwords  = this->frame2ndwords;
    this->asmimport->plane3colors   = 0x0FF00FF0UL;
    this->asmimport->flexisource    = this->text;
    this->asmimport->flexidest      = this->framebuffer+6;
    this->asmimport->tun1           = 8;
    this->tun2                      = 12;

    this->shokadr   = this->briksmoveleft;

    this->asmimport->init(NULL);

    RELAPSE_INIT_PROGRESS______________________________________________________________________________________________________(0);

#   ifndef __TOS__
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRAClog(TRAC_LOG_MEMORY,"Intro memallocator dump", '\n');
        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif


    MEM_FREE (&sys.allocatorMem, temp2);
    MEM_FREE (&sys.allocatorMem, temp);

    EMUL_DELAY(100);

    FSMgotoNextState (_fsm);

    EMUL_END_ASYNC_REGION
}


void InterludePostInit (FSM* _fsm)
{
    Interlude* this = g_screens.interlude;
    u32 sampleadr = (u32) this->sample;

    ScreensSetVideoMode (HW_VIDEO_MODE_4P, 0, 0);

    SYSwriteVideoBase((u32)(this->framebuffer) + 256UL * (u32)(INTERLUDE_BALLHEIGHT + 2));

    this->ballx = 200;
    this->ballincx = 4;

    this->bally = 50;
    this->ballincy = 1;

    *HW_DMASOUND_STARTADR_H = (u8)(sampleadr >> 16);
    *HW_DMASOUND_STARTADR_M = (u8)(sampleadr >> 8);
    *HW_DMASOUND_STARTADR_L = (u8) sampleadr;
 
    sampleadr += this->samplesize - 320;

    *HW_DMASOUND_ENDADR_H = (u8)(sampleadr >> 16);
    *HW_DMASOUND_ENDADR_M = (u8)(sampleadr >> 8);
    *HW_DMASOUND_ENDADR_L = (u8) sampleadr;

    *HW_DMASOUND_MODE = HW_DMASOUND_MODE_MONO | HW_DMASOUND_MODE_25033HZ;
    *HW_DMASOUND_CONTROL = HW_DMASOUND_CONTROL_PLAYLOOP;

    *HW_VIDEO_OFFSET = (256 - 224) / 2;

#if INTERLUDE_TEST()
    {
        u16 t;
    
        for (t = 0 ; t < 32 ; t++)
            SYSfastPrint("ABCDEFGHIJKLMNOPQRSTUVWXYZ ABCDEFGHIJKLMNOPQRSTUVWXYZ", this->framebuffer + 6 + INTERLUDE_PITCH * 8 * t, INTERLUDE_PITCH, 8, (u32) &SYSfont);
    }
#endif

    SYSvblroutines[0] = this->asmimport->vbl;

    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState(_fsm);
}


void InterludeActivity (FSM* _fsm)
{       
    Interlude* this = g_screens.interlude;    
    u8* backbuffer = this->framebuffer;
    u8 pixoffset = (u8)this->ballx & 15;


    IGNORE_PARAM(_fsm);

    *HW_VIDEO_PIXOFFSET = pixoffset;
    if (pixoffset == 0)
    {
        this->asmimport->tun2 = this->tun2;
        *HW_VIDEO_OFFSET = (256 - 224) / 2;
    }
    else
    {
        this->asmimport->tun2 = this->tun2 + 8;
        *HW_VIDEO_OFFSET = (256 - 224) / 2 - 4;
    }

    pixoffset &= 7;
    if (pixoffset == 0)
        this->asmimport->colorsbuffer = this->briks;
    else
        this->asmimport->colorsbuffer = this->briks + 8 - pixoffset;

    if (this->flip)
        backbuffer += 0x10000UL;

    this->asmimport->framebuffer   = backbuffer + ((this->ballx & 0xFFF0) >> 1);
    this->asmimport->frame2nbwords = this->frame2ndwords + this->rotationindex + this->bally;
    this->asmimport->flexisource   = this->text + STDmulu(this->scrollpos & 15, INTERLUDE_SOURCE_PITCH * INTERLUDE_SOURCE_HEIGHT) + ((this->scrollpos & 0xFFF0) >> 3);
    this->asmimport->flexidest     = backbuffer + 6;
    this->asmimport->briksaddresses = this->shokadr;

    this->scrollpos++;
    if (this->scrollpos >= this->scrollen)
        this->scrollpos = 0;

    if (this->shokcount > 0)
    {
        this->shokcount--;
        this->shokadr += INTERLUDE_HEIGHT * 2 - INTERLUDE_BALLHEIGHT;
    }
    else
    {
        this->shokadr = this->briksmoveleft;
    }

    if (this->rotationcount > 0)
    {
        this->rotationcount--;
        this->rotationindex += INTERLUDE_HEIGHT * 2 - INTERLUDE_BALLHEIGHT;
    }
    else
    {
        this->rotationindex = 0;
    }

    this->ballx += this->ballincx;
    if (this->ballx == 0)
    {
        this->ballincx = -this->ballincx;
        this->shokcount = this->nbsteps;
        this->shokadr   = this->briksmoveleft + this->bally;

        this->soundcurrent   = this->sounds[0];
        this->soundend       = this->sounds[0] + this->soundssize[0];
        this->soundplayframe = 0;

    }
    else if (this->ballx == (408 - INTERLUDE_BALLHEIGHT))
    {
        this->ballincx = -this->ballincx;
        this->shokcount = this->nbsteps;
        this->shokadr   = this->briksmoveright + this->bally;

        this->soundcurrent   = this->sounds[0];
        this->soundend       = this->sounds[0] + this->soundssize[0];
        this->soundplayframe = 0;
    }

    this->bally += this->ballincy;
    if ((this->bally == 0) || (this->bally == (256 - INTERLUDE_BALLHEIGHT)))
    {
        this->ballincy = -this->ballincy;

        this->soundcurrent   = this->sounds[1];
        this->soundend       = this->sounds[1] + this->soundssize[1];
        this->soundplayframe = 0;
        
        this->rotationcount = INTERLUDE_ROTATION_NBSTEPS-1;
        this->rotationindex = 0;
    }

    *HW_VIDEO_BASE_H = (u8)((u32)backbuffer >> 16);
    this->flip = !this->flip;

    if (this->soundcurrent != NULL)
    {
        u8* p = this->soundcurrent;
        u16 frame = (p[0] << 8) | p[1];

        if (frame == this->soundplayframe)
        {
            p += 2;

            p = this->asmimport->ymdecode(p);

            if (p >= this->soundend)
                this->soundcurrent = NULL;

            this->soundcurrent = p;
        }

        this->soundplayframe++;
    }

#   if RELAPSE_DEV()
    if (g_screens.justpressed)
    {
        switch (g_screens.scancodepressed)
        {
        case HW_KEY_LEFT:
            if (this->asmimport->tun1 > 0)
            {
                *HW_COLOR_LUT = 0x70;
                this->asmimport->tun1--;
            }
            break;

        case HW_KEY_RIGHT:
            *HW_COLOR_LUT = 0x70;
            this->asmimport->tun1++;
            break;

        case HW_KEY_DOWN:
            if (this->tun2 > 0)
            {
                *HW_COLOR_LUT = 0x70;
                this->tun2--;
            }
            break;

        case HW_KEY_UP:
            if (this->tun2 < 63)
            {
                *HW_COLOR_LUT = 0x70;
                this->tun2++;
            }
            break;

        case HW_KEY_TAB:
            this->shokcount = this->nbsteps;
            this->shokadr   = this->briksmoveleft + 50;
            break;
        }
    }
#   endif

    if (g_screens.persistent.menumode == false)
    {
        this->framecount++;
        if (this->framecount >= 1250)
            g_screens.next = true;
    }

    if (g_screens.next)
    {
        SYSvblroutines[0] = SYSvblend;
        SYSvblroutines[1] = SYSvblend;
        SYSvblroutines[2] = SYSvblend;

        FSMgotoNextState (&g_stateMachineIdle);
        FSMgotoNextState (_fsm);
    }

    STDstop2300();  /* I don't know why it does not work on Hatari without this */

    /* *HW_COLOR_LUT = 0x70; */
}


void InterludeExit (FSM* _fsm)
{
    Interlude* this = g_screens.interlude;


    ScreenFadeOutSound();

    RASnextOpList = NULL;

    *HW_DMASOUND_CONTROL = HW_DMASOUND_CONTROL_OFF;

    HW_YM_SET_REG(HW_YM_SEL_IO_AND_MIXER, 0xFF);
    HW_YM_SET_REG(HW_YM_SEL_LEVELCHA,     0   );
    HW_YM_SET_REG(HW_YM_SEL_LEVELCHB,     0   );
    HW_YM_SET_REG(HW_YM_SEL_LEVELCHC,     0   );

    MEM_FREE ( &sys.allocatorMem, this->briksmoveleft );
    MEM_FREE ( &sys.allocatorMem, this->briksmoveright );
    MEM_FREE ( &sys.allocatorMem, this->asmbuf );
    MEM_FREE ( &sys.allocatorMem, this->text );
    MEM_FREE ( &sys.allocatorMem, this->sample );
    MEM_FREE ( &sys.allocatorMem, this->alignedbuffer );
    MEM_FREE ( &sys.allocatorMem, this->gencode );
    MEM_FREE ( &sys.allocatorMem, this->frame2ndwords );
    MEM_FREE ( &sys.allocatorMem, this->sounds[0] );
    MEM_FREE ( &sys.allocatorMem, this->sounds[1] );

    MEM_FREE(&sys.allocatorMem, this);
    g_screens.interlude = NULL;

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    ScreenNextState(_fsm);
}

