#include "DEMOSDK/BASTYPES.H"
#include "DEMOSDK/STANDARD.H"

#ifndef __TOS__

#include "DEMOSDK\PC\EMUL.H"

static EMUL68k p;

static void GETBITS(void);
static void FILLBITS(void);
static void GET_THEM(void);

// Direct port from Mr Ni! ASM routine for PC

void ARJdepack(void* depack_space, void* packed_data) 
{
    p.a0.l = (u32) depack_space;
    p.a1.l = (u32) packed_data;

    p.d0.l = STDswap32(*EMUL_L_I(&p.a1));                     // MOVE.L  (a1)+,D0

    p.d3.l = p.d0.l;                                          // MOVE.L  D0,D3           ; origsize
    p.d7.l = 0;                                               // MOVEQ   #0,D7           ; bitcount = 0
    p.d0.w = p.a1.w;                                          // MOVE.W  A1,D0           ; for checking rbuf_current

    if ( EMUL_BTST( p.d0.l, p.d7.b ) )                        // BTST    D7,D0           ; does readbuf_current point to an even address?
    {                                                         // BEQ.S   CONT            ; yes
        p.d6.b = *EMUL_B_I(&p.a1);                            // MOVE.B  (A1)+,D6        ; pop eight  bits
        p.d7.l = 8;                                           // MOVEQ   #8,D7           ; 8 bits in subbitbuf
        p.d6.w <<= 8;                                         // LSL.W   #8,D6           ;
    }

    p.d4.l = 0x10;                                            // MOVEQ   #$10,D4         ; push 16 (8) bits into bitbuf
    p.d4.w -= p.d7.w;                                         // SUB.W   D7,D4           ; subtract still available bits from  d5
    EMUL_LSL_L(&p.d6, p.d7.b);                                // LSL.L   D7,D6           ;
    p.d6.w = STDswap16(*EMUL_W_I(&p.a1));                     // MOVE.W  (A1)+,D6        ; word in subbitbuf
    EMUL_LSL_L(&p.d6, p.d4.b);                                // LSL.L   D4,D6           ; fill bitbuf

COUNT_LOOP:                                                   // ; main depack loop
    p.d1.l = p.d6.l;                                          // MOVE.L  D6,D1           ; evaluate most significant bit bitbuf

    if ( ((s32)p.d1.l) < 0 )                                  // BMI.S   START_SLD       ; =1 -> sliding dictionary
        goto START_SLD;

    p.d0.l = 9;                                               // MOVEQ   #9,D0           ; pop bits from bitbuf for literal

    GETBITS();                                                // BSR.S   GETBITS         ;

    *EMUL_B_I(&p.a0) = p.d2.b;                                // MOVE.B  D2,(A0)+        ; push byte in buffer

EVAL_LOOP:

    p.d3.l--;                                                 // SUBQ.L  #1,D3           ;
    
    if ( p.d3.l != 0 )
        goto COUNT_LOOP;                                      // BNE.S   COUNT_LOOP      ;
				
    return;													  // RTS

START_SLD:

	p.a3.l = 8;												  // MOVEA.W #8,A3           ;
    p.d2.l = 0;												  // MOVEQ   #0,D2           ; max power
    GET_THEM();												  // BSR.S   GET_THEM        ;
				
	p.d5.w += p.d2.w;										  // ADD.W   D2,D5           ; length
    p.d4.w  = p.d5.w;           							  // MOVE.W  D5,D4           ;
    p.d1.l  = p.d6.l;										  // MOVE.L  D6,D1           ; bitbuf
	
	p.a3.l -= 3;											      // SUBQ.W  #3,A3           ; move.w  #5,a3
    p.d2.l  = 9;											  // MOVEQ   #9,D2           ; minimum getbits
    GET_THEM();												  // BSR.S   GET_THEM        ;
    EMUL_ROR_W(&p.d5,7);            						  // ROR.W   #7,D5           ;    
	p.d4.w++;				 							      // ADDQ.W  #1,D4           ; increment len by one
    p.d2.w  += p.d5.w;										  // ADD.W   D5,D2           ; calc pointer
    p.d2.w   = -p.d2.w;										  // NEG.W   D2              ; pointer offset negatief
	
	p.a2.l  = p.a0.l + (s16) p.d2.w - 1;					      // LEA     -1(A0,D2.w),A2  ; pointer in dictionary
	
	p.d3.l -= p.d4.l;										  // SUB.L   D4,D3           ; sub 'bytes to copy' from 'bytes to do' (d4 is 1 too less!)

COPY_LOOP_0:

	*EMUL_B_I(&p.a0) = *EMUL_B_I(&p.a2);					  // MOVE.B  (A2)+,(A0)+     ;
	p.d4.w--;												  // DBRA    D4,COPY_LOOP_0  ;
	if ( p.d4.w != 0xFFFF )
		goto COPY_LOOP_0;
    goto EVAL_LOOP;											  // BRA.S   EVAL_LOOP       ;
}


void GET_THEM(void)
{
    p.d0.l = 1;												  // MOVEQ   #1,D0           ; minimum fillbits
    p.d5.l = 0;												  // MOVEQ   #0,D5           ; value
LOOP:
    p.carry = (p.d1.l & 0x80000000UL) != 0;
    p.d1.l += p.d1.l;										  // ADD.L   D1,D1           ; shift bit outside
	if ( p.carry )										      // BCC.S   EINDE           ; if '1' end decode
	{
  	    p.d5.w += p.d5.w;
	    p.d5.w ++;  										  // ADDX.W  D5,D5           ; value *2+1
                
	    p.d0.w ++;											  // ADDQ.W  #1,D0           ; extra fill
        p.d2.w ++;            								  // ADDQ.W  #1,D2           ; extra get
	
	    if ( p.a3.w != p.d0.w )								  // CMP.W   A3,D0           ; max bits
		    goto LOOP;										  // BNE.S   LOOP            ; nog mal
        p.d0.w --;											  // SUBQ.W  #1,D0           ; 1 bit less to trash
   }                                                          // EINDE:

    FILLBITS();												  // BSR.S   FILLBITS        ; trash bits
    p.d0.w = p.d2.w;                                          // MOVE.W  D2,D0           ; bits to get
	GETBITS();
}


static void GETBITS(void)
{
    p.d2.l = p.d6.l;                     			// MOVE.L  D6,D2           ;
    p.d2.w = 0;                         			// CLR.W   D2              ;
    
    EMUL_ROL_L(&p.d2, p.d0.b);   	    			// ROL.L   D0,D2           ;

	FILLBITS();
}

static void FILLBITS(void)
{
    EMUL_SUB_B(&p.d0, &p.d7, &p); 			        // SUB.B   D0,D7           ; decrease subbitbuf count
    
	if (p.carry)									// BCC.S   NO_FILL         ;
	{
		p.d1.b  = p.d7.b;               			// MOVE.B  D7,D1           ;
		p.d1.b += p.d0.b;               			// ADD.B   D0,D1           ;
		p.d0.b -= p.d1.b;               			    // SUB.B   D1,D0           ;
            
		EMUL_ROL_L(&p.d6, p.d1.b);					// ROL.L   D1,D6           ;
		p.d6.w = STDswap16(*EMUL_W_I(&p.a1));       // MOVE.W  (A1)+,D6        ;
		p.d7.b += 16;                     		    // ADD.B   #16,D7          ; bits in subbitbuf
	}                                               // NO_FILL:

    EMUL_ROL_L( &p.d6, p.d0.b );           			// ROL.L   D0,D6           ; bits to pop from buffer
}

#endif
