;
; ARJ Mode 1-3 and 7 decode functions
; Size optimized
; Copyleft 1993-2007 Mr Ni! (the Great) of the TOS-crew
;
; This function uses a BIG amount of stack space!
; It uses about 12kB!
; You can reduce this amount with 11312 bytes
; by suppyling A2 with a pointer to a 11312 bytes big
; workspace and removing the stack allocation and
; deallocation code at the right places in the source
; text. (total is 3 lines, 2 at the start, 1 at main rts)
;
; Note:
; ARJ_OFFS.TTP. This program is an addition to UNARJ_PR. It
; calculates the minimum offset between the source and destination
; address for in memory depacking of files.
; (Depacking A1-referenced data to a4... The calculated 'offset' is
; the minimum amount of bytes required to be reserved before the
; packed data block.)
;
;void decode(char* depack_space, char* packed_data)
;
; CALL:
; A0 = ptr to depack space
; A1 = ptr to packed data
;
; RETURN
; depacked data in depack space
;

export decode_m7

workspacesize EQU 11312

; this specific order of the tables is used in the program!
c_table EQU 0
c_len EQU 8192
avail EQU 9740
pt_len EQU 10780
pt_table EQU 10800

; register usage:
; D0 = count
; D1 = command count
; D2 = temporary usage
; D3 =
; D4 = command tri-nibble
; D5 = const:  #$100
; D6 = bitbuf, subbitbuf
; D7 = .H: command count, .B: bits in subbitbuf
;
; A0 = text_pointer
; A1 = rbuf_current
; A2 = c_table
; A3 = avail
; A4 = pt_len
; A5 = c_len
; A6 = copy_pointer
; A7 = Stack pointer
decode_m7:
     movem.l D3-D7/A2-A6,-(SP) ;
     lea     -workspacesize(SP),SP ; or supply your own workspace here
     lea     (SP),A2          ; remove if alternative workspace supplied
     moveq   #0,D7            ; bitcount = 0
     move.l  (A1),D6          ; fil bitbuf
     move.w  (A1)+,D1         ; block size
     subq.w  #1,D1
     lea     c_len-c_table(A2),A5 ;
     lea     avail-c_len(A5),A3
.enter:
     movem.l D1/A0/A2,-(SP)
     clr.w   (A3)             ; reset avail
     moveq   #-2,D1           ; call-values for read_pt_len()
     bsr     .read_pt_len     ; call read_pt_len, a4 = pt_len
     bsr     .get_them2
     movea.l A5,A6
     move.w  D2,D0
     beq.s   .n_is_0          ;
;
; Register usage:
;
; D0
; d1
; d2
; D3
; d4 = $13
; d5
; d6 = .l (sub) bitbuf
; d7 = .b bits in bitbuf
;
; a0 =
; a1 = rbuf_current
; a2 =
; a3 = avail
; a4 = pt_len
; a5 = c_len
; a6 =
; a7 = sp
;
     move.w  D0,D5
     subq.w  #1,D0
     moveq   #$13,D4          ; reset the high word of D4 over here
.loop_3:
     move.w  D6,D3            ; sub bitbuf
     clr.b   D3
     lsr.w   #7,D3            ; upper 8 bits
     move.w  pt_table-pt_len(A4,D3.w),D2 ; check pt_table
     bge.s   .c_kleiner_NT    ;
     move.b  D6,D3            ; bitbuf
     bsr     .fidel_no
.c_kleiner_NT:                ;
     move.b  0(A4,D2.w),D3    ;
     bsr     .fillbits
     tst.w   D2               ; c=0 -> 1 ptrlen zero
     beq.s   .loop_5          ;
     subq.w  #2,D2            ;
     bgt.s   .c_groter_2      ; c>2 -> 1 ptrlen not zero
     beq.s   .c_2             ; c=2 -> 20+? ptrlen zero
     moveq   #4,D3            ; c=1 -> 2+? ptrlen zero
     bsr     .getbits
     addq.w  #2,D2            ;
     bra.s   .loop_5_init     ;
