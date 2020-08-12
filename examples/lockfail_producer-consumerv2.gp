# Check LICENSE file for copyright and license details.

set terminal png
set output 'lockfail_producer-consumer.png'

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

set title "Locking failures"
set ylabel "accesses"
set xlabel "processors"

system('./lockfail_helper.awk lockfail_10_13.csv lockfail_\(\(val\&1\)\=\=0\?9+\(randu\(\)%6\)\:9-\(randu\(\)%6\)\)_1+\(randu\(\)\&1023\).csv lockfail_1+\(randu\(\)\&1023\)_1+\(randu\(\)\&1023\).csv lockfail_1+\(randu\(\)\&1023\)_\(\(item\&1\)\=\=0\?9+\(randu\(\)%6\)\:9-\(randu\(\)%6\)\).csv')

plot "lockfail_4p.dat" u (1):1,  \
     "lockfail_8p.dat" u (2):1,  \
     "lockfail_16p.dat" u (3):1, \
     "lockfail_32p.dat" u (4):1, \
     "lockfail_64p.dat" u (5):1, \

system('rm -f lockfail*.dat')
