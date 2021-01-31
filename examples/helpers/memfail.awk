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
	memops = $3 + $5 + $7 + $9
	memfail = $4 + $6 + $8 + $10
	rate += memfail / memops
}

END {
	rate /= np
	rate *= 100
	print np, rate
}