*******************************************************************************
.c_2:
     bsr.s   .get_them2
     add.w   D4,D2            ;
.loop_5_init:
     sub.w   D2,D0            ;
.loop_5:
     clr.b   (A6)+            ;
     dbra    D2,.loop_5       ;
     bra.s   .loop_3_test     ;
*******************************************************************************
.n_is_0:
     bsr.s   .get_them2
     clr.b   0(A6,D2.w)       ; clear table
     move.w  #$0FFF,D1
.loop_2:
     move.w  D2,-(A6)
     dbra    D1,.loop_2
     bra.s   .einde
*******************************************************************************
;D3,d1,d2,D0,d4,d5,d6,d7,a4,a1,a2,a3,a0,a5,a6,a7,sp
*******************************************************************************

.c_groter_2:
     move.b  D2,(A6)+         ;
.loop_3_test:
     dbra    D0,.loop_3       ;
     lea     c_table-c_len(A5),A2 ;
     moveq   #$0C,D1          ;
     movea.l A5,A4            ;
     bsr     make_table       ;
.einde:
     moveq   #20,D1           ;
     bsr     .read_pt_len     ; a4 = pt_len now
     movem.l (SP)+,D1/A0/A2
     moveq   #-1,D3           ; D3.h = $FFFF for pointer offsets
     move.w  #256,D5          ; to differentiate between literals and lengts codes
     move.l  #$1FFF0,D0       ; for huffmandecoding and pointer calculation

;***********************
;
; Register usage:
;
; D0 = loopcount
; d1 = command count
; d2 = temporary usage
; D3 = temporary usage
; d4 = command byte
; d5 = const: $100
; d6 = (sub)bitbuf
; d7 = .h: command count, .b byte count
;
; a0 = text
; a1 = rbuf_current
; a2 = c_table
; a3 = avail
; a4 = pt_len
; a5 = c_len
; a6 = source pointer
; a7 = (sp)

.bnz_cont:
     move.l  D0,D2            ; $1 fff0, we use the $1 in the high word later
     and.w   D6,D2
     lsr.w   #3,D2            ; charactertable is 4096 bytes (=12 bits)
     move.w  0(A2,D2.w),D2    ; pop character
     bpl.s   .decode_c_cont   ;
.j_grotergelijk_nc:
     move.b  D6,D3
     roxr.b  #4,D3
     bsr.s   .fidel_start
.decode_c_cont:               ;
     move.b  0(A5,D2.w),D3    ; pop 'charactersize' bits from buffer
     bsr.s   .fillbits
     sub.w   D5,D2            ;
     bcc.s   .sliding_dic     ;
     move.b  D2,(A0)+         ; push character into buffer
.count_test:
     dbra    D1,.bnz_cont     ; Hufmann block size > 0?
.blocksize_zero:              ; load a new Hufmann table
     dbra    D6,.decode_cont
     lea     workspacesize(SP),SP ; remove if alternative workspace supplied
     movem.l (SP)+,D3-D7/A2-A6 ;
     rts                      ;
*******************************************************************************
.get_them2:
     moveq   #9,D3            ;
     bra.s   .getbits
*******************************************************************************
.fidel_no:
     add.b   D3,D3
.fidel_start:
     bcc.s   .fidel_links     ;
     neg.w   D2
.fidel_links:
     move.w  0(A3,D2.w),D2    ;
     bmi.s   .fidel_no        ;
     rts
*******************************************************************************
.sliding_dic:
     move.w  D2,D4            ;
     move.w  D6,D2            ;
     clr.b   D2               ;
     lsr.w   #7,D2            ;
     move.w  pt_table-pt_len(A4,D2.w),D2 ;
     bpl.s   .p_cont          ;
.p_j_grotergelijk_np:
     move.b  D6,D3
     bsr.s   .fidel_no
.p_cont:
     move.b  0(A4,D2.w),D3    ;
     bsr.s   .fillbits
     move.w  D2,D3            ;
     beq.s   .p_einde         ;
     subq.w  #1,D3            ;
     move.w  D6,D2            ; subbitbuf
     swap    D2               ; high word of D2 was 1
     bsr.s   .fillbits0
     move.w  D2,D3
