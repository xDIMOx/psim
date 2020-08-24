# Check LICENSE file for copyright and license details.

	.include "cop2.s"
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
	c2 LOCKPERF0 # toggle performance counter
	jr $ra
	nop

	.end tog_lockperf0

	.text
	.align 0
	.set noreorder
	.global tog_lockperf1
	.ent tog_lockperf1
tog_lockperf1:
	c2 LOCKPERF1 # toggle performance counter
	jr $ra
	nop

	.end tog_lockperf1

	.text
	.align 0
	.set noreorder
	.global tog_lockperf2
	.ent tog_lockperf2
tog_lockperf2:
	c2 LOCKPERF2 # toggle performance counter
	jr $ra
	nop

	.end tog_lockperf2

	.text
	.align 0
	.set noreorder
	.global tog_lockperf3
	.ent tog_lockperf3
tog_lockperf3:
	c2 LOCKPERF3 # toggle performance counter
	jr $ra
	nop

	.end tog_lockperf3

	.text
	.align 0
	.set noreorder
	.global tog_lockperf4
	.ent tog_lockperf4
tog_lockperf4:
	c2 LOCKPERF4 # toggle performance counter
	jr $ra
	nop

	.end tog_lockperf4

	.text
	.align 0
	.set noreorder
	.global tog_lockperf5
	.ent tog_lockperf5
tog_lockperf5:
	c2 LOCKPERF5 # toggle performance counter
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

	.text
	.align 0
	.set noreorder
	.global randu
	.extern Spin_lock
	.extern Spin_unlock
	.ent randu
randu:
	addiu $sp, $sp, -4
	sw $ra, 0($sp)
	la $a0, randulock
	jal Spin_lock
	nop
	la $t0, randuseed
	lw $t1, 0($t0)
	la $t2, 0x10003
	mul $t1, $t1, $t2
	sw $t1, 0($t0)
	la $t2, 0x7fffffff
	and $v0, $t1, $t2
	jal Spin_unlock
	nop
	lw $ra, 0($sp)
	addiu $sp, $sp, 4
	jr $ra
	nop

	.end randu

	.data
	.align 0
	.global randuseed
randulock: .word 1
randuseed: .word 1

	.bss
	.align 0

	.equ STDOUT,0xFFFC
