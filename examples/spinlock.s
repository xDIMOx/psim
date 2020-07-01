# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.global Spin_lock
	.ent Spin_lock
Spin_lock:
	c2 1 # toggle performance count on
LOCK_LOOP:
	ll $t0, 0($a0)
	addiu $t0, $t0, -1
	bltz $t0, LOCK_LOOP
	nop
	sc $t0, 0($a0)
	beqz $t0, LOCK_LOOP
	nop
	c2 1 # toggle performance count off
	c2 2 # toggle performance count on
	jr $ra
	nop

	.end Spin_lock

	.text
	.align 0
	.set noreorder
	.global Spin_unlock
	.ent Spin_unlock
Spin_unlock:
	c2 2 # toggle performance count off
	c2 3 # toggle performance count on
UNLOCK_LOOP:
	ll $t0, 0($a0)
	addiu $t0, $t0, 1
	sc $t0, 0($a0)
	beqz $t0, UNLOCK_LOOP
	nop
	c2 3 # toggle performance count off
	jr $ra
	nop

	.end Spin_unlock
