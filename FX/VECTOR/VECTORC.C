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

#ifndef __TOS__

#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\ALLOC.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\HARDWARE.H"

#include "DEMOSDK\PC\EMUL.H"

#include "FX\VECTOR\VECTOR.H"


// Back port ST 68k ASM routines to use them on PC

#define VEC_XMAX  335
#define VEC_YMAX  199

static void vecClipx(EMUL68k* m);

// ------------------------------------------------------------------------
//   PRECOMPUTE LINE CLIPPING AND PARAMETERS
// ------------------------------------------------------------------------
//   IN:
//     a0......precomputed data
//     a1......coords s16 x1,y1,x2,y2
//     d0......nbedges adr
//   OUT:
//     a0
// ------------------------------------------------------------------------
u16* VECclipline(u16* dlist, u16* coord, u32 _pnbedges)
{
    EMUL68k m;

    
    m.a0.l = (u32) dlist;
    m.a1.l = (u32) coord;
    m.a3.l = _pnbedges;

    m.d0.w = *(u16*)m.a1.l; m.a1.l += 2;            // move.w  (a1)+,d0
    m.d1.w = *(u16*)m.a1.l; m.a1.l += 2;            // move.w  (a1)+,d1
    m.d2.w = *(u16*)m.a1.l; m.a1.l += 2;            // move.w  (a1)+,d2
    m.d3.w = *(u16*)m.a1.l; m.a1.l += 2;            // move.w  (a1)+,d3
    
    if ((s16)m.d1.w > (s16)m.d3.w)                  // cmp.w	d1,d3			* On ordonne les coordonnees
    {                                               // bge.s	.line_yordered	* verticales provisoirement y2 > y1
        STD_SWAP(s32,m.d0.l,m.d2.l);                // exg.l	d0,d2			* pour le clip vertical
        STD_SWAP(s32,m.d1.l,m.d3.l);                // exg.l	d1,d3			*
    }                                               // .line_yordered:
   
    if ((s16)m.d3.w < 0)                            // tst.w	d3				* Si l'ordonnee la plus grande
    {                                               // bge.s	.line_y2positive * est negative (y2 < 0) on trace la droite
        m.d3.l = 0;                                 // moveq.l	#0,d3			* entre x1 et x2 (passe xor oblige)
        m.d1.l = 0;                                 // moveq.l	#0,d1			* On se branche au niveau du clip 
        goto clipx;                                 // bra.s	.clipx			* horizontal    
    }                                               //.line_y2positive:

    if ((s16)m.d1.w >= 0)                           // tst.w	d1				* Si l'ordonnee la plus petite
        goto clipy2;                                // bge.s	.clipy2			* negative et l'autre positive

    // On calcule l'intersection avec le bord superieur clipy

    m.d5.w  = m.d0.w;                               // move.w	d0,d5			*
    m.d5.w -= m.d2.w;                               // sub.w	d2,d5			* d5 = x1-x2
    m.d6.w  = m.d3.w;                               // move.w	d3,d6			* d6 = y2-y1
    m.d6.w -= m.d1.w;                			    // sub.w	d1,d6

    m.d5.l  = STDmuls(m.d1.w, m.d5.w);               // muls.w	d1,d5			* (x1-x2) * y1
    m.d5.l  = STDdivs(m.d5.l, m.d6.w);               // divs.w	d6,d5			* (x1-x2) * y1 / (y2-y1)
    m.d5.w += m.d0.w;                                // add.w	d0,d5			* (x1-x2) * y1 / (y2-y1) + x1 => d5: abscisse d'intersection

    {
        u16 d0_bak = m.d0.w;                         // movem.w	d0-d3/d5/d6,-(sp)
        u16 d1_bak = m.d1.w;
        u16 d2_bak = m.d2.w;
        u16 d3_bak = m.d3.w;
        u16 d5_bak = m.d5.w;
        u16 d6_bak = m.d6.w;

        m.d1.l = 0;                                  // moveq.l	#0,d1			*
        m.d3.l = 0;                                  // moveq.l	#0,d3			*

        m.d2.w = m.d5.w;                             // move.w	d5,d2			*
                
        *(u16*)m.a3.l += 1;                          // addq.w  #1,(a3)            * Inc nb edges

        vecClipx(&m);                                // bsr.s	.clipx	    	  * On trace le bout de droite en haut de l'ecran horizontalement
        
        m.d0.w = d0_bak;                             // movem.w	(sp)+,d0-d3/d5/d6 * (passe xor oblige)
        m.d1.w = d1_bak;
        m.d2.w = d2_bak;
        m.d3.w = d3_bak;
        m.d5.w = d5_bak;
        m.d6.w = d6_bak;
    }

    m.d0.w = m.d5.w;                                 // move.w	d5,d0		      * On trace le bout de droite 
    m.d1.l = 0;                                      // moveq.l	#0,d1		      * restant sur l'ecran 
                                                     //                           * coordonnees non inversees
clipy2:

    if ((s16)m.d1.w > VEC_YMAX)                      // cmp.w	#ymax,d1		  * Si l'ordonnee la plus petite
        goto nothing;                                // bgt.s	.nothing		  * est superieure … ymax                                
                                                    
    if ((s16)m.d3.w <= VEC_YMAX)                     // cmp.w	#ymax,d3		  * Si l'ordonnee la plus grande>ymax
        goto clipx;                                  // ble.s	.clipx			  * et l'autre <ymax
                                                     // On calcule l'intersection avec le bord superieur
    m.d2.w -= m.d0.w;                                // sub.w	d0, d2            * d2 = x2 - x1    
    m.d3.w -= m.d1.w;                                // sub.w	d1, d3            * d3 = y2 - y1
    m.d5.w  = VEC_YMAX;                              // move.w	#ymax, d5        
    m.d5.w -= m.d1.w;                                // sub.w    d1, d5           * d1 = y1 - ymax
                                                    
    m.d2.l  = STDmuls(m.d5.w, m.d2.w);               // muls.w	d5, d2            * (x2 - x1)* (y1 - ymax)
    m.d2.l  = STDdivs(m.d2.l, m.d3.w);               // divs.w	d3, d2            * (x2 - x1)* (y1 - ymax) / (y2 - y1)
    m.d2.w += m.d0.w;                                // add.w	    d0, d2        * (x2 - x1)* (y1 - ymax) / (y2 - y1) + x1
                                                     //                           * d2: abscisse d'intersection
    m.d3.w  = VEC_YMAX;                              // move.w	#ymax, d3

    //----------------------------
    // clipx
    //----------------------------
clipx:
    vecClipx(&m);
    return (void*) m.a0.l;

nothing:
    *(u16*)(m.a3.l + 2) += 1;                         // .nothing: addq.w  #1,2(a3)
    return (void*) m.a0.l;
}

