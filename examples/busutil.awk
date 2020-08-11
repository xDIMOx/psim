#!/bin/awk -f
# Check LICENSE file for copyright and license details.

BEGIN {
	FS = OFS = ","
	np = 0
	cycles = 0
}

/^id/ {
	next
}

/^bus/ {
	buscycles = $2
}

/^[0-9]+/ {
	++np
	cycles += $2
}

END {
	cycles /= np
	busutil = buscycles / cycles
	busutil *= 100
	print np, busutil
}
