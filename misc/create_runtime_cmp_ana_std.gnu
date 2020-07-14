#!/bin/gnuplot

set datafile separator ","
plot  "ana_cmp_runtime.csv" u 1:2, "std_cmp_runtime.csv" u 1:2, "pico_cmp_runtime.csv" u 1:2
pause -1
