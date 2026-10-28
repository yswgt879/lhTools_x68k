	.ORIGIN: 40C5
	.CODE

	;; A variable SHOULD be initialized
%00h	.EQU	0000

	.MACRO: DOIT
		WORD __#0
	__#0	.EQU	[-2].
		BYTE __#1
		LENGTH __#2
		STRINGIFY __#2
		__#2	.EQU .
	.ENDMACRO

	DOIT	%00h 00 SetCRet
	SCF
	RET

	DOIT	%00h 00 ClrCRet
	RCF
	RET

	.END