static void vecClipx(EMUL68k* m)
{
    if ((s16)m->d0.w > (s16)m->d2.w)               // cmp.w	d2, d0
    {                                              // ble.s.clipx_ok1   * Si x1 > x2: Echange x1 x2& y1 y2
        STD_SWAP(s32, m->d0.l, m->d2.l);           // exg.l	d2, d0
        STD_SWAP(s32, m->d1.l, m->d3.l);           // exg.l	d3, d1
    }                                              // .clipx_ok1 :
                                                 
    if (((s16)m->d2.w) < 0)                        // tst.w	d2          * if d2 < 0 = > do nothing
        goto nothing;                              // bge.s.clipx_ok2
                                                   // .nothing:	
                                                   // addq.w  #1,2(a3)
                                                   // rts
                                                   // .clipx_ok2:
                                                 
    m->d7.w = m->d0.w;                             // move.w	d0,d7			* if x1 < 0 => store to fix length when displaying
    if ((s16)m->d7.w >= 0)                         // blt.s	.clipx_ok3
    {                                            
        m->d7.l = 0;                               // moveq.l	#0,d7
    }                                              // .clipx_ok3:
                                                  
    if ((s16)m->d0.w > VEC_XMAX)                   // cmp.w	    #xmax,d0		* Si l'abscisse la plus petite
        goto nothing;                              // bgt.s	    .nothing		* est superieure à xmax * => on trace pas
                                                                                        
    if ((s16)m->d2.w > VEC_XMAX)                   // cmp.w 	#xmax,d2		* Si l'abscisse la plus grande > xmax
    {                                              // ble.s	    .preparedraw	
                                                   // On calcule l'intersection avec le bord superieur
        m->d2.w -= m->d0.w;                        // sub.w	    d0,d2			* d2 = x2-x1
        m->d3.w -= m->d1.w;                        // sub.w	    d1,d3			* d3 = y2-y1
        m->d5.w  = VEC_XMAX;                       // move.w	#xmax,d5	
        m->d5.w -= m->d0.w;                        // sub.w	d0,d5	            * d0 = xmax-x1  
        m->d3.l  = STDmuls(m->d5.w, m->d3.w);      // muls.w	d5,d3	        * (y2-y1) * (xmax-x1)
        m->d3.l  = STDdivs(m->d3.l, m->d2.w);      // divs.w	d2,d3	        * (y2-y1) * (xmax-x1) / (x2-x1)

        m->d3.w += m->d1.w;                        // add.w	d1,d3			    * (y2-y1) * (xmax-x1) / (x2-x1)+y1
                                                   //                           * d3: ordonnee d'intersection
        m->d2.w = VEC_XMAX;                        // move.w	#xmax,d2		
    }
        
    //---------------------------------
    // prepare draw line
    //---------------------------------

    // here we have d0,d1,d2,d3,d7 => x1, y1, x2, y2, lengthfix from x1 clip
    
    // .preparedraw:	
    
    m->a2.l  = -VEC_PITCH;                         // lea	    -pitch.w,a2 	*
    m->d5.w  = 0x8080;                             // move.w   #0x8000,d5       * d5 bit 0 = sign of vertical increment
    m->d0.w -= m->d2.w;                            // sub.w    d2,d0			* d0: largeur									
    if (m->d0.w == 0)                              // beq.s	.nothing		    * if dx = 0 => do nothing
        goto nothing;
    
    m->d0.w  = -m->d0.w;                           // neg.w	d0				        
    m->d1.w -= m->d3.w;                            // sub.w	d3,d1			    * d1: hauteur
    
    if (m->d1.w == 0)
        goto h_line;                               // beq		h_line			* if dy = 0 => hline							

    if (((s16)m->d1.w) >= 0)                       // blt.s	.prepare_draw_ok
    {                               
        m->a2.l = VEC_PITCH;                       // lea		pitch.w,a2  	* if dy negative => invert address increment
        m->d5.l = 0;                               // moveq.l #0,d5             
        m->d1.w = -m->d1.w;                        // neg.w	d1				    * abs (dy)
    }                                              // .prepare_draw_ok:	

    m->d1.w = -m->d1.w;                            // neg.w	d1				    
    m->d3.w = (s16) STDmuls(VEC_PITCH, m->d3.w);   // lea	pitchmul(pc),a1		* compute start address : a0 += y2 * pitch
                                                   // add.w	d3,d3			    
                                                   // move.w	(a1,d3.w),d3	 
    *((u16*)m->a0.l) = m->d3.w;                    // move.w  d3,(a0)                   

    // ================ will store draw method into bits 1 & 2 of offset (unused by the planes)
    if (m->d1.w > m->d0.w)                         // cmp.w	d0,d1			    * Compare dx & dy
        goto vertical;                             // bgt  	vertical		    * => dy > dx => vertical routine

    if (m->d1.w == m->d0.w)
        goto d45;                                  // beq 	d45				    * => equal	 => 45° routine
    
    // * => dx > dy => horizontal routine

    //------------------------------------------------------------------
    //	Dx>Dy
    //
    //  STORAGE :
    //      w: offset address | ((pitch inc < 0) ? $8000 : 0) | 0
    //      w: enter routine offset  => on PC x2     * 4
    //      w: exit routine offset   => on PC length * 4
    //      w: error increment
    //------------------------------------------------------------------
    //horizontal:	

    EMUL_SWAP(&(m->d1));                        // swap	d1				    * dy*65536
    m->d1.w = 0;                                // sub.w	d1,d1			
    m->d1.l = STDdivu (m->d1.l, m->d0.w);       // divu.w	d0,d1			* d1: increment dy*65536/dx 				
                                               
    m->d0.w += m->d7.w;                         // add.w	d7,d0			* fix length according to x1 clip
                                               
                                                // add.w	d2,d2			* start address : get values at x2 * 4
                                                // add.w	d2,d2
    
    m->d7.w = m->d2.w >> 1;
    m->d7.w &= 0xFFF8;                          // on PC store offset rather than // move.l	vecLineDisplayHorizTab(pc,d2.w),d7
    //m->d7.w |= m->d5.w;                       // or.w	d5,d7     		    * add x2 address offset to a0			================= STORE / ADD HORIZONTAL OFFSET d7
                                                // or.w    #0,(a0)                                                  ================= LINE TYPE        
    *(u16*)m->a0.l += m->d7.w;                  // add.w   d7,(a0)+         * set inc pitch sign into high bit
    m->a0.l += 2;                                     
    //EMUL_SWAP(&(m->d7));                      // swap	d7
    m->d7.w  = 15 - (m->d2.w & 15);
    m->d5.b  = 0;
    m->d7.w |= m->d5.w;
    *(u16*)m->a0.l = PCENDIANSWAP16(m->d7.w);   // move.w	d7,(a0)+		* compute offset into routine			================= STORE OPCODE OFFSET (WORD)
    m->a0.l += 2;                               // on PC => rol bit

                                                // add.w	d0,d0			* end address into routine : x2 * 4 + length * 4
                                                // add.w	d0,d0
                                                // sub.w	d0,d2
   
    *(u16*)m->a0.l = m->d0.w;                   // move.w	vecLineDisplayHorizTab(pc,d2.w),(a0)+		================= STORE OPCODE OFFSET END D2 (WORD)
    m->a0.l += 2;                               // on PC => store length 

    *(u16*)m->a0.l = m->d1.w;                   // move.w  d1,(a0)+         *                                       ================ STORE INCREMENT (WORD)
    m->a0.l += 2;

    return;                                     // rts

nothing:
    *(u16*)(m->a3.l + 2) += 1;                    // .nothing: addq.w  #1,2(a3)

    return;                                     // rts

    //------------------------------------------------------------------
    //  Dx<Dy
    //
    //  STORAGE:
    //     w: offset address | ((pitch inc < 0) ? $8000 : 0) | 2
    //     b: rol bit number
    //     b: increment error
    //     w: vertical offset increment
    //     w: offset into routine => on PC nb loops
    //------------------------------------------------------------------

vertical:

    *(u16*)m->a0.l |= 2;                        // or.w    d5,(a0)         * ===================== STORE PITCH SIGN

    m->d1.l <<= 8;                              // lsl.l	#8,d1
    m->d1.l  &= 0xFFFF00;                       // and.l	#$ffff00,d1

    m->d1.l = STDdivu(m->d1.l, m->d0.w);        // divu.w	d0,d1		   * d0: increment dx*65536/dy 	
    
    m->d0.w += m->d7.w;                         // add.w	d7,d0

                                                // add.w	d2,d2
                                                // add.w	d2,d2
    m->d4.w = m->d2.w >> 1;                     // lea	table2(pc),a1
    m->d4.w &= 0xFFF8;                          // move.l	(a1,d2.w),d4
    *(u16*)m->a0.l += m->d4.w;                  // add.w	d4,(a0)+	   * ==================== STORE D4 OFFSET
    m->a0.l += 2;    
                                                
    m->d4.w = 15 - (m->d2.w & 15);              // swap	d4				
    m->d4.b |= m->d5.b;
    *(u8*)m->a0.l = m->d4.b;                    // move.b  d4,(a0)+        * ==================== STORE D4 ROL BIT
    m->a0.l++;

    *(u8*)m->a0.l = m->d1.b;                    // move.b  d1,(a0)+        * ==================== STORE D1 INCREMENT	(BYTE)
    m->a0.l++;

    m->d1.w >>= 8;                              // lsr.w	#8,d1

    m->d6.l = m->a2.l;                          // move.l  a2,d6
    m->d6.l = STDmuls(m->d1.w, m->d6.w);        // muls.w	d1,d6														
    *(u16*)m->a0.l = m->d6.w;                   // move.w  d6,(a0)+        * ==================== STORE D6 ADDRESS VERTICAL INCREMENT LINE
    m->a0.l += 2;
    
    *(u16*)m->a0.l = m->d0.w;                   // move.w	d0,d5		   * compute jmp distance
    m->a0.l += 2;                               // lsl.w	#4,d0
                                                // add.w	d5,d5
                                                // add.w	d5,d0
                                                // neg.w	d0
                                                // move.w  d0,(a0)+        * ===================== STORE OFFSET INTO ROUTINE
                                                
                                                // => on PC stores length instead of offset

    return;                                     // rts

    //------------------------------------------------------------------
    //   HLINE
    //
    //   STORAGE:
    //      w: offset address | 4 | ((pitch inc < 0) ? $8000 : 0)
    //      b: start mask selection
    //      b: end mask selection
    //      w: nb words or 0 when same word
    //------------------------------------------------------------------

h_line:	
    m->d3.w *= VEC_PITCH;       // lea	pitchmul(pc),a1		 
                                // add.w	d3,d3			* stored on word expressing pitch / 2		
                                // move.w	(a1,d3.w),d3	* get the value

    m->d0.w += m->d7.w;         // add.w	d7,d0			* fix length according to x1 clip
    m->d1.w  = m->d2.w;         // move.w	d2,d1			* d2 = x2
    m->d1.w -= m->d0.w;         // sub.w	d0,d1			* d1 = x1

    m->d0.l   = -16;            // moveq.l	#-16,d0
    m->d0.w  &= m->d1.w;        // and.w	d1,d0
    m->d0.w >>= 1;              // lsr.w	#1,d0

    m->d3.w  |= 4;              // or.w    #4,d3            * ===================== STORE HLINE OPERATION SELECTION (do not or because nothing writen in a0 before unlike other methods)
    m->d3.w  += m->d0.w;        // add.w	d0,d3           * d0 = (x1 & FFF0) >> 1						
    *(u16*)m->a0.l = m->d3.w;   // move.w  d3,(a0)+	        * ============== STORE OFFSET D3 + D1
    m->a0.l  += 2;

    m->d5.l  = -16;             // moveq.l	#-16,d5
    m->d5.w &= m->d2.w;         // and.w	d2,d5
    m->d5.w >>= 1;              // lsr.w	#1,d5			* d5 = (x2 & FFF0) >> 1
    
    m->d3.l  = 15;              // moveq.l	#15,d3
    m->d3.w &= m->d1.w;         // and.w	d1,d3			* d3 = x1 & 15
    m->d3.w <<= 1;              // add.w	d3,d3
    *(u8*)m->a0.l = m->d3.b;    // move.b  d3,(a0)+
    m->a0.l++;

    m->d6.l  = 15;              // moveq.l	#15,d6
    m->d6.w &= m->d2.w;         // and.w	d2,d6			* d6 = x2 & 15
    m->d6.w <<= 1;              // add.w	d6,d6
    *(u8*)m->a0.l = m->d6.b;    // move.b  d6,(a0)+
    m->a0.l++;
    
    m->d5.w -= m->d0.w;         // sub.w	d0,d5

    if (m->d5.w != 0)           // bne.s	.h_line_long	* ================ STORE HLINE LONG OR NOT
        goto h_line_long;

    *(u16*)m->a0.l = m->d5.w;   // move.w  d5,(a0)+         * Store 0 (d5 = 0)
    m->a0.l += 2;
    
    return;                     // rts

h_line_long:

    m->d5.w >>= 3;              // lsr.w	#3,d5
    m->d5.w +=  1;              // addq.w	#1,d5
    
    *(u16*)m->a0.l = m->d5.w;   // move.w  d5,(a0)+         * != 0 => Store nb words
    m->a0.l += 2;

    return;                     //  rts

    //------------------------------------------------------------------
    //	Dx=Dy
    //
    //  STORAGE:
    //     w: offset address | 6 | ((pitch inc < 0) ? $8000 : 0)
    //     b: bit num
    //     b: length ==> on PC full length
    //     w: routine offset ==> on PC useless
    //------------------------------------------------------------------

d45:
    *(u16*)m->a0.l |= 6;        // or.w    #6,(a0)         * =================== inc pitch sign

                                // add.w	d2,d2
                                // add.w	d2,d2
    
    m->d4.w = m->d2.w >> 1;     // lea	table2(pc),a1        
    m->d4.w &= 0xFFF8;          // move.l	(a1,d2.w),d4
                                
    *(u16*)m->a0.l += m->d4.w;  // add.w   d4,(a0)+        * =================== STORE OFFSET D4
    m->a0.l += 2;
      
    m->d4.w = 15 - (m->d2.w & 15);      // swap	d4
    m->d4.b |= m->d5.b;
    *(u8*)m->a0.l = m->d4.b;    // move.b  d4,(a0)+        * =================== STORE BIT
    m->a0.l++;
    
    m->d0.w += m->d7.w;         // add.w	d7,d0			
    m->d1.w += m->d7.w;         // add.w	d7,d1
    
    m->d0.w &= 7;               // and.w	#7,d0
    m->d0.w = -m->d0.w;         // neg.w	d0

    m->d0.w *= 12;              // add.w	d0,d0			* let's do the mul 12 manually
                                // add.w	d0,d0
                                // move.w	d0,d7
                                // add.w	d7,d7
                                // add.w	d7,d0

    // m->d1.w >>= 3;              // lsr.w	#3,d1
    *(u8*)m->a0.l = m->d1.b;    // move.b  d1,(a0)+        * =================== STORE LENGTH ==>  on PC  full length
    m->a0.l++;
    *(u16*)m->a0.l = m->d0.w;   // move.w  d0,(a0)+        * =================== STORE ROUTINE OFFSET
    m->a0.l += 2;   
    
    return;                     // rts
}



