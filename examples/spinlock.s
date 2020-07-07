# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.global Spin_lock
	.ent Spin_lock
Spin_lock:
	ll $t0, 0($a0)
	addiu $t0, $t0, -1
	bltz $t0, Spin_lock
	nop
	sc $t0, 0($a0)
	beqz $t0, Spin_lock
	nop
	jr $ra
	nop

	.end Spin_lock

	.text
	.align 0
	.set noreorder
	.global Spin_unlock
	.ent Spin_unlock
Spin_unlock:
	ll $t0, 0($a0)
	addiu $t0, $t0, 1
	sc $t0, 0($a0)
	beqz $t0, Spin_unlock
	nop
	jr $ra
	nop

	.end Spin_unlock
