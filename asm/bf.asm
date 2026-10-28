
	.STRUCT:	basfile3
		c31	byte
		c32	byte
	.ENDSTRUCT
	.STRUCT:	basfile2
		ptr	word,#2
		c3	struct'basfile3,#6
	.ENDSTRUCT
	.STRUCT:	basfile_header
		filename	byte,#11
		filetype	byte
		filelen		word
		fileprev	struct'basfile2
	.ENDSTRUCT

	.CODE
	LDA	SIZEOF'basfile_header
	LD	B,TYPEOF'basfile_header.filelen
	LD	C,ARRAYOF'basfile_header.filename
	ADC	OFFSETOF'basfile_header.fileprev
	SBC	SIZEOF'basfile_header.fileprev.ptr
	CPA	TYPEOF'basfile_header.fileprev.c3.c32
	CP	L,SIZEOF'basfile_header.fileprev.c3
	CP	H,ARRAYOF'basfile_header.fileprev.c3
	LD	D,ELEMENTOF'basfile_header.fileprev.c3
	LD	E,ELEMENTOF'basfile_header.fileprev
	LDA	OFFSETOF'basfile_header.fileprev.c3
	LDA	ELEMENTOF'basfile_header.fileprev.c3
	LDA	ARRAYOF'basfile_header.fileprev.c3
	LDA	SIZEOF'basfile_header.fileprev.c3
	LDA	TYPEOF'basfile_header.fileprev.c3
	LDA	OFFSETOF'basfile_header.filename
	LDA	ELEMENTOF'basfile_header.filename
	LDA	ARRAYOF'basfile_header.filename
	LDA	SIZEOF'basfile_header.filename
	LDA	TYPEOF'basfile_header.filename
	LDA	OFFSETOF'basfile_header
	LDA	ELEMENTOF'basfile_header
	LDA	ARRAYOF'basfile_header
	LDA	SIZEOF'basfile_header
	LDA	TYPEOF'basfile_header
	

	LD	L,10
	BIND	HL,basfile_header
	STA	L
	LD	BC,HL
	CPA	SIZEOF'HL
	UNBIND	HL
	BIND	BC,basfile_header.filetype
	BIND	DE,BC
	DEC	BC
	INC	DE
	BIND	BC,basfile_header.filelen
	CPA	OFFSETOF'BC
	BIND	BC,basfile_header.fileprev.c3.c31
	INC	BC
	CPA	OFFSETOF'BC
	LD	BC,HL
	LD	D,77
	ADD	BC
	PUSH	DE
	POP	DE
	LD	HL,BC
	BIND	BC,basfile_header.fileprev
	INC	BC
	;BIND	BC,basfile3.c32
	BIND	BC,basfile_header
	LDA	ELEMENTOF'DE
	BIND	BC,basfile_header.fileprev.c3
	CPA	SIZEOF'BC
	BIND	DE,basfile_header.fileprev.ptr
	UNBIND	BC
	UNBIND	DE

	;; .USE HL FOR basfile_header AT filename
	;; .ASSIGN DE,(HL)
	;;	LDI (HL)
	;;	STA D
	;;	LDD (HL)
	;;	STA E
	;; .ASSIGN (HL),C
	;;	LDA C
	;;	STA (HL)
	;; .ASSIGN HL.filelen
        ;;      LDA offsetof'basfile_header.filelen
	;;	ADD HL
