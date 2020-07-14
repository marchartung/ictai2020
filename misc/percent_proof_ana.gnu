#!/bin/gnuplot
set key inside left top

set datafile separator ","

set logscale y
set yrange[10:100000]

set term pdfcairo color enhanced size 13cm,9cm font 'Helvetica,19'
#set output sprintf('|ps2pdf - %s', outname)
set output sprintf('%s', outname)
set xlabel 'solved instances' offset 0,0.5,0
set ylabel '% proof verification' offset 2,0,0

set datafile separator ","
plot  "ana_percent_proof.csv" u 1:($2+$3+$4) title "verify LRAT" w filledcurves x1, "ana_percent_proof.csv" u 1:($2+$3) title "generate LRAT" w filledcurves x1, "ana_percent_proof.csv" u 1:($2) title "solve time" w filledcurves x1
