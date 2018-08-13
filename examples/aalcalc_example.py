#!/usr/bin/env python

import multiprocessing
import os, re, errno
import subprocess


def purge(dir, pattern):
    for f in os.listdir(dir):
        if re.search(pattern, f):
                os.remove(os.path.join(dir, f))

def mkdir_p(path):
        try:
                os.makedirs(path)
        except OSError as exc:
                if exc.errno == errno.EEXIST and os.path.isdir(path):
                        pass
                else: raise

# mkdir_p("work/aal2")
mkdir_p("work/summary2")
mkdir_p("results/aal")
purge("work/aal2","")
purge("results/aal","fm_aal*")

total_processes = multiprocessing.cpu_count()
#total_processes = 1 
counter=1
samplesize=100
randomnumbers=1000000
procs = []
# Loops through the processes 
# The wait command informs the script to wait for all child processes to finish.
# The eve process partitions the events to a numbered process (counter). 
# The output stream of events invokes getmodel which calculates the CDFs for that subset of events.
# The CDF stream invokes gulcalc which performs the ground up loss sampling. The required parameters are the number of Samples -S and number of random numbers -R, and output stream type -i.
# The ground up losses are streamed through to fmcalc to apply policy terms and conditions and output insured losses
# The insured losses are streamed through summarycalc to summarize the samples to policy level (sumamry set 1) 
# The policy losses are streamed through aalcalc is output to binary files in a work folder. 


while (counter <= total_processes) :
    # cmd="eve %d %d | getmodel | gulcalc -r -S%d -i - | fmcalc | summarycalc -f -2 - | aalcalc > work/aal2/p%d.bin " % (counter,total_processes,samplesize,counter)
	cmd="eve %d %d | getmodel | gulcalc -r -S%d -i - | fmcalc | summarycalc -f -2 - > work/summary2/p%d.bin " % (counter,total_processes,samplesize,counter)
	print(cmd)
	p1 = subprocess.Popen(cmd,shell=True)
	procs.append(p1)
	print(cmd)
	counter = counter + 1
for p in procs:
	p.wait()
counter=1

# After all processes are finished, the final step is to run aalsummary on the work folder containing the aalcalc samples. 
# cmd="aalsummary -Kaal2 > results/aal/fm_aal.csv"
cmd="aalcalc -Ksummary2 > results/aal/fm_aal.csv"
print(cmd)
p1 = subprocess.Popen(cmd,shell=True)
print("Finished. View outputs in results/aal")
  
