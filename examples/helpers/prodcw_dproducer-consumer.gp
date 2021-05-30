# Check LICENSE file for copyright and license details.

set terminal png
set output 'prodcw_dproducer-consumer.png'

unset key
set style data boxplot
set style fill solid 0.25 border lt -1
set border 2 front linetype -1 linewidth 1.000
set boxwidth 0.5 absolute
set nozero
set tics in
set nogrid
set xtics nomirror ("3" 1, "4" 2, "8" 3, "16" 4, "32" 5, "64" 6)
set ytics nomirror
set size 1.0,1.0
set border 3 lw 2

set ylabel "cycles"
set xlabel "processors"

system('${HELPERS}/cw_helper.awk "cw_10_13.csv" "cw_((i&1)==0?9+(rand()&7):9-(rand()&7))_1+(rand()&1023).csv" "cw_1+(rand()&1023)_1+(rand()&1023).csv" "cw_1+(rand()&1023)_((item&1)==0?9+(rand()&7):9-(rand()&7)).csv"')

plot "prod_cw_3p.dat" u (1):1,  \
     "prod_cw_4p.dat" u (2):1,  \
     "prod_cw_8p.dat" u (3):1,  \
     "prod_cw_16p.dat" u (4):1, \
     "prod_cw_32p.dat" u (5):1, \
     "prod_cw_64p.dat" u (6):1, \
