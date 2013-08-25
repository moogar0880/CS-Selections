#	Prologue
	.text
	.align 4
	.globl	main
main:
	pushl	%ebp
	movl	%esp, %ebp
A$VMT:
	.long Object$VMT
	.long A$Destructor
Object$VMT:
	.long 0
	.long Object$Destructor
	.long Object$Object
	.long Object$equals_Object
A$Destructor:
	ret
A$A:
	ret
#	MainFunction
#	Declaration
	.data
#	VariableList
mainvar$a: .long 0
	.text
#	Assignment
#	Variable
	pushl	$mainvar$a
#	A Class Instance
	pushl	$4
	pushl	$8
	call	calloc
	addl	$8, %esp
	cmpl	$0, %eax
	jne		CI1
	pushl	$10
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
#	Field Reference
	pushl	$mainvar$a
	pushl	$12
	call	RTS_checkForNullReference
	popl	%eax
	popl	%eax
	addl	$4, %eax
	pushl	%eax
#	IntegerLiteral
	pushl	$42
	popl	%eax
	popl	%edx
	movl	%eax, (%edx)
	pushl	%eax
#	ExpressionStatement
	addl	$4, %esp
#	Field Reference
	pushl	$mainvar$a
	pushl	$14
	call	RTS_checkForNullReference
	popl	%eax
	popl	%eax
	addl	$4, %eax
	pushl	%eax
#	Deref
	popl	%eax
	movl	(%eax), %eax
	pushl	%eax
#	Print int
	call	RTS_outputInteger
	addl	$4, %esp
#	Return
	jmp	main$exit
#	Epilogue
main$exit:
	popl	%ebp
	ret
