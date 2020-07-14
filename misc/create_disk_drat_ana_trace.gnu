#!/bin/gnuplot

set datafile separator ","

set key inside right bottom
#unset key


set term pdfcairo color enhanced size 10cm,9cm font 'Helvetica,15'
set output sprintf('%s', outname)

set size square
set grid

set xrange[10:100000]
set yrange[10:100000]

set logscale xy
set xtics 10,10,100000
set ytics 10,10,100000

set xlabel "Base DRAT memory in MB (log-scale)"
set ylabel "PTB memory utilisation in MB (logs-cale)"

f(x)=x
f2(x)=12*x

plot 'ana_std_disk_overhead.csv' using 3:2 ps 1 pt 1 title "", f(x) lc black title "x = y", f2(x) lc rgb "orange" title "12x overhead (average)"


#pause -1
