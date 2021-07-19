#!/bin/awk -f
# Check LICENSE file for copyright and license details.
#
# Network utilization
#	Prints the percentage rate that indicates the network utilization for
#	a system with np processors.
#
# The variable EXCLUDE is an regular expression. If set, the processors that
# match it will be excluded from the computation

BEGIN {
	FS = OFS = ","
	np = 0
	if (EXCLUDE == "")
		EXCLUDE = "ign"
}

/^id/ || $1 ~ EXCLUDE {
	next
}

/^net/ {
	netcycles = $2
}

/^[0-9]+/ {
	++np
	cycles += $16 + $17 + $18
}

END {
	cycles /= np
	netutil = cycles / netcycles
	netutil *= 100
	print np, netutil
}
