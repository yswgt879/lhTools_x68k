	.ORIGIN:	40C5
	.CODE
	AND	(MN)
	OR	(MN),&FF
	AND	(BC) ; comment
	AND	(BC)
	AND	(BC),&00	; comment
	AND	(1234)	; comment
	AND	(1234)
	AND	(1234),55	; comment
	.END
