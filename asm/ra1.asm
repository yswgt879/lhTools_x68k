	;; Standard origin for all PC-1500
	;; without module
	.ORIGIN: 40C5
	
	;; Assembly code
	.CODE

	;; This load accumulator with 0
	CLA

	;; Copy 0 to H and L
	STA	H
	STA	L

	;; Store 0 into the address pointed
	;; by HL and increment HL
	STI	(HL)

	;; Until H greater or equal to &48
	CP	H,&48
	JR	NC,-05	;; five bytes back

	;; Finish. Back to BASIC
	RET

	.END

