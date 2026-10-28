	.ORIGIN: 47C5
	.CODE
	.DEFINE:	"RAISE"	= F0E0 N
		EVAL	doerrH
		INTG	08,doerrH
		STA	H
		LDA	H
		SBR	Z,(E2)
doerrH:
		ERRH

	; Keyword table should be aligned on a 2Kbytes frontier
	.ALIGN:	0800
	; Do not care of the &54 bytes from &xx00 to &xx53
	.HOLE
		.SKIP	054
	; The keyword table starts. Enter into a KEYWORD fragment
	.KEYWORD
		"RAISE"	; our keyword RAISE

	.END
