# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.global C2_input
	.ent C2_input
C2_input:
	mtc2 $a0, $0, 0 # CORR
	c2 2 # INPUT
	mfc2 $v0, $0, 1 # DATA
	jr $ra
	nop

	.end C2_input
