#!/bin/bash


# Following example performancs the following tests
# test 1 - this tests a run using periods but with each period having equal weight - the results of this should very close to not using any periods.bin file at all.
# test 2 = In this test period 24 is deleted and period 25 is set to 0.0002 i.e has the accumulated total of period 24 and 25
# test 3 = In this test period 23 and 24 is deleted and period 25 is set to 0.0003 i.e has the accumulated total of period 23, 24 and 25
# 
# 
# 
# 


dorun()
{
	echo "input/periods.csv  row count "
	wc input/periods.csv 

	echo "Checksum"
	cat input/periods.csv | awk '{ sum+=$2 } ; END {print sum }'


	../src/periodstobin/periodstobin < input/periods.csv > input/periods.bin
	./leccalc_example.py

	# avoid race condition which occurs in Linux under windows 
	# keep this in until microsft fixes it

	sleep 1
	mv "results/lec/fm_lec_full_uncertainty_agg.csv" "results/lec/fm_lec_full_uncertainty_agg$1.csv"
	mv "results/lec/fm_lec_full_uncertainty_occ.csv" "results/lec/fm_lec_full_uncertainty_occ$1.csv"
	sleep 1

	diff "results/lec/test$1/fm_lec_full_uncertainty_agg.csv" "results/lec/fm_lec_full_uncertainty_agg$1.csv"
	diff "results/lec/test$1/fm_lec_full_uncertainty_occ.csv" "results/lec/fm_lec_full_uncertainty_occ$1.csv"


}



seq 1 10000 | awk 'BEGIN{ print "period_no, weighting" };{print $1",", 0.0001}'  > input/periods.csv 

dorun 1

seq 1 10000 | sed '/^24$/d' | awk 'BEGIN{ print "period_no, weighting" };{print $1",", 0.0001}'  | sed "/^25, 0.0001/c25, 0.0002"  > input/periods.csv 

dorun 2

seq 1 10000 | sed '/^24$/d' | sed '/^23$/d'  | awk 'BEGIN{ print "period_no, weighting" };{print $1",", 0.0001}'  | sed "/^25, 0.0001/c25, 0.0003"  > input/periods.csv 

dorun 3


# cleanup
rm input/periods.bin
rm input/periods.csv
