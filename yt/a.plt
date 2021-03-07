set datafile separator "| "

set xdata time
set timefmt "%Y-%m-%d %H:%M:%S"

set format x "%Y-%m-%d %H:%M:%S"
set xtics rotate

#plot '< cat -' using 1:3 t 'view count' w lp
plot 'x' using 1:3 t 'view count' w lp
pause -1
