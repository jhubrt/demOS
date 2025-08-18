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

#ifndef CODEGEN_H
#define CODEGEN_H

/* Size */

#define CG_SIZE_B      0
#define CG_SIZE_W      1
#define CG_SIZE_L      2

/* Condition */

#define CG_COND_T      0
#define CG_COND_F      1
#define CG_COND_HI     2
#define CG_COND_LS     3
#define CG_COND_CC     4
#define CG_COND_CS     5
#define CG_COND_NE     6
#define CG_COND_EQ     7
#define CG_COND_VC     8
#define CG_COND_VS     9
#define CG_COND_PL     0xA
#define CG_COND_MI     0xB
#define CG_COND_GE     0xC
#define CG_COND_LT     0xD
#define CG_COND_GT     0xE
#define CG_COND_LE     0xF

/* Addressing mode */

#define CG_ADRMODE_Dn(Dn)                    (Dn)                         /* 000 reg  */
#define CG_ADRMODE_An(An)            (8    | (An))                        /* 001 reg  */
#define CG_ADRMODE__An_(An)          (0x10 | (An))                        /* 010 reg  */
#define CG_ADRMODE__An_p(An)         (0x18 | (An))                        /* 011 reg  */
#define CG_ADRMODE_p_An_(An)         (0x20 | (An))                        /* 100 reg  */
#define CG_ADRMODE_d16_An_(An)       (0x28 | (An))                        /* 101 reg  */
#define CG_ADRMODE_d8_An_Xn_(An)     (0x30 | (An))                        /* 110 reg  */
#define CG_ADRMODE_d16_PC             0x3A                                /* 111 010  */
#define CG_ADRMODE_d8_PC_Xn           0x3B                                /* 111 011  */
#define CG_ADRMODE_ABS_W              0x38                                /* 111 000  */
#define CG_ADRMODE_ABS_L              0x39                                /* 111 001  */
#define CG_ADRMODE_IMM                0x3C                                /* 111 100  */

#define CG_ADRMODE1_Dn(Dn)                    ((Dn) << 9)                 /* reg 000 xx xxxx */
#define CG_ADRMODE1_An(An)           (0x40  | ((An) << 9))                /* reg 001 xx xxxx */
#define CG_ADRMODE1__An_(An)         (0x80  | ((An) << 9))                /* reg 010 xx xxxx */
#define CG_ADRMODE1__An_p(An)        (0xC0  | ((An) << 9))                /* reg 011 xx xxxx */
#define CG_ADRMODE1_p_An_(An)        (0x100 | ((An) << 9))                /* reg 100 xx xxxx */
#define CG_ADRMODE1_d16_An_(An)      (0x140 | ((An) << 9))                /* reg 101 xx xxxx */
#define CG_ADRMODE1_d8_An_Xn_(An)    (0x180 | ((An) << 9))                /* reg 110 xx xxxx */
#define CG_ADRMODE1_d16_PC            0x5C0                               /* 010 111 xx xxxx */
#define CG_ADRMODE1_d8_PC_Xn          0x7C0                               /* 011 111 xx xxxx */
#define CG_ADRMODE1_ABS_W             0x1C0                               /* 000 111 xx xxxx */
#define CG_ADRMODE1_ABS_L             0x3C0                               /* 001 111 xx xxxx */
#define CG_ADRMODE1_IMM               0x9C0                               /* 100 111 xx xxxx */


/* Opcode 0 */

#define CG_OP_ORI_TO_SR(SIZE)               ( 0x003C | ((SIZE) << 6)             )     /* B or W followed by immediate   0000 000 0 00 111 100    */
#define CG_OP_ORI(SIZE,ADRMODE)             (          ((SIZE) << 6) | (ADRMODE) )                                  /*   0000 000 0  S Adrmode    */
#define CG_OP_ANDI_TO_SR(SIZE)              ( 0x023C | ((SIZE) << 6)             )     /* B or W followed by immediate   0000 001 0 00 111 100    */
#define CG_OP_ANDI(SIZE,ADRMODE)            ( 0x0200 | ((SIZE) << 6) | (ADRMODE) )                                  /*   0000 001 0  S Adrmode    */
#define CG_OP_SUBI(SIZE,ADRMODE)            ( 0x0400 | ((SIZE) << 6) | (ADRMODE) )                                  /*   0000 010 0  S Adrmode    */
#define CG_OP_ADDI(SIZE,ADRMODE)            ( 0x0600 | ((SIZE) << 6) | (ADRMODE) )                                  /*   0000 011 0  S Adrmode    */
#define CG_OP_EORI_TO_SR(SIZE)              ( 0x0A3C | ((SIZE) << 6)             )     /* B or W followed by immediate   0000 101 0 00 111 100    */
#define CG_OP_EORI(SIZE,ADRMODE)            ( 0x0A00 | ((SIZE) << 6) | (ADRMODE) )                                  /*   0000 101 0  S Adrmode    */
#define CG_OP_CMPI(SIZE,ADRMODE)            ( 0x0C00 | ((SIZE) << 6) | (ADRMODE) )                                  /*   0000 110 0  S Adrmode    */
                                                                                      
#define CG_OP_BTST(ADRMODE)                 ( 0x0800 |                | (ADRMODE) )                                 /*   0000 100 0 00 Adrmode    */
#define CG_OP_BCHG(ADRMODE)                 ( 0x0840 |                | (ADRMODE) )                                 /*   0000 100 0 01 Adrmode    */
#define CG_OP_BCLR(ADRMODE)                 ( 0x0880 |                | (ADRMODE) )                                 /*   0000 100 0 10 Adrmode    */
#define CG_OP_BSET(ADRMODE)                 ( 0x08C0 |                | (ADRMODE) )                                 /*   0000 100 0 11 Adrmode    */
                                                                                      
#define CG_OP_BTST_Dn(Dn,ADRMODE)           ( 0x0100 | ((Dn) << 9)    | (ADRMODE) )                                 /*   0000 Dn  1 00 Adrmode    */
#define CG_OP_BCHG_Dn(Dn,ADRMODE)           ( 0x0140 | ((Dn) << 9)    | (ADRMODE) )                                 /*   0000 Dn  1 01 Adrmode    */
#define CG_OP_BCLR_Dn(Dn,ADRMODE)           ( 0x0180 | ((Dn) << 9)    | (ADRMODE) )                                 /*   0000 Dn  1 10 Adrmode    */
#define CG_OP_BSET_Dn(Dn,ADRMODE)           ( 0x01C0 | ((Dn) << 9)    | (ADRMODE) )                                 /*   0000 Dn  1 11 Adrmode    */
                                                                                      
#define CG_OP_MOVEP_R2M(SIZE,Dn,An)         ( 0x0188 | ((Dn) << 9) | ((SIZE - 1) << 6)  | (An))                     /*   0000 Dn  1 11 Adrmode    */
#define CG_OP_MOVEP_M2R(SIZE,Dn,An)         ( 0x0108 | ((Dn) << 9) | ((SIZE - 1) << 6)  | (An))                     /*   0000 Dn  1 01 Adrmode    */
#define CG_OP_MOVEA(SIZE,An,ADRMODE)        ( 0x0008 | (((SIZE+SIZE+SIZE) & 3) << 12) | ((An) << 9) | (ADRMODE))    /*   00 S An  0 01 Adrmode    */
#define CG_OP_MOVE(SIZE,ADRMODE1,ADRMODE)   ( (((SIZE+SIZE+SIZE) & 3) << 12) | (ADRMODE1) | (ADRMODE) )             /*   00 S Adrmode1 Adrmode    */
                                                                                                                   
/* Opcode 4 */

#define CG_OP_MOVE_FROM_SR(ADRMODE)         ( 0x40C0 | (ADRMODE))                                                   /*   0100 0000 11 Adrmode     */
#define CG_OP_MOVE_TO_CCR(ADRMODE)          ( 0x44C0 | (ADRMODE))                                                   /*   0100 0100 11 Adrmode     */
#define CG_OP_MOVE_TO_SR(ADRMODE)           ( 0x46C0 | (ADRMODE))                                                   /*   0100 0110 11 Adrmode     */
#define CG_OP_NEGX(SIZE,ADRMODE)            ( 0x4000 | ((SIZE) << 6) | (ADRMODE))                                   /*   0100 0000 S  Adrmode     */
#define CG_OP_CLR(SIZE,ADRMODE)             ( 0x4200 | ((SIZE) << 6) | (ADRMODE))                                   /*   0100 0010 S  Adrmode     */
#define CG_OP_NEG(SIZE,ADRMODE)             ( 0x4400 | ((SIZE) << 6) | (ADRMODE))                                   /*   0100 0100 S  Adrmode     */
#define CG_OP_NOT(SIZE,ADRMODE)             ( 0x4600 | ((SIZE) << 6) | (ADRMODE))                                   /*   0100 0110 S  Adrmode     */
#define CG_OP_EXT(SIZE,Dn)                  ( 0x4880 | ((SIZE - 1) << 6) | (Dn))                                    /*   0100 1000 1S 000 Dn      */
#define CG_OP_NBCD(ADRMODE)                 ( 0x4800 | (ADRMODE))                                                   /*   0100 1000 00 Adrmode     */
#define CG_OP_SWAP(Dn)                      ( 0x4840 | (Dn))                                                        /*   0100 1000 01 000 Dn      */
#define CG_OP_PEA(ADRMODE)                  ( 0x4840 | (ADRMODE))                                                   /*   0100 1000 01 000 Dn      */
#define CG_OP_ILLEGAL                         0x4AFA                                                                /*   0100 1010 1111 1100      */
#define CG_OP_TAS(ADRMODE)                  ( 0x4AC0 | (ADRMODE))                                                   /*   0100 1010 11 Adrmode     */
#define CG_OP_TST(SIZE,ADRMODE)             ( 0x4A00 | ((SIZE) << 6) | (ADRMODE))                                   /*   0100 1010 S  Adrmode     */
#define CG_OP_TRAP(VECTOR)                  ( 0x4E40 | (VECTOR))                                                    /*   0100 1110 0100 Vectr     */
#define CG_OP_LINK(An)                      ( 0x4E50 | (An))                                                        /*   0100 1110 0101 0  An     */
#define CG_OP_UNLINK(An)                    ( 0x4E58 | (An))                                                        /*   0100 1110 0101 1  An     */
#define CG_OP_MOVE_TO_USP(An)               ( 0x4E60 | (An))                                                        /*   0100 1110 0110 0  An     */
#define CG_OP_MOVE_FROM_USP(An)             ( 0x4E68 | (An))                                                        /*   0100 1110 0110 1  An     */
#define CG_OP_RESET                           0x4E70                                                                /*   0100 1110 0111 0000      */
#define CG_OP_NOP                             0x4E71                                                                /*   0100 1110 0111 0001      */
#define CG_OP_STOP                            0x4E72                                                                /*   0100 1110 0111 0010      */
#define CG_OP_RTE                             0x4E73                                                                /*   0100 1110 0111 0011      */
#define CG_OP_RTS                             0x4E75                                                                /*   0100 1110 0111 0101      */
#define CG_OP_TRAPV                           0x4E76                                                                /*   0100 1110 0111 0110      */
#define CG_OP_RTR                             0x4E77                                                                /*   0100 1110 0111 0111      */
#define CG_OP_JSR(ADRMODE)                  ( 0x4E80 | (ADRMODE))                                                   /*   0100 1110 10 Adrmode     */
#define CG_OP_JMP(ADRMODE)                  ( 0x4EA0 | (ADRMODE))                                                   /*   0100 1110 11 Adrmode     */
#define CG_OP_MOVEM_R2M(SIZE,ADRMODE)       ( 0x4880 | ((SIZE-1) << 6) | (ADRMODE))                                 /*   0100 1000 1S Adrmode     */
#define CG_OP_MOVEM_M2R(SIZE,ADRMODE)       ( 0x4C80 | ((SIZE-1) << 6) | (ADRMODE))                                 /*   0100 1100 1S Adrmode     */
#define CG_OP_LEA(An,ADRMODE)               ( 0x41C0 | ((An) << 9)      | (ADRMODE))                                /*   0100 An 1 11 Adrmode     */
#define CG_OP_CHK(Dn,ADRMODE)               ( 0x4180 | ((Dn) << 9)      | (ADRMODE))                                /*   0100 An 1 11 Adrmode     */
                                                                                                                                                  
/* Opcode 5 */                                                                                                                                    
                                                                                                                                                  
#define CG_OP_ADDQ(Data,SIZE,ADRMODE)       ( 0x5000 | (((Data) & 7) << 9) | ((SIZE) << 6) | (ADRMODE))             /*   0101 Dat 0 S Adrmode     */
#define CG_OP_SUBQ(Data,SIZE,ADRMODE)       ( 0x5100 | (((Data) & 7) << 9) | ((SIZE) << 6) | (ADRMODE))             /*   0101 Dat 1 S Adrmode     */
#define CG_OP_SCC(COND,ADRMODE)             ( 0x50C0 | (COND << 8)                         | (ADRMODE))             /*   0101 Cond 11 Adrmode     */
#define CG_OP_DBCC(COND,Dn)                 ( 0x50C8 | (COND << 8)                         | (Dn))                  /*   0101 Cond 11 001 Dn      */
                                                                                                                                                  
/* Opcode 6 */                                                                                                                                    
                                                                                                                                                  
#define CG_OP_BRA(DISPLACEMENT)             ( 0x6000 | (DISPLACEMENT))                                              /*   0110 0000 Displace       */
#define CG_OP_BSR(DISPLACEMENT)             ( 0x6100 | (DISPLACEMENT))                                              /*   0110 0001 Displace       */
#define CG_OP_BCC(COND,DISPLACEMENT)        ( 0x6000 | ((COND) << 8) | (DISPLACEMENT))                              /*   0110 Cond Displace       */
                                                                                                                         
/* Opcode 7 */                                                                                                     
                                                                                                                   
#define CG_OP_MOVEQ(DATA,Dn)                ( 0x7000 | ((Dn) << 9) | (DATA))                                        /*   0111 Dn 0 Data           */

/* Opcode 8 */

