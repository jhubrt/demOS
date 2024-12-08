
	*-----* A KEY *----*

vertex_key:
	DC.W 0		; SPEED_X	  
	DC.W 4		; SPEED_Y   
	DC.W 6		; SPEED_Z

	DC.W 11-1			* NOMBRE DE POINTS DANS L'OBJETS
	DC.W 50,-140,30		* 0
	DC.W 50,-140,-30	* 1
	DC.W 140,-90,30		* 2
	DC.W 140,-90,-30	* 3
	DC.W 50,90,30		* 4			; FACE DE DERRIERE
	DC.W 50,90,-30	 	* 5
	
	DC.W 70,-110,30		* 6
	DC.W 70,-110,-30	* 7
	DC.W 70,80,30		* 8			; FACE DE DERRIERE
	DC.W 70,80,-30	 	* 9
	
	DC.W 0,80,0			* 10

primitives_key:
	DC.W 13-1			* NOMBRE DE DROITES DANS L'OBJET
	DC.W 0*4,1*4		* TECHNIQUE DE RELIAGE DES POINTS !
	DC.W 0*4,2*4 	
	DC.W 1*4,3*4
	DC.W 2*4,3*4
	DC.W 0*4,4*4
	DC.W 1*4,5*4

	DC.W 6*4,8*4
	DC.W 7*4,9*4
	DC.W 8*4,9*4
	DC.W 8*4,4*4
	DC.W 9*4,5*4
	DC.W 6*4,2*4
	DC.W 7*4,3*4

	DC.W 0
	dc.w 10*4,60*512

anim_none:
anim_key:
	rts

	*-----* 4 KEY *----*

vertex_4keys:
	DC.W 0		; SPEED_X	  
	DC.W 4		; SPEED_Y   
	DC.W 2		; SPEED_Z	  

	DC.W 14-1			* NOMBRE DE POINTS DANS L'OBJETS
	DC.W -156,60,0		* 0
	DC.W 156,60,0		* 1
	DC.W -156,30,0		* 2
	DC.W 156,30,0		* 3
	DC.W -156,0,0		* 4			; FACE DE DERRIERE
	DC.W 156,0,0	 	* 5
	DC.W -156,-30,0		* 6
	DC.W 156,-30,0		* 7
	DC.W -156,-60,0		* 8
	DC.W 156,-60,0		* 9
	
	DC.W -90,30,0		* 10
	DC.W -30,-45,0		* 11
	DC.W 30,45,0		* 12
	DC.W 90,-30,0		* 13

primitives_4keys:
	DC.W 5-1			* NOMBRE DE DROITES DANS L'OBJET
	
	DC.W 0*4,1*4
	DC.W 2*4,3*4 	
	DC.W 4*4,5*4
	DC.W 6*4,7*4
	DC.W 8*4,9*4

circle_primitives_4keys:
	DC.W 3
	dc.w 10*4,15*512
	dc.w 11*4,15*512
	dc.w 12*4,15*512
	dc.w 13*4,15*512

anim_4keys:

	lea			circle_primitives_4keys+4(pc),a1
	move.w		#15*512,d0
	move.w		#1024,d1

	cmp.w		(a1),d0
	beq.s		.noreducevox1
	sub.w		d1,(a1)
.noreducevox1:

	cmp.w		4(a1),d0
	beq.s		.noreducevox2
	sub.w		d1,4(a1)
.noreducevox2:

	cmp.w		8(a1),d0
	beq.s		.noreducevox3
	sub.w		d1,8(a1)
.noreducevox3:

	cmp.w		12(a1),d0
	beq.s		.noreducevox4
	sub.w		d1,12(a1)
.noreducevox4:

	lea			wizinfo+3,a0

	move.w		#29*512,d0

	btst		#0,(a0)
	beq.s		.novx1
	move.w		d0,(a1)
.novx1:	

	btst		#1,(a0)
	beq.s		.novx2
	move.w		d0,4(a1)
.novx2:	

	btst		#2,(a0)
	beq.s		.novx3
	move.w		d0,8(a1)
.novx3:	

	btst		#3,(a0)
	beq.s		.novx4
	move.w		d0,12(a1)
