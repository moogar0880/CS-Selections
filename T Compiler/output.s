#	Prologue
	.text
	.align 4
	.globl	main
main:
	pushl	%ebp
	movl	%esp, %ebp
#	Object Class VMT
	.data
Object$VMT:
	.long	0
	.text
#	MainFunction
#	IntegerLiteral
	pushl	$42
#	Print int
	call	RTS_outputInteger
	addl	$4, %esp
#	Epilogue
main$exit:
	popl	%ebp
	ret