#define CG_OP_DIVU(Dn,ADRMODE)              ( 0x80C0 | ((Dn) << 9)  | (ADRMODE))                                    /*   1000 Dn 0 11 Adrmode     */
#define CG_OP_DIVS(Dn,ADRMODE)              ( 0x81C0 | ((Dn) << 9)  | (ADRMODE))                                    /*   1000 Dn 1 11 Adrmode     */
#define CG_OP_SBCD_Dn(Xn1,Xn2)              ( 0x81C0 | ((Xn1) << 9) | (Xn2))                                        /*   1000 Xn1 10000 0 Xn2     */
#define CG_OP_SBCD_m0An0(Xn1,Xn2)           ( 0x81C8 | ((Xn1) << 9) | (Xn2))                                        /*   1000 Xn1 10000 1 Xn2     */
#define CG_OP_OR_R2M(SIZE,Dn,ADRMODE)       ( 0x8100 | ((Dn) << 9)  | ((SIZE) << 6) | (ADRMODE))                    /*   1000 Dn 1 Size Adrmode   */
#define CG_OP_OR_M2R(SIZE,Dn,ADRMODE)       ( 0x8000 | ((Dn) << 9)  | ((SIZE) << 6) | (ADRMODE))                    /*   1000 Dn 0 Size Adrmode   */
                                                                                                                  
/* Opcode 9 */

#define CG_OP_SUB_R2M(SIZE,Dn,ADRMODE)      ( 0x9100 | ((Dn) << 9) | ((SIZE) << 6) | (ADRMODE))                     /*   1001 Dn  1 Size Adrmode  */
#define CG_OP_SUB_M2R(SIZE,Dn,ADRMODE)      ( 0x9000 | ((Dn) << 9) | ((SIZE) << 6) | (ADRMODE))                     /*   1001 Dn  0 Size Adrmode  */
#define CG_OP_SUBX_Dn(SIZE,Xn1,Xn2)         ( 0x9100 | ((Xn1) << 9)| ((SIZE) << 6) | (Xn2))                         /*   1001 Xn1 1 Size 00 0 Xn2 */
#define CG_OP_SUBX_m0An0(SIZE,Xn1,Xn2)      ( 0x9108 | ((Xn1) << 9)| ((SIZE) << 6) | (Xn2))                         /*   1001 Xn1 1 Size 00 1 Xn2 */
#define CG_OP_SUBA(SIZE,An,ADRMODE)         ( 0x90C0 | ((An) << 9) | (((SIZE) - 1) << 8) | (ADRMODE))               /*   1001 An S 11     Adrmode */

/* Opcode 11 */

#define CG_OP_EOR_R2M(SIZE,Dn,ADRMODE)      ( 0xB100 | ((Dn) << 9) |                 ((SIZE) << 6) | (ADRMODE))     /*   1011 Dn  1 Size Adrmode  */
#define CG_OP_EOR_M2R(SIZE,Dn,ADRMODE)      ( 0xB000 | ((Dn) << 9) |                 ((SIZE) << 6) | (ADRMODE))     /*   1011 Dn  0 Size Adrmode  */
#define CG_OP_CMP_R2M(SIZE,Dn,ADRMODE)      ( 0xB100 | ((Dn) << 9) |                 ((SIZE) << 6) | (ADRMODE))     /*   1011 Dn  1 Size Adrmode  */
#define CG_OP_CMP_M2R(SIZE,Dn,ADRMODE)      ( 0xB000 | ((Dn) << 9) |                 ((SIZE) << 6) | (ADRMODE))     /*   1011 Dn  0 Size Adrmode  */
#define CG_OP_CMPM(SIZE,An1,An2)            ( 0xB108 | ((An1) << 9)                | ((SIZE) << 6) | (An2))         /*   1011 An1 1 Size 001 An2  */
#define CG_OP_CMPA(SIZE,An,ADRMODE)         ( 0xB108 | ((An) << 9) | (((SIZE)-1) << 8)             | (ADRMODE))     /*   1011 An Size 11  Adrmode */

/* Opcode 12 */

