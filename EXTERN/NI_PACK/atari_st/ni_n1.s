; Ni mode 1 decode function
; Size optimized
; Placed in public domain 2022 Hans Wessels (Mr Ni! (the Great) of the TOS-crew)
;
; void decode_n1(char* depack_space, char* packed_data)
; CALL:
; A0 = ptr to depack space
; A1 = ptr to packed data;
;
; Register usage:
; d0: #bytes to copy
; d1: temporary register, bitbuf
; d2: temporary register, pointer offset
; d3: temporary register
; d4: temporary register
; d5: temporary register
; d6: bitbuf,subbitbuf
; d7: #bits in subbitbuf
; a0: depack space
; a1: rbuf_current
; a2: source adres for byte copy
; a3: end address
; a4: not used
; a5: not used
; a6: not used

export decode_n1

decode_n1:
     movem.l D3-D7/A2-A3,-(SP) ; save registers
     moveq   #0,D7            ; bitcount = 0
     move.w  A1,D3            ; for checking rbuf_current
     btst    D7,D3            ; does readbuf_current point to an even address?
     beq.s   .cont            ; yes
     addq.l  #1,A1            ;
     moveq   #8,D7            ; 8 bits in subbitbuf
.cont:
     move.l  -2(A1),D6        ; fill bitbuf
     ror.l   D7,D6            ; 
     moveq   #16,D3           ; 16 bits for packed size
     bsr.s   .getbits         ; first 16 bits
     move.w  D2,D0            ;
     moveq   #16,D3           ; 16 bits for packed size
     swap    D0
     bsr.s   .getbits         ; second 16 bits
     move.w  D2,D0            ;
     lea     0(A0,D0.l),A3    ; end address
     moveq   #9,D3            ; pop bits from bitbuf for literal
.count_loop:                  ; main depack loop
     move.w  D6,D1            ; evaluate most significant bit bitbuf
     bmi.s   .start_sld       ; =1 -> sliding dictionary
     bsr.s   .getbits         ;
     move.b  D2,(A0)+         ; push byte in buffer
.eval_loop:
     cmpa.l  A3,A0            ; end?
     bls.s   .count_loop      ;
     movem.l (SP)+,D3-D7/A2-A3 ;
     rts                      ;

.start_sld:
     moveq   #14,D4           ;
     moveq   #14,D2           ; minimum getbits + D4
     bsr.s   .get_them        ;
     add.w   D2,D5            ; length
     move.w  D6,D1            ; bitbuf
     move.w  D5,D0            ;
     moveq   #6,D4            ;
     moveq   #15,D2           ; minimum getbits + D4
     bsr.s   .get_them        ;
     moveq   #9,D3
     lsl.l   D3,D5            ;
     lea     -1(a0),A2
     move.w  D2,D4            ; clear high word
     sub.l   D4,A2
     sub.l   D5,A2
.copy_loop:
     move.b  (A2)+,(A0)+      ;
     dbra    D0,.copy_loop    ;
     bra.s   .eval_loop       ;

.get_them:
     moveq   #0,D3            ; fillbits
     moveq   #0,D5            ; value
.loop:
     addq.w  #1,D3            ; extra fill
     add.w   D1,D1            ; shift bit outside
     bcc.s   .einde           ; if '1' end decode
     addx.w  D5,D5            ; value *2+1
     dbra    D4,.loop
.einde:
     sub.w   D4,D2            ; correct D2
     bsr.s   .fillbits        ; trash bits
     move.w  D2,D3            ; bits to get
; no_bits=D3
; result=D2
.getbits:
     move.w  D6,D2
     swap    D2
     rol.l   D3,D2
.fillbits:
     sub.b   D3,D7
     bcs.s   .fill
.no_fill:
     rol.l   D3,D6
     rts
.fill:
     add.b   #16,D7
     move.l  (A1),D6
     addq.l  #2,A1
     ror.l   D7,D6
     rts

;d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,sp
********************************************************************************
     END
