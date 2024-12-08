*-------------------------------------------------------------------------------------------------------
* The MIT License (MIT)
*
* Copyright (c) 2015-2024 J.Hubert
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

	output		D:\PROJECTS\DEMOS\OUTPUT\RELAPSE\RBOOTonB.PRG

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
prg_nbsectors:			set 60		; 
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
    addq.l      #4,a1

    bsr         decoden0
	
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

decoden0:
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

     rts

;----------------------------------------------------------------	
mes:
    	dc.b	'STe 1MB mini'
        dc.b    0

carmo:	dc.b	8
        dc.b    0

;----------------------------------------------------------------	

		end