#define CG_OP_MULU(Dn,ADRMODE)              ( 0xC0C0 | ((Dn) << 9)  |                                (ADRMODE))     /*   1100 Dn 0 11 Adrmode     */
#define CG_OP_MULS(Dn,ADRMODE)              ( 0xC1C0 | ((Dn) << 9)  |                                (ADRMODE))     /*   1100 Dn 0 11 Adrmode     */
#define CG_OP_ABCD_Dn(Xn1,Xn2)              ( 0xC100 | ((Xn1) << 9) | (Xn2))                                        /*   1100 Xn1 10000 0 Xn2     */
#define CG_OP_ABCD_m0An0(Xn1,Xn2)           ( 0xC108 | ((Xn1) << 9) | (Xn2))                                        /*   1100 Xn1 10000 1 Xn2     */
#define CG_OP_EXG_Dn_Dn(Xn1,Xn2)            ( 0xC100 | ((Xn1) << 9) | (Xn2))                                        /*   1100 Xn1 1 0 00 0 Xn2    */
#define CG_OP_EXG_An_An(Xn1,Xn2)            ( 0xC188 | ((Xn1) << 9) | (Xn2))                                        /*   1100 Xn1 1 1 00 1 Xn2    */
#define CG_OP_EXG_Dn_An(Xn1,Xn2)            ( 0xC108 | ((Xn1) << 9) | (Xn2))                                        /*   1100 Xn1 1 0 00 1 Xn2    */
#define CG_OP_EXG_An_Dn(Xn1,Xn2)            ( 0xC180 | ((Xn1) << 9) | (Xn2))                                        /*   1100 Xn1 1 1 00 0 Xn2    */
#define CG_OP_AND_R2M(SIZE,Dn,ADRMODE)      ( 0xC100 | ((Dn) << 9)  |  ((SIZE) << 6)               | (ADRMODE))     /*   1100 Dn  1 Size Adrmode  */
#define CG_OP_AND_M2R(SIZE,Dn,ADRMODE)      ( 0xC000 | ((Dn) << 9)  |  ((SIZE) << 6)               | (ADRMODE))     /*   1100 Dn  0 Size Adrmode  */

/* Opcode 13 */

#define CG_OP_ADD_R2M(SIZE,Dn,ADRMODE)      ( 0xD100 | ((Dn) << 9) |                 ((SIZE) << 6) | (ADRMODE))     /*   1101 Dn  1 Size Adrmode  */
#define CG_OP_ADD_M2R(SIZE,Dn,ADRMODE)      ( 0xD000 | ((Dn) << 9) |                 ((SIZE) << 6) | (ADRMODE))     /*   1101 Dn  0 Size Adrmode  */
#define CG_OP_ADDX_R2M(SIZE,Xn1,Xn2)        ( 0xD108 | ((Xn1) << 9)| ((SIZE) << 6) | (Xn2))                         /*   1101 Xn1 1 Size 00 1 Xn2 */
#define CG_OP_ADDX_M2R(SIZE,Xn1,Xn2)        ( 0xD100 | ((Xn1) << 9)| ((SIZE) << 6) | (Xn2))                         /*   1101 Xn1 1 Size 00 0 Xn2 */
#define CG_OP_ADDA(SIZE,An,ADRMODE)         ( 0xD0C0 | ((An) << 9) | (((SIZE) - 1) << 8)           | (ADRMODE))     /*   1101 An S 11     Adrmode */
                                          
/* Opcode 14 */

#define CG_OP_ASR(ADRMODE)                  ( 0xE0C0 | (ADRMODE))                                                   /*   1110 000 0 11 Adrmode    */
#define CG_OP_ASL(ADRMODE)                  ( 0xE1C0 | (ADRMODE))                                                   /*   1110 000 1 11 Adrmode    */
#define CG_OP_LSR(ADRMODE)                  ( 0xE2C0 | (ADRMODE))                                                   /*   1110 001 0 11 Adrmode    */
#define CG_OP_LSL(ADRMODE)                  ( 0xE3C0 | (ADRMODE))                                                   /*   1110 001 1 11 Adrmode    */
#define CG_OP_ROXR(ADRMODE)                 ( 0xE4C0 | (ADRMODE))                                                   /*   1110 010 0 11 Adrmode    */
#define CG_OP_ROXL(ADRMODE)                 ( 0xE5C0 | (ADRMODE))                                                   /*   1110 010 1 11 Adrmode    */
#define CG_OP_ROR(ADRMODE)                  ( 0xE6C0 | (ADRMODE))                                                   /*   1110 011 0 11 Adrmode    */
#define CG_OP_ROL(ADRMODE)                  ( 0xE7C0 | (ADRMODE))                                                   /*   1110 011 1 11 Adrmode    */
                                                                                                                    
