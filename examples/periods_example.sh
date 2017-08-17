#!/bin/bash


seq 1 10000 | awk 'BEGIN{ print "period_no, weighting" };{print $1",", 0.0001}'  > input/periods.csv 

cat input/periods.csv | awk '{ sum+=$2 } ; END {print sum }'

../src/periodstobin/periodstobin < input/periods.csv > input/periods.bin

./leccalc_example.py


diff results/lec/test1/fm_lec_full_uncertainty_agg.csv results/lec/fm_lec_full_uncertainty_agg.csv 
diff results/lec/test1/fm_lec_full_uncertainty_occ.csv results/lec/fm_lec_full_uncertainty_occ.csv 
