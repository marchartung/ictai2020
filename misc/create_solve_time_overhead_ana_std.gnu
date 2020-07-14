#!/bin/gnuplot

set datafile separator ","
set yrange [-5000:15000]
plot "ana_std_cmp_solve_runtime.csv" u 1:2, "ana_std_cmp_solve_runtime.csv" u 1:3
pause -1
