# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.extern main
	.global _start
	.ent _start
_start:
	jal main
	nop

BUSYWAIT:
	bnez $k0, BUSYWAIT
	wait # only processor 0 can stop the simulation
	nop

	.end _start
