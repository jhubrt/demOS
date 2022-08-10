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
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"

#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"

#include "FX\VECTOR\VECTOR.H"


#define VEC_PC_FRAMEDELAY   1

/*----------------------------------------------------------
input dlist ---->
    u16 : nbedges - 1
    u16 (x, y) * nbedges
    
outputed dlist ----->
    u16 : minx, miny, maxx, maxy    
------------------------------------------------------------*/
#ifdef DEMOS_DEBUG
static u16* VECtraceList (u16* _p, VECscene* _scene)
{
/* -----------------------------------------------------------
    Frame display list structure :

    u16: minx, maxx, miny, maxy
    s16: nbedges-1 count

    Then nbedges one of these according to line type:

    DX > DY
    w: offset address | ((pitch inc < 0) ? $8000 : 0) | 0
    w: enter routine offset
    w: exit routine offset
    w: error increment  
    
	DX < DY
    w: offset address | ((pitch inc < 0) ? $8000 : 0) | 2
    b: rol bit number
    b: increment error
    w: vertical offset increment
    w: offset into routine

    HLINE
    w: offset address | ((pitch inc < 0) ? $8000 : 0) | 4
    b: start mask selection
    b: end mask selection
    w: nb words or 0 when same word

    D45
    w: offset address | ((pitch inc < 0) ? $8000 : 0) | 6
    b: bit num
    b: length
    w: routine offset
    -----------------------------------------------------------*/

    u16 polyindex;

    {
        static char trace[] = "\nVECtraceList --------------------------------------------------";
        STDuxtoa(&trace[30], (u32)_p, 8);
        TRAClog(TRAC_LOG_SPECIFIC, trace, '\n');
    }

    {
        static char trace[] = "                    ";
        STDuxtoa(&trace[0], *_p++, 4);
        STDuxtoa(&trace[5], *_p++, 4);
        STDuxtoa(&trace[10], *_p++, 4);
        STDuxtoa(&trace[15], *_p++, 4);
        TRAClog(TRAC_LOG_SPECIFIC, trace, '\n');
    }

    for (polyindex = 0 ; polyindex < _scene->nbPolygons ; polyindex++)
    {
        s16 nbedges = 1 + *_p++;

        {
            static char trace[] = "\nedges:          ";
            STDuxtoa(&trace[7], nbedges, 4);
            STDuxtoa(&trace[12], *(_scene->polygons[polyindex]), 4);
            TRAClog(TRAC_LOG_SPECIFIC, trace, '\n');
        }

        while (nbedges > 0)
        {
            u16 off_type_sign = *_p++;

            switch (off_type_sign & 6)
            {
            case 0: TRAClog(TRAC_LOG_SPECIFIC, "\nDX>DY", '\n'); break;
            case 2: TRAClog(TRAC_LOG_SPECIFIC, "\nDX<DY", '\n'); break;
            case 4: TRAClog(TRAC_LOG_SPECIFIC, "\nHLINE", '\n'); break;
            case 6: TRAClog(TRAC_LOG_SPECIFIC, "\nD45"  , '\n'); break;
            }

            TRAClogNumber(TRAC_LOG_SPECIFIC, "offset", off_type_sign & 0x7FF9, 4);

            TRAClog(TRAC_LOG_SPECIFIC, (off_type_sign & 0x8000) ? "inc-" : "inc+", '\n');

            switch (off_type_sign & 6)
            {
            case 0: 
                {
                    u16 enter_off = *_p++;
                    TRAClogNumber(TRAC_LOG_SPECIFIC, "enter_off", enter_off, 4);
                    ASSERT((enter_off & 1) == 0);
                }
                {
                    u16 exit_off = *_p++;
                    TRAClogNumber(TRAC_LOG_SPECIFIC, "exit_off", exit_off, 4);
                    ASSERT((exit_off & 1) == 0);
                }
                {
                    u16 error_inc = *_p++;
                    TRAClogNumber(TRAC_LOG_SPECIFIC, "error_inc", error_inc, 4);
                }
                break;

            case 2:            
                {
                    u16 bit = *((u8*)_p);
                    TRAClogNumber(TRAC_LOG_SPECIFIC, "rol_bit", bit, 2);
                    ASSERT(bit < 16);
                    TRAClogNumber(TRAC_LOG_SPECIFIC, "error_inc", ((u8*)_p)[1], 2);
                }
                _p++;
                {
                    s16 vert_off = *_p++;
                    TRAClogNumber(TRAC_LOG_SPECIFIC, "vert_off", vert_off, 4);
                    ASSERT((vert_off % 168) == 0);
                    ASSERT(vert_off < 32000);
                }
                {
                    u16 enter_off = *_p++;
                    TRAClogNumber(TRAC_LOG_SPECIFIC, "enter_off", enter_off, 4);
                    ASSERT((enter_off & 1) == 0);
                    ASSERT(((u16)(-enter_off)) <= (200 * 18));
                }
                break;

            case 4: 
                {
                    static char trace[] = "masks:      ";
                    u16 maskl = (*_p) >> 8;
                    u16 maskr = (*_p++) & 0xFF;
                    STDuxtoa(&trace[7], maskl, 2);
                    STDuxtoa(&trace[10], maskr, 2);
                    TRAClog(TRAC_LOG_SPECIFIC, trace, '\n');
                    ASSERT(maskl < 32);
                    ASSERT(maskr < 32);
                }            
                {
                    u16 nbwords = *_p++;
                    TRAClogNumber(TRAC_LOG_SPECIFIC, "nbwords", nbwords, 2);
                    ASSERT(nbwords <= 20);
                }
                break;

            case 6: 
                {
                    u8 bit = *(u8*)_p;
                    TRAClogNumber(TRAC_LOG_SPECIFIC, "rol_bit", bit, 2);
                    ASSERT(bit < 16);
                }
                {
                    u16 length = ((u8*)_p)[1];
                    TRAClogNumber(TRAC_LOG_SPECIFIC, "length", length, 2);
                    ASSERT(length <= 200);
                }
                _p++;
                {
                    u16 enter_off = *_p++;
                    TRAClogNumber(TRAC_LOG_SPECIFIC, "enter_off", enter_off, 4);
                    ASSERT((enter_off & 1) == 0);
                }
                break;
            }
            nbedges--;
        }
    }

    return _p;
}
#endif /* DEMOS_DEBUG */


