#!/bin/awk -f
# Check LICENSE file for copyright and license details.
#
# Bus acess failures
#	Prints the percentage rate that indicates the bus acesses failures a
#	system with np processors has.
#
# The variable EXCLUDE is an regular expression. If set, the processors that
# match it will be excluded from the computation

BEGIN {
	FS = OFS = ","
	np = 0
	if (EXCLUDE == "")
		EXCLUDE = "ign"
}

/^(id|bus)/ || $1 ~ EXCLUDE {
	next
}

/^[0-9]+/ {
	++np
	memops = $3 + $5 + $7 + $9
	memfail = $4 + $6 + $8 + $10
	rate += memfail / memops
}

END {
	rate /= np
	rate *= 100
	print np, rate
}
