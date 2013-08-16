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
	.long A$f
Object$VMT:
	.long 0
	.long Object$Destructor
	.long Object$Object
	.long Object$equals_Object
#	 Method A$f
	.align	4
	.globl	A$f
A$f:
	pushl	%ebp			# save old frame pointer
	movl	%esp, %ebp		# establish new frame pointer
#	Block
#	IntegerLiteral
	pushl	$42
#	Print int
	call	RTS_outputInteger
	addl	$4, %esp
	movl	$0, %eax
A$f$exit:
	popl	%ebp			# restore caller's frame pointer
	ret						# restore caller's program counter
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
	pushl	$11
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
	pushl	$13
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
	pushl	$15
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
#	Delete a
#	Variable
	pushl	$mainvar$a
	pushl	$17
	call	RTS_checkForNullReference
	popl	%ecx			# discard line number
	popl	%eax			# get copy of object address in eax
	pushl	%eax
	movl	(%eax), %eax	# get VMT out of object
	addl	$4, %eax		# destructor is always in slot 1
	movl	(%eax), %eax	# get destructor address from VMT
	call	*%eax
	call	free			# address of object is still on stack
	addl	$4, %esp
#	Return
	jmp	main$exit
#	Epilogue
main$exit:
	popl	%ebp
	ret
