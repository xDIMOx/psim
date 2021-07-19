#!/bin/awk -f
# Check LICENSE file for copyright and license details.
#
# Bus utilization
#	Prints the percentage rate that indicates the bus utilization for
#	a system with np processors.
#
# The variable EXCLUDE is an regular expression. If set, the processors that
# match it will be excluded from the computation

BEGIN {
	FS = OFS = ","
	np = 0
	meminstr = 0
	if (EXCLUDE == "")
		EXCLUDE = "ign"
}

/^id/ || $1 ~ EXCLUDE {
	next
}

/^bus/ {
	buscycles = $2
	next
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
