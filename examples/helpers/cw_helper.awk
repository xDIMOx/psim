#!/bin/awk -f
# Check LICENSE file for copyright and license details.

BEGIN {
	FS = ","
	OFS = "\n"
}

/^[0-9]+/ {
	for (i = 2; i <= NF; ++i) {
		np[$1] = $i OFS np[$1]
		if (i > 3) {
			consumer[$1] = $i OFS consumer[$1]
		}
	}
	prod[$1] = $2 OFS prod[$1]
	buffer[$1] = $3 OFS buffer[$1]
}

END {
	for (i in np) {
		print np[i] >"cw_"i"p.dat"
		print prod[i] >"prod_cw_"i"p.dat"
		print buffer[i] >"buffer_cw_"i"p.dat"
		print consumer[i] >"consumer_cw_"i"p.dat"
	}
}
