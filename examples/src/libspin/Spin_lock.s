# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.global Spin_lock
	.ent Spin_lock
Spin_lock:
	c2 1 # CT0
SPIN0:
	ll $t0, 0($a0)
	addiu $t0, $t0, -1
	bltz $t0, SPIN0
	nop
	sc $t0, 0($a0)
	beqz $t0, SPIN0
	nop
	c2  1 # CT0
	jr $ra
	nop

	.end Spin_lock
