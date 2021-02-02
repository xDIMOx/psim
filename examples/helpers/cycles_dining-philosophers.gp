# Check LICENSE file for copyright and license details.

set terminal png
set output 'cycles_dining-philosophers.png'

set datafile separator ','
set logscale x
set nozero
set tics in
set nogrid
set xrange [4:40]
set xtics nomirror ("5" 5, "8" 8, "10" 10, "16" 16, "20" 20, "32" 32)
set ytics nomirror
set size 1.0,1.0
set border 3 lw 2
set key tmargin

set title "Cycles to execute"
set ylabel "cycles"
set xlabel "processors"

plot "cycles_10_((randu()&3)==1?(50+rem(randu(),25)):(50-rem(randu(),25)))_((randu()&3)==1?(10+rem(randu(),5)):(10-rem(randu(),5))).csv" u 1:2 w linesp title "static", \
     "cycles_((randu()&3)==1?(10+rem(randu(),5)):(10-rem(randu(),5)))_((randu()&3)==1?(50+rem(randu(),25)):(50-rem(randu(),25)))_((randu()&3)==1?(10+rem(randu(),5)):(10-rem(randu(),5))).csv" \
         u 1:2 w linesp title "low", \
     "cycles_((randu()&3)==1?(10+rem(randu(),5)):(10-rem(randu(),5)))_((randu()&3)==1?(10+rem(randu(),5)):(10-rem(randu(),5)))_((randu()&3)==1?(50+rem(randu(),25)):(50-rem(randu(),25))).csv" \
     u 1:2 w linesp title "high", \
     "cycles_((randu()&3)==1?(10+rem(randu(),5)):(10-rem(randu(),5)))_((randu()&3)==1?(50+rem(randu(),25)):(50-rem(randu(),25)))_((randu()&3)==1?(50+rem(randu(),25)):(50-rem(randu(),25))).csv" \
     u 1:2 w linesp title "high 2", \
