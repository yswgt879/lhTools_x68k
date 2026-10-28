# lhTools for X68000 (Human68k)

This is a customized port of lhTools, modified specifically for the Sharp X68000 (Human68k) using the elf2x68k toolchain. 

## X68000 Port Highlights (by Kenoh, 2026)
- Target OS: Sharp X68000 (Human68k).
- Output Executables: lhasm.x, lhdump.x, lhpoke.x, and lhcom.x (Stubbed).
- Privacy: All absolute compiler build paths have been stripped.
- Compiled via elf2x68k (m68k-xelf-gcc and elf2x68k.py).

## Build Instructions
To rebuild all clean utilities from source:
make clean
make

=====================================================================
lhTools is a tool box for desasembling and decompiling the programs BASIC or ML of
the SHARP PC-1500/A and TANDY PC-2.

The lhTools is Copyright 1992-2018 Christophe Gottheimer <cgh750215@gmail.com>

This version is still in pre-alpha release. It is not fully mature and bugs
are present.

           +------------------- DISCLAIMER --------------------+
           | YOU USE THIS CODE AT YOUR OWN RISK ! I AM NOT     |
           | RESPONSIBLE FOR ANY DAMMAGE OR ANY DATA LOST OR   |
           | CORRUPTED BY USING THIS SOFTWARE OR BY USING THE  |
           | BINARY IMAGES CREATED WITH THIS SOFTWARE WHILE    |
           | RUNNING THEM ON A SHARP PC-1500/A or TANDY PC-2.  |
           | BE SURE TO SAVE YOUR IMPORTANT DATA OR PROGRAMS   |
           | BEFORE LOADING AND RUNNING THE BINARY IMAGES.     |
           +---------------------------------------------------+


It is composed of 4 programs:
	lhasm  : To produce a binary image from BASIC assembler, or hexadecimal
		 dump sources.
	lhdump : To decompile, desasemble and decode the binaries images.
	lhcom  : To send or receive binary data or programs with the CE-158
		 serial interface.
	lhpoke : To convert a binary file into a BASIC program using POKE.

Please see the documentation lhTools.pdf inside the zip archive for a complete
documentation of the lhTools.


                 *=======================================*
                 *     THIS README IS NOW DEPRECATED!    *
                 * THE WHOLE DOCUMENTATION UP TO DATE IS *
                 *        IN THE FILE lhTools.pdf        *
                 *=======================================*


1/ lhasm
   =====

Usage: lhasm [-h] [-v] [-d|-ddebug] [-dverbose]] [-i] [-E[E]] [-W]
		[-A argument=substitute...] [-D symbol=value...] [-a] [-na]
		[-T|-L logfile ] [-nc] [-ns] [-r resymfil] [-s resrcfile]
		[-F fragfile] [-K[K][E] keywordfile] [-M macfile] [-S symfile]
		[-m machine] [-O origin] [fragment] [-I includepath]
		[-Z[header=type]] [-Z[name=headername]] [-Z[entry=startup]]
		[-J[loop=n]] [-1] [-N|-no] [-x] [-o outfile] srcfile
where:
	-1		Do assembler pass 1 only and stop
	-a		All symbols treated as local except if .EXPORT: is specified
	-d|-debug	Show debug information
	-dverbose	Enable verbose mode
	-h		This help
	-i		Immediate one pass only assembler. Read from stdin
			Incompatible with -J
	-m machine	Select the machine and modules
	   -m pc1500	RAM=&4000..&47FF ROM=&8000..&FFFF; This is the default
	   -m pc1500A	RAM=&4000..&57FF ROM=&8000..&FFFF LM=&7C01..7FFFF
	   -m pta4000+16	RAM=&0000..&47FF ROM=&8000..&FFFF
	   -m pc1560	RAM=&0000..&8000..&FFFF
	   -m ce151	Add &1000 at end of RAM; This is CE-151
	   -m ce155	Add &800 before RAM and &1800 at end of RAM; This is CE-155
	   -m ce159	Add &1000 before RAM; This is CE-159
	   -m ce161	Add &4000 before RAM; This is CE-161
	   -m ce163	Add &4000 before RAM; This is CE-163, bank1 is not supported
	   Only one -m ce... option may be given
	-na		Disable local symbols list into log file (with -T or -L)
	-nc		Disable comment copy into log file (with -T or -L)
	-ns		Disable symbols/variables list into log file (with -T or -L)
	-o outfile	Output binary code into outfile (.bin)
	-r resymfile	Input file of addresses to be re-symbol'ed
	-s srcfile	Output file (.asm) of the full source with re-symbols
	-v		Show version and exit
	-x		Output hexadecimal dump instead of binary into outfile (.hex) 
	-A argument=substitute	Replace argument by substitute when defining a symbol
			Several -A may be given if several symbols need to be defined
	-D symbol=value	Define the specified symbol to the given value
			Several -D may be given if several symbols need to be defined
	-E		Warnings treated as errors
	-EE		Errors are fatal
	-F fragfile	Output fragments to fragfile (.frag)
	-I includepath	Add the <includepath> to the directories list where to search
			the files to include
	-J		Replace JR by JP if displacement > 255. Incompatible with -i
	-Jloop=n	Like -J, but run only n optimization loops
	-K keywordfile	Output the BASIC keywords table to keywordfile (.keyw)
	-KK keywordfile	Output the BASIC keywords table to keywordfile (.keyw)
			with old format < 0.6.0
	-K[K]E keywordfile	Export declared BASIC keywords to keywordfile (.keyw)
			(with old format < 0.6.0 if option is -KKE)
	-L logfile	Output logs of the assembler processing into logfile (.log)
			This option is exclusive with the -T option
	-M macfile	Output defined macros to macfile (.mac)
	-N|-no		Do not output generated binary code
	-O origin	Set origin address to specified value
	-S symfile	Output declared symbols to symfile (.sym)
	-T		Enable trace mode output to stderr.
			This is exclusive with the -L log option
	-W		Enable low priority warnings
	fragment	Set original fragment
	   -B	BASIC fragment; This is the default
	   -R	RESERVE fragment
	   -X	XREG fragment
	   -V	dynamic VARiables fragment
	   -c	CODE fragment
	   -b	BYTE (8-bits) fragment
	   -w	WORD (16-bits) fragment
	   -l	LONG (32-bits) fragment
	   -t	TEXT fragment
	   -k	KEYWORD fragment
	   -H	HOLE fragment
	-Z		Add a CE158 CSAVE header
	-Zname=name	Set <name> as CSAVE name
	-Zentry=entry	Set <entry> as CSAVE startup routine
	-Zheader=type	Set <type> for CSAVE header
		with <type>: CSAVE CSAVEM CSAVEr or PRINT