#define CG_OP_ASR_IMM(SIZE,ROT,Dn)          ( 0xE000 | ((ROT)   << 9) | ((SIZE) << 6) | (Dn))                       /*   1110 Rot 0 S 0 00 Dn     */
#define CG_OP_ASL_IMM(SIZE,ROT,Dn)          ( 0xE100 | ((ROT)   << 9) | ((SIZE) << 6) | (Dn))                       /*   1110 Rot 1 S 0 00 Dn     */
#define CG_OP_LSR_IMM(SIZE,ROT,Dn)          ( 0xE008 | ((ROT)   << 9) | ((SIZE) << 6) | (Dn))                       /*   1110 Rot 0 S 0 01 Dn     */
#define CG_OP_LSL_IMM(SIZE,ROT,Dn)          ( 0xE108 | ((ROT)   << 9) | ((SIZE) << 6) | (Dn))                       /*   1110 Rot 1 S 0 01 Dn     */
#define CG_OP_ROXR_IMM(SIZE,ROT,Dn)         ( 0xE010 | ((ROT)   << 9) | ((SIZE) << 6) | (Dn))                       /*   1110 Rot 0 S 0 10 Dn     */
#define CG_OP_ROXL_IMM(SIZE,ROT,Dn)         ( 0xE110 | ((ROT)   << 9) | ((SIZE) << 6) | (Dn))                       /*   1110 Rot 1 S 0 10 Dn     */
#define CG_OP_ROR_IMM(SIZE,ROT,Dn)          ( 0xE018 | ((ROT)   << 9) | ((SIZE) << 6) | (Dn))                       /*   1110 Rot 0 S 0 11 Dn     */
#define CG_OP_ROL_IMM(SIZE,ROT,Dn)          ( 0xE118 | ((ROT)   << 9) | ((SIZE) << 6) | (Dn))                       /*   1110 Rot 1 S 0 11 Dn     */
                                                                                                                                                  
#define CG_OP_ASR_REG(SIZE,ROTDn,Dn)        ( 0xE020 | ((ROTDn) << 9) | ((SIZE) << 6) | (Dn))                       /*   1110 Rot 0 S 1 00 Dn     */
#define CG_OP_ASL_REG(SIZE,ROTDn,Dn)        ( 0xE120 | ((ROTDn) << 9) | ((SIZE) << 6) | (Dn))                       /*   1110 Rot 1 S 1 00 Dn     */
#define CG_OP_LSR_REG(SIZE,ROTDn,Dn)        ( 0xE028 | ((ROTDn) << 9) | ((SIZE) << 6) | (Dn))                       /*   1110 Rot 0 S 1 01 Dn     */
#define CG_OP_LSL_REG(SIZE,ROTDn,Dn)        ( 0xE128 | ((ROTDn) << 9) | ((SIZE) << 6) | (Dn))                       /*   1110 Rot 1 S 1 01 Dn     */
#define CG_OP_ROXR_REG(SIZE,ROTDn,Dn)       ( 0xE030 | ((ROTDn) << 9) | ((SIZE) << 6) | (Dn))                       /*   1110 Rot 0 S 1 10 Dn     */
#define CG_OP_ROXL_REG(SIZE,ROTDn,Dn)       ( 0xE130 | ((ROTDn) << 9) | ((SIZE) << 6) | (Dn))                       /*   1110 Rot 1 S 1 10 Dn     */
#define CG_OP_ROR_REG(SIZE,ROTDn,Dn)        ( 0xE038 | ((ROTDn) << 9) | ((SIZE) << 6) | (Dn))                       /*   1110 Rot 0 S 1 11 Dn     */
#define CG_OP_ROL_REG(SIZE,ROTDn,Dn)        ( 0xE138 | ((ROTDn) << 9) | ((SIZE) << 6) | (Dn))                       /*   1110 Rot 1 S 1 11 Dn     */


/*--------------------------------------------------------------------
   Legacy macros 
--------------------------------------------------------------------*/

#define CGEN_0_MOVE_B 0x1000
#define CGEN_0_MOVE_L 0x2000
#define CGEN_0_MOVE_W 0x3000

#define CGEN_0_ADDQ   0x5000
#define CGEN_0_SUBQ   0x5000
#define CGEN_0_SCC    0x5000
#define CGEN_0_DBCC   0x5000
#define CGEN_0_TRAP   0x5000

#define CGEN_0_BCC    0x6000
#define CGEN_0_BSR    0x6000
#define CGEN_0_BRA    0x6000

#define CGEN_0_MOVEQ  0x7000

#define CGEN_0_OR     0x8000
#define CGEN_0_DIV    0x8000
#define CGEN_0_SBCD   0x8000

#define CGEN_0_SUB    0x9000
#define CGEN_0_SUBX   0x9000

#define CGEN_0_CMP    0xB000
#define CGEN_0_EOR    0xB000

#define CGEN_0_AND    0xC000
#define CGEN_0_MUL    0xC000
#define CGEN_0_ABCD   0xC100
#define CGEN_0_EXG    0xC000

#define CGEN_0_ADD    0xD000
#define CGEN_0_ADDX   0xD000

#define CGEN_0_SHIFT  0xE000
#define CGEN_0_ROTATE 0xE000


#define CGEN_MOVESIZE_B 0x1000
#define CGEN_MOVESIZE_L 0x2000
#define CGEN_MOVESIZE_W 0x3000

#define CGEN_OPSIZE_B 0x00
#define CGEN_OPSIZE_W 0x40
#define CGEN_OPSIZE_L 0x80

