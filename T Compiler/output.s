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
#	A Class VMT
	.data
A$VMT:
	.long	Object$VMT
	.text
#	B Class VMT
	.data
B$VMT:
	.long	A$VMT
	.text
#	C Class VMT
	.data
C$VMT:
	.long	B$VMT
	.text
#	D Class VMT
	.data
D$VMT:
	.long	C$VMT
	.text
#	MainFunction
#	Declaration
	.data
#	VariableList
mainvar$a: .long 0
	.text
#	Declaration
	.data
#	VariableList
mainvar$d: .long 0
	.text
#	Declaration
	.data
#	VariableList
mainvar$obj: .long 0
	.text
#	IntegerLiteral
	pushl	$42
#	Print int
	call	RTS_outputInteger
	addl	$4, %esp
#	Epilogue
main$exit:
	popl	%ebp
	ret
