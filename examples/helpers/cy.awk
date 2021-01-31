#!/bin/awk -f
# Check LICENSE file for copyright and license details.

BEGIN {
	FS = OFS = ","
	np = 0
	cycles = 0
}

/^(id|bus)/ {
	next
}

/^[0-9]+/ {
	++np
	cycles += $2
}

END {
	cycles /= np
	print np, cycles
}
