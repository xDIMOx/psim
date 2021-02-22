# Check LICENSE file for copyright and license details.

set terminal png
set output 'cw_producer-consumerv2.png'

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

set title "Cycles locked"
set ylabel "cycles"
set xlabel "processors"

system('${HELPERS}/cw_helper.awk "cw_10_13.csv" "cw_((i&1)==0?9+rem(randu(),7):9-rem(randu(),7))_1+(randu()&1023).csv" "cw_1+(randu()&1023)_1+(randu()&1023).csv" "cw_1+(randu()&1023)_((item&1)==0?9+rem(randu(),7):9-rem(randu(),7)).csv"')

plot "cw_4p.dat" u (1):1,  \
     "cw_8p.dat" u (2):1,  \
     "cw_16p.dat" u (3):1, \
     "cw_32p.dat" u (4):1, \
     "cw_64p.dat" u (5):1, \
