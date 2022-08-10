*-------------------------------------------------------------------------------------------------------
* The MIT License (MIT)
*
* Copyright (c) 2015-2022 J.Hubert
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
* and associated documentation files (the "Software"), 
* to deal in the Software without restriction, including without limitation the rights to use, 
* copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
* and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies 
* or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*--------------------------------------------------------------------------------------------------------

;-------------------------------------------------------------------------
;	BOOTSTRAP 
;
;   - equivalent to bootstrap sector but compiled as a prg to boot on B:
;   - check some hardware requirements and display error
;   - load, decompress, relocate and run program
;-------------------------------------------------------------------------

	output		D:\PROJECTS\DEMOS\OUTPUT\BLITZIK\BOOTon_B.PRG

	opt		o+,a+,p+

;-------------------------------------------------------------------------
; assemble this as Atari executable + no translation + no debug info
;-------------------------------------------------------------------------

test:           set     0
basepagesize:   set     256
textsizeoffset: set     12
datasizeoffset: set     20
bsssizeoffset:  set     28

;-------------------------------------------------------------------------
loadaddress:		set $1000
tempaddress:        set $80000

sectorspertrack:	set 10

compocolor:         set $BF
playoncecolor:      set $F00

;-------------------------------------------------------------------------
; program
;-------------------------------------------------------------------------
prg_startsector:		set 3		; 0 to 9
prg_starttrack:			set 0		; 0 to 81
prg_startside:			set 0		; 0 or 1
prg_nbsectors:			set 93		; 
;-------------------------------------------------------------------------

    ; Optimize a bit: put code instead of OEM and Serial
    clr.l       -(sp)
    move.w      #32,-(sp)
    trap        #1
    addq.l      #6,sp

	; Key sound off
	clr.b		$484.w

	; Mouse off
	lea			carmo(pc),a0
bootsectorstart:

    move.w      #-1,-(sp)      * get control key state
    move.w      #$B,-(sp)
    trap        #13
    addq.w      #4,sp

    moveq.l     #0,d1
    moveq.l     #-1,d2

    btst.l      #3,d0          * ALT
    beq.s       nocompo
    moveq.l     #1,d1
    move.w      #compocolor,d2
nocompo:

    btst.l      #2,d0          * CTRL
    beq.s       noplayoncemode
    or.w        #2,d1
    eor.w       #playoncecolor,d2
noplayoncemode:

    addq.w      #1,d1
    move.w      d1,$ffff8242.w
    move.w      d2,$ffff8240.w

    move.l      a0,-(sp)
	move.l		#25*65536+1,-(sp)	
	trap		#14
    addq.w		#8,sp   

  	; Check computer has at least 1mb
	cmp.w		#$10,$42e.w
    blt.s       error

   	; Check computer is a STe
	cmp.w		#$e0,$4.w
	beq.s		ste_ok

error:  ; Else error message
	pea			mes(pc)
	move.w		#9,-(sp)
	trap		#1
loop:	
	bra.s		loop                ; infinite loop

ste_ok:

    ; Emulator detection : do it before messing the OS state (use vsync xbios function)
    move.l  #'Emu?',d6
    move.l  d6,d7
    move.w  #$25,-(sp)
    trap    #14
    ;    addq.l  #2,sp      * optimize => do not restore sp this time
        
    lea     $600.w,a1
    move.l  d6,(a1)+
    move.l  d7,(a1)+
    move.l  a0,(a1)

    ; Relocate sp before load address
	lea	loadaddress-4.w,sp

	; Load code
	moveq.l		#prg_startsector,d0
	moveq.l		#prg_starttrack,d1
	moveq.l		#prg_startside,d2
	move.w		#prg_nbsectors,d3
	move.l      #tempaddress,a0
    lea         -512(a0),a1         ; prepare for moving code after load
	
    bsr.s       load	

    move.w      #$2700,sr

    ; Move bootsector code to a safe place to avoid being overwritten
    lea         movedcode(pc),a0
    move.l      a1,a2
    moveq.l     #127,d0
copycode:
    move.l      (a0)+,(a1)+
    dbf.w       d0,copycode

    jmp         (a2)

