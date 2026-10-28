	.CODE
	.STRUCT:        basfile_prev
                ptr     word,#2
        .ENDSTRUCT
FILENAMELEN:	.EQU	#11
        .STRUCT:        basfile_header
                filename        text,FILENAMELEN
                filetype        byte
                filelen         word
                fileprev        struct'basfile_prev,#3
        .ENDSTRUCT

	.DATA: TopHeader STRUCT basfile_header,#3 INIT
		aa
	.ENDDATA

	.DATA: PtrHeader STRUCT basfile_header INIT
		"Noname" 00 00 00 00 00
		;01 02 03 04 05 06 07 08 09 0a 0b
		10 ; filetype
		ee dd
		;<TopHeader >TopHeader
		>ADDRESSOF'TopHeader:#1.fileprev.ptr <ADDRESSOF'TopHeader:#1.fileprev.ptr
		OPCODE'NOP
		;FF FF FF FF
	.ENDDATA

	.IF	STRUCT?	basfile_header.filename
	.PRINT2	"Length of filename " BCDOF'SIZEOF'basfile_header.filename
	.ENDIF
	.IF	STRUCT?	basfile_hr.flname
	.PRINT2	"Element of flname" ELEMENTOF'basfile_hr.flname
	.ENDIF

	.CODE

STARTUP:
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
        LDI     (DE)
        STA     H
        LDI     (DE)
        STA     L
	BIND	BC,basfile_header.filetype
        ; Here lhasm does "pass" DE on the next field fileprev.
        ; But to point backward to filename, it needs to substract...
        BIND    DE,basfile_header.filename
        ; And lhasm will generate the good code :)
	UNBIND	DE

	BIND	DE,BC
	UNBIND	DE

        LDA     OFFSETOF'PtrHeader.filetype
	LD	B,<ADDRESSOF'PtrHeader.filelen
	LD	C,>ADDRESSOF'PtrHeader.filelen

	BIND	DE,PtrHeader.fileprev
	BIND	DE,PtrHeader.filelen

	LD	H,<ADDRESSOF'TopHeader:#1.filetype
	LD	L,>ADDRESSOF'TopHeader:#1.filetype

	UNBIND	BC
	BIND	BC,TopHeader
	BIND	HL,DE
	UNBIND	BC
	UNBIND	DE

	BIND	BC,TopHeader:#2.filetype
	BIND	BC,TopHeader:#2.fileprev.ptr
	BIND	BC,TopHeader:#2.fileprev.ptr:#1
	BIND	BC,TopHeader:#1
	BIND	BC,TopHeader:#0
	BIND	BC,TopHeader.filename

	LDA	SIZEOF'PtrHeader
	LDA	ARRAYOF'TopHeader
	LDA	ELEMENTOF'TopHeader
	LDA	SIZEOF'TopHeader
	LDA	SIZEOF'PtrHeader.filelen
	LDA	ARRAYOF'TopHeader.filetype
	LDA	ELEMENTOF'TopHeader.fileprev.ptr
	LDA	SIZEOF'TopHeader.fileprev
	LDA	ELEMENTOF'TopHeader.fileprev
	LDA	ARRAYOF'TopHeader.fileprev
	LDA	SIZEOF'TopHeader:#1
	LDA	ELEMENTOF'TopHeader:#1
	LDA	ARRAYOF'TopHeader:#1

	LDA	OFFSETOF'basfile_header.fileprev:#2.ptr:#1
	LDA	OFFSETOF'TopHeader
	LDA	OFFSETOF'TopHeader.filetype
	LDA	OFFSETOF'TopHeader:#0
	LDA	OFFSETOF'TopHeader:#0.filetype
	LDA	OFFSETOF'TopHeader:#0.filelen
	LDA	OFFSETOF'TopHeader:#0.fileprev
	LDA	OFFSETOF'TopHeader:#0.fileprev:#1.ptr
	LDA	OFFSETOF'TopHeader:#1
	LDA	OFFSETOF'TopHeader:#1.filetype
	LDA	OFFSETOF'TopHeader:#1.filelen
	LDA	OFFSETOF'TopHeader:#1.fileprev
	LDA	OFFSETOF'TopHeader:#1.fileprev:#1.ptr
	LDA	OFFSETOF'TopHeader:#2
	LDA	OFFSETOF'TopHeader:#2.filetype
	LDA	OFFSETOF'TopHeader:#2.filelen
	LDA	OFFSETOF'TopHeader:#2.fileprev
	LDA	OFFSETOF'TopHeader:#2.fileprev:#1.ptr
	CPA	(ADDRESSOF'TopHeader:#0.filelen)
	CPA	(ADDRESSOF'TopHeader:#1.fileprev)
	CPA	(ADDRESSOF'TopHeader:#2.fileprev:#2.ptr)
	CPA	(ADDRESSOF'TopHeader:#2.fileprev:#0.ptr:#1)

	.IF	STRUCT?	basfile_header.fileprev:#2.ptr
	NOP
	.ENDIF
	.IF	STRUCT?	basfile_header
	RET
	.ENDIF
	.IF	STRUCT?	basfile_header.top
	OFF
	.ENDIF

	BIND	BC,TopHeader:#1			; BC points to the base of basfile_header
  	BIND	DE,TopHeader:#2.filelen		; DE points to the field filelen
	LDA	OFFSETOF'DE
	LDI	(DE)
	STA	H
	LDI	(DE)				; Here DE points to fileprev
	STA	L
	; lhasm has updated DE to points to the next field: fileprev
	LDA	OFFSETOF'DE
	; If DE has now to points to filename, the offset should be substracted
	BIND	DE,TopHeader:#2.filename
	LDA	OFFSETOF'BC
	; But lhasm will genrate all the code for me ;)
	BIND	BC,TopHeader:#1.filetype	; BC points to the field filetype
	; So BC has now to points to filelen, the offset should be added
	LDA	OFFSETOF'BC
	BIND	HL,BC
	LDA	OFFSETOF'HL
	DEC	BC
	LDA	OFFSETOF'BC
	BIND	HL,TopHeader:#1.fileprev.ptr
	LDA	OFFSETOF'HL
	UNBIND	BC
	UNBIND	DE
	UNBIND	HL
