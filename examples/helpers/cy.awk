#!/bin/awk -f
# Check LICENSE file for copyright and license details.
#
# Cycles to execute
#	Prints the number of cycles a system with np processors completed
#	execution
#
# The variable EXCLUDE is an regular expression. If set, the processors that
# match it will be excluded from the computation


BEGIN {
	FS = OFS = ","
	np = 0
	if (EXCLUDE == "")
		EXCLUDE = "ign"
}

/^(id|bus|net)/ || $1 ~ EXCLUDE {
	next
}

/^[0-9]+/ {
	++np
	cycles += $2
}

END {
	cycles /= np
	print np, cycles
}
