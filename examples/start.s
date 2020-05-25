# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.extern main
	.global _start
	.ent _start
_start:
	# set stack frame
	c2 0 # get memory size on $k1
	addiu $t0, $zero, STKSZ
	mul $t0, $t0, $k0
	subu $sp, $k1, $t0
	addiu $sp, $sp, -4

	# start code
	jal main
	nop

BUSYWAIT:
	bnez $k0, BUSYWAIT
	wait # only processor 0 can stop the simulation
	nop

	.end _start

	.bss
	.align 0

	.equ STKSZ,(1 << 13)
