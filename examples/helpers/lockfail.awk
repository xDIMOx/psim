#!/bin/awk -f
# Check LICENSE file for copyright and license details.
#
# Number of times locking failed
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
	fail[$1] = $11
}

END {
	printf("%d", np - nexc)
	for (i = 0; i < np; ++i) {
		if (i ~ EXCLUDE) {
			continue
		}
		printf(",%d", fail[i], i)
	}
	printf("\n");
}
