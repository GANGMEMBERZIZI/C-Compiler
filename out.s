	.text
	.global	main
	.type	main, %function
main:
	push	{r4, r5, r6, r7, fp, lr}
	mov	fp, sp
	str	r0, [fp, #-4]
	strb	r1, [fp, #-8]
	str	r2, [fp, #-12]
	str	r3, [fp, #-16]
	sub	sp, sp, #32
	mov	r4, #13
	str	r4, [fp, #-4]
	ldr	r4, [fp, #-4]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	mov	r4, #23
	strb	r4, [fp, #-8]
	ldrb	r4, [fp, #-8]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	mov	r4, #34
	str	r4, [fp, #-12]
	ldr	r4, [fp, #-12]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	mov	r4, #44
	str	r4, [fp, #-16]
	ldr	r4, [fp, #-16]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	mov	r4, #54
	str	r4, [fp, #24]
	ldr	r4, [fp, #24]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	mov	r4, #64
	str	r4, [fp, #28]
	ldr	r4, [fp, #28]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	mov	r4, #74
	str	r4, [fp, #32]
	ldr	r4, [fp, #32]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	mov	r4, #84
	str	r4, [fp, #36]
	ldr	r4, [fp, #36]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	mov	r4, #94
	str	r4, [fp, #-20]
	ldr	r4, [fp, #-20]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	mov	r4, #95
	str	r4, [fp, #-24]
	ldr	r4, [fp, #-24]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	mov	r4, #96
	str	r4, [fp, #-28]
	ldr	r4, [fp, #-28]
	mov	r0, r4
	bl	printint
	mov	r5, r0
	mov	r4, #0
	mov	r0, r4
	b	L1
L1:
	mov	sp, fp
	pop	{r4, r5, r6, r7, fp, pc}
