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

	.text
	.align 0
	.set noreorder
	.global tog_lockperf0
	.ent tog_lockperf0
tog_lockperf0:
	c2 1 # toggle performance counter
	jr $ra
	nop

	.end tog_lockperf0

	.text
	.align 0
	.set noreorder
	.global tog_lockperf1
	.ent tog_lockperf1
tog_lockperf1:
	c2 2 # toggle performance counter
	jr $ra
	nop

	.end tog_lockperf1

	.text
	.align 0
	.set noreorder
	.global tog_lockperf2
	.ent tog_lockperf2
tog_lockperf2:
	c2 3 # toggle performance counter
	jr $ra
	nop

	.end tog_lockperf2

	.text
	.align 0
	.set noreorder
	.global tog_lockperf3
	.ent tog_lockperf3
tog_lockperf3:
	c2 4 # toggle performance counter
	jr $ra
	nop

	.end tog_lockperf3

	.text
	.align 0
	.set noreorder
	.global tog_lockperf4
	.ent tog_lockperf4
tog_lockperf4:
	c2 5 # toggle performance counter
	jr $ra
	nop

	.end tog_lockperf4

	.text
	.align 0
	.set noreorder
	.global tog_lockperf5
	.ent tog_lockperf5
tog_lockperf5:
	c2 6 # toggle performance counter
	jr $ra
	nop

	.end tog_lockperf5

	.text
	.align 0
	.set noreorder
	.global busywait
	.ent busywait
busywait:
	addiu $a0, $a0, -1
	bgtz $a0, busywait
	nop
	jr $ra
	nop

	.end busywait

	.bss
	.align 0

	.equ STDOUT,0xFFFC
