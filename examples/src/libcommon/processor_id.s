# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.global processor_id
	.ent processor_id
processor_id:
	ori $v0, $k0, 0
	jr $ra
	nop

	.end processor_id
