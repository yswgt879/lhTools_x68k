	.ORIGIN: 00C5
	.CODE
		.MACRO: UNTIL
		1:	.EQU	.
			JR	__#0 3:
			LD	B,<2:
			LD	C,>2:
			LDI	(BC)
			JR	#!__#0 1:
		3:
			RET
		2:
		.ENDMACRO
		UNTIL	Z %99n
		;; macro LDRR
		;;	Load RR with the 16-bits value given
		.MACRO:	LDRR
		LD	#>__#0 >__#1
		LD	#<__#0 <__#1
		.ENDMACRO
		;; expand the LDRR macro
		LDRR	HL 8009
		ADC	#*C
	%00a	.EQU	.
	%55c	.EQU	$U
		LDU	(ENDBASIC)
		LDA	%00a
		JR	%00a
		SBR	(F2)
		LDA	(%55c)
		\xE7
		CPI
		\xE5	bibi
		LD	H,<ZEEND
		LD	L,>ZEEND
		LDS	(65)
		STS	(67)
	.MACRO: LDHL_nn
		LD	H,__#0
		LD	L,__#1
	.ENDMACRO
	.MACRO:	LDR
		LD	B,<__#0	; High from __#0
		LD	C,>__#0 ; Low from __#0
	.ENDMACRO
	.MACRO:	DOFILL
		LDA	__#0
		LD	B,__#1
		LD	C,__#2
		LD	L,__#3
	__#6	.EQU	.
		STI	(BC)
		CP	B,__#4
		JR	C,__#5
		DJC	__#6
	__#5	.EQU	.
	.ENDMACRO
	.MACRO:	DOFILL2
		LDA	__#0
		LD	B,<__#1
		LD	C,>__#1
		LD	L,__#2
	3:	.EQU	.
		STI	(BC)
		CP	B,__#3
		JR	C,1:
		DJC	3:
		RCF
	1:	.EQU	.
	.ENDMACRO
	BIBI:
		STI	; also LDI (BC),DE
;		BLLLLLL
	BOBO:	.EQU	1234
		\xE1,\xE2,\xE3
		\$A,\$B,\$C
		\$!
		\"CHRISTOPHE"
	.DEFINE:	"ZEWORD"	= F0EF
		JR	NC,ZEEND
		JR	BIBI
		JR	NC,00C8
		JR	C,00C8
		JR	NH,00C8
		JR	H,00C8
		JR	NZ,00C8
		JR	Z,00C8
		JR	NV,00C8
		JR	V,00C8
		SBR	(E3)
		SBR	NH,(E3)
		SBR	NZ,(E3)
;		SBR	NV,(E3)
		SBR	NC,(E3)
		SBR	Z,(E3)
		SBR	C,(E3)
		SBR	V,(E3)
		SBR	H,(E3)
		SBR	(34),00,41,+08
		ERRH
		ERR1
		LDA	@177
		LDA	#192
		LDA	&43
		LDA	$1
		ADC	(1234)
		CPA	(&5678)
		SBC	(#16383)
		AND	(@7531)
	.INCLUDE:	asm/C.asm
		LDA	(BIBI)
		CPA	(BOBO)
		LD	SP,\X7654
		CALL	\U32768
		JP	\O4444
		JR	Z,BIBI
		\x01
		\o77
		\u99
	DE:	.EQU	DDEE
	BC:
		LDA	DE
		OR	(HL),BC
		LDA	(DE)
		OR	(BC),00
		LDHL_nn	89,AB
		NOP
	BOF:	.EQU	FC96
		LDR	BOF
		LDR	_S
		NOP
		DOFILL	00,79,00,08,7A,ENDFILL,LOOPFILL
		NOP
		DOFILL2	AA,40C5,#100,41
		NOP
		LD	SP,*HERE
		LD	L,*HERE
		JR	+02
		JR	NC,-02
	HERE:
		NOP
		NOP
		NOP
		NOP
		NOP
		RET
		LDA	^03
		LD	B,'40
		LD	C,/02
		LD	SP .
		JP	~FB73
	ZEEND:
		*BYT,^01,/00,'55,~43
		.BYTE
		*BYT *. ^05 /07 '90 ~89 ;+05 -02
		.WORD
		*BYT *. ^000c /0700 '9000 ~0089 +05 -02 .
	BYT:
		.CODE
	;%10a	.EQU	.
	%10a:
	%01z:	.EQU	[+5].
		JR	==,%10a
		JR	%01z
		LDA	95 ; >%55c
	%01z	.EQU	.
		RET
	%55c	.EQU	95
	top:
		LD	L,10
	theloop:
		LDI	(BC)
		BIT	^08
		JR	NZ top
		DJC	theloop
		RET
		.MACRO:	xFER
		LD	D,<__#0
		LD	E,>__#0
		LD	B,<__#1
		LD	C,>__#1
		LD	L,__#2
	__#3	.EQU	.
		STI
		DJC	__#3
		RET
		.ENDMACRO
		xFER	7908 4000 10 %01x
		xFER	7928 7650 04 %02x
		LD	#<HL,88
		LD	#>BC,CC
		LD	#^H,#^(BC)
		LDA	#*C
		LDRR	BC ~1324
		LDRR	BC !5555
		LDA	!20
        .MACRO: JR>
                JR      ==,+02  ; If Z values are equal, test is false
                JR      >=,__#0 ; If C, the test is true
        .ENDMACRO
		LDA	10
		LD	B,09
		JR>	gt
		RET		; test false
	gt:			; Greater than

	.ALIGN: 0800

	.HOLE
	.SKIP	054

;	.KEYWORD
;		"ZEWORD"

	.BYTE

A	.equ &4800
B	.equ &4200
SW:	.ARRAYOF WORD A B
SB:	.ARRAYOF BYTE A B
E:	.ARRAYOF 40 A B
	\xFF >E
	.FILLTO: 4130 WITH FF 00
	.FILL: 10 WITH 00
TEN	.EQU 10
ONE	.EQU 01
TWO	.EQU 02

	.IF	EQUAL? TEN 10
		#10
	.ENDIF
	.IF	LESS? TEN 03
		#3
	.ENDIF
	.IF	GREATER? 03 TWO
		#2
	.ENDIF

	.BYTE
		}}}}}}[=6]&7c
		[=6]&65
		>[+35]&1234

	.code

mondodo	.EQU	&__/DODO/00
here	.EQU	&BEEF

mysym:
	LDA	(1-mysym)
	JP 1234

	LDA	>1+mysym
	LD	B,<mysym
	LD	C,>mysym
	CPA	>2-mysym

%07c:	.ARRAYOF 100 A B
	LD	H,>%07c

thissym	.EQU	__/MYSYM/
	CALL	thissym

	CALL	7-mysym
	JR	4+mysym
	AND	(3-mysym)

	.IF	EXIST?	DORESET
	CALL	&E000
	.ELSE
	NOP	; do nothing
	NOP
	NOP
	.ENDIF

	.IF	EQUAL? <mondodo &BE
	CALL	&0BED
	.ELSE
	RET
	RET
	RET
	.WARNING	"Ooops: mondodo badly defined"
	.ENDIF

	.IF	EXIST? here
	.ELSE
	.ERROR "Missing here"
	.ENDIF

	.END
