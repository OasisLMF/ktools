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

bool operator<(const outkey2& lhs, const outkey2& rhs)
{
	if (lhs.period_no != rhs.period_no) {
		return lhs.period_no < rhs.period_no;
	}
	if (lhs.sidx != rhs.sidx) {
		return lhs.sidx < rhs.sidx;
	}
	else {
		return lhs.summary_id < rhs.summary_id;
	}
}

namespace leccalc {
	bool isSummaryCalcStream(unsigned int stream_type)
	{
		unsigned int stype = summarycalc_id & stream_type;
		return (stype == summarycalc_id);
	}


	void loadoccurence(std::map<int, std::vector<int> >& event_to_periods, int& totalperiods)
	{
		FILE* fin = fopen(OCCURRENCE_FILE, "rb");
		if (fin == NULL) {
			fprintf(stderr, "FATAL: Error reading file %s\n", OCCURRENCE_FILE);
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


	inline void dolecoutputaggsummary(int summary_id, int sidx, OASIS_FLOAT loss, const std::vector<int>& periods,
		std::vector<std::map<outkey2, OutLosses>> &out_loss)
	{
		outkey2 key;
		key.summary_id = summary_id;
		key.sidx = sidx;

		for (auto x : periods) {
			key.period_no = x;
			out_loss[sidx != -1][key].agg_out_loss += loss;
			if (out_loss[sidx != -1][key].max_out_loss < loss) out_loss[sidx != -1][key].max_out_loss = loss;
		}
	}


	void processinputfile(
		unsigned int& samplesize,
		const std::map<int, std::vector<int> >& event_to_periods,
		int& maxsummaryid,
		std::vector<std::map<outkey2, OutLosses>> &out_loss)
	{
		// read_stream_type()
		unsigned int stream_type = 0;
		size_t i = fread(&stream_type, sizeof(stream_type), 1, stdin);
		if (i != 1 || isSummaryCalcStream(stream_type) != true) {
			std::cerr << "FATAL:: Not a summarycalc stream invalid stream type: " << stream_type << "\n";
			exit(-1);
		}

		// read_sample_size()
		i = fread(&samplesize, sizeof(samplesize), 1, stdin);
		if (i != 1) {
			std::cerr << "FATAL: Stream read error: samplesize\n";
			exit(-1);
		}

		// read_summary_set_id()
		unsigned int summaryset_id;
		i = fread(&summaryset_id, sizeof(summaryset_id), 1, stdin);
		if (i != 1) {
			std::cerr << "FATAL: Stream read error: summaryset_id\n";
			exit(-1);
		}

		// read_event()
		while (i != 0) {
			summarySampleslevelHeader sh;
			i = fread(&sh, sizeof(sh), 1, stdin);
			if (i != 1) {
				break;
			}

			// discard samples if eventis not found
			std::map<int, std::vector<int> >::const_iterator iter = event_to_periods.find(sh.event_id);
			if (iter == event_to_periods.end()) { // Event not found so don't process it, but read samples
				while (i != 0) {
					sampleslevelRec sr;
					i = fread(&sr, sizeof(sr), 1, stdin);
					if (i != 1 || sr.sidx == 0) {
						break;
					}
				}
				continue;
			}

			if (maxsummaryid < sh.summary_id)
			{
				maxsummaryid = sh.summary_id;
			}

			// read_samples_and_compute_lec()
			while (i != 0) {
				sampleslevelRec sr;
				i = fread(&sr, sizeof(sr), 1, stdin);
				if (i != 1) {
					break;
				}

				if (sr.sidx == 0) {			//  end of sidx stream
					break;
				}

				if (sr.loss > 0.0)
				{
					dolecoutputaggsummary(sh.summary_id, sr.sidx, sr.loss, iter->second, out_loss);
				}
			}
		}
	}

	void setinputstream(const std::string& inFile)
	{
		if (freopen(inFile.c_str(), "rb", stdin) == NULL) {
			fprintf(stderr, "FATAL: Error opening  %s\n", inFile.c_str());
			exit(-1);
		}

	}

	void doit(const std::string &subfolder, FILE **fout, const bool useReturnPeriodFile, const bool skipheader, bool *outputFlags, bool ordFlag)
	{
		std::string path = "work/" + subfolder;
		if (path.substr(path.length() - 1, 1) != "/") {
			path = path + "/";
		}
		std::map<int, std::vector<int> > event_to_periods;
		int totalperiods;
		loadoccurence(event_to_periods, totalperiods);
		std::vector<std::map<outkey2, OutLosses>> out_loss(2);


		unsigned int samplesize=0;
		int maxsummaryid = -1;
		if (subfolder.size() == 0) {
			processinputfile(samplesize, event_to_periods, maxsummaryid, out_loss);
		}
		else {
			DIR* dir;
			struct dirent* ent;
			if ((dir = opendir(path.c_str())) != NULL) {
				while ((ent = readdir(dir)) != NULL) {
					std::string s = ent->d_name;
					if (s.length() > 4 && s.substr(s.length() - 4, 4) == ".bin") {
						s = path + ent->d_name;
						setinputstream(s);
						processinputfile(samplesize, event_to_periods, maxsummaryid, out_loss);
					}
				}
			}
			else {
				fprintf(stderr, "FATAL: Unable to open directory %s\n", path.c_str());
				exit(-1);
			}
		}

		aggreports agg(totalperiods, maxsummaryid, out_loss, fout, useReturnPeriodFile, samplesize, skipheader, outputFlags, ordFlag);
		agg.OutputOccMeanDamageRatio();
		agg.OutputAggMeanDamageRatio();
		agg.OutputOccFullUncertainty();
		agg.OutputAggFullUncertainty();
		agg.OutputOccWheatsheafAndWheatsheafMean();
		agg.OutputAggWheatsheafAndWheatsheafMean();
		agg.OutputOccSampleMean();
		agg.OutputAggSampleMean();

	}

}
