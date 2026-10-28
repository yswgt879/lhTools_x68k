
	.code

	begin				; 1:
		LDI	(BC)		;	ldi (bc)
	while	NZ			;	jr z,2:
		AND	&7F		;	and 7f
		STI	(DE)		;	sti (de)
	repeat				;	jr 1:
					; 2:

	NOP

	.code

	begin				; 1:
		LDI	(BC)		;	ldi (bc)

	.hole
	.align: 0200
	.code

	while	NZ			;	jr z,2:
		AND	&7F		;	and 7f
		STI	(DE)		;	sti (de)
	repeat				;	jr 1:
					; 2:

	NOP

	.code

	begin				; 1:
		LDI	(BC)		;	ldi (bc)
	while	NZ			;	jr z,2:
		AND	&7F		;	and 7f
		STI	(DE)		;	sti (de)

	.hole
	.align: 0200
	.code

	repeat				;	jr 1:
					; 2:

	NOP

	.code

	begin				; 1:
		LDI	(BC)		;	ldi (bc)

	.hole
	.align: 0200
	.code

	while	NZ			;	jr z,2:
		AND	&7F		;	and 7f
		STI	(DE)		;	sti (de)

	.hole
	.align: 0200
	.code

	repeat				;	jr 1:
					; 2:


	RET

	.end
