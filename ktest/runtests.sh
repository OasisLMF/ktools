#!/bin/sh

installertest()
{
	if [ ! -f ../src/eve/eve ]; then
	    echo "Please run make all before make check"
	    exit
	fi
	mkdir -p testout

	CTRL=ctrl

	if [ -f cwbld ]; then
		CTRL=wctrl
	fi

	if [ -f uwbld ]; then
		echo "Windows test  not supported in Linux"
		exit
	fi

	cd ..
	if [ ! -d examples ]; then
		tar -xzf examples.tar.gz
	fi
	cd examples
	mkdir -p work/gul1
	mkdir -p work/gul2
	mkdir -p work/fm1
	mkdir -p work/fm2
	# test eve
	../src/eve/eve 1 2 > ../ktest/testout/eveout1.bin
	../src/eve/eve 2 2 > ../ktest/testout/eveout2.bin


	# # test getmodel
	 ../src/eve/eve 1 1 | ../src/getmodel/getmodel -i 121 -d 102 > ../ktest/testout/getmodelout.bin
	
	# test gulcalc item stream and coverage stream
	../src/eve/eve 1 1 | ../src/getmodel/getmodel -i 121 -d 102 | ../src/gulcalc/gulcalc -S100 -L0.1 -r -i - > ../ktest/testout/gulcalci.bin
	../src/eve/eve 1 1 | ../src/getmodel/getmodel -i 121 -d 102 | ../src/gulcalc/gulcalc -S100 -L0.1 -r -c - > ../ktest/testout/gulcalcc.bin
	
	# test fmcalc
	 ../src/fmcalc/fmcalc > ../ktest/testout/fmcalc.bin < ../ktest/testout/gulcalci.bin
	
	# test summary samples
	 ../src/summarycalc/summarycalc -g -1 ../ktest/testout/gulsummarycalc1.bin  < ../ktest/testout/gulcalcc.bin  
	 ../src/summarycalc/summarycalc -g -2 ../ktest/testout/gulsummarycalc2.bin  < ../ktest/testout/gulcalcc.bin  
	 ../src/summarycalc/summarycalc -f -1 ../ktest/testout/fmsummarycalc1.bin   < ../ktest/testout/fmcalc.bin
	 ../src/summarycalc/summarycalc -f -2 ../ktest/testout/fmsummarycalc2.bin   < ../ktest/testout/fmcalc.bin
	cp ../ktest/testout/gulsummarycalc2.bin work/gul2/gulsummarycalc2.bin
	cp ../ktest/testout/fmsummarycalc2.bin work/fm2/fmsummarycalc2.bin
	cp ../ktest/testout/gulsummarycalc1.bin work/gul1/gulsummarycalc1.bin
	cp ../ktest/testout/fmsummarycalc1.bin work/fm1/fmsummarycalc1.bin
	# test eltcalc
	../src/eltcalc/eltcalc < ../ktest/testout/gulsummarycalc1.bin > ../ktest/testout/gulelt1.csv
	../src/eltcalc/eltcalc < ../ktest/testout/gulsummarycalc2.bin > ../ktest/testout/gulelt2.csv
	../src/eltcalc/eltcalc < ../ktest/testout/fmsummarycalc1.bin > ../ktest/testout/fmelt1.csv
	../src/eltcalc/eltcalc < ../ktest/testout/fmsummarycalc2.bin > ../ktest/testout/fmelt2.csv

	# test leccalc
	../src/leccalc/leccalc -F ../ktest/testout/gul_full_uncertainty_aep_1.csv -Kgul1 
	../src/leccalc/leccalc -W ../ktest/testout/gul_wheatsheaf_aep_1.csv -Kgul1 
	../src/leccalc/leccalc -S ../ktest/testout/gul_sample_mean_aep_1.csv -Kgul1
	../src/leccalc/leccalc -M ../ktest/testout/gul_wheatsheaf_mean_aep_1.csv -Kgul1
    ../src/leccalc/leccalc -f ../ktest/testout/gul_full_uncertainty_oep_1.csv -Kgul1 
	../src/leccalc/leccalc -w ../ktest/testout/gul_wheatsheaf_oep_1.csv -Kgul1 
	../src/leccalc/leccalc -s ../ktest/testout/gul_sample_mean_oep_1.csv -Kgul1 
    ../src/leccalc/leccalc -m ../ktest/testout/gul_wheatsheaf_mean_oep_1.csv -Kgul1

	../src/leccalc/leccalc -F ../ktest/testout/fm_full_uncertainty_aep_1.csv -Kfm1 
	../src/leccalc/leccalc -W ../ktest/testout/fm_wheatsheaf_aep_1.csv -Kfm1 
	../src/leccalc/leccalc -S ../ktest/testout/fm_sample_mean_aep_1.csv -Kfm1
	../src/leccalc/leccalc -M ../ktest/testout/fm_wheatsheaf_mean_aep_1.csv -Kfm1
    ../src/leccalc/leccalc -f ../ktest/testout/fm_full_uncertainty_oep_1.csv -Kfm1 
	../src/leccalc/leccalc -w ../ktest/testout/fm_wheatsheaf_oep_1.csv -Kfm1 
	../src/leccalc/leccalc -s ../ktest/testout/fm_sample_mean_oep_1.csv -Kfm1 
    ../src/leccalc/leccalc -m ../ktest/testout/fm_wheatsheaf_mean_oep_1.csv -Kfm1

	# test pltcalc
	../src/pltcalc/pltcalc < ../ktest/testout/gulsummarycalc1.bin > ../ktest/testout/gulplt1.csv
	../src/pltcalc/pltcalc < ../ktest/testout/gulsummarycalc2.bin > ../ktest/testout/gulplt2.csv
	../src/pltcalc/pltcalc < ../ktest/testout/fmsummarycalc1.bin > ../ktest/testout/fmplt1.csv
	../src/pltcalc/pltcalc < ../ktest/testout/fmsummarycalc2.bin > ../ktest/testout/fmplt2.csv
	
		# test data conversion utilities
	# stdout to csv
	../src/cdftocsv/cdftocsv < ../ktest/testout/getmodelout.bin > ../ktest/testout/getmodelout.csv

	../src/gultocsv/gultocsv < ../ktest/testout/gulcalci.bin > ../ktest/testout/gulcalci.csv
	../src/gultocsv/gultocsv < ../ktest/testout/gulcalcc.bin > ../ktest/testout/gulcalcc.csv
	../src/fmtocsv/fmtocsv < ../ktest/testout/fmcalc.bin > ../ktest/testout/fmcalc.csv

	../src/summarycalctocsv/summarycalctocsv < ../ktest/testout/gulsummarycalc2.bin > ../ktest/testout/gulsummarycalc2.csv
	../src/summarycalctocsv/summarycalctocsv < ../ktest/testout/gulsummarycalc1.bin > ../ktest/testout/gulsummarycalc1.csv
	../src/summarycalctocsv/summarycalctocsv < ../ktest/testout/fmsummarycalc2.bin > ../ktest/testout/fmsummarycalc2.csv
	../src/summarycalctocsv/summarycalctocsv < ../ktest/testout/fmsummarycalc1.bin > ../ktest/testout/fmsummarycalc1.csv

	# input data to csv and bin
	../src/evetocsv/evetocsv < ../examples/input/events.bin | ../src/evetobin/evetobin > ../ktest/testout/events.bin
	
	../src/randtocsv/randtocsv < ../examples/static/random.bin | ../src/randtobin/randtobin > ../ktest/testout/random.bin

	 ../src/itemtocsv/itemtocsv < ../examples/input/items.bin | ../src/itemtobin/itemtobin > ../ktest/testout/items.bin

	 ../src/coveragetocsv/coveragetocsv < ../examples/input/coverages.bin | ../src/coveragetobin/coveragetobin > ../ktest/testout/coverages.bin

	../src/damagebintocsv/damagebintocsv < ../examples/static/damage_bin_dict.bin | ../src/damagebintobin/damagebintobin > ../ktest/testout/damage_bin_dict.bin
	
	../src/fmprogrammetocsv/fmprogrammetocsv < ../examples/input/fm_programme.bin | ../src/fmprogrammetobin/fmprogrammetobin > ../ktest/testout/fm_programme.bin
	
	../src/fmprofiletocsv/fmprofiletocsv < ../examples/input/fm_profile.bin | ../src/fmprofiletobin/fmprofiletobin > ../ktest/testout/fm_profile.bin
	
	../src/fmpolicytctocsv/fmpolicytctocsv < ../examples/input/fm_policytc.bin | ../src/fmpolicytctobin/fmpolicytctobin > ../ktest/testout/fm_policytc.bin
	
	../src/fmxreftocsv/fmxreftocsv < ../examples/input/fm_xref.bin | ../src/fmxreftobin/fmxreftobin > ../ktest/testout/fm_xref.bin

	../src/gulsummaryxreftocsv/gulsummaryxreftocsv < ../examples/input/gulsummaryxref.bin | ../src/gulsummaryxreftobin/gulsummaryxreftobin > ../ktest/testout/gulsummaryxref.bin
	
	../src/fmsummaryxreftocsv/fmsummaryxreftocsv < ../examples/input/fmsummaryxref.bin | ../src/fmsummaryxreftobin/fmsummaryxreftobin > ../ktest/testout/fmsummaryxref.bin

	../src/occurrencetocsv/occurrencetocsv < ../examples/static/occurrence.bin | ../src/occurrencetobin/occurrencetobin -P10000 > ../ktest/testout/occurrence.bin
	
	cd ../ktest/testout

	
	 md5sum -c ../$CTRL.md5

	 if [ "$?" -ne "0" ]; then
	   echo "Sorry check failed\n"
	   exit 1
	 else
	   echo "All tests passed.\n"
	  exit 0
	 fi

	echo "Finished.  Check sums to do."
}

installertest
