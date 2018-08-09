#!/usr/bin/python
# Warning: This uses Unix specific command os.mkfifo so this sript should only be run in a Linux or Cygwin 64-bit terminal. 

import multiprocessing
import os, re, errno
import subprocess
import tempfile
import shutil

tmp_dir = ""
resultsfolder = ""

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

def rmdir_f(folder):
	for the_file in os.listdir(folder):
		file_path = os.path.join(folder,the_file)
		try:
			if os.path.isfile(file_path):
				os.unlink(file_path)
			elif os.path.isdir(file_path): shutil.rmtree(file_path)
		except Exception (e):
			print (e)

# Sets up the results folder and named pipe folder
def init():
	global tmp_dir
	global resultsfolder
	resultsfolder = "results/samples_all"
	tmp_dir = "tmp"
	mkdir_p(resultsfolder)
	mkdir_p(tmp_dir)
	purge("results/samples_all","")
	

# Runs the workflow
def doit():
	global tmp_dir
	total_processes = multiprocessing.cpu_count()
	counter=1
	samplesize=100
	summaryset=2
	procs = []
	
	cmd_gulkat = 'kat'
	cmd_fmkat = 'kat'

	# Make listening pipes and command strings to concatinate results from main workflow in stream and output to file.
	while (counter <= total_processes) :
		fifo_fmsummary = '%s/fmsummary%d' % (tmp_dir,counter)
		os.mkfifo(fifo_fmsummary)
		fifo_gulsummary = '%s/gulsummary%d' % (tmp_dir,counter)
		os.mkfifo(fifo_gulsummary) 
		cmd_fmkat = cmd_fmkat + ' ' + fifo_fmsummary
		cmd_gulkat = cmd_gulkat + ' ' + fifo_gulsummary
		counter = counter + 1
	
	# Initiate listening pipes to concatinate results from main workflow in stream and output to file. 
	cmd_fmkat = cmd_fmkat + ' > %s/fm_samples.csv' % (resultsfolder)
	p1 = subprocess.Popen(cmd_fmkat,shell=True)
	procs.append(p1)
	print (cmd_fmkat)
	cmd_gulkat = cmd_gulkat + ' > %s/gul_samples.csv' % (resultsfolder)
	p1 = subprocess.Popen(cmd_gulkat,shell=True)
	procs.append(p1)
	print (cmd_gulkat)

	# Main workflow. Creates and initiates processes one by one.
	counter = 1
	cmd_tocsv = 'summarycalctocsv'

	while (counter <= total_processes) :
		#This sets up named pipes, one for each fmcalc process.
		fifo_fm = '%s/fm%d' % (tmp_dir,counter)
		os.mkfifo(fifo_fm) 
		# This is the 'listening' command for the fmcalc pipes. This must be initiated first, to receive the data on the named pipe.
		# The data from the named pipe goes into fmcalc which then streams fm losses to summarycalc and outputs the results to the fmsummary pipe, which is passed to kat.
		cmd="fmcalc < %s | summarycalc -f -%d - | %s > %s/fmsummary%d " % (fifo_fm, summaryset, cmd_tocsv, tmp_dir,counter)
		p1 = subprocess.Popen(cmd,shell=True)
		procs.append(p1)		
		print (cmd)
		# This command sents gulcalc item losses (-i) to the named pipe, to be picked up by the listening command above, while gulcalc coverage losses (-c) are sent down the pipeline to summarycalc which is output to the gulsummary pipe, which is passed to kat. 
		cmd="eve %d %d | getmodel | gulcalc -r -S%d -i %s -c - | summarycalc -g -%d - | %s > %s/gulsummary%d" % (counter,total_processes,samplesize, fifo_fm, summaryset, cmd_tocsv, tmp_dir,counter)
		p1 = subprocess.Popen(cmd,shell=True)
		procs.append(p1)
		print (cmd)
		cmd_tocsv = 'summarycalctocsv -s' # skip headers in output for the 2nd to last process.
		counter = counter + 1

	# Waits for all processes to complete
	for p in procs:
		p.wait()

def fin():
	global tmp_dir
	shutil.rmtree(tmp_dir)
	print ("Finished. View outputs in results/samples_all")

init()
doit()
fin()


