# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.global C2_alt
	.ent C2_alt
C2_alt:
	mtc2 $a0, $0, 0 # CORR
	mtc2 $a1, $0, 5 # NCL
	c2 4 # ALT
	mfc2 $t0, $0, 1 # DATA
	sw $t0, 0($a2)
	addiu $v0, $k1, 0
	jr $ra
	nop

	.end C2_alt
