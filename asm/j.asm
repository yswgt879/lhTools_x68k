	.CODE
top:
	JR Z,end
	JR end
	JR top
	.ALIGN: 40
	JR H top
	JR c,&3F00
	JP 4000
	JR NV 4321
	JR 5000
end:
