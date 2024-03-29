#!/bin/awk -f
# Check LICENSE file for copyright and license details.
#
# d_producer-consumerv2: Average time the consumers wait for the
# input commands to finish

BEGIN {
	FS = OFS = ","

	nf = split(ARGV[1], fields, "_")
	split(fields[nf - 2], topo, "x")
	PRODUCER1 = 1 + topo[2]
}

/^(id|bus|net)/ {
	next
}

/^[0-9]+/ {
	++np
}

$1 > 1 && $1 != PRODUCER1 { # consumers
	++nc;
	cycles += $16 / $13
}

END {
	print np, cycles / nc
}
