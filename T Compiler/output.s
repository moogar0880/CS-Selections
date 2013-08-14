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
#	Declaration
	.data
#	VariableList
mainvar$i: .long 0
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
#	Field Reference
	pushl	$mainvar$a1
	pushl	$14
	call	RTS_checkForNullReference
	popl	%eax
	popl	%eax
	addl	$4, %eax
	pushl	%eax
#	IntegerLiteral
	pushl	$21
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
	popl	%eax
	popl	%edx
	movl	%eax, (%edx)
	pushl	%eax
#	ExpressionStatement
	addl	$4, %esp
#	If then else
#	Variable
	pushl	$mainvar$a1
#	Deref
	popl	%eax
	movl	(%eax), %eax
	pushl	%eax
#	Variable
	pushl	$mainvar$a2
#	Deref
	popl	%eax
	movl	(%eax), %eax
	pushl	%eax
#	Equality
	popl	%eax
	popl	%edx
	cmpl	%eax,%edx
	sete	%al
	movzbl	%al, %eax
	pushl	%eax
	popl	%eax
	cmpl	$0, %eax
	je	L1
#	Block
#	Assignment
#	Variable
	pushl	$mainvar$i
#	IntegerLiteral
	pushl	$21
	popl	%eax
	popl	%edx
	movl	%eax, (%edx)
	pushl	%eax
#	ExpressionStatement
	addl	$4, %esp
	jmp	L2
L1:
#	Block
#	Assignment
#	Variable
	pushl	$mainvar$i
#	IntegerLiteral
	pushl	$1
	popl	%eax
	popl	%edx
	movl	%eax, (%edx)
	pushl	%eax
#	ExpressionStatement
	addl	$4, %esp
L2:
#	Assignment
#	Variable
	pushl	$mainvar$i
#	Variable
	pushl	$mainvar$i
#	Field Reference
	pushl	$mainvar$a2
	pushl	$27
	call	RTS_checkForNullReference
	popl	%eax
	popl	%eax
	addl	$4, %eax
	pushl	%eax
#	Add int
	popl	%edx
	popl	%eax
	addl	%edx, %eax
	pushl	%eax
	popl	%eax
	popl	%edx
	movl	%eax, (%edx)
	pushl	%eax
#	ExpressionStatement
	addl	$4, %esp
#	Variable
	pushl	$mainvar$i
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
