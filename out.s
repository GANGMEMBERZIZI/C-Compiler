	.text
	.data
	.globl	a
a:	.long	0
	.data
	.globl	b
b:	.long	0
	.data
	.globl	c
c:	.long	0
	.text
	.globl	main
	.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	$42,%r8
	movl	%r8d, a(%rip)
	movq	$19,%r8
	movl	%r8d, b(%rip)
	movslq	a(%rip), %r8
	movslq	b(%rip), %r9
	andq	%r8,%r9
	movq	%r9, %rdi
	call	printint
	movq	%rax, %r8
	movslq	a(%rip), %r8
	movslq	b(%rip), %r9
	orq	%r8, %r9
	movq	%r9, %rdi
	call	printint
	movq	%rax, %r8
	movslq	a(%rip), %r8
	movslq	b(%rip), %r9
	xorq	%r8, %r9
	movq	%r9, %rdi
	call	printint
	movq	%rax, %r8
	movq	$1,%r8
	movq	$3,%r9
	movb	%r9b, %cl
	shlq	%cl, %r8
	movq	%r8, %rdi
	call	printint
	movq	%rax, %r9
	movq	$63,%r8
	movq	$3,%r9
	movb	%r9b, %cl
	shrq	%cl, %r8
	movq	%r8, %rdi
	call	printint
	movq	%rax, %r9
	movq	$0,%r8
	movl	%r8d, %eax
	jmp	L1
L1:
	popq %rbp
	ret
