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
#include <algorithm>
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

#ifdef HAVE_PARQUET
#include "../include/oasisparquet.h"
#endif


using namespace std;
struct period_occ {
	int period_no;
	int occ_date_id;
};

struct period_occ_granular {
	int period_no;
	long long occ_date_id;
};

bool operator<(const sampleslevelRec &lhs, const sampleslevelRec &rhs) {
	if (lhs.loss != rhs.loss) {
		return lhs.loss < rhs.loss;
	} else {
		return lhs.sidx < rhs.sidx;
	}
}

namespace pltcalc {
	bool firstOutput = true;
	std::map<int, std::vector<period_occ> > m_occ_legacy;
	std::map<int, std::vector<period_occ_granular> > m_occ_granular;
	int date_algorithm_ = 0;
	int granular_date_ = 0;
	int no_of_periods_ = 0;
	int samplesize_ = 0;
	std::map<int, double> period_weights_;
	std::map<float, interval> intervals_;
	enum { MPLT = 0, SPLT, QPLT };
#ifdef HAVE_PARQUET
	std::map<int, parquet::StreamWriter> os_;
#endif

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

	void getperiodweights()
	{

		FILE *fin = fopen(PERIODS_FILE, "rb");

		// If there is no periods file, period weights are reciprocal
		// of number of periods
		if (fin == nullptr) {
			for (int i = 1; i <= no_of_periods_; i++) {
				period_weights_[i] = 1 / (double)no_of_periods_;
			}
			return;
		}

		Periods p;
		while (fread(&p, sizeof(Periods), 1, fin) != 0){
			period_weights_[p.period_no] = p.weighting;
		}

		fclose(fin);

	}

