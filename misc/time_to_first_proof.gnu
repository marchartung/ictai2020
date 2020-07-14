#!/bin/gnuplot
set key inside left top

set datafile separator ","

set term pdfcairo color enhanced size 13cm,9cm font 'Helvetica,15'
set output sprintf('%s', outname)
set label "▷ lower is better" rotate by -90 left at graph 1.03,1 font ",15"
set xlabel 'solved instances' offset 0,0.5,0
set ylabel 'TNSV in hours' offset 2,0,0

set grid
set datafile separator ","
plot  "std_first_proof.csv" u 1:2 title "DRAT" w linespoints, "ana_first_proof.csv" u 1:2 title "direct LRAT" w linespoints, "pico_first_proof.csv" u 1:2 title "PicoSat" w linespoints lc rgb "red"
