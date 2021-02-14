# Check LICENSE file for copyright and license details.

set terminal png
set output 'netutil_dproducer-consumer.png'

set datafile separator ','
set logscale x
set nozero
set tics in
set nogrid
set xrange [1:128]
set xtics  nomirror ("1" 1, "2" 2, "4" 4, "8" 8, "16" 16, "32" 32, "64" 64)
set ytics nomirror
set size 1.0,1.0
set border 3 lw 2
set key tmargin

set title "Net utilization"
set ylabel "%"
set xlabel "processors"

plot "netutil_10_13.csv" u 1:2 w linesp title "static", \
     "netutil_((i&1)==0?9+rem(randu(),7):9-rem(randu(),7))_1+(randu()&1023).csv" \
         u 1:2 w linesp title "low", \
     "netutil_1+(randu()&1023)_1+(randu()&1023).csv" \
     u 1:2 w linesp title "high", \
     "netutil_1+(randu()&1023)_((item&1)==0?9+rem(randu(),7):9-rem(randu(),7)).csv" \
     u 1:2 w linesp title "high 2", \
