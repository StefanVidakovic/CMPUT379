set term png
set output "test.png"
set title "Working Set Size of Merge Sort Over Time"
set xlabel "Time"
set ylabel "Working Set Size [number of unique pages x 10^8]"
set key off
set tics font ",11"
plot '/dev/stdin' using 0:($1/100000000) with lines
