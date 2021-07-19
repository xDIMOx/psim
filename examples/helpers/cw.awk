#!/bin/awk -f
# Check LICENSE file for copyright and license details.
#
# Cycles waiting for communication
#	Prints the cycles each processor in the system waited for communication
#	to complete.
#
# The variable EXCLUDE is an regular expression. If set, the processors that
# match it will be excluded from the computation

BEGIN {
	FS = OFS = ","
	nexc = np = 0
	if (EXCLUDE == "")
		EXCLUDE = "ign"
}

/^(id|net)/ {
	next
}

$1 ~ EXCLUDE {
	++np
	++nexc
	next
}

/^[0-9]+/ {
	++np
	cycles[$1] = $16 + $17 + $18
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
