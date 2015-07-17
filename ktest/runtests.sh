#!/bin/sh

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
#../src/cdfdatatobin/cdfdatatobin < ../ktest/testout/damage_cdf_chunk_1.csv > ../ktest/testout/damage_cdf_chunk_1.bin

../src/fmdatatocsv/fmdatatocsv < ../examples/fm/fm_data.bin > ../ktest/testout/fm_data.csv
../src/fmdatatocsv/fmdatatocsv < ../ktest/testout/fm_data.csv > ../ktest/testout/fm_data.bin

../src/fmxreftocsv/fmxreftocsv < ../examples/fm/fmxref.bin > ../ktest/testout/fmxref.csv
../src/fmxreftocsv/fmxreftocsv < ../ktest/testout/fmxref.csv > ../ktest/testout/fmxref.bin


cd ../ktest/testout
md5sum -c ../$CTRL.md5

if [ "$?" -ne "0" ]; then
  echo "Sorry check failed\n"
  exit 1
else
  echo "All tests passed.\n"
 exit 0
fi


