	.MACRO:	LDR
		LD	#<__#0,<__#1	; rH register loaded with high 8-bits
		LD	#>__#0,>__#1	; rL register loaded with low 8-bits
	.ENDMACRO
	LDR	HL,8899
