	.MACRO:	JR>
		JR	==,+02	; If Z values are equal, test is false
		JR	>=,__#0	; If C, the test is true 
	.ENDMACRO

		LDA     10
 		LD      B,09
		JR>     gt
		RET		; test false
	gt:			; Greater than
