	.ORIGIN:	40c5
	.CODE
        DOBEEP1 .EQU    e669
                LDA     10
        LOOP:
                PUSH    A
                CALL    DOBEEP1
                POP     A
                DEC     A
                JR      NZ,LOOP
                RET
	.END