.p_einde:
     not.w   D3               ; pointer offset negatief
     lea     0(A0,D3.l),A6    ; pointer in dictionary
     move.b  (A6)+,(A0)+      ;
     move.b  (A6)+,(A0)+      ;
.copy_loop_0:
     move.b  (A6)+,(A0)+      ;
     dbra    D4,.copy_loop_0
     dbra    D1,.bnz_cont     ; Hufmann block size > 0?
     bra.s   .blocksize_zero

;D3,d1,d2,D0,d4,d5,d6,d7,a4,a1,a2,a3,a0,a5,a6,a7,sp
********************************************************************************

; no_bits=D3
; result=D2
.decode_cont:
     move.w  D6,D1            ; blocksize
     moveq   #16,D3           ; pop 16 bits
     pea     .enter(PC)
.getbits:
     move.l  D6,D2
     swap    D2
     clr.w   D2
.fillbits0:
     rol.l   D3,D2
.fillbits:
     sub.b   D3,D7
     bcc.s   .no_fill
     add.b   #16,D7
     move.l  (A1),D6
     addq.l  #2,A1
     ror.l   D7,D6
     rts
.no_fill:
     rol.l   D3,D6
     rts

;D3,d1,d2,D0,d4,d5,d6,d7,a4,a1,a2,a3,a0,a5,a6,a7,sp
*******************************************************************************

.n_is_nul:
     bsr.s   .getbits
     clr.b   0(A4,D2.w)
     moveq   #127,D3
.loop_2a:
     move.w  D2,(A2)+
     move.w  D2,(A2)+
     dbra    D3,.loop_2a
     rts
.read_pt_len:
     moveq   #5,D3
     bsr.s   .getbits
     lea     pt_len-c_len(A5),A4
     lea     pt_table-pt_len(A4),A2
     move.w  D2,D5
     beq.s   .n_is_nul
     subq.w  #1,D2
     moveq   #7,D0
     move.w  D2,D4
     add.w   D2,D1
     movea.l A4,A6
.loop_3a:
     move.l  D6,D2
     swap    D2
     clr.w   D2
     rol.l   #3,D2
     cmp.w   D0,D2
     bne.s   .c_niet_7
     moveq   #12,D3
     bra.s   .loop_4a_test
*******************************************************************************
.loop_4a:
     addq.w  #1,D2
.loop_4a_test:
     btst    D3,D6
     dbeq    D3,.loop_4a
.c_niet_7:
     moveq   #3,D3
     cmp.w   D0,D2
     bcs.s   .endif
     moveq   #-3,D3
     add.w   D2,D3
.endif:
     move.b  D2,(A6)+
     bsr.s   .fillbits
     cmp.w   D1,D4
     bne.s   .loop_3a_test
     moveq   #2,D3
     bsr.s   .getbits
     sub.w   D2,D4
     bra.s   .loop_5a_test
*******************************************************************************
.loop_5a:
     clr.b   (A6)+
.loop_5a_test:
     dbra    D2,.loop_5a
.loop_3a_test:
     dbra    D4,.loop_3a
     moveq   #8,D1
;D3,d1,d2,D0,d4,d5,d6,d7,a4,a1,a2,a3,a0,a5,a6,a7,sp
*******************************************************************************
; d0,d1,d2,d3,d4,d5,d6,d6,a0,a1,a2,a3
;
; D1 = table bit
; D5 = nchar
;
; A4 = len
;
make_table:
     movem.l D6-D7/A1/A4,-(SP)
     lea     -$6C(SP),SP
     move.w  D1,D4
     movea.w D5,A6
     add.w   D4,D4
     lea     $48(SP),A1       ; len_count[1]
     movea.l A1,A0
     moveq   #7,D3
     moveq   #0,D0
.clear_len:                   ; clear len_count
     move.l  D0,(A0)+
     dbra    D3,.clear_len
     movea.l A4,A0            ; charlen
     subq.w  #1,D5            ; nchar - 1