note:
	If srcfile has a .bas extension, the BASIC mode is assumed.
	If srcfile has a .asm, .as or .s extension, the CODE mode is assumed.
	If srcfile has a .hex or .x extension, the BYTE mode is assumed.
	If the -o outfile argument is not specified, <srcfile>.bin is used for output

  1.1/ Mnemonics
       ---------

  The following mnemonics are understood by lhasm:
	ADC[#]	(R)
	ADC	rl
	ADC	rh
	ADC[#]	(mn)
	ADC	n
	ADD	R
	ADD[#]	(R),n
	ADD[#]	(mn),n
	AND[#]	(R),n
	AND[#]	(R)
	AND[#]	(mn),n
	AND[#]	(mn)
	AND	n
	AEX
	ATP
	AM0
	AM1
	BIT[#]	(R),n
	BIT[#]	(R)
	BIT[#]	(mn),n
	BIT[#]	(mn)
	BIT	n
	CALL	mn
	CDV
	CLA
	CPA[#]	(R)
	CPA	rl
	CPA	rh
	CPA[#]	(mn)
	CPA	n
	CPI
	CP	rl,n
	CP	rh,n
	DADC[#]	(R)
	DEC	A
	DEC	rl
	DEC	rh
	DEC	R
	DI
	DJC	d
	DSBC[#]	(R)
	EI
	HALT
	INC	A
	INC	rl
	INC	rh
	INC	R
	INA
	JR	cc,d
	JR	d
	JP	mn
	LDA[#]	(R)
	LDA	rl
	LDA	rh
	LDA	F
	LDA[#]	(mn)
	LDA	n
	LDD[#]	(R)
	LDI[#]	(R)
	LDI
	LD	rl,n
	LD	rh,n
	LD	BC,R
	LD	BC,PC
	LD	BC,SP
	LD	R,BC
	LD	SP,BC
	LD	PC,BC
	LD	SP,mn
	NOP
	OR[#]	(R),n
	OR[#]	(R)
	OR[#]	(mn),n
	OR[#]	(mn)
	OR	n
	OFF
	POP	A
	POP	R
	PUSH	A
	PUSH	R
	RCF
	RDP
	RET
	RL
	RR
	RLD
	RRD
	RPU
	RPV
	RTI
	SBC[#]	(R)
	SBC	rl
	SBC	rh
	SBC[#]	(mn)
	SBC	n
	SBR	(n)
	SBR	cc,(n)
	SCF
	SDP
	SL
	SR
	SPU
	SPV
	STA[#]	(R)
	STA	rl
	STA	rh
	STA	F
	STA[#]	(mn)
	STD[#]	(R)
	STI[#]	(R)
	XOR[#]	(R)
	XOR[#]	(mn)
	XOR	n

  These mnemonics are aliases and provided as a standalone instruction to
  help in coding.
	LDW		:= SBR (&C0)
	LJNE	k,d	:= SBR (&C2)
	JNE	k,d	:= SBR (&C4)
	BKW		:= SBR (&C6)
	LJNES	d	:= SBR (&C8)
	STS	(n)	:= SBR (&CA)
	LDS	(n)	:= SBR (&CC)
	VAR	n,d	:= SBR (&CE)
	INTG	n,d	:= SBR (&D0)
	ARG	d,n	:= SBR (&D2)
	STB	n	:= SBR (&D4)
	LDB	n	:= SBR (&D6)
	IFC		:= SBR (&D8)
	STVP		:= SBR (&DA)
	LDPT		:= SBR (&DC)
	EVAL	d	:= SBR (&DE)
	ERRH		:= SBR (&E0)
	RST		:= SBR (&E2)
	ERR1		:= SBR (&E4)
	LYX		:= SBR (&E6)
	NORM		:= SBR (&E8)
	SLX		:= SBR (&EA)
	CLX		:= SBR (&EC)
	ADN		:= SBR (&EE)
	SXY		:= SBR (&F0)
	CLS		:= SBR (&F2)
	LDU	(mn)	:= SBR (&F4)
	STU	(mn)	:= SBR (&F6)

  These instructions are aliases and are provided for backward compatibility:
	OUTA		:= ATP
	RSET		:= OFF
	STA	T0	:= AM0
	STA	T1	:= AM1
	STI		:= LDI
	SWA		:= AEX
	SWP		:= AEX
	SLD		:= RLD
	SRD		:= RRD

Convention for the mnemonics describe above:
	n    := Byte 8-bits value, within 0..255 (&FF)
	mn   := Word 16-bits value, within 0..65535 (&FF)
	(n)  := Indirect 8-bits value, within 0..255 (&FF)
	(mn) := Indirect 16-bits value, within 0..65535 (&FFFF)
	cc   := Condition: C, NC, V, NV, Z, NZ, V, NV, ==, !=, <, >=
	d    := 8-bits displacement, within 0..255
	rh   := High 8-bits register: B, D, H, M
	rl   := Lowe 8-bits register: C, E, L, N
	R    := Whole 16-bits register: BC, DE, HL, MN
	(R)  := Indirect whole 16-bits register: (BC), (DE), (HL), (MN)
	A    := Accumulator
	F    := Flags (status)
	PC   := Program counter
	SP   := Stack pointer
	T0   := Timer with 9th-bit to 0
	T1   := Timer with 9th-bit to 1
	[#]  := Optional second page access
	k    := BASIC keyword code if k >= &E000 else a 8-bit value is assumed

  1.2/ Base and character specifiers
       -----------------------------

  When specifying an immediate value, the following specifiers are understood:
	n    := Hexadecimal 8-bits value (2-digits) within 00..FF
	&n   := Hexadecimal 8-bits value (1 to 2-digits) within &00..&FF
	#n   := Decimal 8-bits value (1 to 3-digits) within #0..#255
	@n   := Octal 8-bits value (1 to 3-digits) within @0..@377
	\xn  := Hexadecimal 8-bits value (1 to 2-digits) within &00..&FF
	\un  := Decimal 8-bits value (1 to 3-digits) within #0..#255
	\on  := Octal 8-bits value (1 to 3-digits) within @0..@377
	$c   := Character ASCII code of c
	mn   := Hexadecimal 16-bits value (2-digits) within 0000..FFFF
	&mn  := Hexadecimal 16-bits value (1 to 2-digits) within &0000..&FFFF
	#mn  := Decimal 16-bits value (1 to 3-digits) within #0..#65535
	@mn  := Octal 16-bits value (1 to 3-digits) within @0..@177777
	\Xmn := Hexadecimal 16-bits value (1 to 2-digits) within &0000..&FFFF
	\Umn := Decimal 16-bits value (1 to 3-digits) within #0..#65535
	\Omn := Octal 16-bits value (1 to 3-digits) within @0..@177777

  1.3/ Unary operators
       ---------------

  When specifying an immediate value, i.e, d, n, or mn, unary operator may be
  added as follow:
	+n    := Positive displacement, PC+d
	-n    := Negative displacement, PC-d
	*mn   := Offset between current and mn
	*_*mn := Offset between address of the instruction and mn
	*_*mn[pq] := Offset between pq and and mn
	<mn   := High 8-bit from the mn value, ie, m
	>mn   := High 8-bit from the mn value, ie, n
	{mn   := Shift 1 bit left
	}mn   := Shift 1 bit right
	!mn   := &FFFF XOR'ed 16-bits or 8-bits value
	^n    := Set bit to '1 if n (1..16) or 0 (if n = 0)
	'mn   := First bit to '1 in mn starting from left
	/mn   := First bit to '1 in mn starting from right
	~mn   := Swap byte m and n for 16-bits value
	~n    := Swap digits nH and nL for 8-bits value
	[opval]mn := Compute mn op val with the following op:
		+ := addition
		- := subtraction
		* := multiplication
		/ := division
		& := logical AND
		| ;= logical OR
		^ := logical XOR
		= := Align on val bits frontier
		< := Shift left val bits
		> := Shift right val bits

  When specifying a register, unitary operator may be added as follow:
	#<rR := High 8-bits register from rR, ie, B, D, H, M
	#>rR := Low 8-bits register from rR, ie, C, E, L, N
	#^rR := Whole 16-bits register from rR, ie, BC, DE, HL, MN
	#*rR := Indirect 16-bits register from rR, ie, (BC), (DE), (HL), (MN)
	rR is any register: rh, rl, R, (R)

  When specifying a condition, unitary operator may be added as follow:
	#!cc := Return the inverse condition as shown below:
		!cc  := Ncc
		!Ncc := cc
		#!!= := ==
		#!== := !=
		#!>= := <
		#!<  := >=

  1.4/ Symbols and variables
       ---------------------

  A symbol is global to the file and may be used at any time. The symbols defined
  in a source code may be savec into a .sym file by using -S <symfile> option.

  A symbol is declared by setting its name followed by : (colon). Note that no
  instruction is allowed after a symbol declaration.
  With this, the immediate PC value is affected to the symbol.
  To define a specific value to a symbol, use .EQU <value>.
  The name of a symbol should not start with . \ or % because these charactersr
  are reserved for special usage.

  Examples:
	.ORIGIN:	40c5
	.CODE
	DOBEEP1	.EQU	e669
		LDA	10
	LOOP:
		PUSH	A
		CALL	DOBEEP1
		POP	A
		DEC	A
		JR	NZ,LOOP
		RET
	.END

  Running lhasm will give:
    1
		.ORIGIN:	40C5
    2		.CODE	40C5
    3	40C5	DOBEEP1:	.EQU E669
    4	40C5	B5 10          	LDA	10 
    5	40C7	LOOP:	.EQU 40C7
    6	40C7	FD C8          	PUSH	A 
    7	40C9	BE E6 69       	CALL	DOBEEP1 
    8	40CC	FD 8A          	POP	A 
    9	40CE	DF             	DEC	A 
   10	40CF	99 0A          	JR	NZ LOOP 
   11	40D1	9A             	RET	

  A variable is like a symbol but it is local to a source and variables
  are not saved by the -S <symfile> options.
  A variable name starts with % and has the form %nnc where nn is a 2
  digits number from 00 to 99 and c is lowercase character from a to z.
  A maximum of 2600 variables may be declared.

  Example:
	.ORIGIN:	40C5
	.CODE
	%10a	.EQU	10
		LD	L,%10a
	%01l:
		AND	(BC),00
		INC	BC
		DJC	%01l
		RET
	.END

  Running lhasm will give:
    1
		.ORIGIN:	40C5
    2		.CODE	40C5
    3	40C5	%10a	.EQU 0010
    4	40C5	6A 10          	LD	L %10a 
    5	40C7	%01l	.EQU 40C7
    6	40C7	49 00          	AND	(BC) 00 
    7	40C9	44             	INC	BC 
    8	40CA	88 05          	DJC	%01l 
    9	40CC	9A             	RET	


  1.5/ Using macro
       -----------

  A macro is code to be developped each time it is found in the source.
  Imagine we want to have an instruction as JR > which does not exits.
  Just create a macro called JR> and when the assembler will find
	JR> label
  it will expand this code. The parameter label will be passed to the
  code and substituted according to the macro rules.

  A macro is defined by the directive:
	.MACRO: <name>
  followed by any code, with the eventual substitution marker and is
  terminated by
	.ENDMACRO
  The subtitution marker are on the form __#n where n is within 0..9
  When the macro is found in the code, the first parameter after the
  name is __#0, the second __#1, and so on, until the 10th and last
  parameter __#9.

  An example with the macro JR>
	.MACRO:	JR>
		JR	==,+02	; If Z values are equal, test is false
		JR	>=,__#0	; If C, the test is true 
	.ENDMACRO

  An the following source:
                LDA     10
                LD      B,09
                JR>     gt
                RET             ; test false
        gt:                     ; Greater than

  Will give:
  196	01FF	B5 10          	LDA	10 
  197	0201	48 09          	LD	B 09 
  198			JR>	gt
  198				{
  198	0203	8B 02          	JR	== +02 ; If Z values are equal test is false 
  198	0205	83 01          	JR	>= gt ; If C the test is true 
  198				}
  199	0207	9A             	RET	; test false 
  200	0208	gt:	.EQU 0208

  By mixing the unary operators and substitution marker, some powerfull
  macro may be defined:

	.MACRO:	LDR
		LD	#<__#0,<__#1	; rH register loaded with high 8-bits
		LD	#>__#0,>__#1	; rL register loaded with low 8-bits
	.ENDMACRO
  The macro LDR is no define and will expand code to load the 16-bits value into
  a whole 16-bits register.
  Now, write
	LDR	HL,8899
  And see:
    6			LDR	HL 8899
    6				{
    6	40C5	68 88          	LD	#<__#0 <__#1 ; rH register loaded with high 8-bits 
    6	40C7	6A 99          	LD	#>__#0 >__#1 ; rL register loaded with low 8-bits 
    6				}

  When developping complex macros, it is also necessary to have some labels for jumps
  or addresses related into the macro. Because the macros are reentrant, the labels
  should be available only inside the macro. To do this the 10 labels 0: 1: .. 9:
  are available inside a macro. Note that the label x: should NOT be followed by an
  instruction.
  The macro XFER will do a copy in reverse from BC to DE until L is not &FF, but stops
  if the bit 7 (&80 := ^80 ) is set.
	.MACRO:	XFER
		LD	B <__#0
		LD	C >__#0
		LD	D <__#1
		LD	E >__#1
		LD	L >__#2
	1:
		LDI	(BC)	; Load A with (BC) and increment BC
		STD	(DE)	; Store A to (DE) and decrement DE
		BIT	^08	; Bit 7 of A is set
		JR	C,2:	; Yes ! XFER is finished
		DJC	1:	; Decrement L and jump to 1: if not C
	2:
		RET
	.ENDMACRO
  And to transfer the BASIC A$ variable to &47FF, do
	XFER	A$ 47FF \u15

  And see:
   21			XFER	A$ 47FF \u15
   21				{
   21	40C5	48 78          	LD	B <__#0 
   21	40C7	4A C0          	LD	C >__#0 
   21	40C9	58 47          	LD	D <__#1 
   21	40CB	5A FF          	LD	E >__#1 
   21	40CD	6A 0F          	LD	L >__#2 
   21	40CF	1:
	;	#XFER__00__#1	.EQU 40CF
   21	40CF	45             	LDI	(BC) ; Load A with (BC) and increment BC 
   21	40D0	53             	STD	(DE) ; Store A to (DE) and decrement DE 
   21	40D1	BF 80          	BIT	^08 ; Bit 7 of A is set 
   21	40D3	93 02          	JR	C 2: ; Yes ! XFER is finished 
   21	40D5	88 08          	DJC	1: ; Decrement L and jump to 1: if not C 
   21	40D7	2:
	;	#XFER__00__#2	.EQU 40D7
   21	40D7	9A             	RET	
   21				}

  1.6/ Assembly inlining in BASIC program
       ----------------------------------

  To simplify to introduction of assembly code inside BASIC instructions like REM,
  POKE and DATA or when assigning a $ varible, it is now possible to call the assembler
  while a BASIC fragment is active.
  The syntax is the following
	<basic line num> ...<inst>:...<inst> \asm[
		assembly code, with symbols, variables and macros
	\]end <inst>...
  Note the \asm[ should be at the end of the source line and \]end at the begining of
  a source line followed by a space.

  A small example below:
			;.CODE
			.MACRO:	LDBC_nn
			LD	B,<__#0
			LD	C,>__#0
			.ENDMACRO
			;.BASIC
	10 REM	\asm[
		%80h	.EQU	^08
			LDA	00
			LDBC_nn	7750
			LD	L,%80h
		loop:
			STI	(BC)
			DJC	loop
			RET
		\]end
	20 POKE A, \asm[
			SBR	(F2)
			CALL	&ED00
			RET
		\]end
	30 E$="\asm[
			LDBC_nn	str
			RET
		str:	.EQU	.
			\$A \$B \$C
		\]end EFGH"
	40 DATA	\asm[
		PUSH	HL
		PUSH	BC
		CALL	BEEP1
		POP	BC
		POP	HL
		RET
		\]end
	50 END

  Running lhasm on this source will give:
    2			.MACRO:	LDBC_nn
    2				{
    3	; LDBC_nn: 1		LD	B,<__#0
    4	; LDBC_nn: 2		LD	C,>__#0
    5				}
    5			.ENDMACRO	; LDBC_nn
    7	40CA	               	10	REM	\asm[ 
    8	40CA	%80h	.EQU 0080
    9	40CC	               	LDA	00 
   10			LDBC_nn	7750
   10				{
   10	40CE	               	LD	B <__#0 
   10	40D0	               	LD	C >__#0 
   10				}
   11	40D2	               	LD	L %80h 
   12	40D2	loop:	.EQU 40D2
   13	40D3	               	STI	(BC) 
   14	40D5	               	DJC	loop 
   15	40D6	               	RET	
   16	40C5	00 0A 0F F1 AB 	\]end	
		B5 00 48 77 4A 
		50 6A 80 41 88 
		03 9A 0D 
   17	40DE	               	20	POKE A, \asm[ 
   18	40E5	               	SBR	(F2) 
   19	40F1	               	CALL	&ED00 
   20	40F5	               	RET	
   21	40D7	00 14 1C F1 A1 	\]end	
		41 2C 26 43 44 
		2C 26 46 32 2C 
		26 42 45 2C 26 
		45 44 2C 26 30 
		30 2C 26 39 41 
		0D 
   22	40FD	               	30	E$="\asm[ 
   23			LDBC_nn	str
   23				{
   23	40FF	               	LD	B <__#0 
   23	4101	               	LD	C >__#0 
   23				}
   24	4102	               	RET	
   25	4102	str:	.EQU 4102
   26	4105	               	\$A	\$B \$C 
   27	40F6	00 1E 12 45 24 	\]end	EFGH" 
		3D 22 48 41 4A 
		02 9A 41 42 43 
		45 46 47 48 22 
		0D 
   28	4110	               	40	DATA	\asm[ 
   29	4117	               	PUSH	HL 
   30	411F	               	PUSH	BC 
   31	412B	               	CALL	BEEP1 
   32	4133	               	POP	BC 
   33	413B	               	POP	HL 
   34	413F	               	RET	
   35	410B	00 28 32 F1 8D 	\]end	
		26 46 44 2C 26 
		41 38 2C 26 46 
		44 2C 26 38 38 
		2C 26 42 45 2C 
		26 45 36 2C 26 
		36 39 2C 26 46 
		44 2C 26 30 41 
		2C 26 46 44 2C 
		26 32 41 2C 26 
		39 41 0D 
   36	4140	00 32 03 F1 8E 	50	END 
		0D 
	4146	FF            	[END BASIC MARKER]

  and the following BASIC program:

	10 REM \B5\00HwJPj\80A\88\03\9A
	20 POKE A,&CD,&F2,&BE,&ED,&00,&9A
	30 E$="HAJ\02\9AABCEFGH"
	40 DATA &FD,&A8,&FD,&88,&BE,&E6,&69,&FD,&0A,&FD,&2A,&9A
	50 END 

  1.7/ Assembler directives
       --------------------

  .ORIGIN: <base addr>
       Set <base addr> as new origin address.
  .END
       End the assembler and upate pointers for saving binary file. If -ns is
       not specified and -T or -L logfile are given, the symbols and variables
       defined are listed after a .SYMBOLS: banner. If -ns is set, the symbols
       and variables are not listed.
  .COMMENT: <comment>
       Set a comment to the current fragment.
  .BASIC
       Enter into BASIC fragment. BASIC lines are compiled. A BASIC line
       start wih a number 1..65529 followed by BASIC keyword or expression.
  .CODE
       Enter into CODE fragment. LH5801 mnemonics are assembled.
  .BYTE
       Enter into BYTE fragment. Bytes 8-bits values are compiled.
  .WORD
       Enter into WORD fragment. Words 16-bits values are compiled.
  .LONG
       Enter into LONG fragment. Longs 32-bits values are compiled.
  .TEXT
       Enter into TEXT fragment. Text between " are compiled.
  .HOLE
       Enter into HOLE fragment. Obscure area. Only .SKIP <nn> is expected
       to skip <nn> bytes.
  .KEYWORD
       Enter KEYWORD fragment. The BASIC keyword table is built. The word
       pointers area is updated. Note that .KEYWORD is expected to be specified
       on a 2048bytes frontier + &54, i.e, &0054, &0854, &1054, etc...
  .DEFINE: "<keyword>" = <code>
       Define <keyword> with the <code> as a new BASIC keyword. The entry point
       is fixed to the current PC address.
  .CHECKSUM [[+](<code>)]
       Perform a checksum computation and write checksum value as a 16-bits
       word at the current address. The checksum is computed from the first
       .ORIGIN: and up to the current address.
       If (<code>) is given. The checksum will be stored after putting <code>.
       If + is given before (<code>), the <code> will be added to the checksum
       computed.
  .MACRO: <name>
       Define a new macro <name>. All code given is assumed to be part of the
       macro until .ENDMACRO is encountered.
  .ENDMACRO
       End the current macro.
  .INCLUDE: <file>
       Include the file <file>. If the file is already included nothing is done.
  .CHECKSUM:
  .FULLCHECKSUM:
  .DECLARE:
  .SYMBOLS:
       Dummy directives handled for backward compatibilty with lhdump.

  1.8/ Immediate assembler

  The standard assembler has two-passes. But it is also possible to generate code
  immediatly by calling the immediate assembler, ie, one-pass onlyi with the option
  -i. In this case, the source code is read from stdin and if the trace mode is
  redirected to stderr (option -T), the immediate code and informations are printed.
  To exit from immediate assembler, use CTRL+D. Exiting by CTRL+C will not write a
  binary file, and the generation of symbols, fragments and macros files may be
  disturbed by CTRL+C.
  If no -o <binfile> option is given, stdin.bin is used as output binary file.
  Note that when running with immediate assembler, variables and symbols should be
  defined to correct value BEFORE assembling, else an error may generated due to
  to bad value or undefined. But macro definition and expansion are usable with the
  immediate assembler

  An exemple:
	./lhasm -T -i -O 40C5 -c
	.!
    1	40C5	.CODE
	CLA
    2	40C5	34             	CLA	
	LD B,79
    3	40C6	48 79          	LD	B 79 
	LD C,00
    4	40C8	4A 00          	LD	C 00 
	loop:
    5	40CA	loop:	.EQU 40CA
	STI
    6	40CA	F5             	STI	
	CP C,C0
    7	40CB	4E C0          	CP	C C0 
	JR NC loop
    8	40CD	91 05          	JR	NC loop 
	RET
    9	40CF	9A             	RET	
	.END
<use CTRL+D to exit from immediate assembler>
Written 11 bytes (40C5:40D0) to stdin.bin

  When started in immediate mode, the assembler accepts the directives below:
  .!
       Displays the current fragment and address.
  .NOERROR
       Set the -W option. The errors are processed as warnings.
  .NOOUTPUT
       Set the -N option. No binary output will be done.


2/ lhdump
   ======

Usage: lhdump [-h] [-v] [{-s [-inline]|-d}] [-a] [-g]
		[-D:<dis>] [-C[=[start:]addr]] [-Z]
		[-F infile] [-K infile] [-S infile] [-O addr] [fragment, ...]
where:
	-a		BYTE fragments are printed in HEX and ASCII
	-d		Produce listing file; This is the default
	-g		Use graphical character for &27 &5B &5C &5D and &7F
	-h		This help
	-o outfile	Write dump or source to outfile, else use stdout
	-s		Produce source file; exclusive with -d
	-inline		Produce \asm[ .. \]end directive if -s is active
	-v		Show version and exit
	-C		Compute full CHECKSUM
	-C=addr		Compute CHECKSUM to addr-1 and compare to addr
	-C=start:addr	Compute CHECKSUM from start to addr-1 and compare to addr
	-D:<inst>	In BASIC fragment <inst> are disassembled
		where <inst> is DATA, POKE, REM, VAR
		REM and VAR are disassembled only when code is found
	-F infile	Read fragment description from infile
	-K infile	Read keyword from infile
	-O addr		Origin address, else start at 0000
	-S infile	Read symbols from infile
	-Z		Expect and use a CE158 header if valid
with fragment:
	-B [addr]	BASIC fragment; This is the default
	-R [addr]	RESERVE fragment
	-X [addr]	XREG fragment
	-V [addr]	dynamic VARiables fragment
	-c [addr]	CODE fragment
	-b [addr]	BYTE (8-bits) fragment
	-w [addr]	WORD (16-bits) fragment
	-l [addr]	LONG (32-bits) fragment
	-t [addr]	TEXT fragment
	-k [addr]	KEYWORD fragment
	-H [addr]	HOLE fragment

lhdump is the full dumper, decoder, decompiler and disassembler. It works from
a binary image (created by lhasm) and prints the dumped source according to the
options.
	-B : A BASIC image is expected. So the BASIC decompiler is called
	-c : A ML image is expected, the LH5801 disassembler is called
	-X, -V, -R : XREGS, dynamic VARiables, RESERVE image is expected. So the
		decoder is called.
	-b, -w, -l, -t : A data image is expected. So the disassembler is called.
	-k : A BASIC keyword table image is expected.
	-H : A HOLE, i.e. an obscure area for stack, or volatile data. This area
                will be skip by the dumper.

When a BASIC image contains some ML code inside, in REM lines, variables or DATA,
the -D:<inst> may be specified. Depending of the processed BASIC instruction, the
LH5801 disassembler is called.

	-C or -C=aaaa : Computes and prints the code checksum. When -C=aaaa is
		given, the computed checksum and this stored into the ML code at
		the address aaaa is compared.
	-K file : Read the BASIC keyword file to produce the BASIC decompiled
		source. This is useful to decompile BASIC programs written with
		some BASIC extensions.
	-F file : Read the FRAGMENT description from the given file. The let a
		mixed segment of code, data, BASIC, ... in the same binary image.


3/ lhcom
   =====

Usage: ./lhcom [-h] [-v] [-d|-ddebug] [-dverbose]] [-m interface]
		[-Y {[line][=speed,size,parity,stopb]}]
		[-Z[header=type]] [-Z[start=addr]] [-Z[name=headername]] [-Z[entry=addr]]
		[-S symfile] {-r|-s} binfile
where:
	-d|-debug	Show debug information
	-dverbose	Enable verbose mode
	-h		This help
	-m interface	Select the interface type
	   -m ce158	Use the CE158 serial interface setting and discipline
	   Only one -m ce... option may be given
	-F fragfile	Read fragments from <fragfile>
	-S symfile	Read symbols from <symfile>
	-Y line		Use <line> as serial device
	-Y =speed,size,parity,stopb	Set the serial settings
		with <speed>  : 75 100 110 200 300 600 1200 or 2400
		with <size>   : 5 6 7 or 8
		with <parity> : N E or O
		with <stopb>  : 1 or 2
	-Z		Add a CE158 CSAVE header
	-Zname=name	Set <name> as CSAVE header file name
	-Zstart=addr	Set <addr> as CSAVE header start base address
	-Zentry=addr	Set <addr> as CSAVE header startup routine
	-Zheader=type	Set <type> for CSAVE header magic
		with <type>   : CSAVE CSAVEM CSAVEr or PRINT
	-r		Receive data from a PC-1500; exclusive with -s
	-s		Send data to a PC-1500; exclusive with -r


One of -s (send) or -r (receive) action should be specified when calling lhcom.

When called for receiving, the CSAVE header is expected to be received from
the remote PC-1500. If -Z is specified, the header is kept into the binary
file received. Else, the binary file is saved without the CSAVE header. Note
that the CSAVE header is useful when calling lhdump or to send the binary
file again.

When called for sending, the CSAVE header has to be built, if the -Z option
is specified. If not, the CSAVE header is expected inside the binary file to
send. When specifying a -Z option, the same options as lhasm are supported by
lhcom. The start address is retrieved from the first fragment if a
-F <fragfile> is given. The length is filled with the length of the binary
file. If a symbol file is given by -S <symfile>, and a symbol STARTUP exists
and the CSAVE header is CSAVEM (magic BCOM), the entry address is filled with
the STARTUP address found.

The default serial line device is /dev/ttyS0 for Unix/Linux/*BSD platforms.
The default serial port is \\.\COM1 for Window32 platforms. To specify another
serial device, use the option -Y <serial line>. 
The default line settings are 300 bauds, 8 bits, No Parity, 1 stop bit
(300,8,N,1). These are the same as the default CE-158 parameters. To specify
others line settings use the options -Y =<speed>,<wordsize>,<parity>,<stopbit>.
  - The supported values for speed are 100 110 200 300 600 1200 or 2400.
  - The supported values for wordsize are 5 6 7 or 8.
  - The supported values for parity are N (no) E (even) or O (odd).
  - The supported values for stopbit are 1 or 2.

binfile is the binary file to read for sending (-s) or to write for receiving
(-r).

To send a BASIC program from a PC-1500 to a host computer, do on the host
computer:
	lhcom -r myprog.bin
And on the PC-1500:
	SETDEV CO
	OUTSTAT 0
	CSAVE "MYPROGBASIC"
In the example above, we use the default line settings.

To receive a ML program starting at &40c5 from a host computer at the speed
2400, do on the PC-1500:
	SETCOM 2400
	SETDEV CI
	CLOAD M
And on the host computer, do
	lhcom -s -Y =2400 -Zheader=CSAVEM -Zstart=&40c5 myml.bin


4/ lhpoke
   ======

Usage: ./lhpoke [-h] [-v] [-x] [-xx] [-Z] [-O origin] [-A[A] appendline]
		[-B byteperline] [-L linenum] [-I lineincr] [-V varable]
		[-S symfile] [-o basfile] binfile
where:
	-h		This help
	-x		Values in POKE are in hexadecimal
	-xx		Values in POKE are in hexadecimal aligned
	-A appendline	Append <appendline> on the first line
	-AA appendline	Append a new line after the first with <appendline>
	-B byteperline	Write <byteperline> bytes on each line. Default 10
	-I lineincr	Use <lineincr> as line number increment. Default 10
	-L linenum	Use <linenum> as first line number. Default 10
	-O address	Use <address> as origin base address. Default &40C5
	-S symfile	Read symbols from <symfile>
	-V variable	Use <variable> as base address. Default A
	-Z		Expect and use a CE158 CSAVE header
	-o outfile	Output BASIC code into basfile (.bas)


lhpoke is a small utility to convert a binary file into a BASIC program using
POKE. The aim is to offer more freedom to relocate an invariant code.

Image the following assembly program ern.asm :
	.CODE
	LDA	(ERRORNUM)
	JP	&D9E4
	.END
This code is fully invariant and could be installed at any location.
Do a lhasm -c ern.asm and after running lhpoke -o ern.bas ern.bin produce the
following BASIC source:
	10 A=197+256*PEEK &7863
	20 POKE A+0,165,120,155,186,217,228

To enter the bytes and the offsets in hexadecimal, use the option -x:
	10 A=197+256*PEEK &7863
	20 POKE A+&0,&A5,&78,&9B,&BA,&D9,&E4

To enter the bytes and the offsets into a hexadecimal formatted form, use the
option -xx:
	10 A=197+256*PEEK &7863
	20 POKE A+&0000,&A5,&78,&9B,&BA,&D9,&E4

By default, lhpoke uses the BASIC variable A for the base address. To use
another variable, use the option -V var where var is a one-letter variable.
So lhpoke -xx -V U ern.bin gives:
	10 U=197+256*PEEK &7863
	20 POKE U+&0000,&A5,&78,&9B,&BA,&D9,&E4

By default, lhpoke start numbering the line at 10 and use an increment of 10.
To change the first line, use the option -L linenum and to change the
increment, use the option -I increment.

By default, lhpoke prints 10 bytes for each line of POKE. To change this, use
the option -B nbyte wher nbyte is from 1 to 16.

By default, lhpoke uses origin base address as the RAM base + 197 (i.e &mmC5)
where &mm is given by the value of &7863. To set another base, use the option
-O address. 

To add some instruction after the variable assigned to the origin base address,
use the option -A inst1[:inst2[:...]]. The inst1[:inst2[:...]] are inserted
after a colon : on the first line. If you prefer to add them on a new line,
use the option -AA inst1[:inst2[:...]] instead.
For example, lhpoke -A 'INPUT "BASE ADDRESS?",A' -x ern.bin produces:
	10 A=197+256*PEEK &7863:INPUT "BASE ADDRESS?",A
	20 POKE A+&0,&A5,&78,&9B,&BA,&D9,&E4

In the same way,
lhpoke -AA 'PRINT "BASE=";A:INPUT "BASE ADDRESS?",A' \
	-O 4100 -x ern.bin
produces:
	10 A=&4100
	20 PRINT "BASE=";A:INPUT "BASE ADDRESS?",A
	30 POKE A+&0,&A5,&78,&9B,&BA,&D9,&E4

If the binary contains a CE-158 CSAVE header, use the option -Z. With it, some
information like the base address will be retrieved from this header.


5/ fragments
   =========

The usage of fragments describe how the binary image is structured. The following
fragments may be define:
	CODE	: -c option
	BYTE	: -b option
	WORD	: -w option
	LONG	: -l option
	TEXT	: -t option
	KEYWORD	: -k option
	BASIC	: -B option
	RESERVE	: -R option
	VAR	: -V option
	XREG	: -X option
	HOLE	: -H option

The format of the fragment file is:
	.FRAGMENTS:	<origin>
	<fragment type1>	<address1>
	<fragment type2>	<address2>
	...
	<fragment typeN>	<addressN>

Between to <fragment type>, a comment may be added with COMMENT text....

For example:
	.FRAGMENTS:	00C5
		CODE	00C5
		COMMENT	My program
		BYTE	0100
		TEXT	0120
		CODE	0130

The code starts at &00C5 to &00FF. The comment "My program" will be printed
after a ;.
A data table as bytes starts at &0100 to &011F.
A text starts at &0120 to &012F.
A code starts at &0130 to the end of the binary image.


6/ Install (X68000 Cross-Compilation)
   ==================================

You need the elf2x68k toolchain (m68k-xelf-gcc and elf2x68k.py) to compile this port.
Just go into the directory and type "make". This will produce four X68000 
executables: lhasm.x, lhdump.x, lhcom.x, and lhpoke.x.


7/ Bug and license
   ===============

Please report bugs or suggestion to cgh75015@gmail.com

Copyright 1992-2015 Christophe Gottheimer <cgh75015@gmail.com>

lhTools is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License version 2 as published by the Free
Software Foundation.  Note that I am not granting permission to redistribute
or modify lhTools under the terms of any later version of the General Public
License.

This program is distributed in the hope that it will be useful (or at
least amusing), but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with
this program (in the file "COPYING"); if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


---------------------------------------------------------------------
X68000 Port Maintainer: Kenoh (2026)
This port introduces lhpoke.x as a fully functional utility on Human68k.
