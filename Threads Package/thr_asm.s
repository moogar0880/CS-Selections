// Jonathan Nappi Program6 CS520
//  thr_asm.s
//  Program6
//
//  Created by Jonathan Nappi on 4/6/12.
//

.text
.align 4
.globl asm_yield
.globl asm_specialyield

asm_yield:
    pushl   %ebp
    movl    %esp, %ebp
    jmp 	.store
    
asm_specialyield:
	pushl   %ebp
    movl    %esp, %ebp
	jmp 	.restore

.store:
    movl 8(%ebp), %eax #get TBP current pointer to eax
    movl %esi, 0(%eax)
    movl %edi, 4(%eax)
    movl %ebx, 8(%eax)
    movl %esp, 12(%eax)

.restore:
    movl 12(%ebp), 	%eax #get TBP current pointer to eax
    movl 0(%eax), 	%esi
    movl 4(%eax), 	%edi
    movl 8(%eax), 	%ebx
    movl 12(%eax), 	%esp

.exit:
    popl %ebp
    ret
