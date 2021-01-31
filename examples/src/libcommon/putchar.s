# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.global putchar
	.ent putchar
putchar:
	sb $a0, STDOUT($zero)
	jr $ra
	nop

	.end putchar

	.bss
	.align 0

	.equ STDOUT,0xFFFC
