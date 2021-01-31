# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.global rem
	.ent rem
rem:
	div $zero, $a0, $a1
	mfhi $v0
	jr $ra
	nop

	.end rem
