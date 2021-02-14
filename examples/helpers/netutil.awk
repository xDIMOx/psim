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

/^net/ {
	netcycles = $2
}

/^[0-9]+/ {
	++np
	cycles += $16
}

END {
	cycles /= np
	netutil = cycles / netcycles
	netutil *= 100
	print np, netutil
}
