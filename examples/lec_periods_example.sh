#!/bin/bash


# Following example performancs the following tests
# test 1 - this tests a run using periods but with each period having equal weight - the results of this should very close to not using any periods.bin file at all.
# test 2 = In this test period 1 is deleted and period 25 is set to 0.0002 i.e has the accumulated weighting of period 1 and 25
# test 3 = In this test period 1 and 2 are deleted and period 25 is set to 0.0003 i.e has the accumulated weighting of period 1, 2  and 25
#
# The effects of these weighting changes can be seen for instance on lines 746 onwards in the fm_lec_full_uncertainty_occx.csv files
# Similary the effects of weighting on fm_lec_uncertainty_aggx.csv can be seen from line 759 onwards
#


dorun()
{
	echo "row count $(wc -l input/periods.csv)"

	echo "Checksum (should equal 1) : $(cat input/periods.csv | awk '{ sum+=$2 } ; END {print sum }')"

    if [ -f /tmp/foo.txt ]; then
	    ../src/periodstobin/periodstobin < input/periods.csv > input/periods.bin
    fi

	./leccalc_example.py

	# avoid race condition which occurs in Linux under windows
	# keep this in until microsft fixes it

	sleep 1

	mv "results/lec/fm_lec_full_uncertainty_agg.csv" "results/lec/periods/fm_lec_full_uncertainty_agg$1.csv"
	mv "results/lec/fm_lec_full_uncertainty_occ.csv" "results/lec/periods/fm_lec_full_uncertainty_occ$1.csv"
	mv "results/lec/fm_lec_sample_mean_agg.csv" "results/lec/periods/fm_lec_sample_mean_agg$1.csv"
	mv "results/lec/fm_lec_sample_mean_occ.csv" "results/lec/periods/fm_lec_sample_mean_occ$1.csv"

	sleep 1

}

mkdir -p results/lec/periods

rm -f input/periods.bin
rm -f input/periods.csv

dorun 0

seq 1 10000 | awk 'BEGIN{ print "period_no, weighting" };{print $1",", 0.0001}'  > input/periods.csv
dorun 1

seq 2 10000 | awk 'BEGIN{ print "period_no, weighting" };{print $1",", 0.0001}'  | sed "/^25, 0.0001/c25, 0.0002"  > input/periods.csv
dorun 2

seq 3 10000 | awk 'BEGIN{ print "period_no, weighting" };{print $1",", 0.0001}'  | sed "/^25, 0.0001/c25, 0.0003"  > input/periods.csv
dorun 3

seq 24 10000 | awk 'BEGIN{ print "period_no, weighting" };{print $1",", 0.0001}'  | sed "/^25, 0.0001/c25, 0.0024"  > input/periods.csv
dorun 4

#sha1sum -c ctrl.sha1

# cleanup
rm -f input/periods.bin
rm -f input/periods.csv
