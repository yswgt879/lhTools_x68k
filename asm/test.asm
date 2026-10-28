.ORIGIN:	40C5
.CODE
	CPA	(ENDRAM)
	STA	(STARTRAM)
		LDA	(STARTRAM)
		CPA	(ENDRAM)

.TEXT
	"\sqr\pi\yen\ins \papa\nn"
.BASIC
        5 PRINT "\ins\\^\sqr\pipi\papa\yen\nnn"
	10 PRINT "Address=";A$
.CODE
	STA#	(F00A)
	SBR	(34),01,$A,4128,$B,4133
.IF	EXIST?	DOERR
	SBR	Z,(DE),__THIS__$$._start
	SBR	(D0),08,__THIS__$$._start
.ELSE
	SBR	Z,(DE),__THIS__$$._end
	SBR	(D0),08,__THIS__$$._end
.ENDIF
	SBR	(E3)
	INC	BC
	DEC	BC
	INC	HL
	DEC	DE
	LD	BC,SP
	PUSH	BC
	POP	BC

.TEXT
	"CHRISTOPHE\\ABC\x43\x47H\+80"
.END