void VECsceneConstruct (VECscene* _this, u8* _data, u16 _size)
{
    u8  i;
    u32 totalsize = 0;
    u16 nbpolygons;


    DEFAULT_CONSTRUCT(_this);
 
    nbpolygons = *(u16*)_data;
    nbpolygons = PCENDIANSWAP16(nbpolygons);
    _data += 2;

    _this->polygons   = (u16**) MEM_ALLOC ( &sys.allocatorMem, (1 + nbpolygons) * sizeof(u16*) );
    _this->nbPolygons = nbpolygons;

    totalsize = _size - 2;

	_this->data = (u8*) MEM_ALLOC ( &sys.allocatorMem, totalsize );

    {
        u16**               p   = _this->polygons;
        u8*                 adr = _this->data;
        u16                 edges;

        STDmcpy(adr, _data, totalsize);

        for ( i = 0 ; i < nbpolygons ; i++ )
        {
            *p = (u16*) adr;

            edges = **p;
            edges = PCENDIANSWAP16(edges);
            adr += (edges << 2) + 2;

            p++;
        }

        *p = NULL;  /* needed by ASM routine */
        ASSERT( (adr - _this->data) == totalsize );   
    }
}

void VECsceneDestroy(VECscene* _this)
{
    MEM_FREE (&sys.allocatorMem, _this->polygons); _this->polygons = NULL;
    MEM_FREE (&sys.allocatorMem, _this->data);     _this->data     = NULL;
}


