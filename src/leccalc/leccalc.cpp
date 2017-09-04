/*
* Copyright (c)2015 - 2016 Oasis LMF Limited
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*
*   * Neither the original author of this software nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
* THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*/

/*
Loss exceedance curve
Author: Ben Matharu  email : ben.matharu@oasislmf.org
*/


/*

Because we are summarizing over all the events we cannot split this into separate processes and must do the summary in a single process
To achieve this we first output the results from summary calc to a set of files in a sub directory.
This process will then read all the *.bin files in the subdirectory and then compute the loss exceedance curve in a single process

*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <map>
#include "leccalc.h"
#include "aggreports.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#include "../include/dirent.h"
#else
#include <unistd.h>
#include <dirent.h>
#endif


using namespace std;

bool isSummaryCalcStream(unsigned int stream_type)
{
	unsigned int stype = summarycalc_id & stream_type;
	return (stype == summarycalc_id);
}

bool operator<(const outkey2& lhs, const outkey2& rhs)
{
	if (lhs.period_no != rhs.period_no) {
		return lhs.period_no < rhs.period_no;
	}
	if (lhs.sidx != rhs.sidx) {
		return lhs.sidx < rhs.sidx;
	}else {
		return lhs.summary_id < rhs.summary_id;
	}
}


void loadoccurence(std::map<int, std::vector<int> > &event_to_periods, int &totalperiods)
{
	FILE *fin = fopen(OCCURRENCE_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: Error reading file %s\n", __func__, OCCURRENCE_FILE);
		exit(-1);
	}
	int date_algorithm;
	occurrence occ;
	size_t i = fread(&date_algorithm, sizeof(date_algorithm), 1, fin);
	i = fread(&totalperiods, sizeof(totalperiods), 1, fin);
	i = fread(&occ, sizeof(occ), 1, fin);
	while (i != 0) {		
		event_to_periods[occ.event_id].push_back(occ.period_no);
		i = fread(&occ, sizeof(occ), 1, fin);
	}

	fclose(fin);

}

//
//
//

inline void dolecoutputaggsummary(int summary_id, int sidx, OASIS_FLOAT loss, const std::vector<int> &periods, 
	std::map<outkey2, OASIS_FLOAT> &agg_out_loss, std::map<outkey2, OASIS_FLOAT> &max_out_loss)
{
	outkey2 key;
	key.summary_id = summary_id;

	for (auto x : periods) {
		key.period_no = x;
		key.sidx = sidx;
		agg_out_loss[key] += loss;
		if (max_out_loss[key] < loss) max_out_loss[key] = loss;

	}
}


void processinputfile(unsigned int &samplesize,const std::map<int, std::vector<int> > &event_to_periods, 
		int &maxsummaryid, std::map<outkey2, OASIS_FLOAT> &agg_out_loss, std::map<outkey2, OASIS_FLOAT> &max_out_loss)
{
	unsigned int stream_type = 0;
	size_t i = fread(&stream_type, sizeof(stream_type), 1, stdin);
	if (isSummaryCalcStream(stream_type) == true) {
		unsigned int summaryset_id;
		i = fread(&samplesize, sizeof(samplesize), 1, stdin);
		if (i == 1) i = fread(&summaryset_id, sizeof(summaryset_id), 1, stdin);
		if (i == 1) {
			while (i != 0) {
				bool processEvent = true;
				summarySampleslevelHeader sh;
				i = fread(&sh, sizeof(sh), 1, stdin);
				std::map<int, std::vector<int> >::const_iterator iter;
				if (i) {
					iter = event_to_periods.find(sh.event_id);
					if (iter == event_to_periods.end()) {
						// Event not found so don't process it
						processEvent = false;
						//fprintf(stderr, "Event id %d not found in occurrence.bin\n", sh.event_id);
						//exit(-1);
					}else {
						if (maxsummaryid < sh.summary_id) maxsummaryid = sh.summary_id;
					}
				}
				while (i != 0) {
					sampleslevelRec sr;
					i = fread(&sr, sizeof(sr), 1, stdin);
					if (i == 0) break;
					if (sr.sidx == 0) break;
					//				dolecoutput1(sh.summary_id, sr.loss,iter->second);					
					if (sr.sidx != -2) {
						if (sr.loss > 0.0 && processEvent == true) dolecoutputaggsummary(sh.summary_id, sr.sidx, sr.loss, iter->second,agg_out_loss,max_out_loss);
					}
				}
			}
		}
		else {
			std::cerr << "Stream read error\n";
			exit(-1);
		}
		return;
	}
	else {
		std::cerr << "Not a summarycalc stream\n";
		std::cerr << "invalid stream type: " << stream_type << "\n";
		exit(-1);
	}
}

void setinputstream(const std::string &inFile)
{
	if (freopen(inFile.c_str(), "rb", stdin) == NULL) {
		fprintf(stderr, "%s: Error opening  %s\n", __func__, inFile.c_str());
		exit(-1);
	}

}

void doit(const std::string &subfolder, FILE **fout, bool useReturnPeriodFile)
{
	std::string path = "work/" + subfolder;
	if (path.substr(path.length() - 1, 1) != "/") {
		path = path + "/";
	}
	std::map<int, std::vector<int> > event_to_periods;
	int totalperiods;
	loadoccurence(event_to_periods,totalperiods);
	std::map<outkey2, OASIS_FLOAT> agg_out_loss;
	std::map<outkey2, OASIS_FLOAT> max_out_loss;

	
	unsigned int samplesize;
	int maxsummaryid = -1;
	if (subfolder.size() == 0) {		
		processinputfile(samplesize, event_to_periods, maxsummaryid, agg_out_loss, max_out_loss);
	}
	else {
		DIR *dir;
		struct dirent *ent;
		if ((dir = opendir(path.c_str())) != NULL) {
			while ((ent = readdir(dir)) != NULL) {
				std::string s = ent->d_name;
				if (s.length() > 4 && s.substr(s.length() - 4, 4) == ".bin") {
					s = path + ent->d_name;
					setinputstream(s);
					processinputfile(samplesize, event_to_periods, maxsummaryid, agg_out_loss, max_out_loss);
				}
			}
		}
		else {
			fprintf(stderr, "Unable to open directory %s\n", path.c_str());
			exit(-1);
		}
	}

	aggreports agg(totalperiods,maxsummaryid, agg_out_loss, max_out_loss, fout,useReturnPeriodFile,samplesize);
	agg.loadperiodtoweigthing();
	agg.outputAggWheatsheaf();
	agg.outputAggFulluncertainty();
	agg.outputAggWheatSheafMean(samplesize);
	agg.outputAggSampleMean(samplesize);
	agg.outputOccWheatsheaf();
	agg.outputOccFulluncertainty();
	agg.outputOccWheatSheafMean(samplesize);
	agg.outputOccSampleMean(samplesize);

}


void openpipe(int output_id,const std::string &pipe, FILE **fout)
{
	if (pipe == "-") fout[output_id] = stdout;
	else {
		FILE *f = fopen(pipe.c_str(), "wb");
		if (f != nullptr) fout[output_id] = f;
		else {
			fprintf(stderr, "%s: Cannot open %s for output\n", __func__, pipe.c_str());
			::exit(-1);
		}
	}
}


void help()
{
	fprintf(stderr, "-F Aggregate Full Uncertainty\n");
	fprintf(stderr, "-W Aggregate Wheatsheaf\n");
	fprintf(stderr, "-S Aggregate sample mean\n");
	fprintf(stderr, "-M Aggregate Wheatsheaf mean\n");
	fprintf(stderr, "-f Occurrence Full Uncertainty\n");
	fprintf(stderr, "-w Occurrence Wheatsheaf\n");
	fprintf(stderr, "-s Occurrence sample mean\n");
	fprintf(stderr, "-m Occurrence Wheatsheaf mean\n");
	fprintf(stderr, "-K workspace sub folder\n");
	fprintf(stderr, "-r use return period file\n");
	fprintf(stderr, "-h help\n");
	fprintf(stderr, "-v version\n");
}


// 
int main(int argc, char* argv[])
{
	bool useReturnPeriodFile = false;
	FILE *fout[] = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };

	std::string subfolder;
	int opt;
	while ((opt = getopt(argc, argv, "vhrF:W:M:S:K:f:w:s:m:")) != -1) {
		switch (opt) {
		case 'K':
			subfolder = optarg;
			break;
		case 'F':
			openpipe(AGG_FULL_UNCERTAINTY, optarg, fout);
			break;
		case 'f':
			openpipe(OCC_FULL_UNCERTAINTY, optarg, fout);
			break;
		case 'W':
			openpipe(AGG_WHEATSHEAF, optarg, fout);
			break;
		case 'w':
			openpipe(OCC_WHEATSHEAF, optarg, fout);
			break;
		case 'S':
			openpipe(AGG_SAMPLE_MEAN, optarg, fout);
			break;
		case 's':
			openpipe(OCC_SAMPLE_MEAN, optarg, fout);
			break;
		case 'M':
			openpipe(AGG_WHEATSHEAF_MEAN, optarg, fout);
			break;
		case 'm':
			openpipe(OCC_WHEATSHEAF_MEAN, optarg, fout);
			break;
		case 'r':
			useReturnPeriodFile = true;
			break;
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'h':
			help();
			::exit(EXIT_FAILURE);
			break;
		default:
			help();
			::exit(EXIT_FAILURE);
		}
	}

	if (argc == 1) {
		fprintf(stderr, "Invalid parameters\n");
		help();
	}	
	initstreams();	
	doit(subfolder,fout, useReturnPeriodFile);
	return 0;

}

