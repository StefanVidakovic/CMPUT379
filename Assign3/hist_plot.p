
n=10000000
max=3100000000.
min=3000000000.
width=(max-min)/n

hist(x,width)=width*floor(x/width)+width/2.0
set term png
set output "histogram.png"
set xrange [min:max]
set yrange [0:]

set offset graph 0.05,0.05,0.05,0.0
set xtics 10000000
set boxwidth width*0.9
set style fill solid 0.5 #fillstyle
set tics out nomirror
set xlabel "x"
set ylabel "Frequency"
set tics font ",8"

plot "/dev/stdin" u (hist($1,width)):(1.0) smooth freq w boxes lc rgb"green" notitle
