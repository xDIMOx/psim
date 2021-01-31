# Check LICENSE file for copyright and license details.

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
