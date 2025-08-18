;
; ni packer n0 decoder, size optimized
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

export decode_n0

decode_n0:
     move.l  A2,-(SP)         ; save A2
     moveq   #-128,D1         ; bit buffer sentry
.lit:
     move.b  (A1)+,(A0)+      ; copy
.loop:
     bsr.s   .get_bit         ; is het een pointer len of een literal?
     bcc.s   .lit             ; pointer
.ptr_len:
     moveq   #-1,D0           ; init offset
     move.b  (A1)+,D0         ; 1e 8 bits
     bsr.s   .get_bit         ; 8 of 16 bit offset
     bcc.s   .bit_8           ; 8 bit pointer
     lsl.w   #8,D0            ; schuif bits
     beq.s   .done            ; klaar!
     not.w   D0               ; negatief maken
     move.b  (A1)+,D0         ; 2e 8 bits
.bit_8:
     lea     0(A0,D0.l),A2    ; pointer offset
     moveq   #1,D0            ; init waarde
.next_bit:
     bsr.s   .get_bit         ; get value bit in x register
     addx.w  D0,D0            ; verdubbel en tel waarde van bit op bij D0
     bsr.s   .get_bit         ; nog een bit?
     bcc.s   .next_bit        ; yep
.copy_loop:
     move.b  (A2)+,(A0)+      ; copy
     dbra    D0,.copy_loop
     bra.s   .loop
.get_bit:                     ; zet bit in x-bit status register
     add.b   D1,D1            ; schuif bit naar buiten
     bne.s   .get_bit_done    ; klaar
     move.b  (A1)+,D1         ; nieuwe bits
     addx.b  D1,D1            ; bit naar buiten, sentry naar binnen
.get_bit_done:
     rts
.done:
     movea.l (SP)+,A2         ; restore A2
     rts
     END
