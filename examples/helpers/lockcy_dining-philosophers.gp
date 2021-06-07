# Check LICENSE file for copyright and license details.

set terminal png
set output 'lockcy_dining-philosophers.png'

unset key
set style data boxplot
set style fill solid 0.25 border lt -1
set border 2 front linetype -1 linewidth 1.000
set boxwidth 0.5 absolute
set nozero
set tics in
set nogrid
set xtics nomirror
set ytics nomirror
set size 1.0,1.0
set border 3 lw 2

set title "Cycles locked"
set ylabel "cycles"
set xlabel "processors"

system('${HELPERS}/lockcy_helper.awk "lockcy_10_((rem(rand(),13))>6?(9+(rand()&7)):(9-(rand()&7)))_((rem(rand(),13))>6?(9+(rand()&7)):(9-(rand()&7))).csv" "lockcy_((rem(rand(),13))>6?(10+(rand()&7)):(10-(rand()&7)))_((rem(rand(),13))>6?(50+rem(rand(),25)):(50-rem(rand(),25)))_((rem(rand(),13))>6?(9+(rand()&7)):(9-(rand()&7))).csv" "lockcy_((rem(rand(),13))>6?(10+(rand()&7)):(10-(rand()&7)))_((rem(rand(),13))>6?(9+(rand()&7)):(9-(rand()&7)))_((rem(rand(),13))>6?(50+rem(rand(),25)):(50-rem(rand(),25))).csv" "lockcy_((rem(rand(),13))>6?(10+(rand()&7)):(10-(rand()&7)))_((rem(rand(),13))>6?(50+rem(rand(),25)):(50-rem(rand(),25)))_((rem(rand(),13))>6?(50+rem(rand(),25)):(50-rem(rand(),25))).csv"')

plot "lockcy_5p.dat" u (5):1
