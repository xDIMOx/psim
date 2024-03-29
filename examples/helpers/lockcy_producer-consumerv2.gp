# Check LICENSE file for copyright and license details.

set terminal png
set output 'lockcy_producer-consumerv2.png'

unset key
set style data boxplot
set style fill solid 0.25 border lt -1
set border 2 front linetype -1 linewidth 1.000
set boxwidth 0.5 absolute
set nozero
set tics in
set nogrid
set xtics nomirror ("4" 1, "8" 2, "16" 3, "32" 4, "64" 5)
set ytics nomirror
set size 1.0,1.0
set border 3 lw 2

set title "Cycles Sem\_P() takes to complete"
set ylabel "cycles"
set xlabel "processors"

system('${HELPERS}/lockcy_helper.awk "lockcy_10_13.csv" "lockcy_((val&1)==0?9+(rand()&7):9-(rand()&7))_1+(rand()&1023).csv" "lockcy_1+(rand()&1023)_1+(rand()&1023).csv" "lockcy_1+(rand()&1023)_((item&1)==0?9+(rand()&7):9-(rand()&7)).csv"')

plot "lockcy_4p.dat" u (1):1,  \
     "lockcy_8p.dat" u (2):1,  \
     "lockcy_16p.dat" u (3):1, \
     "lockcy_32p.dat" u (4):1, \
     "lockcy_64p.dat" u (5):1, \
