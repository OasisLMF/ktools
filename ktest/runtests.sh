#!/bin/sh

init()
{	
	if [ ! -f ../src/eve/eve ]; then
	    echo "Please run make all before make check"
	    exit
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
	cd examples

	# test eve
	../src/eve/eve 1 2 > ../ktest/testout/eveout1.bin
	../src/eve/eve 2 2 > ../ktest/testout/eveout2.bin

	# # test getmodel
	 ../src/eve/eve 1 1 | ../src/getmodel/getmodel > ../ktest/testout/getmodelout.bin
	
	# test gulcalc item stream and coverage stream
	../src/eve/eve 1 1 | ../src/getmodel/getmodel | ../src/gulcalc/gulcalc -S100 -L0.1 -r -i - > ../ktest/testout/gulcalci.bin
	../src/eve/eve 1 1 | ../src/getmodel/getmodel | ../src/gulcalc/gulcalc -S100 -L0.1 -r -c - > ../ktest/testout/gulcalcc.bin

	# test fmcalc
	 ../src/fmcalc/fmcalc > ../ktest/testout/fmcalc.bin < ../ktest/testout/gulcalci.bin
	
	# test summary samples
	 ../src/summarycalc/summarycalc -g -1 ../ktest/testout/gulsummarycalc1.bin  < ../ktest/testout/gulcalcc.bin  
	 ../src/summarycalc/summarycalc -g -2 ../ktest/testout/gulsummarycalc2.bin  < ../ktest/testout/gulcalcc.bin  
	 ../src/summarycalc/summarycalc -f -1 ../ktest/testout/fmsummarycalc1.bin   < ../ktest/testout/fmcalc.bin
	 ../src/summarycalc/summarycalc -f -2 ../ktest/testout/fmsummarycalc2.bin   < ../ktest/testout/fmcalc.bin

	# test eltcalc
	../src/eltcalc/eltcalc < ../ktest/testout/gulsummarycalc1.bin > ../ktest/testout/gulelt1.csv
	../src/eltcalc/eltcalc < ../ktest/testout/gulsummarycalc2.bin > ../ktest/testout/gulelt2.csv
	../src/eltcalc/eltcalc < ../ktest/testout/fmsummarycalc1.bin > ../ktest/testout/fmelt1.csv
	../src/eltcalc/eltcalc < ../ktest/testout/fmsummarycalc2.bin > ../ktest/testout/fmelt2.csv

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

	# test pltcalc
	../src/pltcalc/pltcalc < ../ktest/testout/gulsummarycalc1.bin > ../ktest/testout/gulplt1.csv
	../src/pltcalc/pltcalc < ../ktest/testout/gulsummarycalc2.bin > ../ktest/testout/gulplt2.csv
	../src/pltcalc/pltcalc < ../ktest/testout/fmsummarycalc1.bin > ../ktest/testout/fmplt1.csv
	../src/pltcalc/pltcalc < ../ktest/testout/fmsummarycalc2.bin > ../ktest/testout/fmplt2.csv
	
	# test aalcalc
	../src/aalcalc/aalcalc < ../ktest/testout/gulsummarycalc1.bin > ../ktest/testout/gulaalcalc1.bin
	../src/aalcalc/aalcalc < ../ktest/testout/gulsummarycalc2.bin > ../ktest/testout/gulaalcalc2.bin
	../src/aalcalc/aalcalc < ../ktest/testout/fmsummarycalc1.bin > ../ktest/testout/fmaalcalc1.bin
	../src/aalcalc/aalcalc < ../ktest/testout/fmsummarycalc2.bin > ../ktest/testout/fmaalcalc2.bin

	# test aalsummary
	cp ../ktest/testout/gulaalcalc1.bin work/gul1/aal/gulaalcalc1.bin
	cp ../ktest/testout/gulaalcalc2.bin work/gul2/aal/gulaalcalc2.bin
	cp ../ktest/testout/fmaalcalc1.bin work/fm1/aal/fmaalcalc1.bin
	cp ../ktest/testout/fmaalcalc2.bin work/fm2/aal/fmaalcalc2.bin
	../src/aalsummary/aalsummary -Kgul1/aal > ../ktest/testout/gulaalsummary1.csv
	../src/aalsummary/aalsummary -Kgul2/aal > ../ktest/testout/gulaalsummary2.csv
	../src/aalsummary/aalsummary -Kfm1/aal > ../ktest/testout/fmaalsummary1.csv
	../src/aalsummary/aalsummary -Kfm2/aal > ../ktest/testout/fmaalsummary2.csv

	# test stream conversion components
	# stdout to csv
	../src/cdftocsv/cdftocsv < ../ktest/testout/getmodelout.bin > ../ktest/testout/getmodelout.csv
	../src/gultocsv/gultocsv -f < ../ktest/testout/gulcalci.bin > ../ktest/testout/gulcalci.csv
	../src/gultocsv/gultocsv -f < ../ktest/testout/gulcalcc.bin > ../ktest/testout/gulcalcc.csv
	../src/fmtocsv/fmtocsv -f < ../ktest/testout/fmcalc.bin > ../ktest/testout/fmcalc.csv

	../src/summarycalctocsv/summarycalctocsv -f < ../ktest/testout/gulsummarycalc2.bin > ../ktest/testout/gulsummarycalc2.csv
	../src/summarycalctocsv/summarycalctocsv -f < ../ktest/testout/gulsummarycalc1.bin > ../ktest/testout/gulsummarycalc1.csv
	../src/summarycalctocsv/summarycalctocsv -f < ../ktest/testout/fmsummarycalc2.bin > ../ktest/testout/fmsummarycalc2.csv
	../src/summarycalctocsv/summarycalctocsv -f < ../ktest/testout/fmsummarycalc1.bin > ../ktest/testout/fmsummarycalc1.csv

	../src/aalcalctocsv/aalcalctocsv < ../ktest/testout/gulaalcalc1.bin > ../ktest/testout/gulaalcalc1.csv

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

	../src/vulnerabilitytocsv/vulnerabilitytocsv < ../examples/static/vulnerability.bin | ../src/vulnerabilitytobin/vulnerabilitytobin -d 102 > ../ktest/testout/vulnerability.bin
	
	cp static/footprint.bin ../ktest/testout/footprint.bin
    cp static/footprint.idx ../ktest/testout/footprint.idx
	
	cd ../ktest/testout
	../../src/footprinttocsv/footprinttocsv > footprint.csv
	mv footprint.bin footprintin.bin
    mv footprint.idx footprintin.idx 
	../../src/footprinttobin/footprinttobin -i 121 < footprint.csv

     # checksums		
	 md5sum -c ../$CTRL.md5

	 if [ "$?" -ne "0" ]; then
	   echo "Sorry check failed\n"
	   cd ../..
	   return
	 else
	   echo "All tests passed.\n"
	  cd ../..
	  return
	 fi
	
}

init
installertest
fin