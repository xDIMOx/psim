#!/bin/awk -f
# Check LICENSE file for copyright and license details.
#
# d_producer-consumer: Average time the consumers wait for the
# input commands to finish

BEGIN {
	FS = OFS = ","
	np = nc = 0
	cycles = 0
}

/^(id|bus|net)/ {
	next
}

/^[0-9]+/ {
	++np
}

$1 > 1 { # consumers
	++nc;
	cycles += $16 / $13
}

END {
	print np, cycles / nc
}
