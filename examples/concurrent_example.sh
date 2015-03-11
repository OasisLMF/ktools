#!/bin/bash

CPUCOUNT=`grep -c ^processor /proc/cpuinfo`
# The number of processes that will be used.
let TOTALPROCESSES=CPUCOUNT/1

COUNTER=1
CHUNKID=1
MAXCHUNK=2
SAMPLESIZE=100

# Clean up previous results
rm -f gul_results*.bin

# Loops through the processes and chunks. The ampersand at the end of line 29 instructs the code to run as a child process. 
# The wait command informs the script to wait for all child processes to finish.
# The inputs are a set of 2 binary files (one for each chunk) with naming convention e_chunk_[chunkid]_data.bin. Each contains a list of events. 
# The eve process partitions the events in a chunk to a numbered process (COUNTER). 
# The output stream of events invokes getmodel which calculates the CDFs for that subset of events.
# The CDF stream invokes gulcalc which performs the ground up loss sampling. The required parameter is the number of Samples -S.
# The losses are output to a binary output file. Alternatively use the command on line 28 to not output the stream to binary file.


while [ ${CHUNKID} -le ${MAXCHUNK} ]; do
        (

                while [ $COUNTER -le ${TOTALPROCESSES} ]; do
 #                      eve ${CHUNKID} ${COUNTER} ${TOTALPROCESSES} | getmodel ${CHUNKID}  | gulcalc -S${SAMPLESIZE} -C${CHUNKID} -R -r > /dev/null  &
                        eve ${CHUNKID} ${COUNTER} ${TOTALPROCESSES} | getmodel ${CHUNKID}  | gulcalc -S${SAMPLESIZE} -C${CHUNKID} -R -r > gul_results${CHUNKID}_$COUNTER.bin  &
                        echo "eve ${CHUNKID} ${COUNTER} ${TOTALPROCESSES} | getmodel ${CHUNKID}  | gulcalc -S${SAMPLESIZE} -C${CHUNKID} -R -r > gul_results${CHUNKID}_$COUNTER.bin" 
                        let COUNTER=COUNTER+1
                done
                echo "**** Waiting for finishing chunk ${CHUNKID} ****"
                wait
        )
COUNTER=1
let CHUNKID=CHUNKID+1
done

echo "**** Wait finished  ****"