movedcode:
    ifne test
    addq.w      #7,$ffff8240.w 
    endc

    ; Decompress code	
    lea         loadaddress.w,a0
    ; move.l      #tempaddress,a1   optimization => a1 ever contains tempaddress...

    bsr         DECODE_F
	
    ; Relocate loaded program
    ; prepare fake basepage

	lea	loadaddress+2.w,a0
    lea -basepagesize-2(a0),a3
    	
	move.l	(a0)+,d0		; D0 = Nombre octets zone texte
    move.l  d0,textsizeoffset(a3)
	move.l	(a0)+,d1		; + Nombre octets zone data
    move.l  d1,datasizeoffset(a3)
    add.l   d1,d0
    move.l  (a0)+,bsssizeoffset(a3)
	add.l	(a0)+,d0		; + Nombre octet table des symboles 
	lea	    $A(a0),a0		; A0 pointe sur debut code

	movea.l	a0,a1			; A1 = A0 = Adresse debut code
	movea.l	a0,a2			; A2 = A0 = Adresse debut code
	move.l	a0,d1			; D1 = A0 = Valeur adr debut code
	adda.l	d0,a1			; A1 = A1 + D0 
							; A1 = Adresse table de relocation
	move.l	(a1)+,d0		; D0 = Premier offset pour trouver
							; la prochaine adresse … modifier

	adda.l	d0,a2			; A2 = A2 + D0
							; A2 = Adresse de la relocation
	add.l	d1,(a2)			; A2 = A2 + D1
							; Ajoute l'adresse du debut du code
							; … la valeur … reloger
	moveq	#0,d0			; Raz de d0

reloc2:	
    move.b	(a1)+,d0	    ; D0 = Offset du mot long suivant
	beq.s	f_reloc			; Si d0 = 0, fin de la relocation
	cmp.b	#1,d0			; Si d0 = 1, c'est qu'il y … plus
	beq.s	reloc3			; de $fe octet avant la prochaine
							; info … reloger
	adda.l	d0,a2			; A2 = A2 + D0
							; A2 = Adresse de la relocation
	add.l	d1,(a2)			; A2 = A2 + D1
							; Ajoute l'adresse du debut du code
							; … la valeur … reloger
	bra.s	reloc2			; Reboucle

reloc3:	
    lea	$fe(a2),a2		    ; A2 = A2 + $FE
	bra.s	reloc2			; Reboucle
f_reloc:

    ; End : jump to loaded code

    move.l  a3,-(sp)        ; push base page address
    clr.l   -(sp)           ; return address set to null

    ifne    test
    addq.b  #7,$ffff8240.w 
    endc

    lea     0.w,a0     
    move.w  $ffff8242.w,d0
    move.w  #$36F,$ffff8242.w
	jmp	loadaddress.w

;--------------------------------------------------------------------------
;  Loader (uses xbios 8)
;	startsector,d0
;	starttrack,d1
;	startside,d2
;	nbsectors,d3		; remaining sectors
;	tempaddress,a0
;--------------------------------------------------------------------------
load:		
	moveq.l		#sectorspertrack,d4
	sub.w		d0,d4
	cmp.w		d3,d4
	ble.s		.notlasttrack
	move.w		d3,d4
.notlasttrack:

	movem.l		d0-d7/a0-a6,-(sp)	* store registers
	
	move.w		d4,-(sp)		    * nb sectors
	move.w		d2,-(sp)		    * side
	move.w		d1,-(sp)		    * track
	move.w		d0,-(sp)		    * first sector
	addq.w		#1,(sp)		        * (1 based)
    move.w      #1,d0
    and.w       #1,d0
	move.w		d0,-(sp)      	    * bootdev floppy
	clr.l		-(sp)		        * Filler
	move.l		a0,-(sp)		    * Adr loading
	move.w		#8,-(sp)		    
	trap		#14
	lea			20(sp),sp

	movem.l		(sp)+,d0-d7/a0-a6	* restore registers
	
	sub.w		d4,d3
	ble.s		.endload

	lsl.w		#1,d4
	lsl.w		#8,d4
	add.w		d4,a0
	moveq.l	 	#0,d0			* start sector on track now 0 for next tracks 

    eor.w       #1,d2
    bne.s       load

    addq.w      #1,d1

	bra.s		load
	
.endload:
    
    rts

;-------------------------------------------------------------------------
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
; void decode_f(ulong origsize, char* depack_space, char* packed_data)
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
; a4: not used
; a5: not used
; a6: not used
;-------------------------------------------------------------------------

DECODE_F:       move.l  (a1)+,d0

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
                SUBQ.L  #1,D3           ;
                BNE.S   COUNT_LOOP      ;
                
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

;----------------------------------------------------------------	
mes:
    	dc.b	'STe 1MB mini'
        dc.b    0

carmo:	dc.b	8
        dc.b    0

;----------------------------------------------------------------	

		end
