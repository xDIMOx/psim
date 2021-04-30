# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.global Sem_V
	.ent Sem_V
Sem_V:
	ll $t0, 0($a0)
	addiu $t0, $t0, 1
	sc $t0, 0($a0)
	beqz $t0, Sem_V
	nop
	jr $ra
	nop

	.end Sem_V