u16* VECclipline(u16* dlist, u16* coord, u32 _pnbedges);

#ifdef __TOS__

u16* VECprecompute (u16* poly, s16 cs, s16 sn, u16 coef, s16 offsetx, s16 offsety, s16 centerx, s16 centery, u16* dlist);

#else

/* ASM version lot faster (x5) but this can work on ST too */
u16* VECprecompute (u16* poly, s16 cs, s16 sn, u16 coef, s16 offsetx, s16 offsety, s16 centerx, s16 centery, u16* dlist) 	
{
    u16 nbEdges = *poly++;
    s16	minx = VEC_S16_MAX;
    s16	maxx = VEC_S16_MIN;
    s16	miny = VEC_S16_MAX;
    s16	maxy = VEC_S16_MIN;

    nbEdges = PCENDIANSWAP16(nbEdges);

    *dlist++ = nbEdges - 1;

    while (nbEdges-- > 0)
    {
        s16 x, y;
        s32 x2, y2;

        x = *poly++;
        x = PCENDIANSWAP16(x);
        x <<= 4;
        x += offsetx;

        y = *poly++;
        y = PCENDIANSWAP16(y);
        y <<= 4;
        y += offsety;

        x2  = (s32) x * cs;
        x2 -= (s32) y * sn;
        y2  = (s32) x * sn;
        y2 += (s32) y * cs;

        x2 >>= 16;
        y2 >>= 16;

        x2 *= coef;
        y2 *= coef;

        x2 >>= 16;
        y2 >>= 16;

        x2 += centerx;
        y2 += centery;

        *dlist++ = (u16) x2;
        *dlist++ = (u16) y2;

        if ( x2 < minx )
            minx = (s16) x2;

        if ( x2 > maxx )
            maxx = (s16) x2;

        if ( y2 < miny )
            miny = (s16) y2;

        if ( y2 > maxy )
            maxy = (s16) y2;
    } /* edges loop */

    dlist[VEC_MINX_IDX] = minx;
    dlist[VEC_MAXX_IDX] = maxx;
    dlist[VEC_MINY_IDX] = miny;
    dlist[VEC_MAXY_IDX] = maxy;

    return dlist;
}

void vecDisplay(u16* _dlist)
{
    s16 e = *_dlist++;

    if ( e >= 0 )
    {
        s16 x1 = *_dlist++;
        s16 y1 = *_dlist++;

        s16 firstx = x1;
        s16 firsty = y1;

        s16 x2;
        s16 y2;

        while (e-- > 0)
        {
            x2 = *_dlist++;
            y2 = *_dlist++;

            WINline ( EMULgetWindow(), x1, y1, x2, y2);

            x1 = x2;
            y1 = y2;
        }

        WINline ( EMULgetWindow(), firstx, firsty, x2, y2);
    }
}

#endif /* __TOS__ */


#ifndef __TOS__
#   define VEC_POLY_DISPLAY_TEST() 0 /* Set to 1 to draw poly with GDI on PC */
#else
#   define VEC_POLY_DISPLAY_TEST() 0
#endif


