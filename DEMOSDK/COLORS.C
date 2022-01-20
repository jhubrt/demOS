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
#include "DEMOSDK\STANDARD.H"

#define COLORS_C

#include "DEMOSDK\COLORS.H"

u8 COLST24b[16] = 
{
    0x0,    /* 0 */
    0x2,    /* 1 */
    0x4,    /* 2 */
    0x6,    /* 3 */
    0x8,    /* 4 */
    0xA,    /* 5 */
    0xC,    /* 6 */
    0xE,    /* 7 */
    0x1,    /* 8 */
    0x3,    /* 9 */
    0x5,    /* A */
    0x7,    /* B */
    0x9,    /* C */
    0xB,    /* D */
    0xD,    /* E */
    0xF     /* F */
};

u8 COL4b2ST[17] =
{
    0x0,    /* 0 */
    0x8,    /* 1 */
    0x1,    /* 2 */
    0x9,    /* 3 */
    0x2,    /* 4 */
    0xA,    /* 5 */
    0x3,    /* 6 */
    0xB,    /* 7 */
    0x4,    /* 8 */
    0xC,    /* 9 */
    0x5,    /* A */
    0xD,    /* B */
    0x6,    /* C */
    0xE,    /* D */
    0x7,    /* E */
    0xF,    /* F */
    0xF     /* Sature */
};


#ifndef __TOS__
void COLswapEndianPC(u16* _colors, u16 _nbColors)
{
    u16 t;

    for (t = 0 ; t < _nbColors ; t++)
    {
        *_colors = PCENDIANSWAP16(*_colors);
        _colors++;
    }
}
#endif


u16* COLcomputeGradientStep (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _nbSteps, s16 _step, u16* _destColors)
{
    u16  n;
    u8 * scp = (u8*) _startColors;
    u8 * ecp = (u8*) _endcolors;


    for (n = 0 ; n < _nbColors ; n++)
    {
        u8  sc1 = *scp++;
        u8  sc2 = *scp++;
        u8  ec1 = *ecp++;
        u8  ec2 = *ecp++;

#       ifndef __TOS__
        BAS_SWAP(u8, sc1, sc2);
        BAS_SWAP(u8, ec1, ec2);
#       endif

        s16 rs = COLST24b[sc1 & 0xF];
        s16 gs = COLST24b[sc2 >> 4 ];
        s16 bs = COLST24b[sc2 & 0xF];

        s16 re = COLST24b[ec1 & 0xF];
        s16 ge = COLST24b[ec2 >> 4 ];
        s16 be = COLST24b[ec2 & 0xF];

        rs += ((re - rs) * _step) / _nbSteps; 
        gs += ((ge - gs) * _step) / _nbSteps;
        bs += ((be - bs) * _step) / _nbSteps;

        *_destColors++ = (COL4b2ST[rs] << 8) | (COL4b2ST[gs] << 4) | COL4b2ST[bs];
    }

    return _destColors;
}

#ifndef __TOS__
u16* COLcomputeGradient16Steps (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _step, u16* _destColors)
{
    u16  n;
    u8 * scp = (u8*) _startColors;
    u8 * ecp = (u8*) _endcolors;


    switch (_step)
    {
    case 0:  
        if (_destColors != _startColors)
        {
            STDmcpy2 (_destColors, _startColors, _nbColors << 1);
        }
        _destColors += _nbColors;
        break;
    
    case 16: 
        if (_destColors != _endcolors)
        {
            STDmcpy2 (_destColors, _endcolors, _nbColors << 1); 
        }
        _destColors += _nbColors;
        break;
    
    default:        
        ASSERT(_step < 16);

        for (n = 0; n < _nbColors; n++)
        {
            u8  sc1 = *scp++;
            u8  sc2 = *scp++;
            u8  ec1 = *ecp++;
            u8  ec2 = *ecp++;

#           ifndef __TOS__
            BAS_SWAP(u8, sc1, sc2);
            BAS_SWAP(u8, ec1, ec2);
#           endif

            s16 rs = COLST24b[sc1 & 0xF];
            s16 gs = COLST24b[sc2 >> 4 ];
            s16 bs = COLST24b[sc2 & 0xF];

            s16 re = COLST24b[ec1 & 0xF];
            s16 ge = COLST24b[ec2 >> 4 ];
            s16 be = COLST24b[ec2 & 0xF];

            rs += ((re - rs) * _step) >> 4;
            gs += ((ge - gs) * _step) >> 4;
            bs += ((be - bs) * _step) >> 4;

            *_destColors++ = (COL4b2ST[rs] << 8) | (COL4b2ST[gs] << 4) | COL4b2ST[bs];
        }
        break;
    }

    return _destColors;
}


