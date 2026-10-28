	.CODE
	.STRUCT:        basfile_prev
                ptr     word,#2
        .ENDSTRUCT
FILENAMELEN:	.EQU	#11
        .STRUCT:        basfile_header
                filename        text,FILENAMELEN
                filetype        byte
                filelen         word
                fileprev        struct'basfile_prev
        .ENDSTRUCT

	.CODE

	LD	C,OFFSETOF'basfile_header.filetype
	LD	L,[-1]ARRAYOF'basfile_header.filename
HEADER_LEN	.EQU	SIZEOF'basfile_header
	ADC	HEADER_LEN
	NOP
        LDA     OFFSETOF'basfile_header.filename
        ADD     BC
        LD      L,SIZEOF'basfile_header.filename
        DEC     L
        CLA
loop:
        STI     (BC)
        DJC     loop

	BIND	BC,basfile_header		; BC points to the base of basfile_header
  	BIND	DE,basfile_header.filelen	; DE points to the field filelen
	LDA	OFFSETOF'DE
	LDI	(DE)
	STA	H
	LDI	(DE)				; Here DE points to fileprev
	STA	L
	; lhasm has updated DE to points to the next field: fileprev
	LDA	OFFSETOF'DE
	; If DE has now to points to filename, the offset should be substracted
	BIND	DE,basfile_header.filename
	LDA	OFFSETOF'BC
	; But lhasm will genrate all the code for me ;)
	BIND	BC,basfile_header.filetype	; BC points to the field filetype
	; So BC has now to points to filelen, the offset should be added
	LDA	OFFSETOF'BC
	BIND	HL,BC
	LDA	OFFSETOF'HL
	DEC	BC
	LDA	OFFSETOF'BC
	BIND	HL,basfile_header.fileprev.ptr
	LDA	OFFSETOF'HL
	UNBIND	BC
	UNBIND	DE
	UNBIND	HL
