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
#include "DEMOSDK\CODEGEN.H"
#include "DEMOSDK\TRACE.H"

#include "DEMOSDK\DATA\SYSTFNT.H"

#include "FX\TEXT\TEXT.H"


#define TEX_OR_OPCODE            0x8168    /* or.w d0,offset(a0) */
#define TEX_MOVEM_LOAD_OPCODE    0x4CBA    /* movem.w offset(pc),d0-dx registermask: AADD offset */
#define TEX_MOVEM_SAVE_OPCODE    0x48A7    /* movem.w d0-dx,-(sp) registermask: DDAA offset */
#define TEX_MOVEM_RESTORE_OPCODE 0x4C9F    /* movem.w (sp)+,d0-dx registermask: AADD offset */
#define TEX_CHAR_YSPACING        4


u16 TEXfont44GenerateCode(u8* _code, char* _str, u16 pitch_, TEXfont44GenerateCodeCallback generatecallback_)
{
    u16* code = (u16*)_code;
    u16  len = STDstrLen(_str);
    s16  offsety = 0;
    u8   ys;
    u8   nbpats = 0;
    u8   pats[8];
    u16  t, p;
    u16  regsmask = 0xFF;


    TRAClogFrameNum(TRAC_LOG_FLOW);
    TRAClog(TRAC_LOG_FLOW, "TEXfont44GenerateCode", '\n');

    pitch_ *= TEX_CHAR_YSPACING;

    for (ys = 0; ys < 8; ys++)
    {
        for (t = 0; t < len; t++)
        {
            u8  c = _str[t];
            u8* bits = SYSfont.data + (SYSfont.charsmap[c] << SYS_FNT_OFFSETSHIFT) + ys;
            u8  v  = *bits++;
            u8  v1 = v >> 4;
            u8  v2 = v & 0xF;
            u8  i;

            /*v1 = v;
            v2 = 0;*/

            if (v1 != 0)
            {
                u8* p = pats;
                for (i = nbpats; i > 0; i--)
                    if (*p++ == v1)
                        break;

                if (i == 0)
                {
                    ASSERT(nbpats < ARRAYSIZE(pats));
                    pats[nbpats++] = v1;
                }
            }

            if (v2 != 0)
            {
                u8* p = pats;
                for (i = nbpats; i > 0; i--)
                    if (*p++ == v2)
                        break;

                if (i == 0)
                {
                    ASSERT(nbpats < ARRAYSIZE(pats));
                    pats[nbpats++] = v2;
                }
            }
        }
    }

    for (p = 0 ; p < nbpats ; p++)
    {
        u8  v     = pats[p] << 4;
        u16 value = 0; 
        u8  xs;

        for (xs = 4; xs > 0; xs--)
        {
            value <<= 4;

            if (v & 0x80)
            {
                value |= 0xE;
            }

            v <<= 1;
        }

        *code++ = value;
    }

    for (; p < 8 ; p++)
    {
        *code++ = 0;
    }

    *code++ = TEX_MOVEM_SAVE_OPCODE;
    *code++ = regsmask << 8;

    *code++ = TEX_MOVEM_LOAD_OPCODE;
    *code++ = regsmask;
    *code++ = -((8 << 1) + 8);

    for (ys = 0; ys < 8; ys++)
    {
        s16 offset = offsety;

        for (t = 0; t < len; t++)
        {
            u8   c = _str[t];
            u8*  bits = SYSfont.data + (SYSfont.charsmap[c] << SYS_FNT_OFFSETSHIFT) + ys;
            u8   v = *bits++;
            u8   v1 = v >> 4;
            u8   v2 = v & 0xF;
            u8   i;


            if (v1 != 0)
            {
                u8* p = pats;
                for (i = 0; i < nbpats; i++)
                    if (*p++ == v1)
                        break;

                ASSERT(i < nbpats);

                code = (*generatecallback_)(code, offset, TEX_OR_OPCODE | (i << 9));
            }

            offset += 8;

            if (v2 != 0)
            {
                u8* p = pats;
                for (i = 0; i < nbpats; i++)
                    if (*p++ == v2)
                        break;

                ASSERT(i < nbpats);

                code = (*generatecallback_)(code, offset, TEX_OR_OPCODE | (i << 9));
            }

            offset += 8;
        }

        offsety += pitch_;
    }

    *code++ = TEX_MOVEM_RESTORE_OPCODE;
    *code++ = regsmask;

    *code++ = CGEN_OPCODE_RTS;

    return (u8*)code - _code;
}


#ifndef __TOS__
void TEXfont44DisplayPC(void* _code, u8* _adr, u16 codeoffset1_, u16 codeoffset2_)
{
    u16* code = (u16*) _code;
    u16  pats[8];
    u16  t;


    for (t = 0; t < ARRAYSIZE(pats); t++)
    {
        pats[t] = *code++;
    }

    code += codeoffset1_;

    while (*code != CGEN_OPCODE_RTS)
    {
        u16 opcode = code[0];
        u16 offset = code[1];
        u16 value  = pats[(opcode >> 9) & 7];

        *(u16*)(_adr + offset) |= PCENDIANSWAP16(value);
        code += codeoffset2_;
    }
}
#endif
