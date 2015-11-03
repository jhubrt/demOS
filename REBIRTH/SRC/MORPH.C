/*------------------------------------------------------------------------------  -----------------
  Copyright J.Hubert 2015

  This file is part of rebirth demo

  rebirth demo is free software: you can redistribute it and/or modify it under the terms of 
  the GNU Lesser General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  rebirth demo is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY ;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with rebirth demo.  
  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------- */

#define MORPH_C

#include "REBIRTH\SRC\MORPH.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\HARDWARE.H"

ASMIMPORT u8 SLIcodepattern;
ASMIMPORT u8 SLIendcodepattern;

void SLIinitMorph    (void* _verttable, void* _horitable, u16 _h, u16 _pitch, u16 _w) PCSTUB;
void SLIstartMorph   (void* _startPos, void* _endPos, u32 _morphCode, u32 _pos, u16 _nbpoints) PCSTUB;
void SLIdisplayMorph (void* _screenbase, void* _pos, u32 _voffsettable, u32 _htable, u16 _nbPoints) PCSTUB;
void SLImorphFunc    (void* _screenbase, void* _pos, u32 _voffsettable, u32 _htable, u32 _morphcode) PCSTUB;

u32 SLIgetMorphCodeLen (u16 _nbPoints)
{
    u16 morphcodelen = (&SLIendcodepattern - &SLIcodepattern);
    return (morphcodelen * _nbPoints) + 2UL;
}

void SLIgenerateMorphCode( u8* _codeBuffer, u16 _nbPoints )
{
    u32 morphcodelen = (&SLIendcodepattern - &SLIcodepattern);
    u16 t;

    for (t = 0 ; t < _nbPoints ; t++)
    {
        STDmcpy(_codeBuffer, &SLIcodepattern, morphcodelen);
        _codeBuffer += morphcodelen;
    }

    *((u16*)_codeBuffer) = HW_68KOP_RTS;
}

