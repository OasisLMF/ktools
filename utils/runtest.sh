#!/bin/bash

# Copyright (c)2015 - 2016 Oasis LMF Limited
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.
#
#   * Neither the original author of this software nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
# THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#

ROWNO=2

initdata()
{
	python GenerateTestData.py -num_location=$1 -num_intensity_bins=$2 -intensity_sparseness=$3 -num_damage_bins=$4 -num_events=$5 -num_sample=$6
}

# to get the timing we run it once with the timeings and once without because on the second run we are writing the data to disk
#
##  parameters : num_location num_intensity_bins intensity_sparseness num_damage_bins num_events num_sample samplesize
doit()
{


	echo "cdf processing "
	/usr/bin/time -o timing.log getmodel < input/events.bin > /dev/null
	echo "cdf timing "
	cat timing.log
	CDF_GENERATION=$(awk 'BEGIN { FS="u" } ; NR==1{print $1} ' timing.log)

	getmodel < input/events.bin > cdf.bin

	# CDF_GENERATION=$(awk 'BEGIN { FS="u" } ; NR==1{print $1} ' timing.log)
	echo $(awk 'BEGIN { FS="u" } ; NR==1{print $1} ' timing.log)
	echo "cdf:  $CDF_GENERATION"
	if [ -z "$CDF_GENERATION" ]
	then
		echo "variable not set"
		exit
	fi
	echo "Gul processing samplesize: $8"
	#echo "gulcalc ${10} -S$8 -L0.1  -i - > gulcalci.bin  < cdf.bin"
 	#/usr/bin/time -o timing.log gulcalc ${10} -S$8 -L0.1  -i - > /dev/null  < cdf.bin
	echo "gulcalc ${10} -S$8 -L0.1  -i - > gulcalci.bin  < cdf.bin"
 	/usr/bin/time -o timing.log gulcalc ${10} -S$8 -L0.1  -i - > /dev/null  < cdf.bin
 	GULCALC=$(awk 'BEGIN { FS="u" } ; {print $1;exit;} ' timing.log)
 	echo "gul timing "
 	cat timing.log
 	gulcalc ${10} -S$8 -L0.1  -i - > gulcalci.bin  < cdf.bin

 	echo "fm processing"
	/usr/bin/time -o timing.log fmcalc > /dev/null < gulcalci.bin
	FM=$(awk 'BEGIN { FS="u" } ; {print $1;exit;} ' timing.log)
	echo "fm timing "
	cat timing.log

	echo "CDF: $CDF_GENERATION"
	echo "GUL: $GULCALC"
	echo "FM: $FM"
	TOTAL=$(perl -e "print $CDF_GENERATION+$GULCALC+$FM")
	TOTAL_PER_EVENT=$(perl -e "print $TOTAL/$5")
	echo "$1, $2, $3, $4, $5, $6, $7, $8,$CDF_GENERATION,$GULCALC,$FM" >> perf.csv


	((ROWNO++))
}


doit_no_r()
{
	doit $1 $2 $3 $4 $5 $6 $7 $8 7.5
}

doit_with_r()
{
	if [ ! -s static/random.bin ]; then
    	echo "static/random.bin not found please generate the file before using the -r option"
    	echo  "example command to create 1,000,000 random numbers:"
    	echo  "randtocsv -g 1000000 | randtobin > static/random.bin"
    	randtocsv -g 1000000 | randtobin > static/random.bin
	fi

	doit $1 $2 $3 $4 $5 $6 $7 $8 3.8 -r
}


NUMLOCATIONS=2000000
DAMAGEBINS=256

varynumberofsamples_no_r()
{

initdata $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1
doit_no_r 1 $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1 1
doit_no_r 1 $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1 5
doit_no_r 1 $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1 10
doit_no_r 1 $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1 50
doit_no_r 1 $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1 100
doit_no_r 1 $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1 200

}

varynumberofsamples_with_r()
{

initdata $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1

doit_with_r 2 $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1 1
doit_with_r 2 $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1 5
doit_with_r 2 $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1 10
doit_with_r 2 $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1 50
doit_with_r 2 $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1 100
doit_with_r 2 $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1 200
}


varynumberofevents()
{

 initdata $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1
 doit_no_r 3 $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1 5

 initdata $NUMLOCATIONS 1 1 $DAMAGEBINS 20 1
 doit_no_r 3 $NUMLOCATIONS 1 1 $DAMAGEBINS 20 1 5

 initdata $NUMLOCATIONS 1 1 $DAMAGEBINS 50 1
 doit_no_r 3 $NUMLOCATIONS 1 1 $DAMAGEBINS 50 1 5

 initdata $NUMLOCATIONS 1 1 $DAMAGEBINS 100 1
 doit_no_r 3 $NUMLOCATIONS 1 1 $DAMAGEBINS 100 1 5

 initdata $NUMLOCATIONS 1 1 $DAMAGEBINS 200 1
 doit_no_r 3 $NUMLOCATIONS 1 1 $DAMAGEBINS 200 1 5
}

varynumberofintensitybins()
{

 initdata $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1
 doit_no_r 4 $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1 5
 initdata $NUMLOCATIONS 5 1 $DAMAGEBINS 10 1
 doit_no_r 4 $NUMLOCATIONS 5 1 $DAMAGEBINS 10 1 5
 initdata $NUMLOCATIONS 10 1 $DAMAGEBINS 10 1
 doit_no_r 4 $NUMLOCATIONS 10 1 $DAMAGEBINS 10 1 5
 initdata $NUMLOCATIONS 50 1 $DAMAGEBINS 10 1
 doit_no_r 4 $NUMLOCATIONS 50 1 $DAMAGEBINS 10 1 5
 initdata $NUMLOCATIONS 100 1 $DAMAGEBINS 10 1
 doit_no_r 4 $NUMLOCATIONS 100 1 $DAMAGEBINS 10 1 5
 # initdata $NUMLOCATIONS 200 1 $DAMAGEBINS 10 1
 # doit_no_r 4 $NUMLOCATIONS 200 1 $DAMAGEBINS 10 1 5
}


varynumberoflocations()
{
// need to start with a sample that takes at least a 10th of second
	initdata 100000 1 1 256 10 1
	doit_no_r 5 100000 1 1 256 10 1 1
	initdata 300000 1 1 256 10 1
	doit_no_r 5 300000 1 1 256 10 1 1
	initdata 700000 1 1 256 10 1
	doit_no_r 5 700000 1 1 256 10 1 1
	initdata 1000000 1 1 256 10 1
	doit_no_r 5 1000000 1 1 256 10 1 1
	initdata 2000000 1 1 256 10 1
	doit_no_r 5 2000000 1 1 256 10 1 1
}

# doit_with_r 2 $NUMLOCATIONS 1 1 $DAMAGEBINS 10 1 1
varynumberofsamples_no_r
varynumberofsamples_with_r
varynumberofevents
varynumberofintensitybins
varynumberoflocations


