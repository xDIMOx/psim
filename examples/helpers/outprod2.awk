#!/bin/awk -f
# Check LICENSE file for copyright and license details.
#
# d_producer-consumerv2: Average time the producers waits for the
# output commands to finish

BEGIN {
	FS = OFS = ","
	np = 0
	cycles = 0

	PRODUCER0 = 0

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

$1 == PRODUCER0 || $1 == PRODUCER1 {
	cycles = $17 / $14
}

END {
	print np, cycles
}