#define CGEN_OPCODE_NOP                                  0x4E71
#define CGEN_OPCODE_RTE                                  0x4E73
#define CGEN_OPCODE_RTS                                  0x4E75
                                                         
#define CGEN_OPCODE_CLR_B_SHORTADR                       0x4238
#define CGEN_OPCODE_CLR_W_SHORTADR                       0x4278
#define CGEN_OPCODE_CLR_L_SHORTADR                       0x42B8

#define CGEN_OPCODE_MOVE_IMMEDIATE_SHORTADR(SIZE)       (0x01FC |  (SIZE))
#define CGEN_OPCODE_MOVE_Dx_SHORTADR(SIZE,REG)          (0x01C0 |  (SIZE) | (REG))
#define CGEN_OPCODE_MOVE_0Ax0_P_Dy(SIZE,REG1,REG2)      (0x0018 |  (SIZE) | ((REG2) << 9) | (REG1))
#define CGEN_OPCODE_MOVE_Dx_0Ay0_P(SIZE,REG1,REG2)      (0x00C0 |  (SIZE) | ((REG2) << 9) | (REG1))
#define CGEN_OPCODE_MOVE_Dx_0Ay0_M(SIZE,REG1,REG2)      (0x0100 |  (SIZE) | ((REG2) << 9) | (REG1))
#define CGEN_OPCODE_MOVE_Dx_0Ay0(SIZE,REG1,REG2)        (0x0080 |  (SIZE) | ((REG2) << 9) | (REG1))
#define CGEN_OPCODE_MOVE_Dx_0Ay0_I(SIZE,REG1,REG2)      (0x0140 |  (SIZE) | ((REG2) << 9) | (REG1))
#define CGEN_OPCODE_MOVE_IMMEDIATE_Dx(SIZE,REG)         (0x003C | ((SIZE) | ((REG) << 9)))
#define CGEN_OPCODE_MOVE_IMMEDIATE_0Ax0(SIZE,REG)       (0x00BC |  (SIZE) | ((REG) << 9))
#define CGEN_OPCODE_MOVE_IMMEDIATE_0Ax0_P(SIZE,REG)     (0x00FC |  (SIZE) | ((REG) << 9))
#define CGEN_OPCODE_MOVE_IMMEDIATE_0Ax0_I(SIZE,REG)     (0x017C |  (SIZE) | ((REG) << 9))
#define CGEN_OPCODE_MOVE_0Ax0_P_SHORTADR(SIZE,REG)      (0x01D8 |  (SIZE) | (REG))
#define CGEN_OPCODE_MOVE_L_Ax_Ay(X,Y)                   (0x2048 |  ((Y) << 9) | (X))
#define CGEN_OPCODE_MOVE_L_0Ax0_P_Ay(X,Y)               (0x2058 |  ((Y) << 9) | (X))
#define CGEN_OPCODE_MOVE_0Ax0_P_0Ay0(SIZE,REG1,REG2)    (0x0098 |  (SIZE) | ((REG2) << 9) | (REG1))
#define CGEN_OPCODE_MOVE_Ax_0Ay0(SIZE,REG1,REG2)        (0x0088 |  (SIZE) | ((REG2) << 9) | (REG1))
#define CGEN_OPCODE_MOVEQ_L_X_Dy(X,Y)                   (0x7000 |  ((Y) << 9) | ((X) & 0xFF))
#define CGEN_OPCODE_MOVE_USP_Ax(REG)                    (0x4E68 |  (REG))
#define CGEN_OPCODE_MOVE_Ax_USP(REG)                    (0x4E60 |  (REG))


#define CGEN_OPCODE_MOVEM_L_REGS_M_SP                    0x48E7
#define CGEN_OPCODE_MOVEM_L_SP_P_REGS                    0x4CDF

