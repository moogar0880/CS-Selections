#	A$VMT
	.data
A$VMT:
	.long Object$VMT
	.long A$Destructor
	.long Object$equals_Object
	.long A$f
	.long A$g
	.text
#	End of A$VMT
#	Object$VMT
	.data
Object$VMT:
	.long 0
	.long Object$Destructor
	.long Object$Object
	.long Object$equals_Object
	.text
#	End of Object$VMT
#	Method A$f
	.align	4
	.globl	A$f
A$f:
	pushl	%ebp			# save old frame pointer
	movl	%esp, %ebp		# establish new frame pointer
#	Block
#	Return
#	IntegerLiteral
	pushl	$1
	popl	%eax
	jmp		A$f$exit
	movl	$0, %eax
A$f$exit:
	popl	%ebp			# restore caller's frame pointer
	ret						# restore caller's program counter
#	Method A$g
	.align	4
	.globl	A$g
A$g:
	pushl	%ebp			# save old frame pointer
	movl	%esp, %ebp		# establish new frame pointer
#	Block
#	MethodInvoke f: 0
	movl	8(%ebp), %eax	# 8(%ebp) is the "this" pointer
	pushl	%eax
	movl	(%eax), %eax	# put VMT pointer into eax
	addl	$12, %eax
	movl	(%eax), %eax	# put method address into eax
	call	*%eax
	addl	$(0+1)*4, %esp	# deallocate arguments from stack
	pushl	%eax			# leave method return value on top of stack
#	Print int
	call	RTS_outputInteger
	addl	$4, %esp
	movl	$0, %eax
A$g$exit:
	popl	%ebp			# restore caller's frame pointer
	ret						# restore caller's program counter
#	Default Destructor A$Destructor
	.align	4
	.globl	A$Destructor
A$Destructor:
	pushl	%ebp			# save old frame pointer
	movl	%esp, %ebp		# establish new frame pointer
	call	Object$Destructor
A$Destructor$exit:
	popl	%ebp			# restore caller's frame pointer
	ret						# restore caller's program counter
#	Default Constructor A$A
	.align	4
	.globl	A$A
A$A:
	pushl	%ebp			# save old frame pointer
	movl	%esp, %ebp		# establish new frame pointer
	call	Object$Object
A$A$exit:
	popl	%ebp			# restore caller's frame pointer
	ret						# restore caller's program counter
#	Constructor Object$Object
	.align	4
	.globl	Object$Object
Object$Object:
	pushl	%ebp			# save old frame pointer
	movl	%esp, %ebp		# establish new frame pointer
	movl	$0, %eax
Object$Object$exit:
	popl	%ebp			# restore caller's frame pointer
	ret						# restore caller's program counter
#	Destructor Object$Destructor
	.align	4
	.globl	Object$Destructor
Object$Destructor:
	pushl	%ebp			# save old frame pointer
	movl	%esp, %ebp		# establish new frame pointer
	movl	$0, %eax
Object$Destructor$exit:
	popl	%ebp			# restore caller's frame pointer
	ret						# restore caller's program counter
#	Method Object$equals_Object
	.align	4
	.globl	Object$equals_Object
Object$equals_Object:
	pushl	%ebp			# save old frame pointer
	movl	%esp, %ebp		# establish new frame pointer
#	Parameter Object obj
	leal	12(%ebp), %eax
	pushl %eax
#	If then else
	movl	8(%ebp), %eax	#8(%ebp) is the "this" pointer
	pushl	%eax
#	Parameter Object obj
	leal	8(%ebp), %eax
	pushl %eax
#	Deref
	popl	%eax
	movl	(%eax), %eax
	pushl	%eax
#	End Deref
#	Equality
	popl	%eax
	popl	%edx
	cmpl	%eax,%edx
	sete	%al
	movzbl	%al, %eax
	pushl	%eax
	popl	%eax
	cmpl	$0, %eax
	je	L0
#	Return
#	IntegerLiteral
	pushl	$1
	popl	%eax
	jmp		Object$equals_Object$exit
	jmp	L1
L0:
#	Return
#	IntegerLiteral
	pushl	$0
	popl	%eax
	jmp		Object$equals_Object$exit
L1:
	movl	$0, %eax
Object$equals_Object$exit:
	popl	%ebp			# restore caller's frame pointer
	ret						# restore caller's program counter
#	MainFunction
	.text
	.align 4
	.globl	main
main:
	pushl	%ebp
	movl	%esp, %ebp
#	Declaration
	.data
#	VariableList
mainvar$a: .long 0
	.text
#	Assignment
#	Variable
	pushl	$mainvar$a
#	A Class Instance
	pushl	$0
	call	RTS_reverseArgumentsOnStack
	popl	%ecx			# discard n+1 argument
	pushl	$4			# unit size to be used by calloc
	pushl	$2			# number of units to be allocated
	call	calloc		# address of new object returned in eax
	addl	$8, %esp	# deallocate arguments to calloc
	cmpl	$0, %eax
	jne		CI3
	pushl	$15
	call	RTS_outOfMemoryError
CI3:
	movl	$A$VMT, (%eax)
	pushl	%eax		# pass the "this" pointer
	call	A$A
	popl	%eax		# get address of new object into eax
	addl	$0,	%esp	# deallocate arguments to constructor
	pushl	%eax
#	End of Class Instance Creation
	popl	%eax
	popl	%edx
	movl	%eax, (%edx)
	pushl	%eax
#	End of Assignment
#	ExpressionStatement
	addl	$4, %esp
#	MethodInvoke g: 1
#	Variable
	pushl	$mainvar$a
#	Deref
	popl	%eax
	movl	(%eax), %eax
	pushl	%eax
#	End Deref
	pushl	$17
	call	RTS_checkForNullReference
	popl	%ecx			# discard line number
	popl	%eax			# get copy of "this" in eax
	pushl	%eax			# put copy of "this" back on stack
	movl	(%eax), %eax	# put VMT pointer into eax
	addl	$16, %eax
	movl	(%eax), %eax	# put method address into eax
	call	*%eax
	addl	$(0+1)*4, %esp	# deallocate arguments from stack
	pushl	%eax			# leave method return value on top of stack
#	ExpressionStatement
	addl	$4, %esp
#	IntegerLiteral
	pushl	$42
#	Print int
	call	RTS_outputInteger
	addl	$4, %esp
#	MainFunctionExit
main$exit:
	popl	%ebp
	ret
