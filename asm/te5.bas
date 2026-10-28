			.MACRO:	LDBC_nn
			LD	B,<__#0
			LD	C,>__#0
			.ENDMACRO
			;.BASIC
	10 REM	\asm[
		%80h	.EQU	^08
			LDA	00
			LDBC_nn	7750
			LD	L,%80h
		loop:
			STI	(BC)
			DJC	loop
			RET
		\]end
	20 POKE A, \asm[
			SBR	(F2)
			CALL	&ED00
			RET
		\]end
	30 E$="\asm[
			LDBC_nn	str
			RET
		str:	.EQU	.
			\$A \$B \$C
		\]end EFGH"
	40 DATA	\asm[
		PUSH	HL
		PUSH	BC
		CALL	BEEP1
		POP	BC
		POP	HL
		RET
		\]end
	50 END
