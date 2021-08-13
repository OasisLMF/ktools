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
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/

#include "../include/oasis.h"
#include <iostream>
#include <vector>
#include <map>
#include <math.h>
#include <chrono>
#include <thread>
#include <string.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif




using namespace std;
struct period_occ {
	int period_no;
	int occ_date_id;
};

namespace pltcalc {
	bool firstOutput = true;
	std::map<int, std::vector<period_occ> > m_occ;
	int date_algorithm_ = 0;
	int samplesize_ = 0;

	void d(long long g, int& y, int& mm, int& dd)
	{
		y = (10000 * g + 14780) / 3652425;
		int ddd = (int) g - (365 * y + y / 4 - y / 100 + y / 400);
		if (ddd < 0) {
			y = y - 1;
			ddd = (int) g - (365 * y + y / 4 - y / 100 + y / 400);
		}
		int mi = (100 * ddd + 52) / 3060;
		mm = (mi + 2) % 12 + 1;
		y = y + (mi + 2) / 12;
		dd = ddd - (mi * 306 + 5) / 10 + 1;
		return;
	}

	void loadoccurrence()
	{
		FILE* fin = fopen(OCCURRENCE_FILE, "rb");
		if (fin == NULL) {
			fprintf(stderr, "FATAL: %s: Error opening file %s\n", __func__, OCCURRENCE_FILE);
			exit(-1);
		}

		int no_of_periods = 0;
		occurrence occ;
		int i = fread(&date_algorithm_, sizeof(date_algorithm_), 1, fin);
		i = fread(&no_of_periods, sizeof(no_of_periods), 1, fin);
		i = fread(&occ, sizeof(occ), 1, fin);
		while (i != 0) {
			period_occ p;
			p.occ_date_id = occ.occ_date_id;
			p.period_no = occ.period_no;
			m_occ[occ.event_id].push_back(p);
			//event_to_periods[occ.event_id].push_back(occ.period_no);
			i = fread(&occ, sizeof(occ), 1, fin);
		}

		fclose(fin);

	}


	struct outrec {
		int type;
		int summary_id;
		int period_no;
		int event_id;
		OASIS_FLOAT mean;
		OASIS_FLOAT standard_deviation;
		OASIS_FLOAT exp_value;
		int occ_date_id;
	};

	inline void writeoutput(const char * buffer, int strLen)
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

	void outputrows(const outrec& o, const int type) {
		char buffer[4096];
		int strLen;
		strLen = sprintf(buffer, "%d,%d,%d,%d,%0.2f,%0.2f,%0.2f,%d\n",
				 type, o.summary_id, o.period_no, o.event_id,
				 o.mean, o.standard_deviation, o.exp_value,
				 o.occ_date_id);
		writeoutput(buffer, strLen);
	}
	void outputrows_da(const outrec& o, const int type) {
		int occ_year, occ_month, occ_day;
		d(o.occ_date_id, occ_year, occ_month, occ_day);

		char buffer[4096];
		int strLen;
		strLen = sprintf(buffer,
				"%d,%d,%d,%d,%0.2f,%0.2f,%0.2f,%d,%d,%d\n",
				 type, o.summary_id, o.period_no, o.event_id,
				 o.mean, o.standard_deviation, o.exp_value,
				 occ_year, occ_month, occ_day);
		writeoutput(buffer, strLen);
	}
	void outputrows_ord(const outrec& o, const int type) {
		int occ_year, occ_month, occ_day;
		d(o.occ_date_id, occ_year, occ_month, occ_day);

		char buffer[4096];
		int strLen;
		strLen = sprintf(buffer,
				 "%d,%d,%d,%d,%d,%d,%d,%0.2f,%0.2f,%0.2f\n",
				 o.period_no, o.event_id, occ_year, occ_month,
				 occ_day, o.summary_id, type, o.mean,
				 o.standard_deviation, o.exp_value);
		writeoutput(buffer, strLen);
	}