//----------------------------------------------------
//	ERASE
//----------------------------------------------------
//   a0: screen
//   a1: coords
//----------------------------------------------------
void VECclrpass(void)
{
    *HW_BLITTER_ENDMASK1 = -1;                  // move.l	# - 1, (a2)+
    *HW_BLITTER_ENDMASK2 = -1;                  // move.w	# - 1, (a2)+
    *HW_BLITTER_ENDMASK3 = -1;

    *HW_BLITTER_XINC_DEST = 8;                  // move.w  #8, (a2)+; inc x dest

    *HW_BLITTER_HOP   = HW_BLITTER_HOP_BIT1;
    *HW_BLITTER_OP    = HW_BLITTER_OP_BIT0;
}

void VECclr(void* screen_, u16* list_)
{
    EMUL68k m;

    m.a0.l = (u32) screen_;
    m.a1.l = (u32) list_;

    m.d4.w = *(u16*)(m.a1.l + 6);               // move.w	6(a1), d4; maxy
    
    if (m.d4.w > VEC_YMAX)                      // cmp.w    #ymax, d4
    {                                           // ble.s.   vecloop_maxyok
        m.d4.w = VEC_YMAX;                      // move.w	#ymax, d4
    }                                           // .vecloop_maxyok:

    m.d3.w = *(u16*)(m.a1.l + 4);               // move.w	4(a1), d3; miny

    if ((s16)m.d3.w < 0)                        // 
    {                                           // bge.s.vecloop_minyok
        m.d3.l = 0;                             // moveq.l	#0, d3
    }                                           // .vecloop_minyok:

    m.d5.w  = m.d4.w;                           // move.w	d4, d5
    m.d5.w -= m.d3.w;                           // sub.w	d3, d5
    m.d5.w += 1;                                // addq.w  #1, d5

    m.d3.w *= VEC_PITCH;                        // lea		pitchmul(pc), a3
                                                // add.w	d3, d3
                                                // move.w(a3, d3.w), d3
    m.a3.l = m.a0.l + (u32)m.d3.w;              // lea(a0, d3.w), a3

    m.d3.w = *(u16*)(m.a1.l + 2);               // move.w	2(a1), d3; word count xo
    m.a3.l += *(u16*)m.a1.l;                    // add.w(a1), a3; adr offset x

    m.d4.w = 1 + VEC_PITCH / 8;                 // move.w	#1 + pitch / 8, d4
    m.d4.w -= m.d3.w;                           // sub.w	d3, d4
    m.d4.w <<= 3;                               // add.w	d4, d4
                                                // add.w	d4, d4
                                                // add.w	d4, d4

                                                // lea	$ffff8a20.w, a2

    *HW_BLITTER_XINC_SOURCE = 0;                // clr.l(a2) + ; inc x source / inc y source
    *HW_BLITTER_YINC_SOURCE = 0;

    *HW_BLITTER_ADDR_SOURCE = m.a3.l;           // move.l	a3, (a2)+; adr source
    *HW_BLITTER_YINC_DEST = m.d4.w;             // move.w	d4, (a2)+; inc y dest

    *HW_BLITTER_ADDR_DEST = m.a3.l;             // move.l	a3, (a2)+; adr dest

    *HW_BLITTER_XSIZE = m.d3.w;                 // move.w	d3, (a2)+; x count
    *HW_BLITTER_YSIZE = m.d5.w;                 // move.w	d5, (a2)+; y count
    
    *HW_BLITTER_CTRL1 = 0;
    *HW_BLITTER_CTRL2 = 0;

    EMULblit();                                 // move.l	#$C000, (a2)
}

