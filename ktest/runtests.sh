#!/bin/sh

installertest()
{
	if [ ! -f ../src/eve/eve ]; then
	    echo "Please run make all before make check"
	    exit
	fi
	mkdir -p testout
	CTRL=ctrl

	if [ -f cw32bld ]; then
		CTRL=w32ctrl
	elif [ -f cw64bld ]; then
		CTRL=w64ctrl	
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
	# test eve
	../src/eve/eve 1 1 2 > ../ktest/testout/eveout1_1.bin
	../src/eve/eve 1 2 2 > ../ktest/testout/eveout1_2.bin
	../src/eve/eve 2 1 2 > ../ktest/testout/eveout2_1.bin
	../src/eve/eve 2 2 2 > ../ktest/testout/eveout2_2.bin

	# test getmodel
	../src/eve/eve 1 1 2 | ../src/getmodel/getmodel 1  > ../ktest/testout/getmodelout1_1.bin
	../src/eve/eve 1 2 2 | ../src/getmodel/getmodel 1  > ../ktest/testout/getmodelout1_2.bin
	../src/eve/eve 2 1 2 | ../src/getmodel/getmodel 2  > ../ktest/testout/getmodelout2_1.bin
	../src/eve/eve 2 2 2 | ../src/getmodel/getmodel 2  > ../ktest/testout/getmodelout2_2.bin

	# test gulcalc
	../src/eve/eve 1 1 2 | ../src/getmodel/getmodel 1 | ../src/gulcalc/gulcalc -C1 -S100 -R -r > ../ktest/testout/gulcalcout1_1.bin
	../src/eve/eve 1 2 2 | ../src/getmodel/getmodel 1 | ../src/gulcalc/gulcalc -C1 -S100 -R -r > ../ktest/testout/gulcalcout1_2.bin
	../src/eve/eve 2 1 2 | ../src/getmodel/getmodel 2 | ../src/gulcalc/gulcalc -C1 -S100 -R -r > ../ktest/testout/gulcalcout2_1.bin
	../src/eve/eve 2 2 2 | ../src/getmodel/getmodel 2 | ../src/gulcalc/gulcalc -C1 -S100 -R -r > ../ktest/testout/gulcalcout2_2.bin

	# test fmcalc
	../src/eve/eve 1 1 2 | ../src/getmodel/getmodel 1 | ../src/gulcalc/gulcalc -C1 -S100 -R -r | ../src/fmcalc/fmcalc >  ../ktest/testout/fmcalcout1_1.bin

	# test outputcalc
	../src/eve/eve 1 1 1 | ../src/getmodel/getmodel 1 | ../src/gulcalc/gulcalc -C1 -S100 -R -r | ../src/outputcalc/outputcalc >  ../ktest/testout/gulout1.csv

	../src/eve/eve 1 1 1 | ../src/getmodel/getmodel 1 | ../src/gulcalc/gulcalc -C1 -S100 -R -r | ../src/fmcalc/fmcalc | ../src/outputcalc/outputcalc >  ../ktest/testout/fmout1.csv

	# test data conversion utilities
	# stdout to csv
	../src/cdftocsv/cdftocsv < ../ktest/testout/getmodelout1_1.bin > ../ktest/testout/getmodelout1_1.csv

	../src/gultocsv/gultocsv < ../ktest/testout/gulcalcout1_1.bin > ../ktest/testout/gulcalcout1_1.csv

	../src/fmtocsv/fmtocsv < ../ktest/testout/fmcalcout1_1.bin > ../ktest/testout/fmcalcout1_1.csv

	# input data to csv and bin
	../src/evetocsv/evetocsv < ../examples/e_chunk_1_data.bin > ../ktest/testout/e_chunk_1_data.csv
	../src/evetobin/evetobin < ../ktest/testout/e_chunk_1_data.csv > ../ktest/testout/e_chunk_1_data.bin

	../src/randtocsv/randtocsv < ../examples/random_1.bin > ../ktest/testout/random_1.csv
	../src/randtobin/randtobin < ../ktest/testout/random_1.csv | ../src/randtocsv/randtocsv > ../ktest/testout/random_12.csv

	../src/exposuretocsv/exposuretocsv < ../examples/exposures.bin > ../ktest/testout/exposures.csv
	../src/exposuretobin/exposuretobin < ../ktest/testout/exposures.csv > ../ktest/testout/exposures.bin

	../src/damagetocsv/damagetocsv < ../examples/damage_bin_dict.bin > ../ktest/testout/damage_bin_dict.csv
	../src/damagetobin/damagetobin < ../ktest/testout/damage_bin_dict.csv > ../ktest/testout/damage_bin_dict.bin

	../src/cdfdatatocsv/cdfdatatocsv < ../examples/cdf/damage_cdf_chunk_1.bin > ../ktest/testout/damage_cdf_chunk_1.csv

	../src/fmprogrammetocsv/fmprogrammetocsv < ../examples/fm/fm_programme.bin > ../ktest/testout/fm_programme.csv
	../src/fmprogrammetobin/fmprogrammetobin < ../ktest/testout/fm_programme.csv > ../ktest/testout/fm_programme.bin

	../src/fmpolicytctocsv/fmpolicytctocsv < ../examples/fm/fm_policytc.bin > ../ktest/testout/fm_policytc.csv
	../src/fmpolicytctobin/fmpolicytctobin < ../ktest/testout/fm_policytc.csv > ../ktest/testout/fm_policytc.bin

	../src/fmprofiletocsv/fmprofiletocsv < ../examples/fm/fm_profile.bin > ../ktest/testout/fm_profile.csv
	../src/fmprofiletobin/fmprofiletobin < ../ktest/testout/fm_profile.csv > ../ktest/testout/fm_profile.bin

	../src/fmxreftocsv/fmxreftocsv < ../examples/fm/fmxref.bin > ../ktest/testout/fmxref.csv
	../src/fmxreftobin/fmxreftobin < ../ktest/testout/fmxref.csv > ../ktest/testout/fmxref.bin


	cd ../ktest/testout
	../../src/cdfdatatobin/cdfdatatobin damage_cdf_chunk_11 102 < damage_cdf_chunk_1.csv
	../../src/cdfdatatocsv/cdfdatatocsv < damage_cdf_chunk_11.bin > damage_cdf_chunk_11.csv
	diff damage_cdf_chunk_1.csv damage_cdf_chunk_11.csv
	
	md5sum -c ../$CTRL.md5

	if [ "$?" -ne "0" ]; then
	  echo "Sorry check failed\n"
	  exit 1
	else
	  echo "All tests passed.\n"
	 exit 0
	fi


}

ftest()
{
	echo "todo"
}


installertest
ftest