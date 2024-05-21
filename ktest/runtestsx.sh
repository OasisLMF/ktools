#!/bin/sh

init()
{	
	if [ ! -f ../src/eve/eve ]; then
		if [ ! -f ../src/eve/eve.exe ]; then
	    		echo "Please run make all before make check"
	    		exit
		fi
	fi
	mkdir -p testout
	cd ..
	if [ ! -d examples ]; then
		tar -xzf examples.tar.gz
	fi
	mkdir -p examples/work/gul1/summary
	mkdir -p examples/work/gul2/summary
	mkdir -p examples/work/fm1/summary
	mkdir -p examples/work/fm2/summary
	mkdir -p examples/work/gul1/aal
	mkdir -p examples/work/gul2/aal
	mkdir -p examples/work/fm1/aal
	mkdir -p examples/work/fm2/aal

}

fin()
{

	rm -rf examples/work
}

installertest()
{
	CTRL=ctrl
	CTRL_PARQUET=ctrl_parquet
	TESTS_PASS=1
	cd examples

	# establish whether binaries have been linked to parquet libraries
	PARQUET_OUTPUT=0
	if ../src/katparquet/katparquet -v 2>&1 | grep -q 'Parquet output enabled'; then
		PARQUET_OUTPUT=1
	fi

	# test eve
	../src/eve/eve -n 1 2 > ../ktest/testout/eveout1.bin
	../src/eve/eve -n 2 2 > ../ktest/testout/eveout2.bin

	# # test getmodel
	 ../src/eve/eve -n  1 1 | ../src/getmodel/getmodel > ../ktest/testout/getmodelout.bin
	
	# test gulcalc item stream and coverage stream
	../src/eve/eve -n 1 1 | ../src/getmodel/getmodel | ../src/gulcalc/gulcalc -S100 -L0.1 -r -a0 -i - > ../ktest/testout/gulcalci.bin
	../src/eve/eve -n 1 1 | ../src/getmodel/getmodel | ../src/gulcalc/gulcalc -S100 -L0.1 -r -c - > ../ktest/testout/gulcalcc.bin

	# test placalc
	../src/placalc/placalc < ../ktest/testout/gulcalci.bin > ../ktest/testout/placalci.bin
	../src/placalc/placalc -f 0.75 < ../ktest/testout/gulcalci.bin > ../ktest/testout/placalci_relf.bin
	../src/placalc/placalc -F 0.75 < ../ktest/testout/gulcalci.bin > ../ktest/testout/placalci_absf.bin

	# test fmcalc
	 ../src/fmcalc/fmcalc > ../ktest/testout/fmcalc.bin < ../ktest/testout/gulcalci.bin
	
	# test summary samples
	 ../src/summarycalc/summarycalc -i -1 ../ktest/testout/gulsummarycalc1.bin  < ../ktest/testout/gulcalci.bin  
	 ../src/summarycalc/summarycalc -i -2 ../ktest/testout/gulsummarycalc2.bin  < ../ktest/testout/gulcalci.bin  
	 ../src/summarycalc/summarycalc -f -1 ../ktest/testout/fmsummarycalc1.bin   < ../ktest/testout/fmcalc.bin
	 ../src/summarycalc/summarycalc -f -2 ../ktest/testout/fmsummarycalc2.bin   < ../ktest/testout/fmcalc.bin

  	# test selt
  	../src/summarycalctocsv/summarycalctocsv -o > ../ktest/testout/gulselt1.csv < ../ktest/testout/gulsummarycalc1.bin
	../src/summarycalctocsv/summarycalctocsv -o > ../ktest/testout/gulselt2.csv < ../ktest/testout/gulsummarycalc2.bin
	if [ ${PARQUET_OUTPUT} -eq 1 ]; then
  		../src/summarycalctocsv/summarycalctocsv -p ../ktest/testout/gulselt1.parquet < ../ktest/testout/gulsummarycalc1.bin
		../src/summarycalctocsv/summarycalctocsv -p ../ktest/testout/gulselt2.parquet < ../ktest/testout/gulsummarycalc2.bin
	fi

	# test eltcalc
	../src/eltcalc/eltcalc < ../ktest/testout/gulsummarycalc1.bin > ../ktest/testout/gulelt1.csv
	../src/eltcalc/eltcalc < ../ktest/testout/gulsummarycalc2.bin > ../ktest/testout/gulelt2.csv
	../src/eltcalc/eltcalc < ../ktest/testout/fmsummarycalc1.bin > ../ktest/testout/fmelt1.csv
	../src/eltcalc/eltcalc < ../ktest/testout/fmsummarycalc2.bin > ../ktest/testout/fmelt2.csv

	# test melt qelt
	if [ ${PARQUET_OUTPUT} -eq 1 ]; then
		../src/eltcalc/eltcalc -m ../ktest/testout/gulmelt1.parquet -q ../ktest/testout/gulqelt1.parquet < ../ktest/testout/gulsummarycalc1.bin
		../src/eltcalc/eltcalc -m ../ktest/testout/gulmelt2.parquet -q ../ktest/testout/gulqelt2.parquet < ../ktest/testout/gulsummarycalc2.bin
	fi
	../src/eltcalc/eltcalc -M ../ktest/testout/gulmelt1.csv -Q ../ktest/testout/gulqelt1.csv < ../ktest/testout/gulsummarycalc1.bin
	../src/eltcalc/eltcalc -M ../ktest/testout/gulmelt2.csv -Q ../ktest/testout/gulqelt2.csv < ../ktest/testout/gulsummarycalc2.bin

	# test mplt qplt splt
	if [ ${PARQUET_OUTPUT} -eq 1 ]; then
		../src/pltcalc/pltcalc -m ../ktest/testout/gulmplt1.parquet -q ../ktest/testout/gulqplt1.parquet -s ../ktest/testout/gulsplt1.parquet < ../ktest/testout/gulsummarycalc1.bin
		../src/pltcalc/pltcalc -m ../ktest/testout/gulmplt2.parquet -q ../ktest/testout/gulqplt2.parquet -s ../ktest/testout/gulsplt2.parquet < ../ktest/testout/gulsummarycalc2.bin
	fi
	../src/pltcalc/pltcalc -M ../ktest/testout/gulmplt1.csv -Q ../ktest/testout/gulqplt1.csv -S ../ktest/testout/gulsplt1.csv < ../ktest/testout/gulsummarycalc1.bin
	../src/pltcalc/pltcalc -M ../ktest/testout/gulmplt2.csv -Q ../ktest/testout/gulqplt2.csv -S ../ktest/testout/gulsplt2.csv < ../ktest/testout/gulsummarycalc2.bin

	# test leccalc
	cp ../ktest/testout/gulsummarycalc2.bin work/gul2/summary/gulsummarycalc2.bin
	cp ../ktest/testout/fmsummarycalc2.bin work/fm2/summary/fmsummarycalc2.bin
	cp ../ktest/testout/gulsummarycalc1.bin work/gul1/summary/gulsummarycalc1.bin
	cp ../ktest/testout/fmsummarycalc1.bin work/fm1/summary/fmsummarycalc1.bin


	../src/leccalc/leccalc  -Kgul1/summary -F ../ktest/testout/gul_full_uncertainty_aep_1.csv -W ../ktest/testout/gul_wheatsheaf_aep_1.csv -S ../ktest/testout/gul_sample_mean_aep_1.csv -M ../ktest/testout/gul_wheatsheaf_mean_aep_1.csv  \
    									   -f ../ktest/testout/gul_full_uncertainty_oep_1.csv -w ../ktest/testout/gul_wheatsheaf_oep_1.csv -s ../ktest/testout/gul_sample_mean_oep_1.csv -m ../ktest/testout/gul_wheatsheaf_mean_oep_1.csv

	../src/leccalc/leccalc -Kfm1/summary -F ../ktest/testout/fm_full_uncertainty_aep_1.csv -W ../ktest/testout/fm_wheatsheaf_aep_1.csv -S ../ktest/testout/fm_sample_mean_aep_1.csv -M ../ktest/testout/fm_wheatsheaf_mean_aep_1.csv \
    									 -f ../ktest/testout/fm_full_uncertainty_oep_1.csv -w ../ktest/testout/fm_wheatsheaf_oep_1.csv -s ../ktest/testout/fm_sample_mean_oep_1.csv -m ../ktest/testout/fm_wheatsheaf_mean_oep_1.csv

	../src/leccalc/leccalc -r -Kgul1/summary -F ../ktest/testout/gul_full_uncertainty_aep_1_r.csv -W ../ktest/testout/gul_wheatsheaf_aep_1_r.csv -S ../ktest/testout/gul_sample_mean_aep_1_r.csv -M ../ktest/testout/gul_wheatsheaf_mean_aep_1_r.csv \
        									 -f ../ktest/testout/gul_full_uncertainty_oep_1_r.csv -w ../ktest/testout/gul_wheatsheaf_oep_1_r.csv -s ../ktest/testout/gul_sample_mean_oep_1_r.csv -m ../ktest/testout/gul_wheatsheaf_mean_oep_1_r.csv

	../src/leccalc/leccalc -r -Kfm1/summary -F ../ktest/testout/fm_full_uncertainty_aep_1_r.csv -W ../ktest/testout/fm_wheatsheaf_aep_1_r.csv -S ../ktest/testout/fm_sample_mean_aep_1_r.csv -M ../ktest/testout/fm_wheatsheaf_mean_aep_1_r.csv \
    									    -f ../ktest/testout/fm_full_uncertainty_oep_1_r.csv -w ../ktest/testout/fm_wheatsheaf_oep_1_r.csv -s ../ktest/testout/fm_sample_mean_oep_1_r.csv -m ../ktest/testout/fm_wheatsheaf_mean_oep_1_r.csv

	../src/leccalc/leccalc -r -Kgul2/summary -F ../ktest/testout/gul_full_uncertainty_aep_2_r.csv -W ../ktest/testout/gul_wheatsheaf_aep_2_r.csv -S ../ktest/testout/gul_sample_mean_aep_2_r.csv -M ../ktest/testout/gul_wheatsheaf_mean_aep_2_r.csv \
    									     -f ../ktest/testout/gul_full_uncertainty_oep_2_r.csv -w ../ktest/testout/gul_wheatsheaf_oep_2_r.csv -s ../ktest/testout/gul_sample_mean_oep_2_r.csv -m ../ktest/testout/gul_wheatsheaf_mean_oep_2_r.csv

	../src/leccalc/leccalc -r -Kfm2/summary -F ../ktest/testout/fm_full_uncertainty_aep_2_r.csv  -W ../ktest/testout/fm_wheatsheaf_aep_2_r.csv -S ../ktest/testout/fm_sample_mean_aep_2_r.csv -M ../ktest/testout/fm_wheatsheaf_mean_aep_2_r.csv \
    									    -f ../ktest/testout/fm_full_uncertainty_oep_2_r.csv -w ../ktest/testout/fm_wheatsheaf_oep_2_r.csv -s ../ktest/testout/fm_sample_mean_oep_2_r.csv -m ../ktest/testout/fm_wheatsheaf_mean_oep_2_r.csv

    # test ORD ept and psept 
 	../src/ordleccalc/ordleccalc  -Kgul1/summary -F -f -W -w -M -m -S -s -O ../ktest/testout/gul_ept_1.csv -o ../ktest/testout/gul_psept_1.csv
	 ../src/ordleccalc/ordleccalc  -Kgul2/summary -F -f -W -w -M -m -S -s -O ../ktest/testout/gul_ept_2.csv -o ../ktest/testout/gul_psept_2.csv
	 ../src/ordleccalc/ordleccalc  -Kfm1/summary -F -f -W -w -M -m -S -s -O ../ktest/testout/fm_ept_1.csv -o ../ktest/testout/fm_psept_1.csv
	 ../src/ordleccalc/ordleccalc  -Kfm2/summary -F -f -W -w -M -m -S -s -O ../ktest/testout/fm_ept_2.csv -o ../ktest/testout/fm_psept_2.csv
 	 ../src/ordleccalc/ordleccalc -r -Kgul1/summary -F -f -W -w -M -m -S -s -O ../ktest/testout/gul_ept_1_r.csv -o ../ktest/testout/gul_psept_1_r.csv
	 ../src/ordleccalc/ordleccalc -r -Kgul2/summary -F -f -W -w -M -m -S -s -O ../ktest/testout/gul_ept_2_r.csv -o ../ktest/testout/gul_psept_2_r.csv
	 ../src/ordleccalc/ordleccalc -r -Kfm1/summary -F -f -W -w -M -m -S -s -O ../ktest/testout/fm_ept_1_r.csv -o ../ktest/testout/fm_psept_1_r.csv
	 ../src/ordleccalc/ordleccalc -r -Kfm2/summary -F -f -W -w -M -m -S -s -O ../ktest/testout/fm_ept_2_r.csv -o ../ktest/testout/fm_psept_2_r.csv
	 if [ ${PARQUET_OUTPUT} -eq 1 ]; then
 		../src/ordleccalc/ordleccalc  -Kgul1/summary -F -f -W -w -M -m -S -s -P ../ktest/testout/gul_ept_1.parquet -p ../ktest/testout/gul_psept_1.parquet
	 	../src/ordleccalc/ordleccalc  -Kgul2/summary -F -f -W -w -M -m -S -s -P ../ktest/testout/gul_ept_2.parquet -p ../ktest/testout/gul_psept_2.parquet
	 	../src/ordleccalc/ordleccalc  -Kfm1/summary -F -f -W -w -M -m -S -s -P ../ktest/testout/fm_ept_1.parquet -p ../ktest/testout/fm_psept_1.parquet
	 	../src/ordleccalc/ordleccalc  -Kfm2/summary -F -f -W -w -M -m -S -s -P ../ktest/testout/fm_ept_2.parquet -p ../ktest/testout/fm_psept_2.parquet
 	 	../src/ordleccalc/ordleccalc -r -Kgul1/summary -F -f -W -w -M -m -S -s -P ../ktest/testout/gul_ept_1_r.parquet -p ../ktest/testout/gul_psept_1_r.parquet
	 	../src/ordleccalc/ordleccalc -r -Kgul2/summary -F -f -W -w -M -m -S -s -P ../ktest/testout/gul_ept_2_r.parquet -p ../ktest/testout/gul_psept_2_r.parquet
	 	../src/ordleccalc/ordleccalc -r -Kfm1/summary -F -f -W -w -M -m -S -s -P ../ktest/testout/fm_ept_1_r.parquet -p ../ktest/testout/fm_psept_1_r.parquet
	 	../src/ordleccalc/ordleccalc -r -Kfm2/summary -F -f -W -w -M -m -S -s -P ../ktest/testout/fm_ept_2_r.parquet -p ../ktest/testout/fm_psept_2_r.parquet
	 fi

	# test pltcalc
	../src/pltcalc/pltcalc < ../ktest/testout/gulsummarycalc1.bin > ../ktest/testout/gulplt1.csv
	../src/pltcalc/pltcalc < ../ktest/testout/gulsummarycalc2.bin > ../ktest/testout/gulplt2.csv
	../src/pltcalc/pltcalc < ../ktest/testout/fmsummarycalc1.bin > ../ktest/testout/fmplt1.csv
	../src/pltcalc/pltcalc < ../ktest/testout/fmsummarycalc2.bin > ../ktest/testout/fmplt2.csv

	# build indexs for the summaries
	# ../src/summaryindex/summaryindex -K gul1/summary
	# ../src/summaryindex/summaryindex -K gul2/summary
	# ../src/summaryindex/summaryindex -K fm1/summary
	# ../src/summaryindex/summaryindex -K fm2/summary

	# test aalcalc	
	../src/aalcalc/aalcalc -Kgul1/summary > ../ktest/testout/gulaalcalc1.csv
	../src/aalcalc/aalcalc -Kgul2/summary > ../ktest/testout/gulaalcalc2.csv
	../src/aalcalc/aalcalc -Kfm1/summary > ../ktest/testout/fmaalcalc1.csv
	../src/aalcalc/aalcalc -Kfm2/summary > ../ktest/testout/fmaalcalc2.csv

	# test alt	
	../src/aalcalc/aalcalc -o -Kgul1/summary -l 0.9 -c ../ktest/testout/gulcalt1.csv > ../ktest/testout/gulalt1.csv
	../src/aalcalc/aalcalc -o -Kgul2/summary -l 0.9 -c ../ktest/testout/gulcalt2.csv > ../ktest/testout/gulalt2.csv
	../src/aalcalc/aalcalc -o -Kfm1/summary -l 0.9 -c ../ktest/testout/fmcalt1.csv > ../ktest/testout/fmalt1.csv
	../src/aalcalc/aalcalc -o -Kfm2/summary -l 0.9 -c ../ktest/testout/fmcalt2.csv > ../ktest/testout/fmalt2.csv
	if [ ${PARQUET_OUTPUT} -eq 1 ]; then
		../src/aalcalc/aalcalc -p ../ktest/testout/gulalt1.parquet -Kgul1/summary -l 0.9 -c ../ktest/testout/gulcalt1.parquet
		../src/aalcalc/aalcalc -p ../ktest/testout/gulalt2.parquet -Kgul2/summary -l 0.9 -c ../ktest/testout/gulcalt2.parquet
		../src/aalcalc/aalcalc -p ../ktest/testout/fmalt1.parquet -Kfm1/summary -l 0.9 -c ../ktest/testout/fmcalt1.parquet
		../src/aalcalc/aalcalc -p ../ktest/testout/fmalt2.parquet -Kfm2/summary -l 0.9 -c ../ktest/testout/fmcalt2.parquet
	fi

	# test aalcalcmeanonly
	../src/aalcalcmeanonly/aalcalcmeanonly -Kgul1/summary > ../ktest/testout/gulaalcalcmeanonly1.csv
	../src/aalcalcmeanonly/aalcalcmeanonly -Kgul2/summary > ../ktest/testout/gulaalcalcmeanonly2.csv
	../src/aalcalcmeanonly/aalcalcmeanonly -Kfm1/summary > ../ktest/testout/fmaalcalcmeanonly1.csv
	../src/aalcalcmeanonly/aalcalcmeanonly -Kfm2/summary > ../ktest/testout/fmaalcalcmeanonly2.csv
	../src/aalcalcmeanonly/aalcalcmeanonly -o -Kgul1/summary > ../ktest/testout/gulaltmeanonly1.csv
	../src/aalcalcmeanonly/aalcalcmeanonly -o -Kgul2/summary > ../ktest/testout/gulaltmeanonly2.csv
	../src/aalcalcmeanonly/aalcalcmeanonly -o -Kfm1/summary > ../ktest/testout/fmaltmeanonly1.csv
	../src/aalcalcmeanonly/aalcalcmeanonly -o -Kfm2/summary > ../ktest/testout/fmaltmeanonly2.csv
	if [ ${PARQUET_OUTPUT} -eq 1 ]; then
		../src/aalcalcmeanonly/aalcalcmeanonly -p ../ktest/testout/gulaltmeanonly1.parquet -Kgul1/summary
		../src/aalcalcmeanonly/aalcalcmeanonly -p ../ktest/testout/gulaltmeanonly2.parquet -Kgul2/summary
		../src/aalcalcmeanonly/aalcalcmeanonly -p ../ktest/testout/fmaltmeanonly1.parquet -Kfm1/summary
		../src/aalcalcmeanonly/aalcalcmeanonly -p ../ktest/testout/fmaltmeanonly2.parquet -Kfm2/summary
	fi

	# test stream conversion components
	# stdout to csv
	../src/cdftocsv/cdftocsv  < ../ktest/testout/getmodelout.bin > ../ktest/testout/getmodelout.csv
	../src/gultocsv/gultocsv -f < ../ktest/testout/gulcalci.bin > ../ktest/testout/gulcalci.csv
	../src/gultocsv/gultocsv -f < ../ktest/testout/gulcalcc.bin > ../ktest/testout/gulcalcc.csv
	../src/gultocsv/gultocsv -f < ../ktest/testout/placalci.bin > ../ktest/testout/placalci.csv
	../src/gultocsv/gultocsv -f < ../ktest/testout/placalci_relf.bin > ../ktest/testout/placalci_relf.csv
	../src/gultocsv/gultocsv -f < ../ktest/testout/placalci_absf.bin > ../ktest/testout/placalci_absf.csv
	../src/fmtocsv/fmtocsv -f < ../ktest/testout/fmcalc.bin > ../ktest/testout/fmcalc.csv

	../src/summarycalctocsv/summarycalctocsv -f < ../ktest/testout/gulsummarycalc2.bin > ../ktest/testout/gulsummarycalc2.csv
	../src/summarycalctocsv/summarycalctocsv -f < ../ktest/testout/gulsummarycalc1.bin > ../ktest/testout/gulsummarycalc1.csv
	../src/summarycalctocsv/summarycalctocsv -f < ../ktest/testout/fmsummarycalc2.bin > ../ktest/testout/fmsummarycalc2.csv
	../src/summarycalctocsv/summarycalctocsv -f < ../ktest/testout/fmsummarycalc1.bin > ../ktest/testout/fmsummarycalc1.csv

	# input data to csv and bin
	../src/evetocsv/evetocsv < ../examples/input/events.bin | ../src/evetobin/evetobin > ../ktest/testout/events.bin
	
	../src/randtocsv/randtocsv -r < ../examples/static/random.bin | ../src/randtobin/randtobin > ../ktest/testout/random.bin

	 ../src/itemtocsv/itemtocsv < ../examples/input/items.bin | ../src/itemtobin/itemtobin > ../ktest/testout/items.bin

	 ../src/coveragetocsv/coveragetocsv < ../examples/input/coverages.bin | ../src/coveragetobin/coveragetobin > ../ktest/testout/coverages.bin

	 # damage bin dictionary, -N argument skips validation checks
	../src/damagebintocsv/damagebintocsv < ../examples/static/damage_bin_dict.bin | ../src/damagebintobin/damagebintobin -N > ../ktest/testout/damage_bin_dict.bin
	../src/damagebintocsv/damagebintocsv < ../examples/static/damage_bin_dict.bin | ../src/damagebintobin/damagebintobin > ../ktest/testout/damage_bin_dict_ctrl.bin 2> ../ktest/testout/damagebintobin_ctrl_stderr.out
	
	../src/fmprogrammetocsv/fmprogrammetocsv < ../examples/input/fm_programme.bin | ../src/fmprogrammetobin/fmprogrammetobin > ../ktest/testout/fm_programme.bin
	
	../src/fmprofiletocsv/fmprofiletocsv < ../examples/input/fm_profile.bin | ../src/fmprofiletobin/fmprofiletobin > ../ktest/testout/fm_profile.bin
	
	../src/fmpolicytctocsv/fmpolicytctocsv < ../examples/input/fm_policytc.bin | ../src/fmpolicytctobin/fmpolicytctobin > ../ktest/testout/fm_policytc.bin
	
	../src/fmxreftocsv/fmxreftocsv < ../examples/input/fm_xref.bin | ../src/fmxreftobin/fmxreftobin > ../ktest/testout/fm_xref.bin

	../src/gulsummaryxreftocsv/gulsummaryxreftocsv < ../examples/input/gulsummaryxref.bin | ../src/gulsummaryxreftobin/gulsummaryxreftobin > ../ktest/testout/gulsummaryxref.bin

	../src/fmsummaryxreftocsv/fmsummaryxreftocsv < ../examples/input/fmsummaryxref.bin | ../src/fmsummaryxreftobin/fmsummaryxreftobin > ../ktest/testout/fmsummaryxref.bin

    ../src/returnperiodtocsv/returnperiodtocsv < ../examples/input/returnperiods.bin | ../src/returnperiodtobin/returnperiodtobin > ../ktest/testout/returnperiods.bin

	../src/occurrencetocsv/occurrencetocsv < ../examples/input/occurrence.bin | ../src/occurrencetobin/occurrencetobin -P10000 > ../ktest/testout/occurrence.bin

	../src/quantiletocsv/quantiletocsv < ../examples/input/quantile.bin | ../src/quantiletobin/quantiletobin  > ../ktest/testout/quantile.bin

	../src/amplificationstocsv/amplificationstocsv < ../examples/input/amplifications.bin | ../src/amplificationstobin/amplificationstobin > ../ktest/testout/amplifications.bin

	../src/lossfactorstocsv/lossfactorstocsv < ../examples/static/lossfactors.bin | ../src/lossfactorstobin/lossfactorstobin > ../ktest/testout/lossfactors.bin

        ../src/aggregatevulnerabilitytocsv/aggregatevulnerabilitytocsv < ../examples/static/aggregate_vulnerability.bin | ../src/aggregatevulnerabilitytobin/aggregatevulnerabilitytobin > ../ktest/testout/aggregate_vulnerability.bin

        ../src/weightstocsv/weightstocsv < ../examples/static/weights.bin | ../src/weightstobin/weightstobin > ../ktest/testout/weights.bin

	# footprint to csv and bin
	cp static/footprint.bin ../ktest/testout/footprint.bin
	cp static/footprint.idx ../ktest/testout/footprint.idx
	cd ../ktest/testout
	../../src/footprinttocsv/footprinttocsv > footprint.csv
	mv footprint.bin footprintin.bin
	mv footprint.idx footprintin.idx
	../../src/footprinttobin/footprinttobin -i 121 -N < footprint.csv
	../../src/footprinttobin/footprinttobin -i 121 -N -z < footprint.csv
	../../src/footprinttobin/footprinttobin -i 121 -N -z -u -b footprint_usize.bin.z -x footprint_usize.idx.z < footprint.csv
	../../src/footprinttobin/footprinttobin -i 121 -b footprint_ctrl.bin -x footprint_ctrl.idx < footprint.csv 2> footprinttobin_ctrl_stderr.out

	# vulnerability to csv and bin
	../../src/vulnerabilitytocsv/vulnerabilitytocsv < ../../examples/static/vulnerability.bin > vulnerability.csv
	../../src/vulnerabilitytobin/vulnerabilitytobin -d 102 -N < vulnerability.csv > vulnerabilityin.bin
	../../src/vulnerabilitytobin/vulnerabilitytobin -d 102 -N -i < vulnerability.csv
	../../src/vulnerabilitytobin/vulnerabilitytobin -d 102 -N -z < vulnerability.csv
	../../src/vulnerabilitytobin/vulnerabilitytobin -d 102 < vulnerability.csv > vulnerability_ctrl.bin 2> vulnerabilitytobin_ctrl_stderr.out

	# Validation checks
	# Damage bin dictionary
	../../src/damagebintocsv/damagebintocsv < ../../examples/static/damage_bin_dict.bin | ../../src/validatedamagebin/validatedamagebin 2> validatedamagebin_ctrl_stderr.out
	echo "See ../../examples/validation/damage_bin_dict_testlist.txt for test details" > validatedamagebin_stderr.out
	for i in `seq -f "%02g" 1 10`; do
		echo Test $i >> validatedamagebin_stderr.out
		../../src/validatedamagebin/validatedamagebin < ../../examples/static/validation/damage_bin_dict_$i.csv 2>> validatedamagebin_stderr.out
		echo Exit code $? >> validatedamagebin_stderr.out
		echo >> validatedamagebin_stderr.out
	done
	echo "See ../../examples/validation/damage_bin_dict_testlist.txt for test details" > damagebintobin_stderr.out
	for i in `seq -f "%02g" 1 10`; do
		echo Test $i >> damagebintobin_stderr.out
		../../src/damagebintobin/damagebintobin < ../../examples/static/validation/damage_bin_dict_$i.csv > /dev/null 2>> damagebintobin_stderr.out
		echo Exit code $? >> damagebintobin_stderr.out
		echo >> damagebintobin_stderr.out
	done
	# Footprint
	../../src/validatefootprint/validatefootprint < footprint.csv 2> validatefootprint_ctrl_stderr.out
	echo "See ../../examples/validation/footprint_testlist.txt for test details" > validatefootprint_stderr.out
	for i in `seq -f "%02g" 1 13`; do
		echo Test $i >> validatefootprint_stderr.out
		../../src/validatefootprint/validatefootprint < ../../examples/static/validation/footprint_$i.csv 2>> validatefootprint_stderr.out
		echo Exit code $? >> validatefootprint_stderr.out
		echo >> validatefootprint_stderr.out
	done
	echo "See ../../examples/validation/footprint_testlist.txt for test details" > footprinttobin_stderr.out
	for i in `seq -f "%02g" 1 14`; do
		echo Test $i >> footprinttobin_stderr.out
		../../src/footprinttobin/footprinttobin -i 4 -b footprint_test_$i.bin -x footprint_test_$i.idx < ../../examples/static/validation/footprint_$i.csv 2>> footprinttobin_stderr.out
		echo Exit code $? >> footprinttobin_stderr.out
		echo >> footprinttobin_stderr.out
		rm footprint_test_$i.bin
		rm footprint_test_$i.idx
	done
	# Vulnerability
	../../src/validatevulnerability/validatevulnerability < vulnerability.csv 2> validatevulnerability_ctrl_stderr.out
	echo "See ../../examples/validation/vulnerability_testlist.txt for test details" > validatevulnerability_stderr.out
	for i in `seq -f "%02g" 1 12`; do
		echo Test $i >> validatevulnerability_stderr.out
		../../src/validatevulnerability/validatevulnerability < ../../examples/static/validation/vulnerability_$i.csv 2>> validatevulnerability_stderr.out
		echo Exit code $? >> validatevulnerability_stderr.out
		echo >> validatevulnerability_stderr.out
	done
	echo "See ../../examples/validation/vulnerability_testlist.txt for test details" > vulnerabilitytobin_stderr.out
	for i in `seq -f "%02g" 1 13`; do
		echo Test $i >> vulnerabilitytobin_stderr.out
		../../src/vulnerabilitytobin/vulnerabilitytobin -d 4 < ../../examples/static/validation/vulnerability_$i.csv > /dev/null 2>> vulnerabilitytobin_stderr.out
		echo Exit code $? >> vulnerabilitytobin_stderr.out
		echo >> vulnerabilitytobin_stderr.out
	done

     # checksums		
	sha1sum -c ../$CTRL.sha1
	if [ "$?" -ne "0" ]; then
		TESTS_PASS=0
	fi

	if [ ${PARQUET_OUTPUT} -eq 1 ]; then
		sha1sum -c ../$CTRL_PARQUET.sha1
		if [ "$?" -ne 0 ]; then
			TESTS_PASS=0
		fi
	fi

	if [ ${TESTS_PASS} -ne 1 ]; then
		echo "Sorry check failed.\n"
		cd ../..
		exit 1
	else
		echo "All tests passed.\n"
		cd ../..
		return
	fi
	
}

SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")
cd $SCRIPTPATH

init
installertest
#fin
