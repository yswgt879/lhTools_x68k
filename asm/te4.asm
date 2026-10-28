	.MACRO:	XFER
		LD	B <__#0
		LD	C >__#0
		LD	D <__#1
		LD	E >__#1
		LD	L >__#2
	1:
		LDI	(BC)	; Load A with (BC) and increment BC
		STD	(DE)	; Store A to (DE) and decrement DE
		BIT	^08	; Bit 7 of A is set
		JR	C,2:	; Yes ! XFER is finished
		DJC	1:	; Decrement L and jump to 1: if not C
	2:
		RET
	.ENDMACRO

	XFER	A$ 47FF \u15