u16* VECscenePrecompute (VECscene* _scene, u16* _dlist, VECprecomputeCheckCallback _checkcallback, s16* _cosTable, s16* _sinTable, VECanimationCallback _animate, VECanimationState* _animationState, u16* _temp, u16 _tempsize)
{
    u16* scenedisplist = _dlist;
    u16  nbpolygons    = _scene->nbPolygons;
    u16  coef;
    s16  xdep;

    do
    {
        u16** plist = _scene->polygons;
        u16 polyindex;
        s16* frameminmax = (s16*) _dlist;
        u16 angle = _animationState->angle;
        s16 cs = _cosTable[angle];
        s16 sn = _sinTable[angle];

        cs = PCENDIANSWAP16 ( cs );
        sn = PCENDIANSWAP16 ( sn );

        if ( _animate(_animationState) )
        {
            break;
        }

        coef = _animationState->coef;
        xdep = _animationState->xdep;

        /* reserve some space for minx / maxx / miny / maxy */
        /*g_screens.cybervector->frames[g_screens.cybervector->nbFrames] = dlist;*/

        *_dlist++ = VEC_S16_MAX;     /* minx */
        *_dlist++ = VEC_S16_MIN;     /* maxx */
        *_dlist++ = VEC_S16_MAX;     /* miny */
        *_dlist++ = VEC_S16_MIN;     /* maxy */

#       ifndef __TOS__
        WINsetColor  ( EMULgetWindow(), 0, 0, 0);
	    WINclear     ( EMULgetWindow() );
    	WINsetColor  ( EMULgetWindow(), 255, 0, 0);
#       endif

        for (polyindex = 0 ; polyindex < nbpolygons ; polyindex++)
        {
            u16* poly        = *plist;
            u16* templist    = _temp;
            s16  nbedges     = *poly;
            s16* currentpolyminmax;


            nbedges = PCENDIANSWAP16(nbedges);

            /* precompute edges coordinates on temporary buffer + bouding rect*/
            currentpolyminmax = (s16*) VECprecompute (poly, cs, sn, coef, xdep, 0, VEC_SCREEN_WIDTH >> 1, VEC_SCREEN_HEIGHT >> 1, templist);
            ASSERT( ((u8*)(currentpolyminmax + 4) - (u8*)templist) < _tempsize );

            {
                if (( currentpolyminmax[VEC_MAXY_IDX] < 0              ) || 
                    ( currentpolyminmax[VEC_MINY_IDX] >= VEC_SCREEN_HEIGHT ) ||
                    ( currentpolyminmax[VEC_MAXX_IDX] < 0              ) ||
                    ( currentpolyminmax[VEC_MINX_IDX] >= VEC_SCREEN_WIDTH ))
                {
                    *_dlist++ = VEC_NOEDGE; /* -1 */
                }
                else
                {
                    u16* pnbedges = _dlist;
                    u16  nbedgesincdec[2];


                    if ( frameminmax[VEC_MINX_IDX] > currentpolyminmax[VEC_MINX_IDX] )	/* min x */
                        frameminmax[VEC_MINX_IDX] = currentpolyminmax[VEC_MINX_IDX];

                    if ( frameminmax[VEC_MAXX_IDX] < currentpolyminmax[VEC_MAXX_IDX] )	/* max x */
                        frameminmax[VEC_MAXX_IDX] = currentpolyminmax[VEC_MAXX_IDX];

                    if ( frameminmax[VEC_MINY_IDX] > currentpolyminmax[VEC_MINY_IDX] )	/* min y */
                        frameminmax[VEC_MINY_IDX] = currentpolyminmax[VEC_MINY_IDX];

                    if ( frameminmax[VEC_MAXY_IDX] < currentpolyminmax[VEC_MAXY_IDX] )	/* max y */
                        frameminmax[VEC_MAXY_IDX] = currentpolyminmax[VEC_MAXY_IDX];

                    /* Precompute clipping + line slopes */                        
                    *_dlist++ = *templist++; /* copy (nbedges - 1) */

                    currentpolyminmax[0] = templist[0];                        
                    currentpolyminmax[1] = templist[1];

                    nbedgesincdec[0] = nbedgesincdec[1] = 0;

#                   if VEC_POLY_DISPLAY_TEST()
                    vecDisplay (_temp);
#                   else

                    while (nbedges-- > 0)
                    {   
                        _dlist = VECclipline (_dlist, templist, (u32)&nbedgesincdec);
                        templist += 2;

#                       ifdef DEMOS_ASSERT
                        if (_checkcallback != NULL)
                        {
                            _checkcallback(_dlist);
                        }
#                       endif
                    }
#                   endif /* VEC_POLY_DISPLAY_TEST() */

                    *pnbedges = *pnbedges + nbedgesincdec[0] - nbedgesincdec[1];
                }
            }

            plist++;
        } /* poly loop */

#       if VEC_POLY_DISPLAY_TEST()
        WINsetColor  ( EMULgetWindow(), 255, 255, 255);
    	WINrectangle ( EMULgetWindow(), -1, -1, VEC_SCREEN_WIDTH, VEC_SCREEN_HEIGHT);
	    WINrender ( EMULgetWindow());
        EMULwait  (VEC_PC_FRAMEDELAY);
#       endif

        /* converts frameminmax[0] and frameminmax[1] into word numbers and offset for blitter pass */
        if ( frameminmax[VEC_MINX_IDX] < 0 )
            frameminmax[VEC_MINX_IDX] = 0;

        if ( frameminmax[VEC_MAXX_IDX] >= VEC_SCREEN_WIDTH )
            frameminmax[VEC_MAXX_IDX] = VEC_SCREEN_WIDTH - 1;

        frameminmax[VEC_MINX_IDX] &= 0xFFF0;
        frameminmax[VEC_MINX_IDX] >>= 1;

        frameminmax[VEC_MAXX_IDX] &= 0xFFF0;
        frameminmax[VEC_MAXX_IDX] >>= 1;

        frameminmax[VEC_MAXX_IDX] -= frameminmax[VEC_MINX_IDX];
        frameminmax[VEC_MAXX_IDX] >>= 3;
        frameminmax[VEC_MAXX_IDX]++;

        /*VECtraceList((u16*) frameminmax, _scene);*/
        _scene->nbframes++;
    }
    while (1);

    _scene->displist = scenedisplist;

    return _dlist;
}


