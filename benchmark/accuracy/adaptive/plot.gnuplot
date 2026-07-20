# plot.gnuplot: render the accuracy-vs-compute-time Pareto curves for the adaptive
#               real oracles from the characterize tool's CSV (issue #1040).
#
# Usage:
#   ./characterize 6 9 > adaptive.csv          # generate a fuller sweep
#   gnuplot -e "DATA='adaptive.csv'" plot.gnuplot
#
# Produces adaptive_pareto.png: correct decimal digits (y) vs time per evaluation
# (x, log scale), one point series per type. Rows with correct_digits < 0
# (arithmetic, no reference) are dropped by the column-6 filter.

if (!exists("DATA")) DATA = 'adaptive.csv'

set datafile separator ","
set datafile commentschars "#"

set terminal pngcairo size 900,600 enhanced
set output 'adaptive_pareto.png'

set title "adaptive-precision oracles: accuracy vs compute time (issue #1040)"
set xlabel "time per evaluation (ns, log scale)"
set ylabel "correct decimal digits"
set logscale x
set grid
set key left top

# column layout: 1=type 2=FpType 3=function 4=knob 5=time_ns 6=correct_digits
plot \
  DATA using (stringcolumn(1) eq "elreal" && $6 >= 0 ? $5 : 1/0):6 \
       with points pt 7 ps 1.2 lc rgb "#1f77b4" title "elreal (depth sweep)", \
  DATA using (stringcolumn(1) eq "ereal"  && $6 >= 0 ? $5 : 1/0):6 \
       with points pt 5 ps 1.2 lc rgb "#d62728" title "ereal (limb sweep)"