//----------------------------------------------------
//	XOR BLITTER PASS
//----------------------------------------------------
//   a0: screen
//   a1: coords
//----------------------------------------------------    
void VECxorpass(u16 hogOpMask)
{
    *(u16*)HW_BLITTER_HOP = PCENDIANSWAP16(hogOpMask);
    *HW_BLITTER_ENDMASK1  = -1;
    *HW_BLITTER_ENDMASK2  = -1;
    *HW_BLITTER_ENDMASK3  = -1;
    *HW_BLITTER_XINC_DEST = 8;
}


void VECxor(void* screen_, u16* list_)
{                                        
    EMUL68k m;                                  // movem.l d3-d5/a2-a3,-(sp)

    m.a0.l = (u32) screen_;
    m.a1.l = (u32) list_;

    m.d4.w = *(u16*)(m.a1.l + 6);               // move.w	6(a1), d4; maxy

    if ((s16)m.d4.w > VEC_YMAX)                 // cmp.w    #ymax, d4
    {                                           // ble.s.   vecloop_maxyok
        m.d4.w = VEC_YMAX;                      // move.w	#ymax, d4
    }                                           // .vecloop_maxyok:

    m.d3.w = *(u16*)(m.a1.l + 4);               // move.w	4(a1), d3; miny

    if ((s16)m.d3.w < 0)                        // 
    {                                           // bge.s.vecloop_minyok
        m.d3.l = 0;                             // moveq.l	#0, d3
    }                                           // .vecloop_minyok:


    m.d5.w  = m.d4.w;                           // move.w	d4, d5
    m.d5.w -= m.d3.w;                           // sub.w	d3, d5

    m.d3.w *= VEC_PITCH;                        // lea		pitchmul(pc),a3
                                                // add.w	d3,d3
                                                // move.w	(a3,d3.w),d3
    m.a3.l = m.a0.l + m.d3.w;                   // lea		(a0,d3.w),a3
   
    m.d3.w = *(u16*)(m.a1.l + 2);               // move.w	2(a1),d3			; word count x
    m.a3.l += *(s16*)m.a1.l;                    // add.w	(a1),a3	    		; adr offset x

    m.d4.w = 1 + VEC_PITCH / 8;                 // move.w	#1+pitch/8,d4		
    m.d4.w -= m.d3.w;                           // sub.w	d3,d4
    m.d4.w <<= 3;                               // add.w	d4,d4
                                                // add.w	d4,d4
                                                // add.w	d4,d4

                                                // lea	$ffff8a20.w,a2

    *HW_BLITTER_XINC_SOURCE = 8;                // move.w	#8,(a2)+			; inc x source 
    *HW_BLITTER_YINC_SOURCE = m.d4.w;           // move.w	d4,(a2)+			; inc y source

    *HW_BLITTER_ADDR_SOURCE = m.a3.l;           // move.l	a3,(a2)+			; adr source

    *HW_BLITTER_ENDMASK1 = -1;                  // move.l	# - 1, (a2)+
    *HW_BLITTER_ENDMASK2 = -1;                  // move.w	# - 1, (a2)+
    *HW_BLITTER_ENDMASK3 = -1;

    *HW_BLITTER_XINC_DEST = 8;                  // move.w  #8, (a2)+; inc x dest
    *HW_BLITTER_YINC_DEST = m.d4.w;             // move.w	d4, (a2)+; inc y dest

    m.a3.l += VEC_PITCH;                        // lea		pitch(a3),a3
    *HW_BLITTER_ADDR_DEST = m.a3.l;             // move.l	a3, (a2)+; adr dest

    *HW_BLITTER_XSIZE = m.d3.w;                 // move.w	d3, (a2)+; x count
    *HW_BLITTER_YSIZE = m.d5.w;                 // move.w	d5, (a2)+; y count

    *HW_BLITTER_OP    = HW_BLITTER_OP_S_XOR_D;
    *HW_BLITTER_CTRL1 = 0;
    *HW_BLITTER_CTRL2 = 0;

    EMULblit();                                 // move.l	#$206C000, (a2)    
                                                // movem.l (sp)+,d3-d5/a2-a3
}                                               // rts

