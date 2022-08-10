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
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\BITMAP.H"
#include "DEMOSDK\HARDWARE.H"

#include "EXTERN\ARJDEP.H"

#include "FX\STARFILD\STARFILD.H"

#define STAR_NBPLANES       3
#define STAR_RND_STAR_X     512
#define STAR_RND_STAR_Y     497
#define STAR_ACTIVATE_COPY  1


ASMIMPORT u16 STARpix0;
ASMIMPORT u16 STARpix1;
ASMIMPORT u16 STARpixloop;
ASMIMPORT u16 STARinc;
ASMIMPORT u8  STARreset;
ASMIMPORT u8  STARloop;
ASMIMPORT u8  STARpixdraw[32767];

void STARasminit (STARparam* _param) PCSTUB;

static void STARinitPixdraw (STARparam* _param)
{
    u16* code    = (u16*) STARpixdraw;
    u16* offsets = _param->pixdrawoffset;
    s16  offset  = 0;
    u16  t, c;


    for (t = 0 ; t < STAR_WIDTH ; t++)
    {
        u16 planeoffset = (t & 0xFFF0) >> 1;
        u16 opcodeindex = t & 7;


        planeoffset += (t & 8) != 0;

        *offsets++ = offset;	/* do not have a color 0 draw routine */

        for (c = 1 ; c < 7 ; c++)
        {
            u16 m;
            s16 firstplanepos = -1;
            u16 currentoffset = planeoffset;

            *offsets++ = offset;

            for (m = 1 ; m < 8 ; m <<= 1)
            {
                if ( c & m )
                {
                    if ( firstplanepos == -1 )
                    {
                        if (currentoffset != 0)
                        {
                            *code++ = (&STARinc)[0];            /* lea planeoffset(a2),a2 */
                            *code++ = currentoffset;
                            offset += 4; 
                        }

                        *code++ = (&STARinc)[2];                /* move.l a2,(a3)+ */
                        *code++ = (&STARpix0)[ opcodeindex ];   /* or of bset dn,(a2) */
                        offset += 4; 

                        firstplanepos = currentoffset;
                    }
                    else
                    {
                        *code++ = (&STARpix1)[ opcodeindex  << 1 ];
                        *code++ = currentoffset - firstplanepos;
                        offset += 4;
                    }
                }

                currentoffset += 2;
            }

            *code++ = (&STARpixloop)[0];        /* dbra */

            {
                s32 branch = &STARloop - (u8*)code;
                *code++ = (u16)branch;
#               ifdef __TOS__
                ASSERT((branch >= -32768L) && (branch < 32768L));
#               endif
            }

            *code++ = (&STARpixloop)[2];        /* rts */
            offset += 6;
        }

        *offsets++ = offsets[-1];   /* do not have a color 7 draw routine => use color 6 */
    }

    ASSERT(((u8*) code - STARpixdraw) < 32768L);

    {
        s32 jmpToReset = (&STARreset) - STARpixdraw;

#       ifdef __TOS__
        ASSERT((jmpToReset >= -32768L) && (jmpToReset < 32768L));
#       endif

        *offsets++ = (u16) jmpToReset;
    }
}


