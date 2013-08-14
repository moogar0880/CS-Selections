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
#	MainFunction
#	Declaration
	.data
#	VariableList
mainvar$a1: .long 0
	.text
#	Declaration
	.data
#	VariableList
mainvar$a2: .long 0
	.text
#	Assignment
#	Variable
	pushl	$mainvar$a1
#	A Class Instance
	pushl	$4
	pushl	$8
	call	calloc
	addl	$8, %esp
	cmpl	$0, %eax
	jne		CI1
	pushl	$12
	call	RTS_outOfMemoryError
CI1:
	movl	$A$VMT, (%eax)
	pushl	%eax
	popl	%eax
	popl	%edx
	movl	%eax, (%edx)
	pushl	%eax
#	ExpressionStatement
	addl	$4, %esp
#	Assignment
#	Variable
	pushl	$mainvar$a2
#	Variable
	pushl	$mainvar$a1
#	Deref
	popl	%eax
	movl	(%eax), %eax
	pushl	%eax
	popl	%eax
	popl	%edx
	movl	%eax, (%edx)
	pushl	%eax
#	ExpressionStatement
	addl	$4, %esp
#	Return
	jmp	main$exit
#	Epilogue
main$exit:
	popl	%ebp
	ret
