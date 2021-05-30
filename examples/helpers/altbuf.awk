#!/bin/awk -f
# Check LICENSE file for copyright and license details.
#
# d_producer-consumer{v2,}: Average time the buffer process is waiting for the
# alternative commands to finish

BEGIN {
	FS = OFS = ","
	np = 0
	cycles = 0
}

/^(id|bus|net)/ {
	next
}

/^[0-9]+/ {
	++np
}

/^1,/ {
	cycles = $18 / $15
}

END {
	print np, cycles
}
