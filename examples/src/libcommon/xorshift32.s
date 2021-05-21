# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.global xorshift32
	.ent xorshift32
xorshift32:
	la $t0, xorshift32state
	ll $t1, 0($t1)
	sll $t2, $t1, 13        # x ^= x << 13
	xor $t1, $t1, $t2
	sra $t2, $t1, 17        # x ^= x >> 17
	xor $t1, $t1, $t2
	sll $t2, $t1, 5         # x ^= x << 5
	xor $t1, $t1, $t2
	move $v0, $t1
	sc $t1, 0($t0)
	beqz $t1, xorshift32
	nop
	jr $ra
	nop

	.end xorshift32

	.data
	.align 0
	.global xorshift32state
xorshift32state: .word 1
