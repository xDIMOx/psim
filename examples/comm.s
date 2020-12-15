# Check LICENSE file for copyright and license details.

	.include "cop2.s"
	.text
	.align 0
	.set noreorder
	.global C2_input
	.ent C2_input
C2_input:
	mtc2 $a0, COMM, CORR
	c2 INPUT
	mfc2 $v0, COMM, DATA
	jr $ra
	nop

	.end C2_input

	.text
	.align 0
	.set noreorder
	.global C2_output
	.ent C2_output
C2_output:
	mtc2 $a0, COMM, CORR
	mtc2 $a1, COMM, DATA
	c2 OUTPUT
	jr $ra
	nop

	.end C2_output
