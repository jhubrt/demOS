	
	output  D:\PROJECTS\DEMOS\OUTPUT\ARJDEP2.O

	xdef	ARJdepack,ARJprogress


; ARJ mode4 decode function
; Size optimized
; Copyleft 1993 Mr Ni! (the Great) of the TOS-crew
;
; Note:
; ARJ_OFFS.TTP. This program is an addition to UNARJ_PR. It
; calculates the minimum offset between the source and destination
; address for in memory depacking of files.
; (Depacking A1-referenced data to A0... The calculated 'offset' is
; the minimum amount of bytes required to be reserved before the
; packed data block.)
;
; void ARJdepack(char* depack_space, char* packed_data)
;
; CALL:
; A0 = ptr to depack space
; A1 = ptr to packed data
;
; Register usage:
; d0: temporary register
; d1: temporary register
; d2: temporary register, pointer offset
; d3: bytes to do counter
; d4: #bytes to copy
; d5: klad
; d6: bitbuf,subbitbuf
; d7: #bits in subbitbuf
; a0: depack space
; a1: rbuf_current
; a2: source adres for byte copy
; a3: temporary register
; a4: progress adress
; a5: not used
; a6: not used

ARJdepack:
                MOVEM.L D0-D7/A2-A4,-(SP) ; save registers

				lea		ARJprogress(pc),a4
				move.l	a1,(a4)

                move.l  (a1)+,d0        ; D0 = origsize (long) added to bytes flow start

                MOVE.L  D0,D3           ; origsize
                MOVEQ   #0,D7           ; bitcount = 0
                MOVE.W  A1,D0           ; for checking rbuf_current
                BTST    D7,D0           ; does readbuf_current point to an even address?
                BEQ.S   CONT            ; yes
                MOVE.B  (A1)+,D6        ; pop eight  bits
                MOVEQ   #8,D7           ; 8 bits in subbitbuf
                LSL.W   #8,D6           ;
CONT:
                MOVEQ   #$10,D4         ; push 16 (8) bits into bitbuf
                SUB.W   D7,D4           ; subtract still available bits from  d5
                LSL.L   D7,D6           ;
                MOVE.W  (A1)+,D6        ; word in subbitbuf
                LSL.L   D4,D6           ; fill bitbuf
COUNT_LOOP:                             ; main depack loop
                MOVE.L  D6,D1           ; evaluate most significant bit bitbuf
                BMI.S   START_SLD       ; =1 -> sliding dictionary
                MOVEQ   #9,D0           ; pop bits from bitbuf for literal
                BSR.S   GETBITS         ;
                MOVE.B  D2,(A0)+        ; push byte in buffer
EVAL_LOOP:
				move.l	a1,(a4)			; progress update

				SUBQ.L  #1,D3           ;
                BNE.S   COUNT_LOOP      ;
                MOVEM.L (SP)+,D0-D7/A2-A4 ;
				
                RTS                     ;

START_SLD:
                MOVEA.W #8,A3           ;
                MOVEQ   #0,D2           ; max power
                BSR.S   GET_THEM        ;
                ADD.W   D2,D5           ; length
                MOVE.W  D5,D4           ;
                MOVE.L  D6,D1           ; bitbuf
                SUBQ.W  #3,A3           ; move.w  #5,a3
                MOVEQ   #9,D2           ; minimum getbits
                BSR.S   GET_THEM        ;
                ROR.W   #7,D5           ;
                ADDQ.W  #1,D4           ; increment len by one
                ADD.W   D5,D2           ; calc pointer
                NEG.W   D2              ; pointer offset negatief
                LEA     -1(A0,D2.w),A2  ; pointer in dictionary
                SUB.L   D4,D3           ; sub 'bytes to copy' from 'bytes to do' (d4 is 1 too less!)
COPY_LOOP_0:
                MOVE.B  (A2)+,(A0)+     ;
                DBRA    D4,COPY_LOOP_0  ;
                BRA.S   EVAL_LOOP       ;

GET_THEM:
                MOVEQ   #1,D0           ; minimum fillbits
                MOVEQ   #0,D5           ; value
LOOP:
                ADD.L   D1,D1           ; shift bit outside
                BCC.S   EINDE           ; if '1' end decode
                ADDX.W  D5,D5           ; value *2+1
                ADDQ.W  #1,D0           ; extra fill
                ADDQ.W  #1,D2           ; extra get
                CMP.W   A3,D0           ; max bits
                BNE.S   LOOP            ; nog mal
                SUBQ.W  #1,D0           ; 1 bit less to trash
EINDE:
                BSR.S   FILLBITS        ; trash bits
                MOVE.W  D2,D0           ; bits to get
GETBITS:
                MOVE.L  D6,D2           ;
                CLR.W   D2              ;
                ROL.L   D0,D2           ;
FILLBITS:
                SUB.B   D0,D7           ; decrease subbitbuf count
                BCC.S   NO_FILL         ;
                MOVE.B  D7,D1           ;
                ADD.B   D0,D1           ;
                SUB.B   D1,D0           ;
                ROL.L   D1,D6           ;
                MOVE.W  (A1)+,D6        ;
                ADD.B   #16,D7          ; bits in subbitbuf
NO_FILL:
                ROL.L   D0,D6           ; bits to pop from buffer
                RTS                     ;

ARJprogress:	dc.l	0