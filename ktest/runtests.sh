#!/bin/sh

if [ ! -f ../src/eve/eve ]; then
    echo "Please run make all before make check"
    exit
fi

cd ../examples

../src/eve/eve 1 1 2 > ../ktest/testout/eveout1.bin
../src/eve/eve 1 2 2 > ../ktest/testout/eveout2.bin

diff ../ktest/ctrl/eveout1.bin ../ktest/testout/eveout1.bin
diff ../ktest/ctrl/eveout2.bin ../ktest/testout/eveout2.bin

../src/eve/eve 1 1 2 | ../src/getmodel/getmodel 1  > ../ktest/testout/getmodelout1.bin
../src/eve/eve 1 2 2 | ../src/getmodel/getmodel 1  > ../ktest/testout/getmodelout2.bin

diff ../ktest/ctrl/getmodelout1.bin ../ktest/testout/getmodelout1.bin
diff ../ktest/ctrl/getmodelout2.bin ../ktest/testout/getmodelout2.bin


../src/eve/eve 1 1 2 | ../src/getmodel/getmodel 1 | ../src/gulcalc/gulcalc -C1 -S100 -R -r > ../ktest/testout/gulcalcout1.bin
../src/eve/eve 1 2 2 | ../src/getmodel/getmodel 1 | ../src/gulcalc/gulcalc -C1 -S100 -R -r > ../ktest/testout/gulcalcout2.bin

diff ../ktest/ctrl/gulcalcout1.bin ../ktest/testout/gulcalcout1.bin
diff ../ktest/ctrl/gulcalcout2.bin ../ktest/testout/gulcalcout2.bin

../src/cdftocsv/cdftocsv < ../ktest/testout/getmodelout1.bin > ../ktest/testout/getmodelout1.csv

diff ../ktest/ctrl/getmodelout1.csv ../ktest/testout/getmodelout1.csv


../src/gultocsv/gultocsv < ../ktest/testout/gulcalcout1.bin > ../ktest/testout/gulcalcout1.csv

diff ../ktest/ctrl/gulcalcout1.csv ../ktest/testout/gulcalcout1.csv
