# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.global C2_output
	.ent C2_output
C2_output:
	mtc2 $a0, $0, 0 # CORR
	mtc2 $a1, $0, 1 # DATA
	c2 3 # OUTPUT
	jr $ra
	nop

	.end C2_output
