# Check LICENSE file for copyright and license details.

set terminal png
set output 'lockcy_producer-consumer.png'

unset key
set style data boxplot
set style fill solid 0.25 border lt -1
set border 2 front linetype -1 linewidth 1.000
set boxwidth 0.5 relative
set nozero
set tics in
set nogrid
set xrange [1:128]
set logscale x 2
set xtics nomirror
set ytics nomirror
set size 1.0,1.0
set border 3 lw 2

set title "Cycles Sem_P() takes to complete"
set ylabel "cycles"
set xlabel "processors"

system('${HELPERS}/lockcy_helper.awk "lockcy_10_13.csv" "lockcy_((i&1)==0?9+(rand()&7):9-(rand()&7))_1+(rand()&1023).csv" "lockcy_1+(rand()&1023)_1+(rand()&1023).csv" "lockcy_1+(rand()&1023)_((item&1)==0?9+(rand()&7):9-(rand()&7)).csv"')

plot "lockcy_2p.dat" u (2):1:(2*0.5),  \
     "lockcy_4p.dat" u (4):1:(4*0.5),  \
     "lockcy_8p.dat" u (8):1:(8*0.5),  \
     "lockcy_16p.dat" u (16):1:(16*0.5), \
     "lockcy_32p.dat" u (32):1:(32*0.5), \
     "lockcy_64p.dat" u (64):1:(64*0.5), \