.len_count_l:
     move.b  (A0)+,D0         ; charlen
     add.w   D0,D0
     addq.w  #1,-2(A1,D0.w)   ; count
     dbra    D5,.len_count_l  ; loop
     lea     2(SP),A0         ; start
     moveq   #0,D0
     move.w  D0,(A0)+         ; start[1]=0
     moveq   #15,D2
.start_loop:
     move.w  (A1)+,D3         ; len_count
     lsl.w   D2,D3            ; len_count<<(16-len)
     add.w   D3,D0            ; start
     move.w  D0,(A0)+         ; start[len]
     dbra    D2,.start_loop   ; at the end of the loop d2 = $ffff, this is used later
     moveq   #$10,D3
     sub.w   D1,D3            ; 16-tablebits
     lea     2(SP),A1         ; start
     lea     $26(SP),A0       ; weight
     moveq   #1,D0
     add.b   D1,D2
     lsl.w   D2,D0
.loop_1a:
     move.w  (A1),D2
     lsr.w   D3,D2
     move.w  D2,(A1)+
     move.w  D0,(A0)+
     lsr.w   #1,D0
     bne.s   .loop_1a
     moveq   #1,D0
     moveq   #-1,D2
     add.w   D3,D2
     lsl.w   D2,D0
.loop_2b:
     move.w  D0,(A0)+
     lsr.w   #1,D0
     bne.s   .loop_2b
     move.w  2(SP,D4.w),D2
     lsr.w   D3,D2
     beq.s   .endif0
     moveq   #1,D5
     lsl.w   D1,D5
     sub.w   D2,D5
     subq.w  #1,D5
     add.w   D2,D2
     lea     0(A2,D2.w),A0
.loop_3b:
     move.w  D0,(A0)+
     dbra    D5,.loop_3b      ; d5 = $ffff
.endif0:
     moveq   #1,D0
     add.b   D3,D5
     lsl.w   D5,D0
     moveq   #0,D2
.loop_4b:
     move.b  (A4)+,D1         ; d1.w -> bovenste acht bits zijn 0
     beq.s   .loop_4b_inc
     add.w   D1,D1
     lea     0(SP,D1.w),A0
     move.w  (A0),D5
     move.w  D5,D6
     add.w   $24(A0),D6
     move.w  D6,(A0)
     cmp.w   D1,D4
     blt.s   .len_groter_tablebits_j
     sub.w   D5,D6
     add.w   D5,D5
     lea     0(A2,D5.w),A0
     subq.w  #1,D6
.j_loop_2:
     move.w  D2,(A0)+
     dbra    D6,.j_loop_2
.loop_4b_inc:
     addq.w  #1,D2
     cmp.w   A6,D2
     blt.s   .loop_4b
     lea     $6C(SP),SP
     movem.l (SP)+,D6-D7/A1/A4
     rts
.len_groter_tablebits_j:
     move.w  D5,D7
     lsr.w   D3,D7
     add.w   D7,D7
     lea     0(A2,D7.w),A0
     move.w  D1,D6
     sub.w   D4,D6
     move.w  D6,D1
.loop_6b:
     move.w  (A0),D7
     bne.s   .p_is_niet_nul
     subq.w  #2,(A3)
     move.w  (A3),D6
     move.w  D6,(A0)
     move.w  D7,0(A3,D6.w)
     neg.w   D6
     move.w  D7,0(A3,D6.w)
     neg.w   D6
     move.w  D6,D7
.p_is_niet_nul:
     move.w  D5,D6
     and.w   D0,D6
     beq.s   .left
     neg.w   D7
.left:
     lea     0(A3,D7.w),A0
     add.w   D5,D5
     subq.w  #2,D1
     bhi.s   .loop_6b
.loop_6b_end:
     move.w  D2,(A0)
     bra.s   .loop_4b_inc

;D3,d1,d2,D0,d4,d5,d6,d7,a4,a1,a2,a3,a0,a5,a6,a7,sp
********************************************************************************

     END
