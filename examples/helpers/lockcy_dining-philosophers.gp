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
set xtics nomirror ("5" 1, "8" 2, "10" 3, "16" 4, "20" 5, "32" 6)
set ytics nomirror
set size 1.0,1.0
set border 3 lw 2

set title "Cycles locked"
set ylabel "cycles"
set xlabel "processors"

system('${HELPERS}/lockcy_helper.awk "lockcy_((randu()&3)==1?(10+rem(randu(),5)):(10-rem(randu(),5)))_((randu()&3)==1?(50+rem(randu(),25)):(50-rem(randu(),25)))_((randu()&3)==1?(50+rem(randu(),25)):(50-rem(randu(),25))).csv" "lockcy_((randu()&3)==1?(10+rem(randu(),5)):(10-rem(randu(),5)))_((randu()&3)==1?(50+rem(randu(),25)):(50-rem(randu(),25)))_((randu()&3)==1?(10+rem(randu(),5)):(10-rem(randu(),5))).csv" "lockcy_((randu()&3)==1?(10+rem(randu(),5)):(10-rem(randu(),5)))_((randu()&3)==1?(10+rem(randu(),5)):(10-rem(randu(),5)))_((randu()&3)==1?(50+rem(randu(),25)):(50-rem(randu(),25))).csv" "lockcy_((randu()&3)==1?(10+rem(randu(),5)):(10-rem(randu(),5)))_((randu()&3)==1?(50+rem(randu(),25)):(50-rem(randu(),25)))_((randu()&3)==1?(50+rem(randu(),25)):(50-rem(randu(),25))).csv"')

plot "lockcy_5p.dat" u (1):1,  \
     "lockcy_8p.dat" u (2):1,  \
     "lockcy_10p.dat" u (3):1, \
     "lockcy_16p.dat" u (4):1, \
     "lockcy_20p.dat" u (5):1, \
     "lockcy_32p.dat" u (6):1, \
