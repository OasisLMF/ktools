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
Author: Ben Matharu  email : ben.matharu@oasislmf.org
*/
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <map>
#include <vector>

#include <chrono>
#include <thread>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif


#include "../include/oasis.h"

using namespace std;

struct interval {
	int integer_part;
	float fractional_part;
};

namespace eltcalc {
	
	bool firstOutput = true;
	std::map<int, double> event_rate_;
	std::map<float, interval> intervals_;
	enum { MELT = 0, QELT };

	bool isSummaryCalcStream(unsigned int stream_type)
	{
		unsigned int stype = summarycalc_id & stream_type;
		return (stype == summarycalc_id);
	}

	template<typename T>
	void GetEventRates(T &occ, FILE * fin)
	{
		int max_period_no = 0;
		int no_of_periods = 0;
		size_t i = fread(&no_of_periods, sizeof(no_of_periods), 1, fin);
		i = fread(&occ, sizeof(occ), 1, fin);
		while (i != 0) {
			event_rate_[occ.event_id] += 1 / (double)no_of_periods;
			if (max_period_no < occ.period_no)
				max_period_no = occ.period_no;
			i = fread(&occ, sizeof(occ), 1, fin);
		}
		fclose(fin);

		if (max_period_no > no_of_periods) {
			fprintf(stderr, "FATAL: Maximum period number in occurrence file exceeds that in header.\n");
			exit(EXIT_FAILURE);
		}
	}

	void GetEventRates()
	{
		FILE * fin = fopen(OCCURRENCE_FILE, "rb");
		if (fin == NULL) {
			fprintf(stderr, "FATAL: %s: Error opening file %s\n",
				__func__, OCCURRENCE_FILE);
			exit(EXIT_FAILURE);
		}

		int date_opts = 0;
		size_t i = fread(&date_opts, sizeof(date_opts), 1, fin);
		int granular_date = date_opts >> 1;
		if (granular_date) {
			occurrence_granular occ;
			GetEventRates(occ, fin);
		} else {
			occurrence occ;
			GetEventRates(occ, fin);
		}
	}

	void GetIntervals(int samplesize)
	{
		FILE * fin = fopen(QUANTILE_FILE, "rb");
		if (fin == NULL) {
			fprintf(stderr, "FATAL: %s: Error opening file %s\n",
				__func__, QUANTILE_FILE);
			exit(EXIT_FAILURE);
		}

		float q;
		size_t i = fread(&q, sizeof(q), 1, fin);
		while (i != 0) {
			interval idx;
			float pos = (samplesize - 1) * q + 1;
			idx.integer_part = (int)pos;
			idx.fractional_part = pos - idx.integer_part;
			intervals_[q] = idx;
			i = fread(&q, sizeof(q), 1, fin);
		}

		fclose(fin);
	}

	inline void WriteOutput(const char * buffer, int strLen, FILE * outFile)
	{
		const char * bufPtr = buffer;
		int num;
		int counter = 0;
		do {

			num = fprintf(outFile, "%s", bufPtr);
			if (num < 0) {   // Write error
				fprintf(stderr, "FATAL: Error writing %s: %s\n",
					buffer, strerror(errno));
				exit(EXIT_FAILURE);
			} else if (num < strLen) {   // Incomplete write
				bufPtr += num;
				strLen -= num;
			} else return;   // Success

			fprintf(stderr, "INFO: Attempt %d to write %s\n",
				++counter, buffer);

		} while (counter < 10);

		fprintf(stderr, "FATAL: Maximum attempts to write %s exceeded\n",
			buffer);
		exit(EXIT_FAILURE);
	}

	void OutputRows(const summarySampleslevelHeader& sh, const int type,
			const OASIS_FLOAT mean, const OASIS_FLOAT stdDev,
			FILE * outFile)
	{

		char buffer[4096];
		int strLen;
		strLen = sprintf(buffer, "%d,%d,%d,%f,%f,%f\n", sh.summary_id,
				 type, sh.event_id, mean, stdDev, sh.expval);

		WriteOutput(buffer, strLen, outFile);

	}

	void OutputRowsORD(const summarySampleslevelHeader& sh, const int type,
			   const OASIS_FLOAT mean, const OASIS_FLOAT stdDev,
			   FILE * outFile)
	{

		if (outFile == nullptr) return;
		char buffer[4096];
		int strLen;
		strLen = sprintf(buffer, "%d,%d,%d,%f,%f,%f,%f\n", sh.event_id,
				sh.summary_id, type, event_rate_[sh.event_id],
				mean, stdDev, sh.expval);
		WriteOutput(buffer, strLen, outFile);

	}

	void OutputQuantiles(const summarySampleslevelHeader& sh,
			     std::vector<OASIS_FLOAT>& losses_vec,
			     FILE * outFile)
	{

		std::sort(losses_vec.begin(), losses_vec.end());

		for (std::map<float, interval>::iterator it = intervals_.begin();
		     it != intervals_.end(); ++it) {

			int idx = it->second.integer_part;
			OASIS_FLOAT loss = 0;
			if (idx == losses_vec.size()) {
				loss = losses_vec[idx-1];
			} else {
				loss = (losses_vec[idx] - losses_vec[idx-1]) * it->second.fractional_part + losses_vec[idx-1];
			}

			char buffer[4096];
			int strLen;
			strLen = sprintf(buffer, "%d,%d,%f,%f\n", sh.event_id,
					 sh.summary_id, it->first, loss);
			WriteOutput(buffer, strLen, outFile);

		}

	}