//----------------------------------------------------
//	DRAW LINES
//----------------------------------------------------
//   a0: screen
//   a1: display list
//   d0: polycount
//----------------------------------------------------
static u16 drawpolycount = 0;

u16* VECloop  (void* _ecran, u16* _dlist, u16 _polycount)
{
    s16 drawedgescount = 0;
    EMUL68k m;

    m.a0.l = (u32) _ecran;
    m.a1.l = (u32) _dlist;
    m.d0.w = _polycount;

    drawpolycount = m.d0.w;                         // move.w d0,drawpolycount

    *HW_BLITTER_HOP = 0;
    *HW_BLITTER_OP  = HW_BLITTER_OP_S_XOR_D;        // move.w	#6,$ffff8a3a.w						* HOP = 0 (bits 1 instead of source) and xor mode

    //------------------------------------------------
    //	Run display list
    //------------------------------------------------
                                                    // move.l	a0,-(sp)
poly_loop:

    drawedgescount = *(u16*)m.a1.l;                 // move.w	(a1)+,drawedgescount
    m.a1.l += 2;

    if (drawedgescount < 0)                         
        goto noedges;                               // blt.s	noedges

edges_loop:	

    m.a0.l = (u32) _ecran;                          // move.l	(sp),a0

    // drawline
    m.d0.w = *(u16*)m.a1.l;                         // move.w  (a1)+,d0
    m.a1.l += 2;
   
    m.d1.w  = m.d0.w;                               // move.w  d0,d1
    m.d1.l &= 0xFFF8;                               // and.l   #$FFF9,d1
    m.a0.l += m.d1.l;                               // add.l   d1,a0

    m.a2.l = VEC_PITCH;                             // lea     pitch.w,a2
                                                    // tst.b   (a1)
    if (*(u8*)m.a1.l & 0x80)                        // bge.s   .pitchpositive
        m.a2.l = - VEC_PITCH;                       // lea     -pitch.w,a2
                                                    // .pitchpositive:    
    m.d0.w <<= 1;                                   // add.w   d0,d0
    m.d0.w &= 0xC;                                  // and.w   #$C,d0

    switch(m.d0.w)
    {
        case 0:   goto pixhorizontal;               
        case 4:   goto pixvert;                     
        case 8:   goto pixhline;
        case 12:  goto pixd45;
        default:  ASSERT(0);
    }
    
noedges:
    drawpolycount--;                                // subq.w	#1,drawpolycount
    if (drawpolycount != 0)
        goto poly_loop;                             // bne.s	poly_loop
    
enddraw:
                                                    // addq.l  #4,sp
    m.a0.l = m.a1.l;                                // move.l	a1,a0
                                                    // movem.l (sp)+,d1-d7/a2-a5
    return (u16*) m.a0.l;                           // rts		* return to caller

    //------------------------------------------------------------------
    //	Dx>Dy
    //
    //   STORAGE :
    //      w: offset address | ((pitch inc < 0) ? $8000 : 0) | 0
    //      w: enter routine offset
    //      w: exit routine offset
    //      w: error increment
    //------------------------------------------------------------------
pixhorizontal:

    // lea		line_display_horiz(pc),a3	* compute routine adresses range
    // lea		line_display_horiz(pc),a3	* compute routine adresses range
    // move.l	a3,a4						* 

    //add.w	(a1)+,a3		* add x2 address offset to a0
    //add.w	(a1)+,a4		* end address into routine : x2 * 4 + length * 4

    m.d2.w = *(u16*)m.a1.l;
    m.a1.l += 2;
    m.d2.w = PCENDIANSWAP16(m.d2.w);
    m.d2.w &= 0x7FFF;
    ASSERT(m.d2.w < 16);

    m.d3.w = *(u16*)m.a1.l;
    m.a1.l += 2;
    ASSERT(m.d3.w <= VEC_XMAX);

    m.d1.w = *(u16*)m.a1.l;     // move.w  (a1)+,d1        * increment
    m.a1.l += 2;
    
    {
        u16 t;
        u16 bit = 1 << m.d2.w;

        m.d5.w = -32768;        // On ST : done with preload registers - movem.w	preload(pc),d0/d2-d7  ================== INIT => draw pass will do this

        ASSERT((m.a0.l & 1) == 0);

        for (t = 0; t < m.d3.w ; t++)
        {
            *(u16*)m.a0.l ^= PCENDIANSWAP16(bit);
            u32 v = (u32) m.d5.w + (u32) m.d1.w;
            m.d5.w = (u16) v;           // add.w	d1,d5
                                        // bcc.s	*+4
            if (v > 0xFFFF)
                m.a0.l += m.a2.l;       // add.l	a2,a0

            bit <<= 1;
            if (bit == 0)
            {
                bit = 1;
                m.a0.l -= 8;
            }
        }
    }    

    //move.w (a4),a5			* backup opcode
    //move.w #$4E75,(a4)		* put an rts into the routine
    //jsr	(a3)
    //move.w	a5,(a4)			* restore overwritten opcode

    drawedgescount -= 1;            // subq.w	#1,drawedgescount
    if (drawedgescount >= 0)        // bge 	edges_loop
        goto edges_loop;

    drawpolycount -= 1;             // subq.w	#1,drawpolycount
    if (drawpolycount != 0)         // bne 	poly_loop
        goto poly_loop;

    goto enddraw;                   // bra     enddraw

    //------------------------------------------------------------------
    //	Dx<Dy
    //
    //   STORAGE:
    //      w: offset address | ((pitch inc < 0) ? $8000 : 0) | 2
    //      b: rol bit number
    //      b: increment error
    //      w: vertical offset increment
    //      w: offset into routine  => on PC length
    //------------------------------------------------------------------

pixvert:						

    m.d0.b = *(u8*)m.a1.l;          // move.b  (a1)+,d0        * SET ROL BIT
    m.a1.l++;
    m.d0.b &= 0x7F;
    ASSERT(m.d0.b < 16);
    m.d4.l = 0;                     // moveq.l #0,d4
    m.d4.w = 1 << m.d0.b;           // bset.l  d0,d4
    
    m.d1.b = *(u8*)m.a1.l;          // move.b  (a1)+,d1        * ERROR INCREMENT
    m.a1.l++;

    m.d6.w = *(u16*)m.a1.l;         // move.w  (a1)+,d6        * ADDRESS VERTICAL INCREMENT LINE
    //ASSERT(((s16)m.d6.w % VEC_PITCH) == 0);
    m.a1.l += 2;

    m.d6.l = (s32)(s16)m.d6.w;      // ext.l   d6              *
    m.d0.w = *(u16*) m.a1.l;        // move.w  (a1)+,d0        * OFFSET INTO ROUTINE => on PC length
    m.a1.l += 2;

    m.d5.w = -32768;                // move.w	#-32768,d5		* Accumulateur … 0							===================== INIT DRAW

    // lea		vertical_display(pc),a3										
    // jmp		(a3,d0.w)
    
    {
        u16 t, v;
        for (t = 0 ; t < m.d0.w ; t++)  // rept	ymax+1
        {
            *(u16*) m.a0.l ^= PCENDIANSWAP16(m.d4.w);   // eor.w	d4,(a0)			* Affichage point
            m.a0.l += m.d6.l;           // add.l	d6,a0			* adr affichage-160 (trace a l'envers)
            m.d4.w <<= 1;               // add.w	d4,d4			* on decale de 1 le bit tournant
            if (m.d4.w == 0)            // bcc.s	*+6				* Si abscisse bit > 15
            {
                m.a0.l -= 8;            // subq.l	#8,a0			* adr affichage-8 (trace a l'envers)
                m.d4.l  = 1;            // moveq.l	#1,d4
            }

            v = (u16)m.d5.b + (u16)m.d1.b; // add.b	d1,d5			* Incremente le taux d'erreur
            m.d5.b = (u8) v;
            if (v > 0xFF)               // bcc.s	*+4				* Si bit carry , taux > 1 unite (65536)
                m.a0.l += m.a2.l;       // add.l	a2,a0			*        
        }                               // endr
    }                                   // vertical_display:
    
    drawedgescount--;                   // subq.w	#1,drawedgescount
    if (drawedgescount >= 0)
        goto edges_loop;                // bge 	edges_loop

    drawpolycount--;                    // subq.w	#1,drawpolycount
    if (drawpolycount != 0)
        goto poly_loop;                 // bne 	poly_loop
    
    goto enddraw;                       // bra     enddraw

    //----------------------------
    //   HLINE
    //   STORAGE:
    //      w: offset address | 4 | ((pitch inc < 0) ? $8000 : 0)
    //      b: start mask selection
    //      b: end mask selection
    //      w: nb words or 0 when same word
    //----------------------------

    static u16 mask_start[] = { 0x7FFF, 0x3FFF, 0x1FFF,  0xFFF,  0x7FF,  0x3FF,  0x1FF,   0xFF,   0x7F,   0x3F,   0x1F,    0xF,    0x7,    0x3,    0x1, 0       };
    static u16 mask_end  [] = { 0x8000, 0xC000, 0xE000, 0xF000, 0xF800, 0xFC00, 0xFE00, 0xFF00, 0xFF80, 0xFFC0, 0xFFE0, 0xFFF0, 0xFFF8, 0xFFFC, 0xFFFE, 0xFFFF  };

pixhline:	
         
    m.d3.l = 0;                         // moveq.l #0,d3
    m.d3.b = *(u8*)m.a1.l;              // move.b  (a1)+,d3
    m.a1.l++;                          
    m.d3.b &= 0x7F;
    m.d6.l = 0;                         // moveq.l #0,d6
    m.d6.b = *(u8*)m.a1.l;              // move.b  (a1)+,d6
    m.a1.l++;                          
                                       
    m.d5.w = *(u16*)m.a1.l;             // move.w  (a1)+,d5
    m.a1.l += 2;                       
    if (m.d5.w != 0)                   
        goto h_line_long;               // bne.s	.h_line_long										================ STORE HLINE LONG OR NOT

    m.d1.w  = mask_start[m.d3.w >> 1];  // move.w	.mask_start(pc,d3.w),d1				* masque final 1 - 2 - 3 => il faut les setter		================ STORE D3
    m.d1.w &= mask_end  [m.d6.w >> 1];  // and.w	.mask_end(pc,d6.w),d1																	================ STORE D6
    
    ASSERT((m.d3.w & 1) == 0);
    ASSERT((m.d6.w & 1) == 0);
    ASSERT(m.d3.w < 32);
    ASSERT(m.d6.w < 32);

    *HW_BLITTER_ENDMASK1  = PCENDIANSWAP16(m.d1.w);     // move.w	d1,$ffff8a28.w
    *HW_BLITTER_ENDMASK2  = -1;
    *HW_BLITTER_ENDMASK3  = -1;

    *HW_BLITTER_ADDR_DEST = m.a0.l;     // move.l	a0,$ffff8a32.w						* dest adr											================ DRAW INIT A0 with STORED offset

    *HW_BLITTER_XSIZE = 1;              // move.l	#$10001,$ffff8a36.w					* nb word to transfer horizontal + vertical			================ STORE WIDTH = 1
    *HW_BLITTER_YSIZE = 1;
    *HW_BLITTER_CTRL1 = 0;
    *HW_BLITTER_CTRL2 = 0;

    EMULblit();                         // move.w	#$C000,$ffff8a3c.w					* go exclusive

    drawedgescount--;                   // subq.w	#1,drawedgescount
    if (drawedgescount >= 0)
        goto edges_loop;                // bge 	edges_loop

    drawpolycount--;                    // subq.w	#1,drawpolycount
    if (drawpolycount != 0)
        goto poly_loop;                 // bne 	poly_loop

    goto enddraw;                       // bra     enddraw

h_line_long:

    *HW_BLITTER_ENDMASK1 = mask_start[m.d3.w >> 1]; // move.w	.mask_start(pc,d3.w),$ffff8a28.w	* masque final 1 - 2 - 3 => il faut les setter		================ STORE D3
    *HW_BLITTER_ENDMASK1 = PCENDIANSWAP16(*HW_BLITTER_ENDMASK1);
    *HW_BLITTER_ENDMASK2 = -1;
    *HW_BLITTER_ENDMASK3 = mask_end  [m.d6.w >> 1]; // move.w	.mask_end(pc,d6.w),$ffff8a2c.w															================ STORE D6
    *HW_BLITTER_ENDMASK3 = PCENDIANSWAP16(*HW_BLITTER_ENDMASK3);

    *HW_BLITTER_ADDR_DEST = m.a0.l;     // move.l	a0,$ffff8a32.w						* dest adr											================ DRAW INIT A0 with STORED offset
    *HW_BLITTER_XSIZE = m.d5.w;         // move.w	d5,$ffff8a36.w						* nb word to transfer horizontal				 	================ STORE WIDTH = D5
    *HW_BLITTER_YSIZE = 1;              // move.w	#1,$ffff8a38.w						* nb lines
    *HW_BLITTER_CTRL1 = 0;
    *HW_BLITTER_CTRL2 = 0;

    EMULblit();                         // move.w	#$C000,$ffff8a3c.w					* go exclusive

    drawedgescount--;                   // subq.w	#1,drawedgescount
    if (drawedgescount >= 0)
        goto edges_loop;                // bge 	edges_loop

    drawpolycount--;                    // subq.w	#1,drawpolycount
    if (drawpolycount != 0)
        goto poly_loop;                 // bne 	poly_loop

    goto enddraw;                       // bra     enddraw

    //------------------------------------------------------------------
    //	Dx=Dy
    //
    //   STORAGE:
    //       w: offset address | 6 | ((pitch inc < 0) ? $8000 : 0)
    //       b: bit num
    //       b: length
    //       w: routine offset
    //------------------------------------------------------------------
pixd45:
    m.d4.l  = 0;                        // moveq.l #0,d4
    m.d5.b  = *(u8*) m.a1.l;            
    m.a1.l++;                           // move.b  (a1)+,d5
    m.d5.b  &= 0x7F;
    ASSERT(m.d5.b < 16);

    m.d4.l = 1 << m.d5.b;               // bset.l  d5,d4           * ROL BIT
    
    m.d1.l = 0 ;                        // moveq.l #0,d1
    m.d1.b = *(u8*)m.a1.l;              // move.b  (a1)+,d1        * =================== LENGTH
    m.a1.l++;
    
    m.d0.w = *(u16*)m.a1.l;             // move.w  (a1)+,d0        * =================== ROUTINE OFFSET => on PC length
    m.a1.l += 2;

    {
        u16 t;

        for (t = 0 ; t < m.d1.w ; t++)
        {
            *(u16*)m.a0.l ^= PCENDIANSWAP16(m.d4.w);    // eor.w	d4,(a0)			* Affiche le point
            m.a0.l += m.a2.l;           // add.l	a2,a0			* Mvt vert
            m.d4.w <<= 1;               // add.w	d4,d4			* Mvt hori
            if (m.d4.w == 0)            // bcc.s	*+6				* Incremente adr si abscisse mod 16 = 0
            {
                m.a0.l -= 8;            // subq.l	#8,a0			*
                m.d4.l = 1;             // moveq.l	#1,d4			*
            }
        }                               // dbra.w	d1,diago	    * Longueur
    }

    drawedgescount--;                   // subq.w	#1,drawedgescount
    if (drawedgescount >= 0)
        goto edges_loop;                // bge 	edges_loop

    drawpolycount--;                    // subq.w	#1,drawpolycount
    if (drawpolycount != 0)
        goto poly_loop;                 // bne 	poly_loop

    goto enddraw;                       // bra     enddraw
}


