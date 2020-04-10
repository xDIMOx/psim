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

_BUSYWAIT:
	j _BUSYWAIT
	nop

	.end _start
