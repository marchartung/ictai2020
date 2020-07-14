#!/bin/gnuplot
set key inside left top

set datafile separator ","

set term pdfcairo color enhanced size 13cm,9cm font 'Helvetica,19'
set output sprintf('%s', outname)

set multiplot layout 2,1
set tmargin at screen 0.9; set bmargin at screen 0.50
#unset xtics 

set xrange[1:100]
set logscale y
set yrange[1:100000]
set ylabel '% proof verification' offset 2,0,0

plot  "cmp_proof.csv" u 1:($5) title "verify DRAT" w filledcurves x1

set tmargin at screen 0.50; set bmargin at screen 0.1
set yrange[100000:1]
set logscale y
set key outside center bottom

xcut(x)=(x < 1) ? 1 : x

plot 1 w filledcurves x1 title "verify LRAT" , "cmp_proof.csv" u 1:(xcut($3)) title "generate LRAT" w filledcurves x1 , "cmp_proof.csv" u 1:(xcut($3+$4)) title "" w filledcurves x1 lc rgb "white"
