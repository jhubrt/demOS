; nrv2s decompression in pure 68k asm
; by ross, changed for the n2 decoder by Hans -Mr Ni!- Wessels
;
; void decode_n1(char* depack_space, char* packed_data)
; CALL:
; A0 = ptr to depack space
; A1 = ptr to packed data
; (decompress from A0 to A1)
;
; Register usage:
;       a2      m_pos
;       a3      constant: -$1024, max len 2 pointer
;
;       d0      bit buffer
;       d1      m_off
;       d2      m_len or -1
;
;       d3      last_ptr_1
;		d4		last_ptr_0
;       d5      constant: 3
;
; Notes:
;       we have max_offset = 2^23, so we can use some word arithmetics on d1
;       we have max_match = 65535, so we can use word arithmetics on d2
;

export decode_n2

decode_n2:
     movem.l D3-D5/A2-A3,-(SP)
; ------------- setup constants -----------

     moveq   #-$80,D0         ; d0.b = $80 (byte refill flag)
     moveq   #-1,D2
     moveq   #-1,D3           ; last_ptr1 = 0
     moveq   #-2,D4           ; last_ptr0 = 1
     moveq   #3,D5
     movea.w #-1024,A3        ; max len 2 ptr

; ------------- DECOMPRESSION -------------
decompr_literal:
     move.b  (A1)+,(A0)+      ; copy literal
decompr_loop:
     bsr.s   get_bit
     bcc.s   decompr_literal  ; literal
decompr_match:
     moveq   #-2,D1           ; ptr init
get_pointer:
     bsr.s   get_bit
     addx.w  D1,D1            ; shift bit in D1, max 2^23!
     bsr.s   get_bit
     bcc.s   get_pointer      ; nog een bit!
decompr_select:
     addq.w  #3,D1            ; 0 if last pointer, 1 or 2 for exit token
     bcs.s   special_token    ; d1 was -2 or -1
     lsl.l   #8,D1            ; 8 extra bits
     move.b  (A1)+,D1         ; insert bits, dus is D1 max 2^23
     move.l  D1,D3            ; last_ptr1 = m_off

decompr_get_mlen:             ; implicit d2 = -1
     bsr.s   get_bit
     addx.w  D2,D2            ; schuif bit in D2
     bsr.s   get_bit
     addx.w  D2,D2            ; schuif bit in D2, D2=-1..-4
     lea     0(A0,D3.l),A2    ; src ptr in A2
     addq.w  #2,D2            ;
     ble.s   decompr_tiny_mlen ; D1=1: lange lengte, anders D1=-2..0
get_len:                      ; implicit d2 = 1
     bsr.s   get_bit
     addx.w  D2,D2            ; shift bit in d2
     bsr.s   get_bit
     bcc.s   get_len          ; nog een bit
     subq.w  #1,D2
decompr_tiny_mlen:
     move.l  D3,D1            ; last_offset
     sub.l   A3,D1            ; a3=max len 2 ptr, d1 = last_offset+max len 2 ptr
     addx.w  D5,D2            ; d5 = 2, d2+=2+x bit, last offset groter A3 dan extra byte copy... nice, D2=0..2 + x-bit= 2..4+xbit bytes copy
L_copy1:
     move.b  (A2)+,(A0)+      ; copy bytes
     dbra    D2,L_copy1
     bsr.s   get_bit
     bcc.s   decompr_literal  ; literal
     exg     D3,D4
     bra.s   decompr_match
special_token:
     beq.s   decompr_get_mlen ; last m_off gebruik last pointer
decompr_exit_token:
     movem.l (SP)+,D3-D5/A2-A3
     rts

get_bit:                      ; zet bit in x-bit status register
     add.b   D0,D0            ; schuif bit naar buiten
     bne.s   get_bit_done     ; klaar
     move.b  (A1)+,D0         ; nieuwe bits
     addx.b  D0,D0            ; bit naar buiten, sentry naar binnen
get_bit_done:
     rts

     END
