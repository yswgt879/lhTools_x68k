
	.CODE

	.STRUCT: sd
		b	byte
		w	word
		l	long
		t	byte,#16
	.ENDSTRUCT

	.DATA:	d2	STRUCT sd	INIT
		#8
		#16
		#32
		"ABCD" 00
	.ENDDATA

	.END

