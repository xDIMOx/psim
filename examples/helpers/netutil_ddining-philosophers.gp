# Check LICENSE file for copyright and license details.

set terminal png
set output 'netutil_ddining-philosophers.png'

set datafile separator ','
set nozero
set tics in
set nogrid
set xtics nomirror
set ytics nomirror
set size 1.0,1.0
set border 3 lw 2
set key center top
set style data histograms
set style fill solid
set boxwidth 0.5

set ylabel "\\%"
set xlabel "processors"

plot "netutil_10_((rem(rand(),13))>6?(9+(rand()&7)):(9-(rand()&7)))_((rem(rand(),13))>6?(9+(rand()&7)):(9-(rand()&7))).csv" \
        u 2:xtic(1) title "fixed ideas", \
     "netutil_((rem(rand(),13))>6?(10+(rand()&7)):(10-(rand()&7)))_((rem(rand(),13))>6?(50+rem(rand(),25)):(50-rem(rand(),25)))_((rem(rand(),13))>6?(9+(rand()&7)):(9-(rand()&7))).csv" \
        u 2:xtic(1) title "thinkers", \
     "netutil_((rem(rand(),13))>6?(10+(rand()&7)):(10-(rand()&7)))_((rem(rand(),13))>6?(9+(rand()&7)):(9-(rand()&7)))_((rem(rand(),13))>6?(50+rem(rand(),25)):(50-rem(rand(),25))).csv" \
        u 2:xtic(1) title "glutons", \
     "netutil_((rem(rand(),13))>6?(10+(rand()&7)):(10-(rand()&7)))_((rem(rand(),13))>6?(50+rem(rand(),25)):(50-rem(rand(),25)))_((rem(rand(),13))>6?(50+rem(rand(),25)):(50-rem(rand(),25))).csv" \
        u 2:xtic(1) title "hungry thinkers", \
