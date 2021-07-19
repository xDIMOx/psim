#!/bin/awk -f
# Check LICENSE file for copyright and license details.
#
# Cycles locked
#	Prints the number of locked cycles waiting for a P() (semaphore
#	operation) each processor of a system with np processors waited.
#
# The variable EXCLUDE is an regular expression. If set, the processors that
# match it will be excluded from the computation

BEGIN {
	FS = OFS = ","
	nexc = np = 0
	if (EXCLUDE == "")
		EXCLUDE = "ign"
}

/^(id|bus)/ {
	next
}

$1 ~ EXCLUDE {
	++np
	++nexc
	next
}

/^[0-9]+/ {
	++np
	cycles[$1] = $12
}

END {
	printf("%d", np - nexc)
	for (i = 0; i < np; ++i) {
		if (i ~ EXCLUDE) {
			continue
		}
		printf(",%d", cycles[i], i)
	}
	printf("\n");
}
