#!/bin/awk -f
# Check LICENSE file for copyright and license details.

BEGIN {
	FS = OFS = ","
	np = 0
	meminstr = 0
}

/^id/ {
	next
}

/^bus/ {
	buscycles = $2
}

/^[0-9]+/ {
	++np
	meminstr += $3 + $5 + $7 + $9
}

END {
	meminstr /= np
	busutil = meminstr / buscycles
	busutil *= 100
	print np, busutil
}
