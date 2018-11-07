#!/bin/bash

# script to output all input files to csv
# pleaces them in the test out folder

mkdir -p ../ktest/testout/

doit_input()
{
	rm ../ktest/testout/$2.csv
	$1 < input/$2.bin > ../ktest/testout/$2.csv	
}

dotest_input()
{
	$1 < input/$2.bin | head -n 20
}

doit_static()
{
	rm ../ktest/testout/$2.csv
	$1 < static/$2.bin > ../ktest/testout/$2.csv	
}

dotest_static()
{
	$1 < static/$2.bin | head -n 20
}


ls -al static
# ls -al /d/data/cserver/workspaces/ktest2/csvcontrol/fm_*

doit_input "evetocsv" "events"
doit_input "coveragetocsv" "coverages"
doit_input "fmpolicytctocsv" "fm_policytc"
doit_input "fmprofiletocsv" "fm_profile"
doit_input "fmprogrammetocsv" "fm_programme"
doit_input "fmxreftocsv" "fm_xref"
doit_input "fmsummaryxreftocsv" "fmsummaryxref"
doit_input "gulsummaryxreftocsv" "gulsummaryxref"
doit_input "itemtocsv" "items"
doit_input "occurrencetocsv" "occurrence"
doit_input "returnperiodtocsv" "returnperiods"
doit_static "damagebintocsv" "damage_bin_dict"
doit_static "vulnerabilitytocsv" "vulnerability"



randtocsv -r < static/random.bin >../ktest/testout/random.csv	

pushd .
cd static 
footprinttocsv > ../../ktest/testout/footprint.csv	
popd 
# dotest_static "randtocsv" "random"



ls -altr   ../ktest/testout/foot* ../ktest/testout/rand*

