
		output  D:\PROJECTS\DEMOS\OUTPUT\XSTARTUP.O

        xdef _BasPag
        xdef _app
        xdef _RedirTab
        xdef _StkLim
        xdef _PgmSize

        xdef __text,__data,__bss

        xref main
        xref _StkSize

; Base page structure

        OFFSET 0

TpaStart:
        DS.L   1
TpaEnd:
        DS.L   1
TextSegStart:
        DS.L   1
TextSegSize:
        DS.L   1
DataSegStart:
        DS.L   1
DataSegSize:
        DS.L   1
BssSegStart:
        DS.L   1
BssSegSize:
        DS.L   1
DtaPtr:
        DS.L   1
PntPrcPtr:
        DS.L   1
Reserved0:
        DS.L   1
EnvStrPtr:
        DS.L   1
Reserved1:
        DS.B   7
CurDrv:
        DS.B   1
Reserved2:
        DS.L   18
CmdLine:
        DS.B   128
BasePageSize:
        DS     0

; Data segment

        BSS
__bss:

; Pointer to base page

_BasPag:
        DS.L   1

; Application flag

_app:
        DS.W   1

; Stack limit

_StkLim:
        DS.L   1

; Program size

_PgmSize:
        DS.L   1

; Redirection address table

_RedirTab:
        DS.L   6

; Initialized data segment

        DATA
__data:

; Code segment

        TEXT
__text:

Start:
        BRA.B   Run
			
; Configuration data

; Redirection array pointer

        DC.L   _RedirTab

; Stack size entry
        DC.L   _StkSize

        EVEN

; Setup pointer to base page

Run:
        MOVE.L  4(A7),A3   ; BasePagePointer from Stack
        MOVE.W  #1,_app    ; Program is Application
        MOVE.L  A3,_BasPag

; Compute size of required memory
; := text segment size + data segment size + bss segment size
;  + stack size + base page size
; (base page size includes stack size)

        MOVE.L  TextSegSize(A3),A1
        ADD.L   DataSegSize(A3),A1
        ADD.L   BssSegSize(A3),A1
        ADD.W   #BasePageSize,A1
        MOVE.L  A1,_PgmSize

; Setup longword aligned application stack

        MOVE.L  A3,D1
        ADD.L   A1,D1
        AND.B   #$FC,D1
        MOVE.L  D1,A7

; Setup stack limit variable

        SUB.L   #_StkSize-256,D1
        MOVE.L  D1,_StkLim
	
        move.l  d1,a2
        lea     256(a2),a2
        move.w  #_StkSize-1,d2
.signstack:
        move.b  #$FA,(a2)+
        dbf.w   d2,.signstack

        JSR     main

loop:	BRA.S	loop

        END
