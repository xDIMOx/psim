# Check LICENSE file for copyright and license details.

set terminal png
set output 'lockfail_2p_producer-consumer.png'

unset key
set style data boxplot
set style fill solid 0.25 border lt -1
set border 2 front linetype -1 linewidth 1.000
set boxwidth 0.5 absolute
set nozero
set tics in
set nogrid
set xtics nomirror ("2" 1)
set ytics nomirror
set size 1.0,1.0
set border 3 lw 2

set title "Locking failures"
set ylabel "accesses"
set xlabel "processors"

system('./lockfail_helper.awk "lockfail_10_13.csv" "lockfail_((i&1)==0?9+rem(randu(),7):9-rem(randu(),7))_1+(randu()&1023).csv" "lockfail_1+(randu()&1023)_1+(randu()&1023).csv" "lockfail_1+(randu()&1023)_((item&1)==0?9+rem(randu(),7):9-rem(randu(),7)).csv"')

plot "lockfail_2p.dat" u (1):1

system('rm -f lockfail*.dat')