u16* COLcomputeGradient16Steps4b4b (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _step, u16* _destColors)
{
    u16  n;
    u16* scp = _startColors;
    u16* ecp = _endcolors;


    switch (_step)
    {
    case 0:  
        if (_destColors != _startColors)
        {
            STDmcpy2 (_destColors, _startColors, _nbColors << 1);
        }
        _destColors += _nbColors;
        break;

    case 16: 
        if (_destColors != _endcolors)
        {
            STDmcpy2 (_destColors, _endcolors, _nbColors << 1); 
        }
        _destColors += _nbColors;
        break;

    default:        
        ASSERT(_step < 16);

    for (n = 0 ; n < _nbColors ; n++)
    {
        u16 sc = *scp++;
        u16 ec = *ecp++;

            s16 rs = (sc & 0xF00) >> 4;
            s16 gs =  sc & 0xF0;
            s16 bs =  sc & 0xF;

            s16 re = (ec & 0xF00) >> 4;
            s16 ge =  ec & 0xF0;
            s16 be =  ec & 0xF;

            rs += ((re - rs) * _step) >> 4;
            gs += ((ge - gs) * _step) >> 4;
            bs += ((be - bs) * _step) >> 4;

            rs <<= 4;
            rs &= 0xF00;
            gs &= 0xF0;

            *_destColors++ = rs | gs | bs;
        }
        break;
    }

    return _destColors;
}


u16* COLcomputeGradient16Steps4bSTe (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _step, u16* _destColors)
{
    u16  n;
    u8* scp = (u8*) _startColors;
    u8* ecp = (u8*) _endcolors;


    switch (_step)
    {
    case 0:  
        for (n = 0; n < _nbColors; n++)
        {
            u8  sc1 = *scp++;
            u8  sc2 = *scp++;

#           ifndef __TOS__
            BAS_SWAP(u8, sc1, sc2);
#           endif

            u16 rd = sc1 & 0xF;
            u16 gd = sc2 >> 4;
            u16 bd = sc2 & 0xF;

            *_destColors++ = (COL4b2ST[rd] << 8) | (COL4b2ST[gd] << 4) | COL4b2ST[bd];
        }
        break;

    case 16: 
    for (n = 0 ; n < _nbColors ; n++)
    {
            u8  ec1 = *ecp++;
            u8  ec2 = *ecp++;

#           ifndef __TOS__
            BAS_SWAP(u8, ec1, ec2);
#           endif

            u16 rd = ec1 & 0xF;
            u16 gd = ec2 >> 4;
            u16 bd = ec2 & 0xF;

        *_destColors++ = (COL4b2ST[rd] << 8) | (COL4b2ST[gd] << 4) | COL4b2ST[bd];
    }
        break;

    default:        
        ASSERT(_step < 16);

        for (n = 0; n < _nbColors; n++)
        {
            u8  sc1 = *scp++;
            u8  sc2 = *scp++;
            u8  ec1 = *ecp++;
            u8  ec2 = *ecp++;

#           ifndef __TOS__
            BAS_SWAP(u8, sc1, sc2);
            BAS_SWAP(u8, ec1, ec2);
#           endif

            s16 rs =  sc1 & 0xF;
            s16 gs =  sc2 >> 4;
            s16 bs =  sc2 & 0xF;

            s16 re =  ec1 & 0xF;
            s16 ge =  ec2 >> 4;
            s16 be =  ec2 & 0xF;

            rs += ((re - rs) * _step) >> 4;
            gs += ((ge - gs) * _step) >> 4;
            bs += ((be - bs) * _step) >> 4;

            *_destColors++ = (COL4b2ST[rs] << 8) | (COL4b2ST[gs] << 4) | COL4b2ST[bs];
        }
        break;
    }

    return _destColors;
}

#endif