static void STARinitField (STARparam* _param)
{
    s16 t,i;

    for (i = 0 ; i <= STAR_ZMAX ; i++)
    {
        s16* prex = &(_param->prex[i]);
        s16* prey = &(_param->prey[i]);
        u16  height = _param->height;


        for (t = 0 ; t < height ; t++)
        {
            s16 y = ((t - (height >> 1)) * STAR_PERSPEC / ((STAR_ZMAX - i) + STAR_PERSPEC)) + (height >> 1);

            /*y = t;*/

            *prey = y * _param->pitch;
            /* prey = (s16*) (((u8*)prey) + (1 << INTRO_ZSHIFT) * sizeof(s16)); */
            prey += 1L << STAR_ZSHIFT;
        }

        for (t = 0 ; t < STAR_WIDTH ; t++)
        {
            s16 x     = ((t - (STAR_WIDTH >> 1)) * STAR_PERSPEC / ((STAR_ZMAX - i) + (STAR_PERSPEC * 2 / 3))) + (STAR_WIDTH >> 1);
            u16 color = (i / 21) + 1;

            /*x = i;*/

            if (((x >= 0) && (x < STAR_WIDTH)) && (i != STAR_ZMAX))
            {
                *prex = _param->pixdrawoffset[(x << STAR_NBPLANES) + color];
            }
            else
            {
                *prex = _param->pixdrawoffset[STAR_WIDTH << STAR_NBPLANES];
            }

            prex += 1L << STAR_ZSHIFT;
        }
    }
}



u8* STARstaticIniField1P (MEMallocator* _allocator)
{
    s16 t,i;
    u16 size = (STAR_ZMAX - STAR_INCMAX + 1) * ((STAR_WIDTH >> 1) + 1);

    u8* buffer = (u8*) MEM_ALLOC(_allocator, size);
    u8* p = buffer;

    for (i = 0; i < (STAR_ZMAX - STAR_INCMAX + 1); i++)
    {
        s16 ir = (STAR_ZMAX - STAR_INCMAX - i) + STAR_PERSPEC;

        for (t = 0; t <= ((STAR_WIDTH >> 1) * STAR_PERSPEC); t += STAR_PERSPEC)
        {
            *p++ = t / ir;
        }
    }

    return buffer;
}


void STARiniField1P (STARparam* _param, u8* _starfielddiv)
#ifdef __TOS__
;
#else
{
    u16 height = _param->height;
    s16 t,i;


    for (i = 0; i < STAR_WIDTH; i++)
    {
        u8 bitshift = 7 - (i & 7);
        u8 offset   = (i & 0xFFF0) >> 1;

        if ((i & 15) >= 8)
            offset++;

        _param->xval[i] = (bitshift << 8) | offset;
    }

    for (i = 0 ; i < (STAR_ZMAX - STAR_INCMAX + 1) ; i++)
    {
        s16* prex = &(_param->prex[i]);
        s16* prey = &(_param->prey[i]);

#       if STAR_ACTIVATE_COPY          
        s16* copyposx = &(_param->copyposx[i]);
        u8*  copyposy = &(_param->copyposy[i]);
#       endif       

        for (t = height >> 1 ; t > 0; t--)
        {
            s16 y = -(s16) _starfielddiv[t] + (height >> 1);

            *prey = _param->pitchmul[y];

#           if STAR_ACTIVATE_COPY
            *copyposy = (u8)y;
            copyposy += 1L << STAR_ZSHIFT;
#           endif

            prey += 1L << STAR_ZSHIFT;
        }

        for (t = 0 ; t < (height >> 1) ; t++)
        {
            s16 y = (s16) _starfielddiv[t] + (height >> 1);

            *prey = _param->pitchmul[y];

#           if STAR_ACTIVATE_COPY
            *copyposy = (u8) y;
            copyposy += 1L << STAR_ZSHIFT;
#           endif

            prey += 1L << STAR_ZSHIFT;
        }

        for (t = STAR_WIDTH >> 1 ; t > 0 ; t--)
        {
            s16 x = - (s16) _starfielddiv[t] + (STAR_WIDTH >> 1);

#           ifndef __TOS__
            ASSERT ((x >= 0) && (x < STAR_WIDTH));
#           endif

            *prex = _param->xval[x];
            prex += 1L << STAR_ZSHIFT;

#           if STAR_ACTIVATE_COPY
            *copyposx = x;
            copyposx += 1L << STAR_ZSHIFT;
#           endif
        }

        for (t = 0 ; t < (STAR_WIDTH >> 1) ; t++)
        {
            s16 x = (s16) _starfielddiv[t] + (STAR_WIDTH >> 1);

#           ifndef __TOS__
            ASSERT ((x >= 0) && (x < STAR_WIDTH));
#           endif

            *prex = _param->xval[x];
            prex += 1L << STAR_ZSHIFT;

#           if STAR_ACTIVATE_COPY
            *copyposx = x;
            copyposx += 1L << STAR_ZSHIFT;
#           endif
        }

        _starfielddiv += (STAR_WIDTH >> 1) + 1;
    }
    
    for (i = (STAR_ZMAX - STAR_INCMAX + 1) ; i <= STAR_ZMAX ; i++)
    {
        s16* prex = &(_param->prex[i]);
        s16* prey = &(_param->prey[i]);
            
        for (t = 0; t < height; t++)
        {
            *prey = 0;
            prey += 1L << STAR_ZSHIFT;
        }

        for (t = 0; t < STAR_WIDTH; t++)
        {
            *prex = 0x8000;
            prex += 1L << STAR_ZSHIFT;
        }
    }
}
#endif

