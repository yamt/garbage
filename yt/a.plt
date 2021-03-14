set datafile separator "| "

set xdata time
set timefmt "%Y-%m-%d %H:%M:%S"

set format x "%Y-%m-%d %H:%M:%S"
set xtics rotate

set xrange ["2021-03-03":"2021-03-19"]
set yrange [490000:630000]
set ytics 490000,4000

#plot '< cat -' using 1:3 t 'view count' w lp
plot 'x' using 1:3 t 'view count' w lp
pause -1
