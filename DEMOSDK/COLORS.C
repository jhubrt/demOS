/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2018 J.Hubert

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


u16* COLcomputeGradientStep (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _nbSteps, s16 _step, u16* _destColors)
{
    u16  n;
    u16* scp = _startColors;
    u16* ecp = _endcolors;


    for (n = 0 ; n < _nbColors ; n++)
    {
        u16 sc = *scp++;
        u16 ec = *ecp++;

        s16 rs = COLST24b[(sc & 0xF00) >> 8];
        s16 gs = COLST24b[(sc & 0xF0 ) >> 4];
        s16 bs = COLST24b[(sc & 0xF  )     ];

        s16 re = COLST24b[(ec & 0xF00) >> 8];
        s16 ge = COLST24b[(ec & 0xF0 ) >> 4];
        s16 be = COLST24b[(ec & 0xF  )     ];

        s16 rd = ( ((re - rs) * _step) / _nbSteps ) + rs; 
        s16 gd = ( ((ge - gs) * _step) / _nbSteps ) + gs;
        s16 bd = ( ((be - bs) * _step) / _nbSteps ) + bs;

        *_destColors++ = (COL4b2ST[rd] << 8) | (COL4b2ST[gd] << 4) | COL4b2ST[bd];
    }

    return _destColors;
}


u16* COLcomputeGradient (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _nbSteps, u16* _destColors)
{
    s16 s;


    _nbSteps--;

    for (s = 0 ; s <= _nbSteps ; s++)
    {
        _destColors = COLcomputeGradientStep (_startColors, _endcolors, _nbColors, _nbSteps, s, _destColors);
    }

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