static void STARinitStarPositions (STARparam* _param)
{
    u16  height = _param->height;
    u32* z = _param->starz;
    u16  t;

    for (t = 0 ; t < _param->nbstars ; t++)
    {
        s32 xpos = STDifrnd() % STAR_WIDTH;
        s32 ypos = STDifrnd() % height;
        u32 zpos = STDifrnd() & STAR_ZMAX;

        *z++ = (u32) (_param->prex + (xpos << STAR_ZSHIFT) + zpos);
        *z++ = (u32) (_param->prey + (ypos << STAR_ZSHIFT) + zpos);
    }
}


static void STARinitRandom (STARparam* _param)
{
    u16 t, v;
    u16 height = _param->height;

    for (t = 0 ; t < (STAR_RND_STAR_X - 1) ; t++)
    {
        do
        {
            v = STDifrnd() & 0x1FF;
        }
        while (v >= STAR_WIDTH);

        _param->rndx[t] = (u8*)_param->prex + ((v << STAR_ZSHIFT) * sizeof(u16));
    }

    _param->rndx[t] = NULL;

    for (t = 0 ; t < (STAR_RND_STAR_Y - 1) ; t++)
    {
        do
        {
            v = STDifrnd() & 0xFF;
        }
        while (v >= height);

        _param->rndy[t] = (u8*)_param->prey + ((v << STAR_ZSHIFT) * sizeof(u16));
    }

    _param->rndy[t] = NULL;
}


void STARinit (MEMallocator* _allocator, STARparam* _param)
{
    u16 nbstars = _param->nbstars;


    _param->starx           = (s16*) MEM_ALLOC(_allocator, sizeof(s16) * nbstars);
    _param->stary           = (s16*) MEM_ALLOC(_allocator, sizeof(s16) * nbstars);
    _param->starz           = (u32*) MEM_ALLOC(_allocator, sizeof(u32) * nbstars * 2);
    _param->pixdrawoffset   = (u16*) MEM_ALLOC(_allocator, sizeof(u16) * ((STAR_WIDTH << STAR_NBPLANES) + 1));
    _param->erasebuffers[0] = (u8*)  MEM_ALLOC(_allocator, sizeof(void*) * nbstars * 2);
    _param->erasebuffers[1] = _param->erasebuffers[0] + (sizeof(void*) * nbstars);

    STDmset (_param->erasebuffers[0], 0UL, 4);
    STDmset (_param->erasebuffers[1], 0UL, 4);

    _param->prex    = (s16*)   MEM_ALLOC(_allocator, sizeof(s16)   * ((u32)STAR_WIDTH * (u32)(STAR_ZMAX + 1)) );
    _param->prey    = (s16*)   MEM_ALLOC(_allocator, sizeof(s16)   * (_param->height * (STAR_ZMAX + 1)) );
    _param->rndx    = (void**) MEM_ALLOC(_allocator, sizeof(void*) * STAR_RND_STAR_X );
    _param->rndy    = (void**) MEM_ALLOC(_allocator, sizeof(void*) * STAR_RND_STAR_Y );

    _param->currentrndx = _param->rndx;
    _param->currentrndy = _param->rndy;

    STARinitPixdraw       (_param);
    STARinitField         (_param);
    STARinitRandom        (_param);
    STARinitStarPositions (_param);

    STARasminit (_param);
}


