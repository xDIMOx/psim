# Check LICENSE file for copyright and license details.

set terminal png
set output 'cycles_dining-philosophers.png'

set datafile separator ','
set nozero
set tics in
set nogrid
set xtics nomirror
set ytics nomirror
set size 1.0,1.0
set border 3 lw 2
set style data histograms
set style fill solid
set boxwidth 0.5
set key inside left top

set title "Cycles to execute"
set ylabel "cycles"
set xlabel "processors"

plot "cycles_10_((rem(rand(),13))>6?(9+(rand()&7)):(9-(rand()&7)))_((rem(rand(),13))>6?(9+(rand()&7)):(9-(rand()&7))).csv" \
        u 2:xtic(1) title "fixed ideas", \
     "cycles_((rem(rand(),13))>6?(10+(rand()&7)):(10-(rand()&7)))_((rem(rand(),13))>6?(50+rem(rand(),25)):(50-rem(rand(),25)))_((rem(rand(),13))>6?(9+(rand()&7)):(9-(rand()&7))).csv" \
        u 2:xtic(1) title "thinkers", \
     "cycles_((rem(rand(),13))>6?(10+(rand()&7)):(10-(rand()&7)))_((rem(rand(),13))>6?(9+(rand()&7)):(9-(rand()&7)))_((rem(rand(),13))>6?(50+rem(rand(),25)):(50-rem(rand(),25))).csv" \
        u 2:xtic(1) title "glutons", \
     "cycles_((rem(rand(),13))>6?(10+(rand()&7)):(10-(rand()&7)))_((rem(rand(),13))>6?(50+rem(rand(),25)):(50-rem(rand(),25)))_((rem(rand(),13))>6?(50+rem(rand(),25)):(50-rem(rand(),25))).csv" \
        u 2:xtic(1) title "hungry thinkers", \
