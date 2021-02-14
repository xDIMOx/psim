#!/bin/awk -f
# Check LICENSE file for copyright and license details.

BEGIN {
	FS = ","
	OFS = "\n"
}

/^[0-9]+/ {
	for (i = 2; i <= NF; ++i)
		np[$1] = $i OFS np[$1]
}

END {
	for (i in np)
		print np[i] >"cw_"i"p.dat"
}
