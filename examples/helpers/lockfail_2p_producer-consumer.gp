# Check LICENSE file for copyright and license details.

set terminal png
set output 'lockfail_2p_producer-consumer.png'

unset key
set style data boxplot
set style fill solid 0.25 border lt -1
set border 2 front linetype -1 linewidth 1.000
set boxwidth 0.5 relative
set nozero
set tics in
set nogrid
set xtics nomirror ("2" 2)
set ytics nomirror
set size 1.0,1.0
set border 3 lw 2

set title "Locking failures (2 processors)"
set ylabel "accesses"
set xlabel "processors"

system('${HELPERS}/lockfail_helper.awk "lockfail_10_13.csv" "lockfail_((i&1)==0?9+(rand()&7):9-(rand()&7))_1+(rand()&1023).csv" "lockfail_1+(rand()&1023)_1+(rand()&1023).csv" "lockfail_1+(rand()&1023)_((item&1)==0?9+(rand()&7):9-(rand()&7)).csv"')

plot "lockfail_2p.dat" u (2):1:(2*0.5)
