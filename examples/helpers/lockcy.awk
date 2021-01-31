#!/bin/awk -f
# Check LICENSE file for copyright and license details.

BEGIN {
	FS = OFS = ","
	np = 0
}

/^(id|bus)/ {
	next
}

/^[0-9]+/ {
	++np
	cycles[$1] = $12
}

END {
	printf("%d,", np)
	for (i = 0; i < (np - 1); ++i)
		printf("%d,", cycles[i])
	printf("%d\n", cycles[np - 1])
}
