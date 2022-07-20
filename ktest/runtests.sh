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
	mkdir -p examples/log

}

fin()
{

	rm -rf examples/work
	rm -rf examples/log
}

installertest()
{
	CTRL=ctrl
	CTRL_PARQUET=ctrl_parquet
	SYSTEMNAME="$(uname -s)"
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

	../src/leccalc/leccalc  -Kgul1/summary -F ../ktest/testout/gul_full_uncertainty_aep_1.csv 
	../src/leccalc/leccalc  -Kgul1/summary -W ../ktest/testout/gul_wheatsheaf_aep_1.csv 
	../src/leccalc/leccalc  -Kgul1/summary -S ../ktest/testout/gul_sample_mean_aep_1.csv
	../src/leccalc/leccalc  -Kgul1/summary -M ../ktest/testout/gul_wheatsheaf_mean_aep_1.csv 
    ../src/leccalc/leccalc  -Kgul1/summary -f ../ktest/testout/gul_full_uncertainty_oep_1.csv
	../src/leccalc/leccalc  -Kgul1/summary -w ../ktest/testout/gul_wheatsheaf_oep_1.csv
	../src/leccalc/leccalc  -Kgul1/summary -s ../ktest/testout/gul_sample_mean_oep_1.csv 
    ../src/leccalc/leccalc  -Kgul1/summary -m ../ktest/testout/gul_wheatsheaf_mean_oep_1.csv

	../src/leccalc/leccalc -Kfm1/summary -F ../ktest/testout/fm_full_uncertainty_aep_1.csv  
	../src/leccalc/leccalc -Kfm1/summary -W ../ktest/testout/fm_wheatsheaf_aep_1.csv
	../src/leccalc/leccalc -Kfm1/summary -S ../ktest/testout/fm_sample_mean_aep_1.csv
	../src/leccalc/leccalc -Kfm1/summary -M ../ktest/testout/fm_wheatsheaf_mean_aep_1.csv
    ../src/leccalc/leccalc -Kfm1/summary -f ../ktest/testout/fm_full_uncertainty_oep_1.csv
	../src/leccalc/leccalc -Kfm1/summary -w ../ktest/testout/fm_wheatsheaf_oep_1.csv
	../src/leccalc/leccalc -Kfm1/summary -s ../ktest/testout/fm_sample_mean_oep_1.csv
    ../src/leccalc/leccalc -Kfm1/summary -m ../ktest/testout/fm_wheatsheaf_mean_oep_1.csv

	../src/leccalc/leccalc -Kfm2/summary -F ../ktest/testout/fm_full_uncertainty_aep_2.csv
	../src/leccalc/leccalc -Kfm2/summary -W ../ktest/testout/fm_wheatsheaf_aep_2.csv
	../src/leccalc/leccalc -Kfm2/summary -S ../ktest/testout/fm_sample_mean_aep_2.csv
	../src/leccalc/leccalc -Kfm2/summary -M ../ktest/testout/fm_wheatsheaf_mean_aep_2.csv
	../src/leccalc/leccalc -Kfm2/summary -f ../ktest/testout/fm_full_uncertainty_oep_2.csv
	../src/leccalc/leccalc -Kfm2/summary -w ../ktest/testout/fm_wheatsheaf_oep_2.csv
	../src/leccalc/leccalc -Kfm2/summary -s ../ktest/testout/fm_sample_mean_oep_2.csv
	../src/leccalc/leccalc -Kfm2/summary -m ../ktest/testout/fm_wheatsheaf_mean_oep_2.csv

	../src/leccalc/leccalc -r -Kgul1/summary -F ../ktest/testout/gul_full_uncertainty_aep_1_r.csv 
	../src/leccalc/leccalc -r -Kgul1/summary -W ../ktest/testout/gul_wheatsheaf_aep_1_r.csv 
	../src/leccalc/leccalc -r -Kgul1/summary -S ../ktest/testout/gul_sample_mean_aep_1_r.csv
	../src/leccalc/leccalc -r -Kgul1/summary -M ../ktest/testout/gul_wheatsheaf_mean_aep_1_r.csv 
    ../src/leccalc/leccalc -r -Kgul1/summary -f ../ktest/testout/gul_full_uncertainty_oep_1_r.csv
	../src/leccalc/leccalc -r -Kgul1/summary -w ../ktest/testout/gul_wheatsheaf_oep_1_r.csv
	../src/leccalc/leccalc -r -Kgul1/summary -s ../ktest/testout/gul_sample_mean_oep_1_r.csv 
    ../src/leccalc/leccalc -r -Kgul1/summary -m ../ktest/testout/gul_wheatsheaf_mean_oep_1_r.csv

	../src/leccalc/leccalc -r -Kfm1/summary -F ../ktest/testout/fm_full_uncertainty_aep_1_r.csv  
	../src/leccalc/leccalc -r -Kfm1/summary -W ../ktest/testout/fm_wheatsheaf_aep_1_r.csv
	../src/leccalc/leccalc -r -Kfm1/summary -S ../ktest/testout/fm_sample_mean_aep_1_r.csv
	../src/leccalc/leccalc -r -Kfm1/summary -M ../ktest/testout/fm_wheatsheaf_mean_aep_1_r.csv
    ../src/leccalc/leccalc -r -Kfm1/summary -f ../ktest/testout/fm_full_uncertainty_oep_1_r.csv
	../src/leccalc/leccalc -r -Kfm1/summary -w ../ktest/testout/fm_wheatsheaf_oep_1_r.csv
	../src/leccalc/leccalc -r -Kfm1/summary -s ../ktest/testout/fm_sample_mean_oep_1_r.csv
    ../src/leccalc/leccalc -r -Kfm1/summary -m ../ktest/testout/fm_wheatsheaf_mean_oep_1_r.csv

	../src/leccalc/leccalc -r -Kgul2/summary -F ../ktest/testout/gul_full_uncertainty_aep_2_r.csv 
	../src/leccalc/leccalc -r -Kgul2/summary -W ../ktest/testout/gul_wheatsheaf_aep_2_r.csv 
	../src/leccalc/leccalc -r -Kgul2/summary -S ../ktest/testout/gul_sample_mean_aep_2_r.csv
	../src/leccalc/leccalc -r -Kgul2/summary -M ../ktest/testout/gul_wheatsheaf_mean_aep_2_r.csv 
    ../src/leccalc/leccalc -r -Kgul2/summary -f ../ktest/testout/gul_full_uncertainty_oep_2_r.csv
	../src/leccalc/leccalc -r -Kgul2/summary -w ../ktest/testout/gul_wheatsheaf_oep_2_r.csv
	../src/leccalc/leccalc -r -Kgul2/summary -s ../ktest/testout/gul_sample_mean_oep_2_r.csv 
    ../src/leccalc/leccalc -r -Kgul2/summary -m ../ktest/testout/gul_wheatsheaf_mean_oep_2_r.csv

	../src/leccalc/leccalc -r -Kfm2/summary -F ../ktest/testout/fm_full_uncertainty_aep_2_r.csv 
	../src/leccalc/leccalc -r -Kfm2/summary -W ../ktest/testout/fm_wheatsheaf_aep_2_r.csv 
	../src/leccalc/leccalc -r -Kfm2/summary -S ../ktest/testout/fm_sample_mean_aep_2_r.csv  
	../src/leccalc/leccalc -r -Kfm2/summary -M ../ktest/testout/fm_wheatsheaf_mean_aep_2_r.csv  
	../src/leccalc/leccalc -r -Kfm2/summary -f ../ktest/testout/fm_full_uncertainty_oep_2_r.csv 
	../src/leccalc/leccalc -r -Kfm2/summary -w ../ktest/testout/fm_wheatsheaf_oep_2_r.csv	
	../src/leccalc/leccalc -r -Kfm2/summary -s ../ktest/testout/fm_sample_mean_oep_2_r.csv  
	../src/leccalc/leccalc -r -Kfm2/summary -m ../ktest/testout/fm_wheatsheaf_mean_oep_2_r.csv 
    
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

	# test aalcalc	
	../src/aalcalc/aalcalc -Kgul1/summary > ../ktest/testout/gulaalcalc1.csv
	../src/aalcalc/aalcalc -Kgul2/summary > ../ktest/testout/gulaalcalc2.csv
	../src/aalcalc/aalcalc -Kfm1/summary > ../ktest/testout/fmaalcalc1.csv
	../src/aalcalc/aalcalc -Kfm2/summary > ../ktest/testout/fmaalcalc2.csv

	# test alt	
	../src/aalcalc/aalcalc -o -Kgul1/summary > ../ktest/testout/gulalt1.csv
	../src/aalcalc/aalcalc -o -Kgul2/summary > ../ktest/testout/gulalt2.csv
	../src/aalcalc/aalcalc -o -Kfm1/summary > ../ktest/testout/fmalt1.csv
	../src/aalcalc/aalcalc -o -Kfm2/summary > ../ktest/testout/fmalt2.csv
	if [ ${PARQUET_OUTPUT} -eq 1 ]; then
		../src/aalcalc/aalcalc -p ../ktest/testout/gulalt1.parquet -Kgul1/summary
		../src/aalcalc/aalcalc -p ../ktest/testout/gulalt2.parquet -Kgul2/summary
		../src/aalcalc/aalcalc -p ../ktest/testout/fmalt1.parquet -Kfm1/summary
		../src/aalcalc/aalcalc -p ../ktest/testout/fmalt2.parquet -Kfm2/summary
	fi

	# test stream conversion components
	# stdout to csv
	../src/cdftocsv/cdftocsv  < ../ktest/testout/getmodelout.bin > ../ktest/testout/getmodelout.csv
	../src/gultocsv/gultocsv -f < ../ktest/testout/gulcalci.bin > ../ktest/testout/gulcalci.csv
	../src/gultocsv/gultocsv -f < ../ktest/testout/gulcalcc.bin > ../ktest/testout/gulcalcc.csv
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

	../src/damagebintocsv/damagebintocsv < ../examples/static/damage_bin_dict.bin | ../src/damagebintobin/damagebintobin > ../ktest/testout/damage_bin_dict.bin
	
	../src/fmprogrammetocsv/fmprogrammetocsv < ../examples/input/fm_programme.bin | ../src/fmprogrammetobin/fmprogrammetobin > ../ktest/testout/fm_programme.bin
	
	../src/fmprofiletocsv/fmprofiletocsv < ../examples/input/fm_profile.bin | ../src/fmprofiletobin/fmprofiletobin > ../ktest/testout/fm_profile.bin
	
	../src/fmpolicytctocsv/fmpolicytctocsv < ../examples/input/fm_policytc.bin | ../src/fmpolicytctobin/fmpolicytctobin > ../ktest/testout/fm_policytc.bin
	
	../src/fmxreftocsv/fmxreftocsv < ../examples/input/fm_xref.bin | ../src/fmxreftobin/fmxreftobin > ../ktest/testout/fm_xref.bin

	../src/gulsummaryxreftocsv/gulsummaryxreftocsv < ../examples/input/gulsummaryxref.bin | ../src/gulsummaryxreftobin/gulsummaryxreftobin > ../ktest/testout/gulsummaryxref.bin
	
	../src/fmsummaryxreftocsv/fmsummaryxreftocsv < ../examples/input/fmsummaryxref.bin | ../src/fmsummaryxreftobin/fmsummaryxreftobin > ../ktest/testout/fmsummaryxref.bin
    
    ../src/returnperiodtocsv/returnperiodtocsv < ../examples/input/returnperiods.bin | ../src/returnperiodtobin/returnperiodtobin > ../ktest/testout/returnperiods.bin

	../src/occurrencetocsv/occurrencetocsv < ../examples/input/occurrence.bin | ../src/occurrencetobin/occurrencetobin -P10000 > ../ktest/testout/occurrence.bin

	../src/vulnerabilitytocsv/vulnerabilitytocsv < ../examples/static/vulnerability.bin | ../src/vulnerabilitytobin/vulnerabilitytobin -d 102 > ../ktest/testout/vulnerability.bin
	
	../src/quantiletocsv/quantiletocsv < ../examples/input/quantile.bin | ../src/quantiletobin/quantiletobin  > ../ktest/testout/quantile.bin

	cp static/footprint.bin ../ktest/testout/footprint.bin
    cp static/footprint.idx ../ktest/testout/footprint.idx
	
	cd ../ktest/testout
	../../src/footprinttocsv/footprinttocsv > footprint.csv
	mv footprint.bin footprintin.bin
    mv footprint.idx footprintin.idx 
	../../src/footprinttobin/footprinttobin -i 121 < footprint.csv

     # checksums
 	SHA_SUM_EXE='sha1sum'
	if [ "$SYSTEMNAME" == "Darwin" ]; then
		SHA_SUM_EXE='shasum'
	fi
	
	$SHA_SUM_EXE -c ../$CTRL.sha1
	if [ "$?" -ne "0" ]; then
		TESTS_PASS=0
	fi

	if [ ${PARQUET_OUTPUT} -eq 1 ]; then
		$SHA_SUM_EXE -c ../$CTRL_PARQUET.sha1
		if [ "$?" -ne "0" ]; then
			TESTS_PASS=0
		fi
	fi

	if [ ${TESTS_PASS} -ne 1 ]; then
		echo "Sorry some checks failed.\n"
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
fin
