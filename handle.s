	.file	"handle.c"
	.section	.rodata
.LC0:
	.string	"Nice try.\n"
	.text
	.globl	sigint_handler
	.type	sigint_handler, @function
sigint_handler:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movl	$1, -12(%rbp)
	movl	-12(%rbp), %eax
	movl	$10, %edx
	movl	$.LC0, %esi
	movl	%eax, %edi
	call	write
	movq	%rax, -8(%rbp)
	cmpq	$10, -8(%rbp)
	je	.L3
	movl	$-999, %edi
	call	exit
.L3:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	sigint_handler, .-sigint_handler
	.section	.rodata
.LC1:
	.string	"exiting\n"
	.text
	.globl	sigusr1_handler
	.type	sigusr1_handler, @function
sigusr1_handler:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movl	$1, -12(%rbp)
	movl	-12(%rbp), %eax
	movl	$10, %edx
	movl	$.LC1, %esi
	movl	%eax, %edi
	call	write
	movq	%rax, -8(%rbp)
	cmpq	$10, -8(%rbp)
	je	.L5
	movl	$-999, %edi
	call	exit
.L5:
	movl	$1, %edi
	call	exit
	.cfi_endproc
.LFE3:
	.size	sigusr1_handler, .-sigusr1_handler
	.section	.rodata
.LC2:
	.string	"%d"
.LC3:
	.string	"SIGINT signal error"
.LC4:
	.string	"SIGUSR1 signal error"
	.text
	.globl	main
	.type	main, @function
main:
.LFB4:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$1088, %rsp
	movl	%edi, -1076(%rbp)
	movq	%rsi, -1088(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	call	getpid
	movl	%eax, -1060(%rbp)
	movl	-1060(%rbp), %edx
	leaq	-1040(%rbp), %rax
	movl	$.LC2, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	sprintf
	leaq	-1040(%rbp), %rax
	movq	%rax, %rdi
	call	puts
.L9:
	movl	$sigint_handler, %esi
	movl	$2, %edi
	call	signal
	cmpq	$-1, %rax
	jne	.L7
	movl	$.LC3, %edi
	call	unix_error
.L7:
	movl	$sigusr1_handler, %esi
	movl	$10, %edi
	call	signal
	cmpq	$-1, %rax
	jne	.L8
	movl	$.LC4, %edi
	call	unix_error
.L8:
	movq	$1, -1056(%rbp)
	movq	$0, -1048(%rbp)
	leaq	-1056(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	nanosleep
	leaq	-1040(%rbp), %rax
	movabsq	$2407013925056504183, %rcx
	movq	%rcx, (%rax)
	movw	$10, 8(%rax)
	leaq	-1040(%rbp), %rax
	movq	%rax, %rdi
	call	puts
	jmp	.L9
	.cfi_endproc
.LFE4:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.10) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