void STARinit1P (MEMallocator* _allocator, STARparam* _param, u8* _starfielddiv)
{
    u16 nbstars = _param->nbstars;

    ASSERT((nbstars % 6) == 0);

    _param->starx           = (s16*) MEM_ALLOC(_allocator, sizeof(s16) * nbstars);
    _param->stary           = (s16*) MEM_ALLOC(_allocator, sizeof(s16) * nbstars);
    _param->starz           = (u32*) MEM_ALLOC(_allocator, sizeof(u32) * nbstars * 2);
    _param->pixdrawoffset   = NULL;
    _param->erasebuffers[0] = (u8*)  MEM_ALLOC(_allocator, sizeof(void*) * nbstars * 2);
    _param->erasebuffers[1] = _param->erasebuffers[0] + (sizeof(void*) * nbstars);

    STDmset (_param->erasebuffers[0], 0UL, 4);
    STDmset (_param->erasebuffers[1], 0UL, 4);

    _param->prex    = (s16*)   MEM_ALLOC(_allocator, sizeof(s16)    * ((u32)STAR_WIDTH * (u32)(STAR_ZMAX + 1)) );
    _param->prey    = (s16*)   MEM_ALLOC(_allocator, sizeof(s16)    * (_param->height * (STAR_ZMAX + 1)) );
    _param->rndx    = (void**) MEM_ALLOC(_allocator, sizeof(void**) * STAR_RND_STAR_X );
    _param->rndy    = (void**) MEM_ALLOC(_allocator, sizeof(void**) * STAR_RND_STAR_Y );

    _param->xval    = (u16*)   MEM_ALLOC(_allocator, STAR_WIDTH * sizeof(u16));

    _param->currentrndx = _param->rndx;
    _param->currentrndy = _param->rndy;

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "STARiniField1P", '\n');

    STARiniField1P        (_param, _starfielddiv);

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "STARinitRandom", '\n');
    STARinitRandom        (_param);

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "STARinitStarPositions", '\n');
    STARinitStarPositions (_param);

    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "STARinit1P_End", '\n');
}



void STARerase (void* _erasebuffer, u16 _nbstars)
#ifdef __TOS__
    ;
#else
{
    u16** erasebuffer = (u16**) _erasebuffer;
    u16 t;

    _nbstars++;
    _nbstars *= 6;

    for (t = 0 ; t < _nbstars ; t++)
    {
        u16* p = *erasebuffer++;

        p[0] = 0;
        p[1] = 0;
        p[2] = 0;
    }
}
#endif


void STARerase1P (void* _erasebuffer, u16 _nbstars)
#ifdef __TOS__
;
#else
{
    u8** erasebuffer = (u8**) _erasebuffer;
    u16 t;

    _nbstars++;
    _nbstars *= 6;

    for (t = 0 ; t < _nbstars ; t++)
    {
        u8* p = *erasebuffer++;
        *p = 0;
    }
}
#endif


