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
randomnumbers=1000000
procs = []
# Loops through the processes 
# The wait command informs the script to wait for all child processes to finish.
# The eve process partitions the events to a numbered process (counter). 
# The output stream of events invokes getmodel which calculates the CDFs for that subset of events.
# The CDF stream invokes gulcalc which performs the ground up loss sampling. The required parameter is the number of Samples -S and number of random numbers -R.
# The losses are output to a binary output file. Alternatively use the command on line 31 to not output the stream to binary file.

while (counter <= total_processes) :
	cmd="eve 1 %d %d | getmodel 1 | gulcalc -S%d -R%d -i - > gul_results_%d.bin " % (counter,total_processes,samplesize,randomnumbers,counter)
#               cmd="eve %d %d %d | getmodel %d | gulcalc -S%d -C%d -R -r  >/dev/null " % (chunkid,counter,total_processes,chunkid,samplesize,chunkid)
	p1 = subprocess.Popen(cmd,shell=True)
	procs.append(p1)
	print cmd
	counter = counter + 1
for p in procs:
	p.wait()
chunkid = chunkid + 1
counter=1

