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
#include <set>
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

bool operator<(const summary_keyz& lhs, const summary_keyz& rhs)
{
	if (lhs.summary_id != rhs.summary_id) {
		return lhs.summary_id < rhs.summary_id;
	} else {
		return lhs.fileindex < rhs.fileindex;
	}
}

namespace leccalc {
	bool isSummaryCalcStream(unsigned int stream_type)
	{
		unsigned int stype = summarycalc_id & stream_type;
		return (stype == summarycalc_id);
	}


	template<typename T>
	void loadoccurrence(std::map<int, std::vector<int> >& event_to_periods,
			    T &occ, FILE * fin)
	{

		size_t i = fread(&occ, sizeof(occ), 1, fin);
		while (i != 0) {
			event_to_periods[occ.event_id].push_back(occ.period_no);
			i = fread(&occ, sizeof(occ), 1, fin);
		}
	}


	void loadoccurrence(std::map<int, std::vector<int> >& event_to_periods,
			    int& totalperiods)
	{
		FILE* fin = fopen(OCCURRENCE_FILE, "rb");
		if (fin == NULL) {
			fprintf(stderr, "FATAL: Error reading file %s\n",
				OCCURRENCE_FILE);
			exit(-1);
		}

		int date_opts = 0;
		size_t i = fread(&date_opts, sizeof(date_opts), 1, fin);
		i = fread(&totalperiods, sizeof(totalperiods), 1, fin);
		int granular_date = date_opts >> 1;
		if (granular_date) {
			occurrence_granular occ;
			loadoccurrence(event_to_periods, occ, fin);
		} else {
			occurrence occ;
			loadoccurrence(event_to_periods, occ, fin);
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


	inline void checkinputfile(unsigned int& samplesize, FILE * fin)
	{
		// read_stream_type()
		unsigned int stream_type = 0;
		size_t i = fread(&stream_type, sizeof(stream_type), 1, fin);
		if (i != 1 || isSummaryCalcStream(stream_type) != true) {
			std::cerr << "FATAL:: Not a summarycalc stream invalid stream type: " << stream_type << "\n";
			exit(-1);
		}

		// read_sample_size()
		i = fread(&samplesize, sizeof(samplesize), 1, fin);
		if (i != 1) {
			std::cerr << "FATAL: Stream read error: samplesize\n";
			exit(-1);
		}

		// read_summary_set_id()
		unsigned int summaryset_id;
		i = fread(&summaryset_id, sizeof(summaryset_id), 1, fin);
		if (i != 1) {
			std::cerr << "FATAL: Stream read error: summaryset_id\n";
			exit(-1);
		}
	}


	void processinputfile(
		unsigned int& samplesize,
		const std::map<int, std::vector<int> >& event_to_periods,
		std::set<int> &summaryids,
		std::vector<std::map<outkey2, OutLosses>> &out_loss)
	{
		checkinputfile(samplesize, stdin);

		// read_event()
		size_t i = 1;
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

			summaryids.insert(sh.summary_id);

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

	inline void outputaggreports(const int totalperiods, const std::set<int> &summaryids, std::vector<std::map<outkey2, OutLosses>> &out_loss,
				     FILE **fout, const bool useReturnPeriodFile, const int samplesize, const bool skipheader,
				     const bool *outputFlags, const bool ordFlag)
	{
		aggreports agg(totalperiods, summaryids, out_loss, fout, useReturnPeriodFile, samplesize, skipheader, outputFlags, ordFlag);
		agg.OutputOccMeanDamageRatio();
		agg.OutputAggMeanDamageRatio();
		agg.OutputOccFullUncertainty();
		agg.OutputAggFullUncertainty();
		agg.OutputOccWheatsheafAndWheatsheafMean();
		agg.OutputAggWheatsheafAndWheatsheafMean();
		agg.OutputOccSampleMean();
		agg.OutputAggSampleMean();
	}

	void doit(const std::string &subfolder, FILE **fout, const bool useReturnPeriodFile, bool skipheader, bool *outputFlags, bool ordFlag)
	{
		std::string path = "work/" + subfolder;
		if (path.substr(path.length() - 1, 1) != "/") {
			path = path + "/";
		}
		std::map<int, std::vector<int> > event_to_periods;
		int totalperiods;
		loadoccurrence(event_to_periods, totalperiods);
		std::vector<std::map<outkey2, OutLosses>> out_loss(2);

		std::vector<std::string> files;
		std::vector<FILE*> fileHandles;
		std::vector<std::string> indexFiles;
		unsigned int samplesize=0;
		std::set<int> summaryids;
		if (subfolder.size() == 0) {
			processinputfile(samplesize, event_to_periods, summaryids, out_loss);
			outputaggreports(totalperiods, summaryids, out_loss, fout, useReturnPeriodFile, samplesize, skipheader, outputFlags, ordFlag);
			return;
		} else {
			// Store order of input files for future reference
			std::string fileList = path + "filelist.idx";
			FILE * flist = fopen(fileList.c_str(), "w");
			if (flist == nullptr) {
				fprintf(stderr, "FATAL: Cannot open %s\n", fileList.c_str());
				exit(EXIT_FAILURE);
			}
			DIR* dir;
			struct dirent* ent;
			if ((dir = opendir(path.c_str())) != NULL) {
				while ((ent = readdir(dir)) != NULL) {
					std::string s = ent->d_name;
					if (s.length() > 4 && s.substr(s.length() - 4, 4) == ".bin") {
						s = path + ent->d_name;
						FILE * in = fopen(s.c_str(), "rb");
						if (in != nullptr) {
							checkinputfile(samplesize, in);
						} else {
							fprintf(stderr, "FATAL: Cannot open %s\n", s.c_str());
							exit(EXIT_FAILURE);
						}
						files.push_back(s);   // Build input file list
						fileHandles.push_back(in);
						fprintf(flist, "%s\n", s.c_str());
						// Check for index file
						s.erase(s.length() - 4);
						s += ".idx";
						FILE * fidx = fopen(s.c_str(), "r");
						if (fidx == nullptr) continue;
						indexFiles.push_back(s);
						fclose(fidx);
					}
				}
			}
			else {
				fprintf(stderr, "FATAL: Unable to open directory %s\n", path.c_str());
				exit(-1);
			}
			fclose(flist);
		}

		if (files.size() == indexFiles.size()) {
			// Combine summary index files into one master summary index file
			int file_index = 0;
			char line[4096];
			std::map<summary_keyz, std::vector<long long>> summary_file_to_offset;
			for (std::vector<std::string>::iterator it = indexFiles.begin(); it != indexFiles.end(); ++it) {
				FILE * fidx = fopen(it->c_str(), "r");
				summary_keyz summaryFile;
				summaryFile.fileindex = file_index++;
				while (fgets(line, sizeof(line), fidx) != 0) {
					long long offset;
					int ret = sscanf(line, "%d,%lld", &summaryFile.summary_id, &offset);
					if (ret != 2) {
						fprintf(stderr, "FATAL: Invalid data in file %s:\n%s",
							it->c_str(), line);
						exit(-1);
					}
					summary_file_to_offset[summaryFile].push_back(offset);
				}
				fclose(fidx);
			}
			std::string summaryFile = path + "summaries.idx";
			FILE * fsummaries = fopen(summaryFile.c_str(), "w");
			if (fsummaries == nullptr) {
				fprintf(stderr, "Cannot open %s\n", summaryFile.c_str());
				exit(EXIT_FAILURE);
			}
			for (std::map<summary_keyz, std::vector<long long>>::iterator it_s = summary_file_to_offset.begin();
			     it_s != summary_file_to_offset.end(); ++it_s) {
				for(std::vector<long long>::iterator it_v = it_s->second.begin(); it_v != it_s->second.end(); ++it_v) {
					fprintf(fsummaries, "%d,%d,%lld\n", it_s->first.summary_id, it_s->first.fileindex, *it_v);
				}
			}
			summary_file_to_offset.clear();   // Recover memory
			fclose(fsummaries);

			// Read input files by summary ID using master summary index file
			int summary_id;
			file_index = 0;
			long long file_offset;
			int last_summary_id = -1;
			int last_file_index = -1;
			FILE * fidx = fopen(summaryFile.c_str(), "r");
			FILE * summary_fin = nullptr;
			while (fgets(line, sizeof(line), fidx) != 0) {
				int ret = sscanf(line, "%d,%d,%lld", &summary_id, &file_index, &file_offset);
				if (ret != 3) {
					fprintf(stderr, "FATAL: Invalid data in file %s:\n%s", summaryFile.c_str(), line);
					exit(-1);
				}
				if (last_summary_id != summary_id) {
					if (last_summary_id != -1) {
						summaryids.clear();
						summaryids.insert(last_summary_id);
						outputaggreports(totalperiods, summaryids, out_loss, fout, useReturnPeriodFile, samplesize, skipheader, outputFlags, ordFlag);
						out_loss = std::vector<std::map<outkey2, OutLosses>>(2);   // Reset
						skipheader = true;   // Only print header for first summary ID if requested
					}
					last_summary_id = summary_id;
				}
				if (last_file_index != file_index) {
					summary_fin = fileHandles[file_index];
					last_file_index = file_index;
				}
				if (summary_fin != nullptr) {
					std::vector<sampleslevelRec> vrec;
					flseek(summary_fin, file_offset, SEEK_SET);
					summarySampleslevelHeader sh;
					size_t i = fread(&sh, sizeof(sh), 1, summary_fin);
					std::map<int, std::vector<int> >::const_iterator iter = event_to_periods.find(sh.event_id);
					if (iter == event_to_periods.end()) continue;   // Event not found so don't process it
					while (i != 0) {
						sampleslevelRec sr;
						i = fread(&sr, sizeof(sr), 1, summary_fin);
						if (i != 1 || sr.sidx == 0) break;
						dolecoutputaggsummary(sh.summary_id, sr.sidx, sr.loss, iter->second, out_loss);
					}
				}
			}
			summaryids.clear();
			summaryids.insert(last_summary_id);
			outputaggreports(totalperiods, summaryids, out_loss, fout, useReturnPeriodFile, samplesize, skipheader, outputFlags, ordFlag);
		} else {
			if (indexFiles.size() > 0) {
				fprintf(stderr, "%d summary index files missing: "
						"Ignoring summary index files.\n",
					(int)indexFiles.size() - (int)files.size());
			}
			for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it) {
				setinputstream(*it);
				processinputfile(samplesize, event_to_periods, summaryids, out_loss);
			}
			outputaggreports(totalperiods, summaryids, out_loss, fout, useReturnPeriodFile, samplesize, skipheader, outputFlags, ordFlag);
		}
	}

}