u16* COLcomputeGradient (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _nbSteps, u16* _destColors)
{
    s16 s;


    if (_nbSteps == 16)
    {
        for (s = 0; s < _nbSteps; s++)
            _destColors = COLcomputeGradient16Steps(_startColors, _endcolors, _nbColors, s, _destColors);
    }
    else
    {
    _nbSteps--;

    for (s = 0 ; s <= _nbSteps ; s++)
        _destColors = COLcomputeGradientStep (_startColors, _endcolors, _nbColors, _nbSteps, s, _destColors);
    }

    return _destColors;
}

u16* COLcomputeGradient4b4b (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _nbSteps, u16* _destColors)
{
    s16 s;


    ASSERT(_nbSteps == 16);

    for (s = 0; s < _nbSteps; s++)
        _destColors = COLcomputeGradient16Steps4b4b(_startColors, _endcolors, _nbColors, s, _destColors);

    return _destColors;
}


u16 COLcomputeGradientEx (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _nbSteps, u16* _destColors1, u16* _destColors2)
{
    u16* destColors1buf = _destColors1;
    s16 s;


    _nbSteps--;

    for (s = 0 ; s <= _nbSteps ; s++)
    {
        u16  n;
        u16* scp = _startColors;
        u16* ecp = _endcolors;

        for (n = 0 ; n < _nbColors ; n++)
        {
            u16 sc = *scp++;
            u16 ec = *ecp++;

            s16 rs = (COLST24b[(sc & 0xF00) >> 8] << 1) | ((sc & 0x4000) != 0);
            s16 gs = (COLST24b[(sc & 0xF0 ) >> 4] << 1) | ((sc & 0x2000) != 0);
            s16 bs = (COLST24b[(sc & 0xF  )     ] << 1) | ((sc & 0x1000) != 0);

            s16 re = (COLST24b[(ec & 0xF00) >> 8] << 1) | ((sc & 0x4000) != 0);
            s16 ge = (COLST24b[(ec & 0xF0 ) >> 4] << 1) | ((sc & 0x2000) != 0);
            s16 be = (COLST24b[(ec & 0xF  )     ] << 1) | ((sc & 0x1000) != 0);

            s16 rd = ( ((re - rs) * s) / _nbSteps ) + rs;
            s16 gd = ( ((ge - gs) * s) / _nbSteps ) + gs;
            s16 bd = ( ((be - bs) * s) / _nbSteps ) + bs;

            u8  lowbits = (( rd & 1 ) << 2) | (( gd & 1 ) << 1) | ( bd & 1 );

            s16 rd2 = rd >> 1;
            s16 gd2 = gd >> 1;
            s16 bd2 = bd >> 1;

            rd = rd2;
            gd = gd2;
            bd = bd2;

            switch ( lowbits )
            {
            case 1:                 bd2++;  break;
            case 2:         gd2++;          break;
            case 3:         gd++;   bd2++;  break;
            case 4: rd2++;                  break;
            case 5: rd++;           bd2++;  break;
            case 6: rd2++;  gd++;           break;
            case 7: rd++;   gd2++;  bd++;   break;  
            }

            *_destColors1++ = (COL4b2ST[rd] << 8) | (COL4b2ST[gd] << 4) | COL4b2ST[bd];
            *_destColors2++ = (COL4b2ST[rd2] << 8) | (COL4b2ST[gd2] << 4) | COL4b2ST[bd2];
        }
    }

    return ((u8*)_destColors1 - (u8*)destColors1buf) >> 1;  /* pointer arithmetics generate ldiv in PureC if sizeof != 1 ...*/
}

#ifdef DEMOS_UNITTEST

#ifndef __TOS__
#pragma optimize("", on)
#endif


void COLrunUnitTests(void) 
{
    u16 t,i,s;

    for (t = 0 ; t < 4096 ; t += 5)
    {
        printf ("%d\n", t);

        for (i = 0; i < 4096; i += 3)
        {
            for (s = 0; s <= 16; s++)
            {
                u16 c1,c2,r1,r2,r3;

                c1 = COL_STE_TO_4B(i);
                c2 = COL_STE_TO_4B(t);
                COLcomputeGradient16Steps4b4b (&c1, &c2, 1, s, &r1);
                r1 = COL_4B_TO_STE(r1);

                COLcomputeGradient16Steps4bSTe (&c1, &c2, 1, s, &r2);

                COLcomputeGradient16Steps (&i, &t, 1, s, &r3);

                ASSERT(r1 == r2);
                ASSERT(r2 == r3);
            }
        }
    }
}

#endif