void STARdraw (void* _framebuffer, u32* z, u16 _nbstars, u32 _erasebuffer, STARparam* _param)
#ifdef __TOS__
;
#else
{
    s16* prex = _param->prex;
    s16* prey = _param->prey;
    void** erasebuffer = (void**) _erasebuffer;
    u16 t;
    u16 i;


    _nbstars++;

    for (t = 0 ; t < _nbstars ; t++)
    {
        u16* xt = (u16*) z[0];
        u16* yt = (u16*) z[1];

        u16 opcodeoffset = *xt;
        u16 yoffset      = *yt;

        z[0] += 2;
        z[1] += 2;

        for (i = 0 ; i <= (STAR_WIDTH << STAR_NBPLANES) ; i++)
        {
            if ( opcodeoffset == _param->pixdrawoffset[i] )
            {
                break;
            }
        }

        ASSERT(i <= (STAR_WIDTH << STAR_NBPLANES));

        if ( i == (STAR_WIDTH << STAR_NBPLANES) )
        {
            z[0] = (u32) *_param->currentrndx++;

            if ( z[0] == 0 )
            {
                _param->currentrndx = _param->rndx;
                z[0] = (u32) *_param->currentrndx++;
            }

            z[1] = (u32) *_param->currentrndy++;

            if ( z[1] == 0 )
            {
                _param->currentrndy = _param->rndy;
                z[1] = (u32) *_param->currentrndy++;
            }

            *erasebuffer++ = _framebuffer;
        }
        else
        {
            u16 x = i >> STAR_NBPLANES;
            u16 color = i & 7;
            u8* p = (u8*) _framebuffer;
            u16 pix;

            p += (x >> 1) & 0xF8;
            p += yoffset;

            *erasebuffer++ = p;

            pix = 0x8000 >> (x & 15);
            pix = PCENDIANSWAP16(pix);

            if (color & 1)
                ((u16*)p)[0] |= pix;

            if (color & 2)
                ((u16*)p)[1] |= pix;

            if (color & 4)
                ((u16*)p)[2] |= pix;
        }

        z += 2;
    }
}

#endif


void STARdraw1P (void* _framebuffer, u32* z, u16 _nbstars, u32 _erasebuffer, u32 _param)
#ifdef __TOS__
;
#else
{
    STARparam* param = (STARparam*) _param;
    s16* prex = param->prex;
    s16* prey = param->prey;
    void** erasebuffer = (void**) _erasebuffer;
    u16 t;


    _nbstars++;

    for (t = 0 ; t < _nbstars ; t++)
    {
        u16* xt = (u16*) z[0];
        u16 xpix     = *xt;

        z[0] += param->inc;
        z[1] += param->inc;

        if (xpix == 0x8000)
        {
            z[0] = (u32) *param->currentrndx++;

            if ( z[0] == 0 )
            {
                param->currentrndx = param->rndx;
                z[0] = (u32) *param->currentrndx++;
            }

            z[1] = (u32) *param->currentrndy++;

            if ( z[1] == 0 )
            {
                param->currentrndy = param->rndy;
                z[1] = (u32) *param->currentrndy++;
            }

            *erasebuffer++ = _framebuffer;
        }
        else
        {
            u8* p = (u8*) _framebuffer;
            s16* yt = (s16*) z[1];
            s16 yoffset  = *yt;

            p += xpix & 0xFF;
            p += yoffset;

            *erasebuffer++ = p;

            xpix >>= 8;
            ASSERT(xpix < 8);

            *p |= 1 << xpix;
        }

        z += 2;
    }
}

#endif


void STARshutdown (MEMallocator* _allocator, STARparam* _param)
{
    MEM_FREE(_allocator, _param->starx );
    MEM_FREE(_allocator, _param->stary );
    MEM_FREE(_allocator, _param->starz );
    if ( _param->pixdrawoffset != NULL )
        MEM_FREE(_allocator, _param->pixdrawoffset);
    MEM_FREE(_allocator, _param->erasebuffers[0]);
    MEM_FREE(_allocator, _param->prex );
    MEM_FREE(_allocator, _param->prey );
    MEM_FREE(_allocator, _param->rndx );
    MEM_FREE(_allocator, _param->rndy );
    if ( _param->xval != NULL )
        MEM_FREE(_allocator, _param->xval );
}