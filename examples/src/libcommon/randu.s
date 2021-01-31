# Check LICENSE file for copyright and license details.

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
