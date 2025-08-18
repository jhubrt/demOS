; Ni mode 1 decode function
; Speed optimized
; Placed in public domain 2022 Hans Wessels (Mr Ni! (the Great) of the TOS-crew)
;
; void decode_n1(char* depack_space, char* packed_data)
; CALL:
; A0 = ptr to depack space
; A1 = ptr to packed data;
; void decode_n1(char* depack_space, char* packed_data)
; CALL:
; A0 = ptr to depack space
; A1 = ptr to packed data
;
; Register usage:
; d0: #bytes to copy
; d1: temporary register, bitbuf
; d2: #9
; d3: temporary register
; d4: temporary register
; d5: temporary register
; d6: bitbuf,subbitbuf
; d7: #bits in subbitbuf
; a0: depack space
; a1: rbuf_current
; a2: source adres for byte copy
; a3: end address
; a4: #16
; a5: not used
; a6: not used

export decode_n1

decode_n1:
     movem.l D3-D7/A2-A4,-(SP) ; save registers
     moveq   #0,D7            ; bitcount = 0
     movea.w #16,A4           ; #16 for bitbuf add
     movea.l (A1)+,A3         ; original size
     adda.l  A0,A3            ; end address
     move.w  (A1),D6          ; fill bitbuf
     moveq   #9,D2            ; 
.count_loop:                  ; main depack loop
     move.w  D6,D1            ; evaluate most significant bit bitbuf
     bmi.s   .start_sld       ; =1 -> sliding dictionary
     ror.w   #7,D1
     move.b  D1,(A0)+         ; push byte in buffer
     sub.b   D2,D7
     bcc.s   .nofill0
     add.w   A4,D7
     move.l  (A1),D6
     addq.l  #2,A1
     ror.l   D7,D6
     cmpa.l  A3,A0            ; end? yes-> fall through
     bls.s   .count_loop      ;
.nofill0:
     rol.l   D2,D6
     cmpa.l  A3,A0            ; end?
     bls.s   .count_loop      ;
     movem.l (SP)+,D3-D7/A2-A4 ;
     rts                      ;

.start_sld:
     add.w   D1,D1            ; trash type bit
     moveq   #1,D3            ; fillbits
     moveq   #1,D0            ; value
     moveq   #13,D4           ;
     moveq   #14,D5           ;
.loop1:
     addq.w  #1,D3            ; extra fill
     add.w   D1,D1            ; shift bit outside
     bcc.s   .einde1          ; if '1' continue
     addx.w  D0,D0            ; value *2+1
     dbra    D4,.loop1
.einde1:
     sub.w   D4,D5            ; correct D5
     sub.b   D3,D7
     bcs.s   .fill1
     rol.l   D3,D6
.done1:
     move.w  D6,D3
     swap    D3
     rol.l   D5,D3
     sub.b   D5,D7
     bcs.s   .fill2
     rol.l   D5,D6
.done2:
     add.w   D3,D0            ; length
     move.w  D6,D1            ; bitbuf
     moveq   #6,D4            ;
     moveq   #0,D3            ; fillbits
     moveq   #0,D5            ; value
.loop3:
     addq.w  #1,D3            ; extra fill
     add.w   D1,D1            ; shift bit outside
     bcc.s   .einde3          ; if '1' end decode
     addx.w  D5,D5            ; value *2+1
     dbra    D4,.loop3
.einde3:
     moveq   #15,D1           ; minimum getbits + D4
     sub.w   D4,D1            ; correct D1
     sub.b   D3,D7
     bcs.s   .fill4
     rol.l   D3,D6
.done4:
     move.w  D6,D3
     swap    D3
     rol.l   D1,D3
     sub.b   D1,D7
     bcc.s   .nofill5
     add.w   A4,D7
     move.l  (A1),D6
     addq.l  #2,A1
     ror.l   D7,D6
.done5:
     lsl.l   D2,D5            ;
     move.w  D3,D4
     lea     -1(A0),A2
     sub.l   D4,A2
     sub.l   D5,A2
.copy_loop:
     move.b  (A2)+,(A0)+      ;
     dbra    D0,.copy_loop    ;
     cmpa.l  A3,A0            ; end?
     bls     .count_loop      ;
     movem.l (SP)+,D3-D7/A2-A4 ;
     rts                      ;
.fill1:
     add.w   A4,D7
     move.l  (A1),D6
     addq.l  #2,A1
     ror.l   D7,D6
     bra.s   .done1
.fill2:
     add.w   A4,D7
     move.l  (A1),D6
     addq.l  #2,A1
     ror.l   D7,D6
     bra.s   .done2
.fill4:
     add.w   A4,D7
     move.l  (A1),D6
     addq.l  #2,A1
     ror.l   D7,D6
     bra.s   .done4
.nofill5:
     rol.l   D1,D6
     bra.s   .done5

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
********************************************************************************
     END
