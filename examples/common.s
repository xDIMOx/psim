# Check LICENSE file for copyright and license details.

	.text
	.align 0
	.set noreorder
	.global thread_id
	.ent thread_id
thread_id:
	ori $v0, $k0, 0
	jr $ra
	nop

	.end thread_id
