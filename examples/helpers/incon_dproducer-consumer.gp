# Check LICENSE file for copyright and license details.

set terminal png
set output 'incon_dproducer-consumer.png'

set datafile separator ','
set nozero
set tics in
set nogrid
set logscale x 2
set xrange [2:128]
set xtics nomirror
set ytics nomirror
set size 1.0,1.0
set border 3 lw 2
set key left center

set ylabel "cycles"
set xlabel "consumers"

plot "incon_10_13.csv" u 1:2 w linesp title "static", \
     "incon_((i&1)==0?9+(rand()&7):9-(rand()&7))_1+(rand()&1023).csv" \
         u 1:2 w linesp title "low", \
     "incon_1+(rand()&1023)_1+(rand()&1023).csv" \
         u 1:2 w linesp title "high", \
     "incon_1+(rand()&1023)_((item&1)==0?9+(rand()&7):9-(rand()&7)).csv" \
         u 1:2 w linesp title "high-p", \
