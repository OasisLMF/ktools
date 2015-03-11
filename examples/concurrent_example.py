#!/usr/bin/python

import multiprocessing
import os, re
import subprocess


def purge(dir, pattern):
    for f in os.listdir(dir):
        if re.search(pattern, f):
                os.remove(os.path.join(dir, f))


purge(".","gul_results*")
total_processes = multiprocessing.cpu_count()
counter=1
chunkid=1
maxchunk=2
samplesize=100
procs = []
# Loops through the processes and chunks.
# The wait command informs the script to wait for all child processes to finish.
# The inputs are a set of 2 binary files (one for each chunk) with naming convention e_chunk_[chunkid]_data.bin. Each contains a list of events. 
# The eve process partitions the events in a chunk to a numbered process (counter). 
# The output stream of events invokes getmodel which calculates the CDFs for that subset of events.
# The CDF stream invokes gulcalc which performs the ground up loss sampling. The required parameter is the number of Samples -S.
# The losses are output to a binary output file. Alternatively use the command on line 32 to not output the stream to binary file.

while (chunkid <= maxchunk):
	while (counter <= total_processes) :
		cmd="eve %d %d %d | getmodel %d | gulcalc -S%d -C%d -R -r  >gul_results%d_%d.bin " % (chunkid,counter,total_processes,chunkid,samplesize,chunkid,chunkid,counter)
#                cmd="eve %d %d %d | getmodel %d | gulcalc -S%d -C%d -R -r  >/dev/null " % (chunkid,counter,total_processes,chunkid,samplesize,chunkid)
		p1 = subprocess.Popen(cmd,shell=True)
		procs.append(p1)
		print cmd
		counter = counter + 1
	for p in procs:
		p.wait()
	chunkid = chunkid + 1
	counter=1

