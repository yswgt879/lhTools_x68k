
	.code

	LDA	(HL)			;	lda (hl)
	if	Z			;	jr nz,4:
		XOR	&FF		;	xor &ff
	endif				; 5:

	NOP

	.code

	LDA	(HL)			;	lda (hl)
	if	Z			;	jr nz,4:
		XOR	&FF		;	xor &ff

	.hole
	.align: 0200
	.code

	endif				; 5:

	NOP

	.code

	LDA	(HL)			;	lda (hl)
	if	Z			;	jr nz,4:
		XOR	&FF		;	xor &ff
	else				;       jr 5:
		AND	&7F		; 4:	and 7f
	endif				; 5:

	NOP

	.code

	LDA	(HL)			;	lda (hl)
	if	Z			;	jr nz,4:
		XOR	&FF		;	xor &ff

	.hole
	.align: 0200
	.code

	else				;       jr 5:
		AND	&7F		; 4:	and 7f
	endif				; 5:

	NOP

	.code

	LDA	(HL)			;	lda (hl)
	if	Z			;	jr nz,4:
		XOR	&FF		;	xor &ff
	else				;       jr 5:
		AND	&7F		; 4:	and 7f

	.hole
	.align: 0200
	.code

	endif				; 5:

	NOP

	.code

	LDA	(HL)			;	lda (hl)
	if	Z			;	jr nz,4:

	.hole
	.align: 0200
	.code

		XOR	&FF		;	xor &ff
	else				;       jr 5:
		AND	&7F		; 4:	and 7f

	.hole
	.align: 0200
	.code

	endif				; 5:

	RET

	.end
