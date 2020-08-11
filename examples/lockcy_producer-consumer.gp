# Check LICENSE file for copyright and license details.

set terminal png
set output 'lockcy_producer-consumer.png'

unset key
set style data boxplot
set style fill solid 0.25 border lt -1
set border 2 front linetype -1 linewidth 1.000
set boxwidth 0.5 absolute
set nozero
set tics in
set nogrid
set xtics nomirror ("2" 1, "4" 2, "8" 3, "16" 4, "32" 5, "64" 6)
set ytics nomirror
set size 1.0,1.0
set border 3 lw 2

set title "Cycles locked"
set ylabel "cycles"
set xlabel "processors"

system('./lockcy_helper.awk lockcy_10_13.csv lockcy_\(\(i\&1\)\=\=0\?9+\(randu\(\)%6\)\:9-\(randu\(\)%6\)\)_1+\(randu\(\)\&1023\).csv lockcy_1+\(randu\(\)\&1023\)_1+\(randu\(\)\&1023\).csv lockcy_1+\(randu\(\)\&1023\)_\(\(item\&1\)\=\=0\?9+\(randu\(\)%6\)\:9-\(randu\(\)%6\)\).csv')

plot "lockcy_2p.dat" u (1):1,  \
     "lockcy_4p.dat" u (2):1,  \
     "lockcy_8p.dat" u (3):1,  \
     "lockcy_16p.dat" u (4):1, \
     "lockcy_32p.dat" u (5):1, \
     "lockcy_64p.dat" u (6):1, \

system('rm -f lockcy*.dat')