.novx4:	

	rts

	*-----* stuff *----*

vertex_discs:
	DC.W 0		; SPEED_X	  
	DC.W 0		; SPEED_Y   
	DC.W 4   	; SPEED_Z

	DC.W 4-1			* NOMBRE DE POINTS DANS L'OBJETS
	DC.W -70,-70,0		* 0
	DC.W 70,-70,0		* 1
	DC.W -70,70,0		* 2
	DC.W 70,70,0		* 3
	
primitives_discs:
	DC.W -1				

circle_primitives_discs:
	DC.W 7
	dc.w 0*4,30*512
	dc.w 1*4,30*512
	dc.w 2*4,30*512
	dc.w 3*4,30*512

	dc.w 0*4,30*512
	dc.w 1*4,30*512
	dc.w 2*4,30*512
	dc.w 3*4,30*512

anim_discs:

	lea			circle_primitives_discs+4(pc),a1
	move.w		#30*512,d0
	move.w		#1024,d1

	cmp.w		(a1),d0
	beq.s		.noreducevox1
	sub.w		d1,(a1)
.noreducevox1:

	cmp.w		4(a1),d0
	beq.s		.noreducevox2
	sub.w		d1,4(a1)
.noreducevox2:

	cmp.w		8(a1),d0
	beq.s		.noreducevox3
	sub.w		d1,8(a1)
.noreducevox3:

	cmp.w		12(a1),d0
	beq.s		.noreducevox4
	sub.w		d1,12(a1)
.noreducevox4:

	lea			wizinfo+3,a0

	move.w		#60*512,d0

	btst		#0,(a0)
	beq.s		.novx1
	move.w		d0,(a1)
.novx1:	

	btst		#1,(a0)
	beq.s		.novx2
	move.w		d0,4(a1)
.novx2:	

	btst		#2,(a0)
	beq.s		.novx3
	move.w		d0,8(a1)
.novx3:	

	btst		#3,(a0)
	beq.s		.novx4
	move.w		d0,12(a1)
.novx4:	

	rts

objet_1:
	DC.W 2		; SPEED_X	  
	DC.W 2		; SPEED_Y   
	DC.W 2		; SPEED_Z	  

	DC.W 8-1			* NOMBRE DE POINTS DANS L'OBJETS
	DC.W -88,-88,+88	* 0			; FACE DE DEVANT
	DC.W +88,-88,+88	* 1
	DC.W +88,+88,+88	* 2
	DC.W -88,+88,+88	* 3
	DC.W -88,-88,-88	* 4			; FACE DE DERRIERE
	DC.W +88,-88,-88 	* 5
	DC.W +88,+88,-88	* 6
	DC.W -88,+88,-88	* 7

table_1:
	DC.W 12-1			* NOMBRE DE DROITES DANS L'OBJET
	DC.W 0*4,1*4		* TECHNIQUE DE RELIAGE DES POINTS !
	DC.W 1*4,2*4 	
	DC.W 2*4,3*4
	DC.W 3*4,0*4
	DC.W 4*4,5*4
	DC.W 5*4,6*4
	DC.W 6*4,7*4
	DC.W 7*4,4*4
	DC.W 4*4,0*4
	DC.W 7*4,3*4
	DC.W 6*4,2*4
	DC.W 5*4,1*4

	DC.W -1


	*-----* UNE PYRAMIDE *----*

objet_2:
	DC.W 2		; SPEED_X	  
	DC.W 2		; SPEED_Y   
	DC.W 2		; SPEED_Z	  
	
	DC.W 5-1
	DC.W +00,-200,+00
	DC.W +88,+88,+88
	DC.W -88,+88,+88
	DC.W -88,+88,-88
	DC.W +88,+88,-88
	
table_2:
	DC.W 8-1
	DC.W 0*4,1*4	
	DC.W 0*4,2*4	
	DC.W 0*4,3*4	
	DC.W 0*4,4*4	
	DC.W 1*4,2*4	
	DC.W 2*4,3*4	
	DC.W 3*4,4*4	
	DC.W 4*4,1*4
	
	DC.W -1