void VECcircle(void* screen_, u16 centerx_, u16 centery_, u16 radius_)
{  
    EMUL68k m;

    m.a0.l = (u32) screen_;
    m.d0.w = centerx_;
    m.d1.w = centery_;
    m.d2.w = radius_;

    m.d5.w = m.d2.w;            // move.w	d2,d5 			* Delta = r

    m.a0.l += m.d1.w * VEC_PITCH + ((m.d0.w & 0xFFF0) >> 1);
                                // lea	tabley,a6
                                // move.w	d0,d3			* xc And $fff0 + adr Affichage
                                // and.w	#$fff0,d3
                                // lsr.w	#1,d3
                                // add.w	d3,a0
                                // add.w	d1,d1
                                // add.w	(a6,d1.w),a0

    m.a4.l = m.a0.l + m.d2.w * VEC_PITCH;
                                // move.w	d2,d3
                                // add.w	d3,d3
                                // move.w	(a6,d3.w),d3            
                                // lea	(a0,d3.w),a4

    m.a0.l -= m.d2.w * VEC_PITCH;
                                // sub.w	d3,a0	

    m.a1.l = m.a0.l;            // move.l	a0,a1
    m.a5.l = m.a4.l;            // move.l	a4,a5

    m.d0.w &= 0xF;              // and.w	#15,d0			* Init bit tournant
                                // neg.w	d0
    m.d0.w = 15 - m.d0.w;       // add.w	#15,d0

    m.d6.l = 0;                 // moveq.l	#0,d6
    m.d6.w = 1 << m.d0.w;       // bset.l	d0,d6

    m.d1.w = m.d6.w;            // move.w	d6,d1

    m.d3.l = -1;                // moveq.l	#-1,d3			* X: d3 = -1 * Y: d2 = r
    
    *(u16*)m.a0.l ^= PCENDIANSWAP16(m.d6.w);  // or.w	d6,(a0)			* Affichage
    *(u16*)m.a4.l ^= PCENDIANSWAP16(m.d6.w);  // or.w	d6,(a4)			* Affichage

loop:
    m.d4.w = m.d5.w;            // move.w	d5,d4			* delta: D5
    m.d4.w <<= 1;               // add.w	d4,d4			* d: D4  	d = delta * 2

    if ((s16)m.d4.w < 0)
    {                           // bge.s	else			* Si d < 0
        m.d2.w--;               // subq.w	#1,d2		
        if ( (s16) m.d2.w < 0)      // tst.w	d2			* While y>= 0
        {                           // bge.s	.ok
            return;                 // rts
        }                           // .ok:

        m.d4.w += m.d3.w;       // add.w	d3,d4			* d = d + x

        if ((s16)m.d4.w < 0)
        {                       // bge.s	else1			* Si d < 0           
            m.d5.w += m.d2.w;   // add.w	d2,d5			* delta = delta + y	
            //bra.s	loop
        }
        else                    // else1:					*Sinon
        {
            m.d3.w += 1;        // addq.w	#1,d3			* Inc x

            if (EMUL_ROR_W(&m.d6, 1))  // ror.w	d0,d6
            {                               //  bool carry;
                m.a4.l += 8;     // addq.w	#8,a4
                m.a0.l += 8;     // addq.w	#8,a0
            }

            m.d1.w <<= 1;        // add.w	d1,d1
            if (m.d1.w == 0)
            {
                m.d1.l = 1;     // moveq.l	#1,d1                
                m.a1.l -= 8;    // subq.w	#8,a1
                m.a5.l -= 8;    // subq.w	#8,a5
            }

            *(u16*)m.a0.l ^= PCENDIANSWAP16(m.d6.w);  // or.w	d6,(a0)			* Affichage
            *(u16*)m.a4.l ^= PCENDIANSWAP16(m.d6.w);  // or.w	d6,(a4)			* Affichage
            *(u16*)m.a1.l ^= PCENDIANSWAP16(m.d1.w);  // or.w	d1,(a1)			* Affichage
            *(u16*)m.a5.l ^= PCENDIANSWAP16(m.d1.w);  // or.w	d1,(a5)			* Affichage

            m.d5.w += m.d2.w;   // add.w	d2,d5			* 
            m.d5.w -= m.d3.w;   // sub.w	d3,d5			* delta = delta + y - x                                
        }                       // bra.s	loop

        m.a0.l += VEC_PITCH;    // lea	160(a0),a0			
        m.a1.l += VEC_PITCH;    // lea	160(a1),a1
        m.a4.l -= VEC_PITCH;    // lea	-160(a4),a4			
        m.a5.l -= VEC_PITCH;    // lea	-160(a5),a5
    }
    else                        // else:					* Sinon
    {
        m.d3.w += 1;            // addq.w	#1, d3* Inc x
        
        if (EMUL_ROR_W(&m.d6, 1))  // ror.w	d0,d6
        {                               //  bool carry;
            m.a4.l += 8;     // addq.w	#8,a4
            m.a0.l += 8;     // addq.w	#8,a0
        }

        m.d1.w <<= 1;        // add.w	d1,d1
        if (m.d1.w == 0)
        {
            m.d1.l = 1;     // moveq.l	#1,d1                
            m.a1.l -= 8;    // subq.w	#8,a1
            m.a5.l -= 8;    // subq.w	#8,a5
        }

        *(u16*)m.a0.l ^= PCENDIANSWAP16(m.d6.w);  // or.w	d6,(a0)			* Affichage
        *(u16*)m.a4.l ^= PCENDIANSWAP16(m.d6.w);  // or.w	d6,(a4)			* Affichage
        *(u16*)m.a1.l ^= PCENDIANSWAP16(m.d1.w);  // or.w	d1,(a1)			* Affichage
        *(u16*)m.a5.l ^= PCENDIANSWAP16(m.d1.w);  // or.w	d1,(a5)			* Affichage

        m.d4.w -= m.d2.w;   // sub.w	d2, d4* d = d - y

        if ((s16)m.d4.w >= 0)
        {                       // blt.s	else2 * Si y >= 0
            m.d5.w -= m.d3.w;   // sub.w	d3, d5* delta = delta - x
        }
        else  // else2 : *Sinon
        {         
            m.d2.w -= 1;        // subq.w	#1, d2* Dec y
            if ( (s16) m.d2.w < 0)      // tst.w	d2			* While y>= 0
            {                           // bge.s	.ok
                return;                 // rts
            }                           // .ok:

            m.a0.l += VEC_PITCH;    // lea	160(a0),a0			
            m.a1.l += VEC_PITCH;    // lea	160(a1),a1
            m.a4.l -= VEC_PITCH;    // lea	-160(a4),a4			
            m.a5.l -= VEC_PITCH;    // lea	-160(a5),a5

            m.d5.w -= m.d3.w;   // sub.w	d3, d5* delta = delta + y - x
            m.d5.w += m.d2.w;   // add.w	d2, d5
        }   //bra.s	loop
    }
    
    goto loop;
}

#endif // !__TOS__
