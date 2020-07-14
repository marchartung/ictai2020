#!/bin/gnuplot

set datafile separator ","

set key inside left top
#unset key

#set terminal pngcairo size 512,512 enhanced
#set output 'pcm_all.png'
set size square

set xrange[100:100000]
set yrange[100:100000]

set logscale xy
set xtics 1,100,50000
set ytics 1,100,50000

set xlabel "Time verifiying DRAT in s"
set ylabel "Online trace overhead"

f(x)=x

plot 'ana_std_disk_overhead.csv' using 2:1 ps 1 pt 1 title "unsat", f(x) lc black title ""


pause -1
