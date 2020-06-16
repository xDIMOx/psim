# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.global thread_id
	.ent thread_id
thread_id:
	ori $v0, $k0, 0
	jr $ra
	nop

	.end thread_id

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
