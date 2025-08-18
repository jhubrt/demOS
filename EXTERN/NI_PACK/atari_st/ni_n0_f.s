;
; ni packer n0 decoder, speed optimized
; 2022 Hans Wessels

;void decode_n0(uint8_t *dst, uint8_t *data)
; registers
; D0 =
; D1 = bitbuffer
;
; A0 = dst
; A1 = src
; A2 = copy pointer
; alternatief pointer 8 of 16 bit, min match = 3

	output D:\PROJECTS\DEMOS\OUTPUT\NI_N0_F.O

    xdef decoden0
    xref SYSdpakActive

decoden0:
     move.b  #1,SYSdpakActive

     move.l  A2,-(SP)         ; save A2
     moveq   #-128,D1         ; bit buffer sentry
.lit:
     move.b  (A1)+,(A0)+      ; copy
     add.b   D1,D1            ; schuif bit naar buiten
     bcc.s   .lit             ; literal
.loop:
     bne.s   .ptr_len         ; klaar
     move.b  (A1)+,D1         ; nieuwe bits
     addx.b  D1,D1            ; bit naar buiten, sentry naar binnen
     bcc.s   .lit             ; literal
.ptr_len:
     moveq   #-1,D0           ; init offset
     move.b  (A1)+,D0         ; 1e 8 bits
     add.b   D1,D1            ; schuif bit naar buiten
     bcc.s   .bit_8           ; 8 bit pointer
     bne.s   .get_bit_done1   ; klaar
     move.b  (A1)+,D1         ; nieuwe bits
     addx.b  D1,D1            ; bit naar buiten, sentry naar binnen
     bcc.s   .bit_8           ; 8 bit pointer
.get_bit_done1:
     lsl.w   #8,D0            ; schuif bits
     beq.s   .done            ; klaar!
     not.w   D0               ; negatief maken
     move.b  (A1)+,D0         ; 2e 8 bits
.bit_8:
     lea     0(A0,D0.l),A2    ; pointer offset
     moveq   #1,D0            ; init waarde
.next_bit:
     add.b   D1,D1            ; schuif bit naar buiten
     bne.s   .get_bit_done2   ; klaar
     move.b  (A1)+,D1         ; nieuwe bits
     addx.b  D1,D1            ; bit naar buiten, sentry naar binnen
.get_bit_done2:
     addx.w  D0,D0            ; verdubbel en tel waarde van bit op bij D0
     add.b   D1,D1            ; schuif bit naar buiten
     bcc.s   .next_bit        ; yep
     bne.s   .get_bit_done3   ; klaar
     move.b  (A1)+,D1         ; nieuwe bits
     addx.b  D1,D1            ; bit naar buiten, sentry naar binnen
     bcc.s   .next_bit        ; yep
.get_bit_done3:
     subq.w  #1,D0
     lsr.w   #1,D0
     bcc.s   .copy_loop
     move.b  (A2)+,(A0)+      ; copy
.copy_loop:
     move.b  (A2)+,(A0)+      ; copy
     move.b  (A2)+,(A0)+      ; copy
     dbra    D0,.copy_loop
     add.b   D1,D1            ; schuif bit naar buiten
     bcc.s   .lit             ; literal
     bra.s   .loop            ; continue loop
.done:
     movea.l (SP)+,A2         ; restore A2

     clr.b  SYSdpakActive

     rts

