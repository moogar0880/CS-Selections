#	Prologue
	.text
	.align 4
	.globl	main
main:
	pushl	%ebp
	movl	%esp, %ebp
#	Object Class VMT
	.data
A$VMT:
	.long	0
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
#	Variable
	pushl	$mainvar$a1
#	A Class Instance
	pushl	$4
	pushl	$8
	call	calloc
	addl	$8, %esp
	cmpl	$0, %eax
	jne		L1
	pushl	$12
	call	RTS_outOfMemoryError
L1:
	movl	$A$VMT, (%eax)
	pushl	%eax
#	Assignment
	popl	%eax
	popl	%edx
	movl	%eax, (%edx)
	pushl	%eax
#	ExpressionStatement
	addl	$4, %esp
#	Field Reference
	pushl	$mainvar$a1
	pushl	$13
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
#	Assignment
	popl	%eax
	popl	%edx
	movl	%eax, (%edx)
	pushl	%eax
#	ExpressionStatement
	addl	$4, %esp
#	IntegerLiteral
	pushl	$99
#	Print int
	call	RTS_outputInteger
	addl	$4, %esp
#	Field Reference
	pushl	$mainvar$a1
	pushl	$16
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
#	Variable
	pushl	$mainvar$a2
#	Variable
	pushl	$mainvar$a1
#	Deref
	popl	%eax
	movl	(%eax), %eax
	pushl	%eax
#	Assignment
	popl	%eax
	popl	%edx
	movl	%eax, (%edx)
	pushl	%eax
#	ExpressionStatement
	addl	$4, %esp
#	IntegerLiteral
	pushl	$76
#	Print int
	call	RTS_outputInteger
	addl	$4, %esp
#	Field Reference
	pushl	$mainvar$a2
	pushl	$19
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
#	IntegerLiteral
	pushl	$17
#	Print int
	call	RTS_outputInteger
	addl	$4, %esp
#	Variable
	pushl	$mainvar$i
#	IntegerLiteral
	pushl	$21
#	Assignment
	popl	%eax
	popl	%edx
	movl	%eax, (%edx)
	pushl	%eax
#	ExpressionStatement
	addl	$4, %esp
	je	L2
L1:
#	Block
#	Variable
	pushl	$mainvar$i
#	IntegerLiteral
	pushl	$1
#	Assignment
	popl	%eax
	popl	%edx
	movl	%eax, (%edx)
	pushl	%eax
#	ExpressionStatement
	addl	$4, %esp
L2:
#	IntegerLiteral
	pushl	$103
#	Print int
	call	RTS_outputInteger
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
#	Variable
	pushl	$mainvar$i
#	Variable
	pushl	$mainvar$i
#	Deref
	popl	%eax
	movl	(%eax), %eax
	pushl	%eax
#	Field Reference
	pushl	$mainvar$a2
	pushl	$32
	call	RTS_checkForNullReference
	popl	%eax
	popl	%eax
	addl	$4, %eax
	pushl	%eax
#	Deref
	popl	%eax
	movl	(%eax), %eax
	pushl	%eax
#	Add int
	popl	%edx
	popl	%eax
	addl	%edx, %eax
	pushl	%eax
#	Assignment
	popl	%eax
	popl	%edx
	movl	%eax, (%edx)
	pushl	%eax
#	ExpressionStatement
	addl	$4, %esp
#	IntegerLiteral
	pushl	$274
#	Print int
	call	RTS_outputInteger
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
