#!/usr/bin/python

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

mkdir_p("results/plt")
purge("results/plt","fm_plt*")

total_processes = multiprocessing.cpu_count()
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
# The insured losses are streamed through summarycalc to summarize the samples to portfolio level 
# The portfolio level insured losses are streamed through eltcalc to calculate the sample statistics and output to a csv file. 


while (counter <= total_processes) :
	cmd="eve %d %d | getmodel | gulcalc -r -S%d -i - | fmcalc | summarycalc -f -2 - | pltcalc > results/plt/fm_plt_summary2_p%d.csv " % (counter,total_processes,samplesize,counter)
	p1 = subprocess.Popen(cmd,shell=True)
	procs.append(p1)
	print(cmd)
	counter = counter + 1
for p in procs:
	p.wait()
counter=1

# After all processes are finished, the final step is to concatenate the output files together. 

filenames = []
for file in os.listdir("results/plt"):
    if file.endswith(".csv"):
        file = "results/plt/"+ file
        filenames.append(file)

with open('results/plt/fm_plt_summary2.csv', 'w') as outfile:
    outfile.write("Summary_id, period_num, event_id, mean, standard_deviation, exposure_value, occ_year, occ_month, occ_date\n")
    for fname in filenames:
        with open(fname) as infile:
            lineno=1
            for line in infile:          	
                if lineno>1:
                	outfile.write(line)
                lineno=lineno+1  

print ("Finished. View outputs in results/plt")