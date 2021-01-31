# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.global printhex
	.ent printhex
printhex:
	sw $a0, STDOUT($zero)
	jr $ra
	nop

	.end printhex

	.bss
	.align 0

	.equ STDOUT,0xFFFC
