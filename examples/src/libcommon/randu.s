# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.global randu
	.ent randu
randu:
	addiu $sp, $sp, -4
	sw $ra, 0($sp)
	nop
	la $t0, randstate
RANDU:
	ll $t1, 0($t0)
	la $t2, 0x10003
	mul $t1, $t1, $t2
	move $t3, $t1
	sc $t3, 0($t0)
	beqz $t3, RANDU
	nop
	la $t2, 0x7fffffff
	and $v0, $t1, $t2
	lw $ra, 0($sp)
	addiu $sp, $sp, 4
	jr $ra
	nop

	.end randu
