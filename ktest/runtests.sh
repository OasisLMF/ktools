#!/bin/sh

if [ ! -f ../src/eve/eve ]; then
    echo "Please run make all before make check"
    exit
fi

cd ../examples

../src/eve/eve 1 1 2 > ../ktest/testout/eveout1_1.bin
../src/eve/eve 1 2 2 > ../ktest/testout/eveout1_2.bin
../src/eve/eve 2 1 2 > ../ktest/testout/eveout2_1.bin
../src/eve/eve 2 2 2 > ../ktest/testout/eveout2_2.bin

diff ../ktest/ctrl/eveout1_1.bin ../ktest/testout/eveout1_1.bin
diff ../ktest/ctrl/eveout1_2.bin ../ktest/testout/eveout1_2.bin
diff ../ktest/ctrl/eveout2_1.bin ../ktest/testout/eveout2_1.bin
diff ../ktest/ctrl/eveout2_2.bin ../ktest/testout/eveout2_2.bin

../src/eve/eve 1 1 2 | ../src/getmodel/getmodel 1  > ../ktest/testout/getmodelout1_1.bin
../src/eve/eve 1 2 2 | ../src/getmodel/getmodel 1  > ../ktest/testout/getmodelout1_2.bin
../src/eve/eve 2 1 2 | ../src/getmodel/getmodel 2  > ../ktest/testout/getmodelout2_1.bin
../src/eve/eve 2 2 2 | ../src/getmodel/getmodel 2  > ../ktest/testout/getmodelout2_2.bin

diff ../ktest/ctrl/getmodelout1_1.bin ../ktest/testout/getmodelout1_1.bin
diff ../ktest/ctrl/getmodelout1_2.bin ../ktest/testout/getmodelout1_2.bin
diff ../ktest/ctrl/getmodelout2_1.bin ../ktest/testout/getmodelout2_1.bin
diff ../ktest/ctrl/getmodelout2_2.bin ../ktest/testout/getmodelout2_2.bin

../src/eve/eve 1 1 2 | ../src/getmodel/getmodel 1 | ../src/gulcalc/gulcalc -C1 -S100 -R -r > ../ktest/testout/gulcalcout1_1.bin
../src/eve/eve 1 2 2 | ../src/getmodel/getmodel 1 | ../src/gulcalc/gulcalc -C1 -S100 -R -r > ../ktest/testout/gulcalcout1_2.bin
../src/eve/eve 2 1 2 | ../src/getmodel/getmodel 2 | ../src/gulcalc/gulcalc -C1 -S100 -R -r > ../ktest/testout/gulcalcout2_1.bin
../src/eve/eve 2 2 2 | ../src/getmodel/getmodel 2 | ../src/gulcalc/gulcalc -C1 -S100 -R -r > ../ktest/testout/gulcalcout2_2.bin

diff ../ktest/ctrl/gulcalcout1_1.bin ../ktest/testout/gulcalcout1_1.bin
diff ../ktest/ctrl/gulcalcout1_2.bin ../ktest/testout/gulcalcout1_2.bin
diff ../ktest/ctrl/gulcalcout2_1.bin ../ktest/testout/gulcalcout2_1.bin
diff ../ktest/ctrl/gulcalcout2_2.bin ../ktest/testout/gulcalcout2_2.bin

../src/cdftocsv/cdftocsv < ../ktest/testout/getmodelout1_1.bin > ../ktest/testout/getmodelout1_1.csv

diff ../ktest/ctrl/getmodelout1_1.csv ../ktest/testout/getmodelout1_1.csv

../src/gultocsv/gultocsv < ../ktest/testout/gulcalcout1_1.bin > ../ktest/testout/gulcalcout1_1.csv

diff ../ktest/ctrl/gulcalcout1_1.csv ../ktest/testout/gulcalcout1_1.csv
