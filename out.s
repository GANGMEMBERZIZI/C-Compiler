	.data
	.globl	a
a:	.word	0
	.globl	b
b:	.word	0
	.globl	c
c:	.word	0
	.text
	.globl	main
	.type	main, %function
main:
	push	{fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #16
	mov	r4, #10
	str	r4, [fp, #-12]
	mov	r4, #20
	str	r4, [fp, #-8]
	mov	r4, #30
	strb	r4, [fp, #-4]
	ldr	r4, [fp, #-12]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	ldr	r4, [fp, #-8]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	ldrb	r4, [fp, #-4]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	mov	r4, #5
	ldr	r3, =a
	str	r4, [r3]
	mov	r4, #15
	ldr	r3, =b
	str	r4, [r3]
	mov	r4, #25
	ldr	r3, =c
	str	r4, [r3]
	ldr	r3, =a
	ldr	r4, [r3]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	ldr	r3, =b
	ldr	r4, [r3]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	ldr	r3, =c
	ldr	r4, [r3]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	mov	r4, #0
	mov	r0, r4
	b	L1
L1:
	sub	sp, fp, #4
	pop	{fp, pc}