#define CGEN_OPCODE_SUBQ_X_Dy(SIZE,X,Y)                 (0x5100 | (((X) << 9) & 0xE00) | (Y) | (SIZE))
#define CGEN_OPCODE_ADDQ_L_X_Ay(X,Y)                    (0x5288 | (((X) << 9) & 0xE00) | (Y))
#define CGEN_OPCODE_LEA_X_Ax_Ay(X,Y)                    (0x41E8 |  ((Y) << 9) | (X))
#define CGEN_OPCODE_LEA_SHORTADR_Ay(Y)                  (0x41F8 |  ((Y) << 9))
#define CGEN_OPCODE_LEA_LONGADR_Ay(Y)                   (0x41F9 |  ((Y) << 9))
#define CGEN_OPCODE_OR_L_IMMEDIATE_0Ax0_P(X)            (0x0098 |   (X))
#define CGEN_OPCODE_OR_L_Dx_0Ay0_P(X,Y)                 (0x8198 |  ((X) << 9) | (Y))
#define CGEN_OPCODE_OR_L_Dx_0Ay0(X,Y)                   (0x8190 |  ((X) << 9) | (Y))
#define CGEN_OPCODE_OR_W_IMMEDIATE_I0Ax0(X)             (0x0068 |   (X))
#define CGEN_OPCODE_OR_B_Dx_0Ay0_I(X,Y)                 (0x8128 |  ((X) << 9) | (Y))
#define CGEN_OPCODE_OR_B_0Ax0_Dy(X,Y)                   (0x8010 |  ((Y) << 9) | (X))
#define CGEN_OPCODE_AND_B_Dx_0Ay0_I(X,Y)                (0xC128 |  ((X) << 9) | (Y))
#define CGEN_OPCODE_AND_L_Dx_0Ay0_P(X,Y)                (0xC198 |  ((X) << 9) | (Y))
#define CGEN_OPCODE_AND_L_Dx_0Ay0(X,Y)                  (0xC190 |  ((X) << 9) | (Y))
#define CGEN_OPCODE_AND_Dx_Dy(SIZE,X,Y)                 (0xC000 |  ((Y) << 9) | (X) | (SIZE))
#define CGEN_OPCODE_OR_Dx_Dy(SIZE,X,Y)                  (0x8000 |  ((Y) << 9) | (X) | (SIZE))
#define CGEN_OPCODE_OR_IMMEDIATE_Dy(SIZE,Y)             (0x0000 |  (Y) | (SIZE))
#define CGEN_OPCODE_AND_IMMEDIATE_Dy(SIZE,Y)            (0x0200 |  (Y) | (SIZE))
#define CGEN_OPCODE_AND_IMMEDIATE_0Ay0_I(SIZE,Y)        (0x0228 |  (Y) | (SIZE))
#define CGEN_OPCODE_AND_Dx_0Ay0_I(SIZE,X,Y)             (0xC128 |  (Y) | ((X) << 9) | (SIZE))
#define CGEN_OPCODE_ADD_W_0Ax0_P_Ay(X,Y)                (0xD0DA |  ((Y) << 9) | (X))
#define CGEN_OPCODE_ADD_W_Dx_Dy(X,Y)                    (0xD040 |  ((Y) << 9) | (X))
#define CGEN_OPCODE_ADD_W_Dx_0Ay0(X,Y)                  (0xD150 |  ((X) << 9) | (Y))
#define CGEN_OPCODE_SUB_B_Dx_Dy(X,Y)                    (0x9000 |  ((Y) << 9) | (X))
#define CGEN_OPCODE_BCLR_Dx_0Ay0_I(X,Y)                 (0x01A8 |  ((X) << 9) | (Y))
#define CGEN_OPCODE_JMP_0Ax0(X)                         (0x4ED0 |   (X))
#define CGEN_OPCODE_JMP_ABS_L                            0x4EF9
#define CGEN_OPCODE_B_S(CONDITION,OFFSET)               (0x6000 | ((CONDITION) << 8) | (OFFSET))
#define CGEN_OPCODE_S_Dx(CONDITION,REG)                 (0x50C0 | ((CONDITION) << 8) | (REG))  
#define CGEN_OPCODE_S_0Ax0(CONDITION,X)                 (0x50D0 | ((CONDITION) << 8) | (X)) 
#define CGEN_OPCODE_NEG_B(REG)                          (0x4400 |  (REG))
#define CGEN_OPCODE_LSL_W_Dx_Dy(X,Y)                    (0xE168 | ((X) << 9) | (Y))



struct CGENdesc_
{
    u8  opcodelen_div2;    
    u8  cycles_div2;
    u8* codeaddress;
};
typedef struct CGENdesc_ CGENdesc;

u8* aCGENaddnops(u8* _output, u16 _cycles);

#define CGENgenerateSimple(_desc, _output)\
{   s16 len = ((_desc)->opcodelen_div2) << 1;\
    STDmcpy2(_output, (_desc)->codeaddress, len);\
    _output += len; }

#define CGENgenerate(_desc, _cycles, _output) { CGENgenerateSimple(_desc, _output) _cycles += ((_desc)->cycles_div2) << 1; }

#define CGENaddNops(_currentCycles, _targetCycles, _output)\
{   s16  diffCycles = ((s16)(_targetCycles) - (s16)(_currentCycles));\
    ASSERT(diffCycles >= 0);\
    ASSERT((diffCycles & 3) == 0);\
    _output = aCGENaddnops(_output, diffCycles); }

void aCGENpatchWords(void* _address, u16* _offsets, u32 _values, u16 _nbvalues);

#define CGENpatchWords(_adr, _offsets, _values, _nbvalues) {\
    STATIC_ASSERT((_nbvalues) <= 20); \
    aCGENpatchWords(_adr, _offsets, (u32)(_values), _nbvalues); }

u16 CGENfindWords(CGENdesc* _desc, u16* _offsets, u16 _value);

#endif
