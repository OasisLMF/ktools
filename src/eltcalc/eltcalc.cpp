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
#include <iostream>
#include <fstream>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <chrono>
#include <thread>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif


#include "../include/oasis.h"

using namespace std;

namespace eltcalc {
	
	bool firstOutput = true;

	bool isSummaryCalcStream(unsigned int stream_type)
	{
		unsigned int stype = summarycalc_id & stream_type;
		return (stype == summarycalc_id);
	}

	inline void WriteOutput(const char * buffer, int strLen)
	{
		const char * bufPtr = buffer;
		int num;
		int counter = 0;
		do {

			num = printf("%s", bufPtr);
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
			const OASIS_FLOAT mean, const OASIS_FLOAT stdDev=0)
	{

		char buffer[4096];
		char * bufPtr = buffer;
		int strLen;
		if (type == 1) {
			strLen = sprintf(buffer, "%d,1,%d,%f,0,%f\n",
					 sh.summary_id, sh.event_id, mean,
					 sh.expval);
		} else {   // type == 2
			strLen = sprintf(buffer, "%d,2,%d,%f,%f,%f\n",
					 sh.summary_id, sh.event_id, mean,
					 stdDev, sh.expval);
		}

		WriteOutput(buffer, strLen);

	}

	void OutputRowsORD(const summarySampleslevelHeader& sh, const int type,
			   const OASIS_FLOAT mean, const OASIS_FLOAT stdDev=0)
	{

		char buffer[4096];
		int strLen;
		strLen = sprintf(buffer, "%d,%d,%d,%f,%f,%f\n", sh.event_id,
				 sh.summary_id, type, mean, stdDev, sh.expval);
		WriteOutput(buffer, strLen);

	}

	void doetloutput(int samplesize, bool skipHeader, bool ordOutput)
	{
		OASIS_FLOAT sumloss = 0.0;
		OASIS_FLOAT sample_mean = 0.0;
		OASIS_FLOAT analytical_mean = 0.0;
		OASIS_FLOAT sd = 0;
		OASIS_FLOAT sumlosssqr = 0.0;
		void (*OutputData)(const summarySampleslevelHeader&, const int,
				   const OASIS_FLOAT, const OASIS_FLOAT);
		if (ordOutput) {
			if (skipHeader == false) {
				printf("EventId,SummaryID,SampleType,MeanLoss,SDLoss,FootprintExposure\n");
			}
			OutputData = &eltcalc::OutputRowsORD;
		} else {
			if (skipHeader == false) {
				printf("summary_id,type,event_id,mean,standard_deviation,exposure_value\n");
			}
			OutputData = &eltcalc::OutputRows;
		}

		summarySampleslevelHeader sh;
		size_t i = fread(&sh, sizeof(sh), 1, stdin);
		while (i != 0) {
			sampleslevelRec sr;
			i = fread(&sr, sizeof(sr), 1, stdin);
			while (i != 0 && sr.sidx != 0) {
				if (sr.sidx > 0) {
					sumloss += sr.loss;
					sumlosssqr += (sr.loss * sr.loss);
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
				OutputData(sh, 1, analytical_mean, 0);
				if (firstOutput == true) {
					std::this_thread::sleep_for(std::chrono::milliseconds(PIPE_DELAY)); // used to stop possible race condition with kat
					firstOutput = false;
				}
				if (samplesize) OutputData(sh, 2, sample_mean, sd);
			}


			if (i) i = fread(&sh, sizeof(sh), 1, stdin);

			sumloss = 0.0;
			sumlosssqr = 0.0;
			sd = 0.0;
			analytical_mean = 0.0;
		}

	}
	void doit(bool skipHeader, bool ordOutput)
	{
		unsigned int stream_type = 0;
		size_t i = fread(&stream_type, sizeof(stream_type), 1, stdin);

		if (isSummaryCalcStream(stream_type) == true) {
			unsigned int samplesize;
			unsigned int summaryset_id;
			i = fread(&samplesize, sizeof(samplesize), 1, stdin);
			if (i == 1) i = fread(&summaryset_id, sizeof(summaryset_id), 1, stdin);
			if (i == 1) {
				doetloutput(samplesize, skipHeader, ordOutput);
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