	void domeanout(const summarySampleslevelHeader& sh , OASIS_FLOAT mean_val, void (*OutputData)(const outrec&, const int))
	{
		std::vector<period_occ>& vp = m_occ[sh.event_id];
		outrec o;
		o.event_id = sh.event_id;
		o.summary_id = sh.summary_id;
		o.exp_value = sh.expval;
		o.mean = mean_val;
		o.standard_deviation = 0;
		for (auto p : vp) {
			o.period_no = p.period_no;
			o.occ_date_id = p.occ_date_id;
			OutputData(o, 1);
		}
	}
	void dopltcalc(const summarySampleslevelHeader& sh, const std::vector<sampleslevelRec>& vrec, void (*OutputData)(const outrec&, const int))
	{
		std::vector<period_occ>& vp = m_occ[sh.event_id];
		bool hasrec = false;
		bool firsttime = true;
		outrec o;
		o.event_id = sh.event_id;
		o.summary_id = sh.summary_id;
		o.exp_value = sh.expval;
		o.mean = 0;
		o.standard_deviation = 0;

		OASIS_FLOAT squared_loss_sum = 0;
		OASIS_FLOAT loss_sum = 0;
		for (auto p : vp) {
			o.period_no = p.period_no;
			o.occ_date_id = p.occ_date_id;
			if (firsttime == true) { // only do this once
				for (auto v : vrec) {
					if (v.sidx > 0) {
						hasrec = true;
						loss_sum += v.loss;
						squared_loss_sum += (v.loss * v.loss);
					}
				}
				firsttime = false;
			}
			if (hasrec) {
				o.mean = loss_sum / samplesize_;
				//o.standard_deviation = ((squared_loss_sum - loss_sum)/ samplesize_)/(samplesize_ -1);
				OASIS_FLOAT sd = (squared_loss_sum - ((loss_sum * loss_sum) / samplesize_)) / (samplesize_ - 1);
				OASIS_FLOAT x = sd / squared_loss_sum;
				if (x < 0.0000001) sd = 0;   // fix OASIS_FLOATing point precision problems caused by using large numbers
				o.standard_deviation = sqrt(sd);
				if (o.mean > 0 || o.standard_deviation > 0) {
					OutputData(o, 2);
				}
			}
		}

	}

	void doit(bool skipHeader, bool ordOutput)
	{
		loadoccurrence();
		int summarycalcstream_type = 0;
		int i = fread(&summarycalcstream_type, sizeof(summarycalcstream_type), 1, stdin);
		int stream_type = summarycalcstream_type & summarycalc_id;

		if (stream_type != summarycalc_id) {
			std::cerr << "Not a summarycalc stream type\n";
			exit(-1);
		}
		stream_type = streamno_mask & summarycalcstream_type;
		void (*OutputData)(const outrec&, const int);
		if (ordOutput) {
			if (skipHeader == false) {
				printf("Period,EventId,Year,Month,Day,SummaryId,SampleType,MeanLoss,SDLoss,FootprintExposure\n");
				OutputData = &outputrows_ord;
			}
		} else {
			if (date_algorithm_) {
				if (skipHeader == false) {
					printf("type,summary_id,period_no,event_id,mean,standard_deviation,exposure_value,occ_year,occ_month,occ_day\n");
					OutputData = &outputrows_da;
				}
			} else {
				if (skipHeader == false) {
					printf("type,summary_id,period_no,event_id,mean,standard_deviation,exposure_value,occ_date_id\n");
					OutputData = &outputrows;
				}
			}
		}

		if (firstOutput == true) {
			std::this_thread::sleep_for(std::chrono::milliseconds(PIPE_DELAY)); // used to stop possible race condition with kat
			firstOutput = false;
		}
		if (stream_type == 1) {
			int summary_set = 0;
			int j = 0;
			i = fread(&samplesize_, sizeof(samplesize_), 1, stdin);
			if (i != 0) i = fread(&summary_set, sizeof(summary_set), 1, stdin);
			std::vector<sampleslevelRec> vrec;
			summarySampleslevelHeader sh;
			while (i != 0) {
				i = fread(&sh, sizeof(sh), 1, stdin);
				while (i != 0) {
					sampleslevelRec sr;
					i = fread(&sr, sizeof(sr), 1, stdin);
					if (i == 0 || sr.sidx == 0) {
						dopltcalc(sh, vrec, OutputData);
						vrec.clear();
						break;
					}
					if (sr.sidx == -1) {
						domeanout(sh, sr.loss, OutputData);
					}else {
						vrec.push_back(sr);
					}
				}

				j++;
			}

		}
	}

}
