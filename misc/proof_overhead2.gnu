#!/bin/gnuplot
set key inside left top

set datafile separator ","

set term pdfcairo color enhanced size 13cm,9cm font 'Helvetica,15'
set output sprintf('%s', outname)

set label "▷ lower is better" rotate by -90 left at graph 1.03,1 font ",15"
set grid
set xrange[1:100]
set logscale y
set yrange[1:100000]
set ylabel 'time in seconds (log-scale)' offset 2,0,0

plot  "cmp_proof.csv" u 1:($6) title "Base DRAT" w l ,"cmp_proof.csv" u 1:($3+$4+$5) title "PTB LRAT" w filledcurves x1, "cmp_proof.csv" u 1:($3+$4) title "PTB solve overhead" w filledcurves x1