void VECdisplayBar (u16 xa_, u16 xb_, u16 xc_, u16 xd_, u16 yab_, u16 ycd_, u16* _dlist)
{
    u16   poly[10];
    s16   nbedges = 4;
    u16*  frameminmax = _dlist;


    poly[0] = poly[8] = xa_; 
    poly[2] = xb_; 
    poly[4] = xc_; 
    poly[6] = xd_; 

    poly[1] = poly[3] = poly[9] = yab_; 
    poly[5] = poly[7] = ycd_; 

    /* reserve some space for minx / maxx / miny / maxy */
    *_dlist++ = STD_MIN(xa_, xd_);     /* minx */
    *_dlist++ = STD_MAX(xb_, xc_);     /* maxx */
    *_dlist++ = STD_MIN(yab_, ycd_);   /* miny */
    *_dlist++ = STD_MAX(yab_, ycd_);   /* maxy */

    {
        u16* pnbedges = _dlist;
        u16  nbedgesincdec[2] = { 0,0 };
        u16* p = poly;

        *_dlist++ = nbedges - 1; /* copy (nbedges - 1) */

        while (nbedges-- > 0)
        {
            _dlist = VECclipline(_dlist, p, (u32)&nbedgesincdec);
            p += 2;
        }

        *pnbedges += nbedgesincdec[0] - nbedgesincdec[1];
    }

    /* converts frameminmax[0] and frameminmax[1] into word numbers and offset for blitter pass */
    frameminmax[VEC_MINX_IDX] &= 0xFFF0;
    frameminmax[VEC_MINX_IDX] >>= 1;

    frameminmax[VEC_MAXX_IDX] &= 0xFFF0;
    frameminmax[VEC_MAXX_IDX] >>= 1;

    frameminmax[VEC_MAXX_IDX] -= frameminmax[VEC_MINX_IDX];
    frameminmax[VEC_MAXX_IDX] >>= 3;
    frameminmax[VEC_MAXX_IDX]++;

    ASSERT((_dlist - frameminmax) < 32);
}
