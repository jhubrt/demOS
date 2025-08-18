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

#include "FX\MOD\MOD.H"


/*      Protracker 16 note conversion table / MOD Period table
       +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
PT16 : I    1I    2I    3I    4I    5I    6I    7I    8I    9I   10I   11I   12I
MOD  : I 1712I 1616I 1524I 1440I 1356I 1280I 1208I 1140I 1076I 1016I  960I  906I
Note : I  C-0I  C#0I  D-0I  D#0I  E-0I  F-0I  F#0I  G-0I  G#0I  A-0I  A#0I  B-0I
       +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
       I   13I   14I   15I   16I   17I   18I   19I   20I   21I   22I   23I   24I
       I  856I  808I  762I  720I  678I  640I  604I  570I  538I  508I  480I  453I
       I  C-1I  C#1I  D-1I  D#1I  E-1I  F-1I  F#1I  G-1I  G#1I  A-1I  A#1I  B-1I
       +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
       I   25I   26I   27I   28I   29I   30I   31I   32I   33I   34I   35I   36I
       I  428I  404I  381I  360I  339I  320I  302I  285I  269I  254I  240I  226I
       I  C-2I  C#2I  D-2I  D#2I  E-2I  F-2I  F#2I  G-2I  G#2I  A-2I  A#2I  B-2I
       +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
       I   37I   38I   39I   40I   41I   42I   43I   44I   45I   46I   47I   48I
       I  214I  202I  190I  180I  170I  160I  151I  143I  135I  127I  120I  113I
       I  C-3I  C#3I  D-3I  D#3I  E-3I  F-3I  F#3I  G-3I  G#3I  A-3I  A#3I  B-3I
       +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
       I   49I   50I   51I   52I   53I   54I   55I   56I   57I   58I   59I   60I
       I  107I  101I   95I   90I   85I   80I   75I   71I   67I   63I   60I   56I
       I  C-4I  C#4I  D-4I  D#4I  E-4I  F-4I  F#4I  G-4I  G#4I  A-4I  A#4I  B-4I
       +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+ 
*/

void MODcreatePeriodConversionTable (u8* table)
{
    table[1712] =         0; /* C-0 */
    table[1616] =         1; /* C#0 */
    table[1524] =         2; /* D-0 */
    table[1440] =         3; /* D#0 */
    table[1356] =         4; /* E-0 */
    table[1280] =         5; /* F-0 */
    table[1208] =         6; /* F#0 */
    table[1140] =         7; /* G-0 */
    table[1076] =         8; /* G#0 */
    table[1016] =         9; /* A-0 */
    table[960 ] =        10; /* A#0 */
    table[906 ] =        11; /* B-0 */

    table[856 ] = 1*16 +  0; /* C-1 */
    table[808 ] = 1*16 +  1; /* C#1 */
    table[762 ] = 1*16 +  2; /* D-1 */
    table[720 ] = 1*16 +  3; /* D#1 */
    table[678 ] = 1*16 +  4; /* E-1 */
    table[640 ] = 1*16 +  5; /* F-1 */
    table[604 ] = 1*16 +  6; /* F#1 */
    table[570 ] = 1*16 +  7; /* G-1 */
    table[538 ] = 1*16 +  8; /* G#1 */
    table[508 ] = 1*16 +  9; /* A-1 */
    table[480 ] = 1*16 + 10; /* A#1 */
    table[453 ] = 1*16 + 11; /* B-1 */

    table[428 ] = 2*16 +  0; /* C-2 */
    table[404 ] = 2*16 +  1; /* C#2 */
    table[381 ] = 2*16 +  2; /* D-2 */
    table[360 ] = 2*16 +  3; /* D#2 */
    table[339 ] = 2*16 +  4; /* E-2 */
    table[320 ] = 2*16 +  5; /* F-2 */
    table[302 ] = 2*16 +  6; /* F#2 */
    table[285 ] = 2*16 +  7; /* G-2 */
    table[269 ] = 2*16 +  8; /* G#2 */
    table[254 ] = 2*16 +  9; /* A-2 */
    table[240 ] = 2*16 + 10; /* A#2 */
    table[226 ] = 2*16 + 11; /* B-2 */

    table[214 ] = 3*16 +  0; /* C-3 */
    table[202 ] = 3*16 +  1; /* C#3 */
    table[190 ] = 3*16 +  2; /* D-3 */
    table[180 ] = 3*16 +  3; /* D#3 */
    table[170 ] = 3*16 +  4; /* E-3 */
    table[160 ] = 3*16 +  5; /* F-3 */
    table[151 ] = 3*16 +  6; /* F#3 */
    table[143 ] = 3*16 +  7; /* G-3 */
    table[135 ] = 3*16 +  8; /* G#3 */
    table[127 ] = 3*16 +  9; /* A-3 */
    table[120 ] = 3*16 + 10; /* A#3 */
    table[113 ] = 3*16 + 11; /* B-3 */

    table[107 ] = 4*16 +  0; /* C-4 */
    table[101 ] = 4*16 +  1; /* C#4 */
    table[95  ] = 4*16 +  2; /* D-4 */
    table[90  ] = 4*16 +  3; /* D#4 */
    table[85  ] = 4*16 +  4; /* E-4 */
    table[80  ] = 4*16 +  5; /* F-4 */
    table[75  ] = 4*16 +  6; /* F#4 */
    table[71  ] = 4*16 +  7; /* G-4 */
    table[67  ] = 4*16 +  8; /* G#4 */
    table[63  ] = 4*16 +  9; /* A-4 */
    table[60  ] = 4*16 + 10; /* A#4 */
    table[56  ] = 4*16 + 11; /* B-4 */
}
