	.code

	begin				; 1:
		LDI	(BC)		;	ldi (bc)
	while	NZ			;	jr z,2:
		AND	&7F		;	and 7f
		STI	(DE)		;	sti (de)
	repeat				;	jr 1:
					; 2:


	begin				; 3:
		AND	(BC),&00	;	and (bc),&00
		INC	BC		;	inc bc
		CP	C,&80		;	cp c,&80
		if	Z		;	jr nz,31:
			INC	BC	;	inc bc
		endif			; 31:
		CP	C,&FF		;	cp c,&ff
	until	Z			;	jr nz,3:


	LDA	(HL)			;	lda (hl)
	BIT	&80			;	bit &80
	if	=			;	jr nz,4:
		XOR	&FF		;	xor &ff
	else				;       jr 5:
		AND	&7F		; 4:	and 7f
	endif				; 5:


	CLA				;	cla
	begin				; 6:
		STI	(BC)		;	sti (bc)
	until	DJC			;	djc 6:

	RET

	.end
