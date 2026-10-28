
	.code

	begin				; 3:
		AND	(BC),&00	;	and (bc),&00
		INC	BC		;	inc bc
		CP	C,&FF		;	cp c,&ff
	until	H			;	jr nh,3:

	NOP

	.code

	begin				; 4:
		INC	BC		;	inc bc
		CP	C,&FF		;	cp c,&ff

	.hole
	.align: 0200
	.code

	until	NZ			;	jr nz,+03
					;	jp 4:

	NOP

	.code

	CLA				;	cla
	begin				; 5:
		STI	(BC)		;	sti (bc)
	until	DJC			;	djc 5:

	NOP

	.code

	CLA				;	cla
	begin				; 6:
		STI	(BC)		;	sti (bc)

	.hole
	.align: 0200
	.code

	until	DJC			;	jr nc,+03
					;	jp 6:

	RET

	.end