	void doetloutput(int samplesize, bool skipHeader, bool ordOutput, FILE **fout)
	{
		OASIS_FLOAT sumloss = 0.0;
		OASIS_FLOAT sample_mean = 0.0;
		OASIS_FLOAT analytical_mean = 0.0;
		OASIS_FLOAT sd = 0;
		OASIS_FLOAT sumlosssqr = 0.0;
		void (*OutputData)(const summarySampleslevelHeader&, const int,
				   const OASIS_FLOAT, const OASIS_FLOAT, FILE*);
		FILE * outFile = nullptr;
		if (ordOutput) {
			if (skipHeader == false) {
				if (fout[MELT] != nullptr) {
					fprintf(fout[MELT], "EventId,SummaryId,SampleType,EventRate,MeanLoss,SDLoss,FootprintExposure\n");
				}
				if (fout[QELT] != nullptr) {
					fprintf(fout[QELT], "EventId,SummaryId,Quantile,Loss\n");
				}
			}
			OutputData = &eltcalc::OutputRowsORD;
			outFile = fout[MELT];
		} else {
			if (skipHeader == false) {
				printf("summary_id,type,event_id,mean,standard_deviation,exposure_value\n");
			}
			OutputData = &eltcalc::OutputRows;
			outFile = stdout;
		}

		// Losses for calculating quantiles
		std::vector<OASIS_FLOAT> losses_vec;
		if (fout[QELT] != nullptr) losses_vec.resize(samplesize, 0);

		summarySampleslevelHeader sh;
		size_t i = fread(&sh, sizeof(sh), 1, stdin);
		while (i != 0) {
			sampleslevelRec sr;
			i = fread(&sr, sizeof(sr), 1, stdin);
			while (i != 0 && sr.sidx != 0) {
				if (sr.sidx > 0) {
					sumloss += sr.loss;
					sumlosssqr += (sr.loss * sr.loss);
					if (fout[QELT] != nullptr) losses_vec[sr.sidx-1] = sr.loss;
				}
				if (sr.sidx == -1) analytical_mean = sr.loss;
				i = fread(&sr, sizeof(sr), 1, stdin);
			}
			if (samplesize > 1) {
				sample_mean = sumloss / samplesize;
				sd = (sumlosssqr - ((sumloss*sumloss) / samplesize)) / (samplesize - 1);
				OASIS_FLOAT x = sd / sumlosssqr;
				if (x < 0.0000001) sd = 0;   // fix OASIS_FLOATing point precision problems caused by using large numbers
				sd = sqrt(sd);
			}
			else {
				if (samplesize == 0) {
					sd = 0;
					sample_mean = 0;
				}
				if (samplesize == 1) {
					sample_mean = sumloss / samplesize;
					sd = 0;
				}
			}
			if (sh.expval > 0) {   // only output rows with a non-zero exposure value
				OutputData(sh, 1, analytical_mean, 0, outFile);
				if (firstOutput == true) {
					std::this_thread::sleep_for(std::chrono::milliseconds(PIPE_DELAY)); // used to stop possible race condition with kat
					firstOutput = false;
				}
				if (samplesize) {
					OutputData(sh, 2, sample_mean, sd, outFile);
					if (fout[QELT] != nullptr) {
						OutputQuantiles(sh, losses_vec, fout[QELT]);
						std::fill(losses_vec.begin(), losses_vec.end(), 0);
					}
				}
			}


			if (i) i = fread(&sh, sizeof(sh), 1, stdin);

			sumloss = 0.0;
			sumlosssqr = 0.0;
			sd = 0.0;
			analytical_mean = 0.0;
		}

	}
	void doit(bool skipHeader, bool ordOutput, FILE **fout)
	{
		unsigned int stream_type = 0;
		size_t i = fread(&stream_type, sizeof(stream_type), 1, stdin);

		if (ordOutput) {
			if (fout[MELT] != nullptr) GetEventRates();
		}

		if (isSummaryCalcStream(stream_type) == true) {
			unsigned int samplesize;
			unsigned int summaryset_id;
			i = fread(&samplesize, sizeof(samplesize), 1, stdin);

			if (ordOutput && fout[QELT] != nullptr) {
				GetIntervals(samplesize);
			}

			if (i == 1) i = fread(&summaryset_id, sizeof(summaryset_id), 1, stdin);
			if (i == 1) {
				doetloutput(samplesize, skipHeader, ordOutput, fout);
			}
			else {
				fprintf(stderr, "FATAL: Stream read error\n");
			}
			return;
		}

		fprintf(stderr, "FATAL: %s: Not a gul stream\n", __func__);
		fprintf(stderr, "FATAL: %s: invalid stream type %d\n", __func__, stream_type);
		exit(-1);
	}


	void touch(const std::string &filepath)
	{
		FILE *fout = fopen(filepath.c_str(), "wb");
		fclose(fout);
	}
	void setinitdone(int processid)
	{
		if (processid) {
			std::ostringstream s;
			s << SEMA_DIR_PREFIX << "_elt/" << processid << ".id";
			touch(s.str());
		}
	}
}
