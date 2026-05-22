	.text
	.global	param8
	.type	param8, %function
param8:
	push	{r4, r5, r6, r7, fp, lr}
	mov	fp, sp
	str	r0, [fp, #-4]
	str	r1, [fp, #-8]
	str	r2, [fp, #-12]
	str	r3, [fp, #-16]
	sub	sp, sp, #16
	ldr	r4, [fp, #-4]
	ldr	r4, [fp, #-4]
	mov	r0, r4
	bl	printint
	mov	r4, r0
	ldr	r4, [fp, #-8]
	ldr	r4, [fp, #-8]
	mov	r0, r4
	bl	printint
	mov	r4, r0
	ldr	r4, [fp, #-12]
	ldr	r4, [fp, #-12]
	mov	r0, r4
	bl	printint
	mov	r4, r0
	ldr	r4, [fp, #-16]
	ldr	r4, [fp, #-16]
	mov	r0, r4
	bl	printint
	mov	r4, r0
	ldr	r4, [fp, #24]
	ldr	r4, [fp, #24]
	mov	r0, r4
	bl	printint
	mov	r4, r0
	ldr	r4, [fp, #28]
	ldr	r4, [fp, #28]
	mov	r0, r4
	bl	printint
	mov	r4, r0
	ldr	r4, [fp, #32]
	ldr	r4, [fp, #32]
	mov	r0, r4
	bl	printint
	mov	r4, r0
	ldr	r4, [fp, #36]
	ldr	r4, [fp, #36]
	mov	r0, r4
	bl	printint
	mov	r4, r0
	mov	r4, #0
	mov	r0, r4
	b	L1
L1:
	mov	sp, fp
	pop	{r4, r5, r6, r7, fp, pc}
	.global	fred
	.type	fred, %function
fred:
	push	{r4, r5, r6, r7, fp, lr}
	mov	fp, sp
	str	r0, [fp, #-4]
	str	r1, [fp, #-8]
	str	r2, [fp, #-12]
	sub	sp, sp, #16
	ldr	r4, [fp, #-4]
	ldr	r5, [fp, #-8]
	add	r5, r4, r5
	ldr	r4, [fp, #-12]
	add	r4, r5, r4
	mov	r0, r4
	b	L2
L2:
	mov	sp, fp
	pop	{r4, r5, r6, r7, fp, pc}
	.global	main
	.type	main, %function
main:
	push	{r4, r5, r6, r7, fp, lr}
	mov	fp, sp
	sub	sp, sp, #8
	mov	r4, #1
	mov	r4, #2
	mov	r4, #3
	mov	r4, #5
	mov	r4, #8
	mov	r4, #13
	mov	r4, #21
	mov	r4, #34
	mov	r4, #34
	str	r4, [sp, #-4]!
	mov	r4, #21
	str	r4, [sp, #-4]!
	mov	r4, #13
	str	r4, [sp, #-4]!
	mov	r4, #8
	str	r4, [sp, #-4]!
	mov	r4, #5
	mov	r3, r4
	mov	r4, #3
	mov	r2, r4
	mov	r4, #2
	mov	r1, r4
	mov	r4, #1
	mov	r0, r4
	bl	param8
	add	sp, sp, #16
	mov	r4, r0
	mov	r4, #2
	mov	r4, #3
	mov	r4, #4
	mov	r4, #4
	mov	r2, r4
	mov	r4, #3
	mov	r1, r4
	mov	r4, #2
	mov	r0, r4
	bl	fred
	mov	r4, r0
	str	r4, [fp, #-4]
	ldr	r4, [fp, #-4]
	ldr	r4, [fp, #-4]
	mov	r0, r4
	bl	printint
	mov	r4, r0
	mov	r4, #0
	mov	r0, r4
	b	L3
L3:
	mov	sp, fp
	pop	{r4, r5, r6, r7, fp, pc}