	void getintervals()
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
			float pos = (samplesize_ - 1) * q + 1;
			idx.integer_part = (int)pos;
			idx.fractional_part = pos - idx.integer_part;
			intervals_[q] = idx;
			i = fread(&q, sizeof(q), 1, fin);
		}

		fclose(fin);

	}

	template<typename occT, typename periodT, typename moccT>
	void loadoccurrence(occT &occ, periodT &p, moccT &m_occ, FILE * fin)
	{
		size_t i = fread(&no_of_periods_, sizeof(no_of_periods_), 1,
				 fin);
		i = fread(&occ, sizeof(occ), 1, fin);
		while (i != 0) {
			p.occ_date_id = occ.occ_date_id;
			p.period_no = occ.period_no;
			m_occ[occ.event_id].push_back(p);
			i = fread(&occ, sizeof(occ), 1, fin);
		}
	}

	void loadoccurrence()
	{
		FILE* fin = fopen(OCCURRENCE_FILE, "rb");
		if (fin == NULL) {
			fprintf(stderr, "FATAL: %s: Error opening file %s\n",
				__func__, OCCURRENCE_FILE);
			exit(-1);
		}

		int date_opts;
		size_t i = fread(&date_opts, sizeof(date_opts), 1, fin);
		date_algorithm_ = date_opts & 1;
		granular_date_ = date_opts >> 1;

		if (granular_date_) {
			occurrence_granular occ;
			period_occ_granular p;
			loadoccurrence(occ, p, m_occ_granular, fin);
		} else {
			occurrence occ;
			period_occ p;
			loadoccurrence(occ, p, m_occ_legacy, fin);
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
		OASIS_FLOAT max_impact_exp;
		OASIS_FLOAT mean_impact_exp;
		OASIS_FLOAT chance_of_loss;
		OASIS_FLOAT max_loss;
		int occ_date_id;
	};

	struct outrec_granular {
		int type;
		int summary_id;
		int period_no;
		int event_id;
		OASIS_FLOAT mean;
		OASIS_FLOAT standard_deviation;
		OASIS_FLOAT exp_value;
		OASIS_FLOAT max_impact_exp;
		OASIS_FLOAT mean_impact_exp;
		OASIS_FLOAT chance_of_loss;
		OASIS_FLOAT max_loss;
		long long occ_date_id;
	};

	inline void writeoutput(const char * buffer, int strLen,
				FILE * outFile)
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

	template<typename T>
	void outputrows(const T& o, const int type, FILE * outFile)
	{
		char buffer[4096];
		int strLen;
		strLen = sprintf(buffer, "%d,%d,%d,%d,%0.2f,%0.2f,%0.2f,%d\n",
				 type, o.summary_id, o.period_no, o.event_id,
				 o.mean, o.standard_deviation, o.exp_value,
				 o.occ_date_id);
		writeoutput(buffer, strLen, outFile);
	}

	template<typename T>
	void outputrows_da(const T& o, const int type, FILE * outFile)
	{
		int occ_year, occ_month, occ_day;
		int days = o.occ_date_id / (1440 - 1439 * !granular_date_);
		d(days, occ_year, occ_month, occ_day);

		char buffer[4096];
		int strLen;
		strLen = sprintf(buffer,
				"%d,%d,%d,%d,%0.2f,%0.2f,%0.2f,%d,%d,%d\n",
				 type, o.summary_id, o.period_no, o.event_id,
				 o.mean, o.standard_deviation, o.exp_value,
				 occ_year, occ_month, occ_day);
		writeoutput(buffer, strLen, outFile);
	}

	template<typename occDateT>
	inline void getdates(const occDateT occ_date_id, int &occ_year,
			     int &occ_month, int &occ_day, int &occ_hour,
			     int &occ_minute)
	{
		int days = occ_date_id / (1440 - 1439 * !granular_date_);
		d(days, occ_year, occ_month, occ_day);
		int minutes = (occ_date_id % 1440) * granular_date_;
		occ_hour = minutes / 60;
		occ_minute = minutes % 60;
	}



	template<typename T>
	void outputrows_ord(const T& o, const int type, FILE * outFile)
	{
#ifdef HAVE_PARQUET
		if (outFile == nullptr && os_.find(OasisParquet::MPLT) != os_.end())
#else
		if (outFile == nullptr)
#endif
		{
			return;
		}

		int occ_year, occ_month, occ_day, occ_hour, occ_minute;
		getdates(o.occ_date_id, occ_year, occ_month, occ_day, occ_hour,
			 occ_minute);

		if (outFile != nullptr) {
			char buffer[4096];
			int strLen;
			strLen = sprintf(buffer, "%d,%f,%d,%d,%d,%d,%d,%d,%d,"
					 "%d,%0.4f,%0.2f,%0.2f,%0.2f,%0.2f,"
					 "%0.2f,%0.2f\n", o.period_no,
					 period_weights_[o.period_no],
					 o.event_id, occ_year, occ_month,
					 occ_day, occ_hour, occ_minute,
					 o.summary_id, type, o.chance_of_loss,
					 o.mean, o.standard_deviation,
					 o.max_loss, o.exp_value,
					 o.mean_impact_exp, o.max_impact_exp);
			writeoutput(buffer, strLen, outFile);
		}
#ifdef HAVE_PARQUET
		if (os_.find(OasisParquet::MPLT) != os_.end()) {
			os_[MPLT] << o.period_no
				  << period_weights_[o.period_no] << o.event_id
				  << occ_year << occ_month << occ_day
				  << occ_hour << occ_minute << o.summary_id
				  << type << o.chance_of_loss << o.mean
				  << o.standard_deviation << o.max_loss
				  << o.exp_value << o.mean_impact_exp
				  << o.max_impact_exp << parquet::EndRow;
		}
#endif
	}

	template<typename moccT, typename periodT>
	void outputrows_splt(const summarySampleslevelHeader& sh,
			     const sampleslevelRec &sr, FILE * outFile,
			     moccT &m_occ, std::vector<periodT> &vp,
			     const OASIS_FLOAT impacted_exposure)
	{
#ifdef HAVE_PARQUET
		if (outFile == nullptr && os_.find(OasisParquet::SPLT) != os_.end())
#else
		if (outFile == nullptr)
#endif
		{
			return;
		}

		vp = m_occ[sh.event_id];
		for (auto p : vp) {
			int occ_year, occ_month, occ_day, occ_hour, occ_minute;
			getdates(p.occ_date_id, occ_year, occ_month, occ_day,
				 occ_hour, occ_minute);
			
			if (outFile != nullptr) {
				char buffer[4096];
				int strLen;
				strLen = sprintf(buffer, "%d,%f,%d,%d,%d,%d,%d,"
						 "%d,%d,%d,%0.2f,%0.2f\n",
						 p.period_no,
						 period_weights_[p.period_no],
						 sh.event_id, occ_year,
						 occ_month, occ_day, occ_hour,
						 occ_minute, sh.summary_id,
						 sr.sidx, sr.loss,
						 impacted_exposure);
				writeoutput(buffer, strLen, outFile);
			}
#ifdef HAVE_PARQUET
			if (os_.find(OasisParquet::SPLT) != os_.end()) {
				os_[SPLT] << p.period_no
					  << period_weights_[p.period_no]
					  << sh.event_id << occ_year
					  << occ_month << occ_day << occ_hour
					  << occ_minute << sh.summary_id
					  << sr.sidx << sr.loss
					  << impacted_exposure
					  << parquet::EndRow;
			}
#endif

		}
	}

	template<typename moccT, typename periodT>
	void outputrows_qplt(const summarySampleslevelHeader& sh,
			     std::vector<sampleslevelRec>& vrec, FILE *outFile,
			     moccT& m_occ, std::vector<periodT>& vp)
	{
#ifdef HAVE_PARQUET
		if (outFile == nullptr && os_.find(OasisParquet::QPLT) != os_.end())
#else
		if (outFile == nullptr)
#endif
		{
			return;
		}

		sampleslevelRec emptyRec = { 0, 0.0 };
		vrec.resize(samplesize_, emptyRec);   // Pad with zero losses
		std::sort(vrec.begin(), vrec.end());

		std::map<float, OASIS_FLOAT> quantile_to_loss;
		for (std::map<float, interval>::iterator it = intervals_.begin();
		     it != intervals_.end(); ++it) {

			int idx = it->second.integer_part;
			OASIS_FLOAT loss = 0;
			if (idx == samplesize_) {
				loss = vrec[idx-1].loss;
			} else {
				loss = (vrec[idx].loss - vrec[idx-1].loss) * it->second.fractional_part + vrec[idx-1].loss;
			}
			quantile_to_loss[it->first] = loss;
		}

		vp = m_occ[sh.event_id];
		for (auto p : vp) {
			int occ_year, occ_month, occ_day, occ_hour, occ_minute;
			getdates(p.occ_date_id, occ_year, occ_month, occ_day,
				 occ_hour, occ_minute);

			for (std::map<float, OASIS_FLOAT>::iterator it = quantile_to_loss.begin();
			     it != quantile_to_loss.end(); ++it) {

				if (outFile != nullptr) {
					char buffer[4096];
					int strLen;
					strLen = sprintf(buffer, "%d,%f,%d,%d,"
							 "%d,%d,%d,%d,%d,%0.2f,"
							 "%0.2f\n", p.period_no,
							 period_weights_[p.period_no],
							 sh.event_id, occ_year,
							 occ_month, occ_day,
							 occ_hour, occ_minute,
							 sh.summary_id,
							 it->first, it->second);
					writeoutput(buffer, strLen, outFile);
				}

#ifdef HAVE_PARQUET
				if (os_.find(OasisParquet::QPLT) != os_.end()) {
					os_[QPLT] << p.period_no
						  << period_weights_[p.period_no]
						  << sh.event_id << occ_year
						  << occ_month << occ_day
						  << occ_hour << occ_minute
						  << sh.summary_id << it->first
						  << it->second
						  << parquet::EndRow;
				}
#endif
			}
		}

	}

	template<typename outrecT, typename moccT, typename periodT>
	void domeanout(const summarySampleslevelHeader& sh,
		       OASIS_FLOAT mean_val,
		       void (*OutputData)(const outrecT&, const int, FILE*),
		       FILE * outFile, moccT &m_occ, std::vector<periodT> &vp,
		       const OASIS_FLOAT chance_of_loss,
		       const OASIS_FLOAT max_loss)
	{
		vp = m_occ[sh.event_id];
		outrecT o;
		o.event_id = sh.event_id;
		o.summary_id = sh.summary_id;
		o.exp_value = sh.expval;
		o.max_impact_exp = sh.expval;
		o.mean_impact_exp = sh.expval;
		o.chance_of_loss = chance_of_loss;
		o.max_loss = max_loss;
		o.mean = mean_val;
		o.standard_deviation = 0;
		for (auto p : vp) {
			if (o.mean > 0 || o.standard_deviation > 0) {
				o.period_no = p.period_no;
				o.occ_date_id = p.occ_date_id;
				OutputData(o, 1, outFile);
			}
		}
	}

	template<typename outrecT, typename moccT, typename periodT>
	void dopltcalc(const summarySampleslevelHeader& sh,
		       const std::vector<sampleslevelRec>& vrec,
		       void (*OutputData)(const outrecT&, const int, FILE*),
		       FILE * outFile, moccT &m_occ, std::vector<periodT> &vp,
		       const OASIS_FLOAT max_impacted_exposure,
		       const OASIS_FLOAT mean_impacted_exposure,
		       const OASIS_FLOAT chance_of_loss,
		       const OASIS_FLOAT max_loss)
	{
		vp = m_occ[sh.event_id];
		bool hasrec = false;
		bool firsttime = true;
		outrecT o;
		o.event_id = sh.event_id;
		o.summary_id = sh.summary_id;
		o.exp_value = sh.expval;
		o.max_impact_exp = max_impacted_exposure;
		o.mean_impact_exp = mean_impacted_exposure;
		o.chance_of_loss = chance_of_loss;
		o.max_loss = max_loss;
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
				OASIS_FLOAT sd = (squared_loss_sum - ((loss_sum * loss_sum) / samplesize_)) / (samplesize_ - 1);
				OASIS_FLOAT x = sd / squared_loss_sum;
				if (x < 0.0000001) sd = 0;   // fix OASIS_FLOATing point precision problems caused by using large numbers
				o.standard_deviation = sqrt(sd);
				if (o.mean > 0 || o.standard_deviation > 0) {
					OutputData(o, 2, outFile);
				}
			}
		}
	}

	template<typename T, typename moccT, typename periodT>
	inline void read_input(void (*OutputData)(const T&, const int, FILE*),
			       FILE * outFile, FILE ** fout,
			       moccT &m_occ, periodT &vp)
	{
		int summary_set = 0;
		size_t i = fread(&summary_set, sizeof(summary_set), 1, stdin);
		std::vector<sampleslevelRec> vrec;
		summarySampleslevelHeader sh;
		while (i != 0) {
			OASIS_FLOAT max_impacted_exposure = 0;
			OASIS_FLOAT mean_impacted_exposure = 0;
			OASIS_FLOAT chance_of_loss = 0;
			OASIS_FLOAT max_loss = 0;
			i = fread(&sh, sizeof(sh), 1, stdin);
			while (i != 0) {
				OASIS_FLOAT impacted_exposure = 0;
				sampleslevelRec sr;
				i = fread(&sr, sizeof(sr), 1, stdin);
				if (i == 0 || sr.sidx == 0) {
					dopltcalc(sh, vrec, OutputData, outFile,
						  m_occ, vp,
						  max_impacted_exposure,
						  mean_impacted_exposure,
						  chance_of_loss, max_loss);
					outputrows_qplt(sh, vrec, fout[QPLT],
							m_occ, vp);
					vrec.clear();
					break;
				} else if (sr.sidx >= -1) {
					impacted_exposure = sh.expval * (sr.loss > 0);
					outputrows_splt(sh, sr, fout[SPLT],
							m_occ, vp,
							impacted_exposure);
				}

				if (sr.sidx == chance_of_loss_idx) {
					chance_of_loss = sr.loss;
				} else if (sr.sidx == max_loss_idx) {
					max_loss = sr.loss;
				} else if (sr.sidx == mean_idx) {
					domeanout(sh, sr.loss, OutputData,
						  outFile, m_occ, vp,
						  chance_of_loss, max_loss);
				} else {
					vrec.push_back(sr);
					mean_impacted_exposure += impacted_exposure / samplesize_;
					if (impacted_exposure > max_impacted_exposure) {
						max_impacted_exposure = impacted_exposure;
					}
				}
			}
		}
	}

	void doit(bool skipHeader, bool ordOutput, FILE **fout,
		  std::map<int, std::string> &parquetFileNames)
	{
		loadoccurrence();
		if (ordOutput || parquetFileNames.size() != 0) getperiodweights();
		int summarycalcstream_type = 0;
		int i = fread(&summarycalcstream_type,
			      sizeof(summarycalcstream_type), 1, stdin);
		int stream_type = summarycalcstream_type & summarycalc_id;

		if (stream_type != summarycalc_id) {
			std::cerr << "Not a summarycalc stream type\n";
			exit(-1);
		}
		stream_type = streamno_mask & summarycalcstream_type;
		i = fread(&samplesize_, sizeof(samplesize_), 1, stdin);
#ifdef HAVE_PARQUET
		if (fout[QPLT] != nullptr || parquetFileNames.find(OasisParquet::QPLT) != parquetFileNames.end())
#else
		if (fout[QPLT] != nullptr)
#endif
		{
			getintervals();
		}

		void (*OutputDataGranular)(const outrec_granular&, const int,
					   FILE*) = nullptr;
		void (*OutputDataLegacy)(const outrec&, const int,
					 FILE*) = nullptr;
		FILE * outFile = nullptr;
		if (ordOutput || parquetFileNames.size() != 0) {
			if (granular_date_) {
				OutputDataGranular = outputrows_ord<const outrec_granular&>;
			} else {
				OutputDataLegacy = outputrows_ord<const outrec&>;
			}
		}

		if (ordOutput) {
			if (skipHeader == false) {
				if (fout[MPLT] != nullptr) {
					fprintf(fout[MPLT],
						"Period,PeriodWeight,EventId,"
						"Year,Month,Day,Hour,Minute,"
						"SummaryId,SampleType,"
						"ChanceOfLoss,MeanLoss,SDLoss,"
						"MaxLoss,FootprintExposure,"
						"MeanImpactedExposure,"
						"MaxImpactedExposure\n");
				}
				if (fout[SPLT] != nullptr) {
					fprintf(fout[SPLT],
						"Period,PeriodWeight,EventId,"
						"Year,Month,Day,Hour,Minute,"
						"SummaryId,SampleId,Loss,"
						"ImpactedExposure\n");
				}
				if (fout[QPLT] != nullptr) {
					fprintf(fout[QPLT],
						"Period,PeriodWeight,EventId,"
						"Year,Month,Day,Hour,Minute,"
						"SummaryId,Quantile,Loss\n");
				}
			}
			outFile = fout[MPLT];
		} else if (parquetFileNames.size() == 0) {
			if (date_algorithm_) {
				if (skipHeader == false) {
					printf("type,summary_id,period_no,event_id,mean,standard_deviation,exposure_value,occ_year,occ_month,occ_day\n");
				}
				if (granular_date_) {
					OutputDataGranular = outputrows_da<const outrec_granular&>;
				} else {
					OutputDataLegacy = outputrows_da<const outrec&>;
				}
			} else {
				if (skipHeader == false) {
					printf("type,summary_id,period_no,event_id,mean,standard_deviation,exposure_value,occ_date_id\n");
				}
				if (granular_date_) {   // Should not get here
					fprintf(stderr, "FATAL: Unknown date algorithm\n");
					exit(EXIT_FAILURE);
				} else {
					OutputDataLegacy = outputrows<const outrec&>;
				}
			}
			outFile = stdout;
		}

#ifdef HAVE_PARQUET
		for (auto iter = parquetFileNames.begin();
		  iter != parquetFileNames.end(); ++iter) {
			os_[iter->first] = OasisParquet::GetParquetStreamWriter_(iter->first, iter->second);
		}
#endif

		if (firstOutput == true) {
			std::this_thread::sleep_for(std::chrono::milliseconds(PIPE_DELAY)); // used to stop possible race condition with kat
			firstOutput = false;
		}
		if (stream_type == 1) {
			if (granular_date_) {
				std::vector<period_occ_granular> vp_granular;
				read_input(OutputDataGranular, outFile, fout,
					   m_occ_granular, vp_granular);
			} else {

				std::vector<period_occ> vp_legacy;
				read_input(OutputDataLegacy, outFile, fout,
					   m_occ_legacy, vp_legacy);
			}
		}
	}

}
