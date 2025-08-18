;
; unstore routine, size optimized
;
; void unstore(unsigned long size, uint8_t *dst, uint8_t *data)

export unstore

; DO size
; A0 dst
; A1 src

loop:
		move.b	(A1)+,(A0)+
unstore:
		subq.l	#1,D0
		bcc.s	loop
		rts
