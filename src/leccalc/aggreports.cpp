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

#include <vector>
#include <algorithm>    // std::sort
#include <assert.h>
#include <string.h>
#include "aggreports.h"
#include "../include/oasis.h"

struct line_points {
	OASIS_FLOAT from_x;
	OASIS_FLOAT from_y;
	OASIS_FLOAT to_x;
	OASIS_FLOAT to_y;
};
inline OASIS_FLOAT linear_interpolate(line_points lp, OASIS_FLOAT xpos)
{
	return ((xpos - lp.to_x) * (lp.from_y - lp.to_y) / (lp.from_x - lp.to_x)) + lp.to_y;
}

bool operator<(const summary_id_period_key& lhs, const summary_id_period_key& rhs)
{
	if (lhs.summary_id != rhs.summary_id) {
		return lhs.summary_id < rhs.summary_id;
	}
	if (lhs.period_no != rhs.period_no) {
		return lhs.period_no < rhs.period_no;
	}
	else {
		return lhs.type < rhs.type;
	}
}

bool operator<(const lossval& lhs, const lossval& rhs)
{
	return lhs.value < rhs.value;
}


bool operator<(const summary_id_type_key& lhs, const summary_id_type_key& rhs)
{
	if (lhs.summary_id != rhs.summary_id) {
		return lhs.summary_id < rhs.summary_id;
	}
	else {
		return lhs.type < rhs.type;
	}
}


bool operator<(const wheatkey& lhs, const wheatkey& rhs)
{
	if (lhs.summary_id != rhs.summary_id) {
		return lhs.summary_id < rhs.summary_id;
	}
	else {
		return lhs.sidx < rhs.sidx;
	}
}

aggreports::aggreports(int totalperiods, int maxsummaryid, std::map<outkey2, OASIS_FLOAT> &agg_out_loss,
	std::map<outkey2, OASIS_FLOAT> &max_out_loss, FILE **fout, bool useReturnPeriodFile, int samplesize, bool skipheader,
	FILE *ord_out) :
	totalperiods_(totalperiods), maxsummaryid_(maxsummaryid), agg_out_loss_(agg_out_loss), max_out_loss_(max_out_loss),
	fout_(fout), useReturnPeriodFile_(useReturnPeriodFile), samplesize_(samplesize),skipheader_(skipheader), ord_out_(ord_out)
{
	loadreturnperiods();
};

// Load and normalize weigthting table
// we must have entry for every return period!!!
// otherwise no way to pad missing ones
// The weighting should already be normalized - i.e range within 0 and 1 and they should total upto one.
void aggreports::loadperiodtoweigthing()
{
	FILE *fin = fopen(PERIODS_FILE, "rb");
	if (fin == NULL) return;
	Periods p;
	OASIS_FLOAT total_weighting = 0;
	size_t i = fread(&p, sizeof(Periods), 1, fin);
	while (i != 0) {
		//printf("%d, %lf\n", p.period_no, p.weighting);
		total_weighting += (OASIS_FLOAT) p.weighting;
		periodstoweighting_[p.period_no] = p.weighting;
		i = fread(&p, sizeof(Periods), 1, fin);
	}
	// If we are going to have weightings we should have them for all periods
	//if (periodstowighting_.size() != totalperiods_) {
	//	fprintf(stderr, "Total number of periods in %s does not match the number of periods in %s\n", PERIODS_FILE, OCCURRENCE_FILE);
	//	exit(-1);
	//}
	// Now normalize the weigthing ...
	auto iter = periodstoweighting_.begin();
	while (iter != periodstoweighting_.end()) {   // weightings already normalized so just split between samples
	//	iter->second = iter->second / total_weighting;
		if (samplesize_) iter->second = iter->second / samplesize_;		// now split them between the samples
		iter++;
	}
}

void aggreports::loadreturnperiods()
{
	if (useReturnPeriodFile_ == false) return;

	FILE *fin = fopen(RETURNPERIODS_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "FATAL: Error opening file %s\n", RETURNPERIODS_FILE);
		exit(-1);
	}

	int return_period;

	while (fread(&return_period, sizeof(return_period), 1, fin) == 1) {
		returnperiods_.push_back(return_period);
	}

	fclose(fin);

	if (returnperiods_.size() == 0) {
		useReturnPeriodFile_ = false;
		// std::cerr << __func__ << ": No return periods loaded - running without defined return periods option\n";
	}
}


void aggreports::loadensemblemapping() {

	FILE *fin = fopen(ENSEMBLE_FILE, "rb");
	if (fin == NULL) return;

	Ensemble e;
	std::vector<int> sidxtoensemble;
	sidxtoensemble.resize(samplesize_ + 1, 0);
	std::fill(sidxtoensemble.begin(), sidxtoensemble.end(), 0);
	size_t i = fread(&e, sizeof(Ensemble), 1, fin);
	while (i != 0) {
		sidxtoensemble[e.sidx] = e.ensemble_id;
		ensembletosidx_[e.ensemble_id].push_back(e.sidx);

		i = fread(&e, sizeof(Ensemble), 1, fin);

	}

	// Check all sidx have ensemble IDs
	for (std::vector<int>::const_iterator it = std::next(sidxtoensemble.begin()); it != sidxtoensemble.end(); it++) {
		if (*it == 0) {
			fprintf(stderr, "FATAL: All sidx must have associated ensemble IDs\n");
			exit(-1);
		}
	}

}


void aggreports::initordout() {

	if (ord_out_ == nullptr) return;
	if (skipheader_) return;

	fprintf(ord_out_, "SummaryID,EPCalc,EPType,ReturnPeriod,Loss\n");

}


inline void aggreports::outputrows(const int handle, const char * buffer,
				   int strLen) {

	const char * bufPtr = buffer;
	int num;
	int counter = 0;
	do {

		num = fprintf(fout_[handle], "%s", bufPtr);
		if (num < 0) {   // Write error
			fprintf(stderr, "FATAL: Error writing %s: %s\n",
				buffer, strerror(errno));
			exit(EXIT_FAILURE);
		} else if (num < strLen) {   // Incomplete write
			bufPtr += num;
			strLen -= num;
		} else return;   // Success

		fprintf(stderr, "INFO: Attempt %d to write %s\n", ++counter,
			buffer);

	} while (counter < 10);

	fprintf(stderr, "FATAL: Maximum attempts to write %s exceeded\n",
		buffer);
	exit(EXIT_FAILURE);

}

inline void aggreports::outputrows(const char * buffer, int strLen) {

	const char * bufPtr = buffer;
	int num;
	int counter = 0;

	do {

		num = fprintf(ord_out_, "%s", bufPtr);
		if (num < 0) {   // Write error
			fprintf(stderr, "FATAL: Error writing %s: %s\n",
				buffer, strerror(errno));
			exit(EXIT_FAILURE);
		} else if (num < strLen) {   // Incomplete write
			bufPtr += num;
			strLen -= num;
		} else return;   // Success

		fprintf(stderr, "INFO: Attempt %d to write %s\n", ++counter,
			buffer);

	} while (counter < 10);

	fprintf(stderr, "FATAL: Maximum attempts to write %s exceeded\n",
		buffer);
	exit(EXIT_FAILURE);

}

void aggreports::writefulluncertaintywithweighting(const int handle,
	const int type, const std::map<outkey2, OASIS_FLOAT> &out_loss,
	const int ensemble_id) {

	// Variables for ORD output
	int epcalc = 0;
	int eptype = 0;
	int eptype_tvar = 0;
	if (ord_out_) {
		if (type == 1) {
			epcalc = MEANDR;
		} else if (type == 2) {
			epcalc = FULL;
		}

		if (handle == OCC_FULL_UNCERTAINTY) {
			eptype = OEP;
			eptype_tvar = OEPTVAR;
		} else if (handle == AGG_FULL_UNCERTAINTY) {
			eptype = AEP;
			eptype_tvar = AEPTVAR;
		}
	}

	std::map<int, lossvec2> items;
	for (auto x : out_loss) {
		if ((type == 1 && x.first.sidx == -1) || 
		    (type == 2 && x.first.sidx > 0)) { 
			lossval lv;
			lv.value = x.second;
			// assume weighting is zero if not supplied
			auto iter = periodstoweighting_.find(x.first.period_no);
			if (iter != periodstoweighting_.end()) {
				lv.period_weighting = iter->second;
				lv.period_no = x.first.period_no;   // for debugging
				items[x.first.summary_id].push_back(lv);
			}
		}
	}

	for (auto s : items) {
		OASIS_FLOAT cummulative_weighting = 0;
		lossvec2 &lpv = s.second;
		std::sort(lpv.rbegin(), lpv.rend());
		size_t nextreturnperiodindex = 0;
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		OASIS_FLOAT max_retperiod = 0;
		bool largest_loss = false;
		for (auto lp : lpv) {
			if (type == 1) {
				cummulative_weighting += (lp.period_weighting * samplesize_);
			} else if (type == 2) {
				cummulative_weighting += lp.period_weighting;
			}

			if (lp.period_weighting) {
				OASIS_FLOAT retperiod = 1 / cummulative_weighting;
				if (!largest_loss) {
					max_retperiod = retperiod;
					largest_loss = true;
				}
				if (useReturnPeriodFile_) {
					doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp.value, s.first, type, max_retperiod, ensemble_id, epcalc, eptype);
				}
				else {
					const int bufferSize = 4096;
					char buffer[bufferSize];
					int strLen;
					strLen = snprintf(buffer, bufferSize, "%d,%d,%f,%f", s.first, type, retperiod, lp.value);
					if (ensembletosidx_.size() > 0)
						strLen += snprintf(buffer+strLen, bufferSize-strLen, "%d", ensemble_id);
					strLen += snprintf(buffer+strLen, bufferSize-strLen, "\n");
					outputrows(handle, buffer, strLen);

					// ORD output
					if (ord_out_ != nullptr && ensemble_id == 0) {
						if (epcalc == FULL || (epcalc == MEANDR && meanDR_[eptype] == false)) {
							buffer[0] = 0;
							strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", s.first, epcalc, eptype, retperiod, lp.value);
							outputrows(buffer, strLen);
						}
					}
				}
			}
		}
	}

	// Tail Value at Risk (TVaR) - ORD output only
	if (ord_out_ != nullptr && ensemble_id == 0) {
		if (epcalc == FULL || (epcalc == MEANDR && meanDR_[eptype] == false)) {
			for (auto s : items) {
				OASIS_FLOAT tvar = 0;
				OASIS_FLOAT cummulative_weighting = 0;
				lossvec2 &lpv = s.second;
				std::sort(lpv.rbegin(), lpv.rend());
				size_t nextreturnperiodindex = 0;
				OASIS_FLOAT last_computed_rp = 0;
				OASIS_FLOAT last_computed_loss = 0;
				OASIS_FLOAT max_retperiod = 0;
				bool largest_loss = false;
				int i = 1;
				for (auto lp : lpv) {
					if (type == 1) {
						cummulative_weighting += (lp.period_weighting * samplesize_);
					} else if (type == 2) {
						cummulative_weighting += lp.period_weighting;
					}

					if (lp.period_weighting) {
						OASIS_FLOAT retperiod = 1 / cummulative_weighting;
						if (!largest_loss) {
							max_retperiod = retperiod;
							largest_loss = true;
						}
						if (useReturnPeriodFile_) {
							doreturnperiodout_tvar(nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp.value, tvar, i, s.first, epcalc, eptype_tvar, max_retperiod);
							tvar = tvar - ((tvar - lp.value) / i);
						} else {
							tvar = tvar - ((tvar - lp.value) / i);
							const int bufferSize = 4096;
							char buffer[bufferSize];
							int strLen;
							strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", s.first, epcalc, eptype_tvar, retperiod, tvar);
							outputrows(buffer, strLen);
						}
						i++;
					}
				}
			}
		}
	}

	// By ensemble ID
	if (ensembletosidx_.size() > 0 && type == 2 && ensemble_id == 0) {
		std::map<outkey2, OASIS_FLOAT> out_loss_by_ensemble_id;
		for (auto ensemble : ensembletosidx_) {
			out_loss_by_ensemble_id.clear();
			for (auto sidx : ensemble.second) {
				for (auto x : out_loss) {
					if (x.first.sidx == sidx) out_loss_by_ensemble_id[x.first] = x.second;
				}
			}
			writefulluncertaintywithweighting(handle, type, 
							  out_loss_by_ensemble_id,
							  ensemble.first);
		}
	}

	meanDR_[eptype] = true;   // Do not output type 1 to ORD file again

}

// this one should run using period table
void aggreports::fulluncertaintywithweighting(int handle, 
	const std::map<outkey2, OASIS_FLOAT> &out_loss) {

	if (fout_[handle] == nullptr) return;

	if (skipheader_ == false) {
		fprintf(fout_[handle], "summary_id,type,return_period,loss");
		if (ensembletosidx_.size() > 0)
			fprintf(fout_[handle], ",ensemble_id");
		fprintf(fout_[handle], "\n");
	}
	writefulluncertaintywithweighting(handle, 1, out_loss);   // sidx = -1
	writefulluncertaintywithweighting(handle, 2, out_loss);   // sidx > 0

}

void aggreports::writefulluncertainty(const int handle, const int type,
	const std::map<outkey2, OASIS_FLOAT> &out_loss, const int ensemble_id) {

	// Variables for ORD output
	int epcalc = 0;
	int eptype = 0;
	int eptype_tvar = 0;
	if (ord_out_) {
		if (type == 1) {
			epcalc = MEANDR;
		} else if (type == 2) {
			epcalc = FULL;
		}

		if (handle == OCC_FULL_UNCERTAINTY) {
			eptype = OEP;
			eptype_tvar = OEPTVAR;
		} else if (handle == AGG_FULL_UNCERTAINTY) {
			eptype = AEP;
			eptype_tvar = AEPTVAR;
		}
	}

	std::map<int, lossvec> items;
	for (auto x : out_loss) {
		if ((type == 1 && x.first.sidx == -1) ||
		    (type == 2 && x.first.sidx > 0)) {
			items[x.first.summary_id].push_back(x.second);
		}
	}

	for (auto s : items) {
		lossvec &lpv = s.second;
		std::sort(lpv.rbegin(), lpv.rend());
		size_t nextreturnperiodindex = 0;
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		int i = 1;
		OASIS_FLOAT t = (OASIS_FLOAT)totalperiods_;
		if (type == 2) {
			if (ensemble_id != 0) t *= ensembletosidx_[ensemble_id].size();
			else if (samplesize_) t *= samplesize_;
		}
		OASIS_FLOAT max_retperiod = t;
		for (auto lp : lpv) {
			OASIS_FLOAT retperiod = t / i;
			if (useReturnPeriodFile_) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp, s.first, type, max_retperiod, ensemble_id, epcalc, eptype);
			} else {
				const int bufferSize = 4096;
				char buffer[bufferSize];
				int strLen;
				strLen = snprintf(buffer, bufferSize, "%d,%d,%f,%f", s.first, type, retperiod, lp);
				if (ensembletosidx_.size() > 0)
					strLen += snprintf(buffer+strLen, bufferSize-strLen, ",%d", ensemble_id);
				strLen += snprintf(buffer+strLen, bufferSize-strLen, "\n");
				outputrows(handle, buffer, strLen);

				// ORD output
				if (ord_out_ != nullptr && ensemble_id == 0) {
					if (epcalc == FULL || (epcalc == MEANDR && meanDR_[eptype] == false)) {
						buffer[0] = 0;
						strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", s.first, epcalc, eptype, retperiod, lp);
						outputrows(buffer, strLen);
					}
				}
			}
			i++;
		}
		if (useReturnPeriodFile_) {
			doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first, type, max_retperiod, ensemble_id, epcalc, eptype);
			while (nextreturnperiodindex < returnperiods_.size()) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first, type, max_retperiod, ensemble_id, epcalc, eptype);
			}
		}
	}

	// Tail Value at Risk (TVaR) - ORD output only
	if (ord_out_ != nullptr && ensemble_id == 0) {
		if (epcalc == FULL || (epcalc == MEANDR && meanDR_[eptype] == false)) {
			for (auto s : items) {
				lossvec &lpv = s.second;
				std::sort(lpv.rbegin(), lpv.rend());
				OASIS_FLOAT tvar = 0;
				size_t nextreturnperiodindex = 0;
				OASIS_FLOAT last_computed_rp = 0;
				OASIS_FLOAT last_computed_loss = 0;
				int i = 1;
				OASIS_FLOAT t = (OASIS_FLOAT)totalperiods_;
				if (type == 2) {
					if (samplesize_) t *= samplesize_;
				}
				OASIS_FLOAT max_retperiod = t;
				for (auto lp : lpv) {
					OASIS_FLOAT retperiod = t / i;
					if (useReturnPeriodFile_) {
						doreturnperiodout_tvar(nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp, tvar, i, s.first, epcalc, eptype_tvar, max_retperiod);
						tvar = tvar - ((tvar - lp) / i);
					} else {
						tvar = tvar - ((tvar - lp) / i);
						const int bufferSize = 4096;
						char buffer[bufferSize];
						int strLen;
						strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", s.first, epcalc, eptype_tvar, retperiod, tvar);
						outputrows(buffer, strLen);
					}
					i++;
				}
				if (useReturnPeriodFile_) {
					do {
						doreturnperiodout_tvar(nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, tvar, i, s.first, epcalc, eptype_tvar, max_retperiod);
					} while (nextreturnperiodindex < returnperiods_.size());
				}
			}
		}
	}

	// By ensemble ID
	if (ensembletosidx_.size() > 0 && type == 2 && ensemble_id == 0) {
		std::map<outkey2, OASIS_FLOAT> out_loss_by_ensemble_id;
		for (auto ensemble : ensembletosidx_) {
			out_loss_by_ensemble_id.clear();
			for (auto sidx : ensemble.second) {
				for (auto x : out_loss) {
					if (x.first.sidx == sidx) out_loss_by_ensemble_id[x.first] = x.second;
				}
			}
			writefulluncertainty(handle, type,
					     out_loss_by_ensemble_id,
					     ensemble.first);
		}
	}

	meanDR_[eptype] = true;   // Do not output type 1 to ORD file again

}

void aggreports::fulluncertainty(int handle,const std::map<outkey2, OASIS_FLOAT> &out_loss)
{
	if (fout_[handle] == nullptr) return;

	if (skipheader_ == false) {
		fprintf(fout_[handle], "summary_id,type,return_period,loss");
		if (ensembletosidx_.size() > 0) {
			fprintf(fout_[handle], ",ensemble_id");
		}
		fprintf(fout_[handle], "\n");
	}
	writefulluncertainty(handle, 1, out_loss);   // sidx = -1
	writefulluncertainty(handle, 2, out_loss);   // sidx > 0

}

void aggreports::outputOccFulluncertainty()
{
	if (periodstoweighting_.size() == 0) {
		fulluncertainty(OCC_FULL_UNCERTAINTY, max_out_loss_);
	}
	else {
		fulluncertaintywithweighting(OCC_FULL_UNCERTAINTY, max_out_loss_);;
	}

}

// Full uncertainty output
void aggreports::outputAggFulluncertainty()
{
	if (periodstoweighting_.size() == 0) {
		fulluncertainty(AGG_FULL_UNCERTAINTY, agg_out_loss_);
	}
	else {
		fulluncertaintywithweighting(AGG_FULL_UNCERTAINTY, agg_out_loss_);;
	}
}

// the next_return_period must be between last_return_period and current_return_period
OASIS_FLOAT aggreports::getloss(OASIS_FLOAT next_return_period, OASIS_FLOAT last_return_period, OASIS_FLOAT last_loss, OASIS_FLOAT current_return_period, OASIS_FLOAT current_loss) const
{
	if (current_return_period == 0.0) return 0.0;
	if (current_return_period == next_return_period) {
		return current_loss;
	}
	else {
		if (current_return_period < next_return_period) {
			line_points lpt;
			lpt.from_x = last_return_period;
			lpt.from_y = last_loss;
			lpt.to_x = current_return_period;
			lpt.to_y = current_loss;
			OASIS_FLOAT zz = linear_interpolate(lpt, next_return_period);
			return zz;
		}
	}
	return -1;		// we should NOT get HERE hence the -1 !!!
}

void aggreports::doreturnperiodout(int handle, size_t &nextreturnperiod_index,
				   OASIS_FLOAT &last_return_period,
				   OASIS_FLOAT &last_loss,
				   OASIS_FLOAT current_return_period,
				   OASIS_FLOAT current_loss, int summary_id,
				   int type, OASIS_FLOAT max_retperiod,
				   int ensemble_id, int epcalc, int eptype)
{
	if (nextreturnperiod_index >= returnperiods_.size()) {
		return;
	}
	OASIS_FLOAT nextreturnperiod_value = returnperiods_[nextreturnperiod_index];
	while (current_return_period <= nextreturnperiod_value) {
		// Interpolated return period loss should not exceed that for
		// maximum return period
		OASIS_FLOAT loss;
		if (max_retperiod < nextreturnperiod_value) {
			nextreturnperiod_index++;
			if (nextreturnperiod_index < returnperiods_.size()) {
				nextreturnperiod_value = (OASIS_FLOAT)returnperiods_[nextreturnperiod_index];
			} else {
				break;
			}
			continue;
		} else {
			loss = getloss(nextreturnperiod_value, last_return_period, last_loss, current_return_period, current_loss);
		}
		const int bufferSize = 4096;
		char buffer[bufferSize];
		int strLen;
		if (epcalc && eptype && !ensemble_id) {   // ORD output
			if (!(epcalc == MEANDR && meanDR_[eptype] == true)) {
				strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", summary_id, epcalc, eptype, (OASIS_FLOAT)nextreturnperiod_value, loss);
				outputrows(buffer, strLen);
			}
		}
		if(type) {
			strLen = snprintf(buffer, bufferSize, "%d,%d,%f,%f", summary_id, type, (OASIS_FLOAT)nextreturnperiod_value, loss);
			if (ensembletosidx_.size() > 0)
				strLen += snprintf(buffer+strLen, bufferSize-strLen, ",%d", ensemble_id);
			strLen += snprintf(buffer+strLen, bufferSize-strLen, "\n");
			outputrows(handle, buffer, strLen);
		} else {
			strLen = sprintf(buffer, "%d,%f,%f\n", summary_id, (OASIS_FLOAT)nextreturnperiod_value, loss);
			outputrows(handle, buffer, strLen);
		}
		nextreturnperiod_index++;
		if (nextreturnperiod_index < returnperiods_.size()) {
			nextreturnperiod_value = (OASIS_FLOAT) returnperiods_[nextreturnperiod_index];
		}else{
			break;
		}
	}
	if (current_return_period > 0) {
		last_return_period = current_return_period;
		last_loss = current_loss;
	}
}


void aggreports::doreturnperiodout_tvar(size_t &nextreturnperiod_index,
					OASIS_FLOAT &last_return_period,
					OASIS_FLOAT &last_loss,
					const OASIS_FLOAT current_return_period,
					const OASIS_FLOAT current_loss,
					OASIS_FLOAT tvar, int counter,
					const int summary_id, const int epcalc,
					const int eptype,
					const OASIS_FLOAT max_retperiod)
{
	if (nextreturnperiod_index >= returnperiods_.size()) {
		return;
	}
	OASIS_FLOAT nextreturnperiod_value = returnperiods_[nextreturnperiod_index];
	while (current_return_period <= nextreturnperiod_value) {
		// Interpolated return period loss should not exceed that for
		// maximum return period
		OASIS_FLOAT loss;
		if (max_retperiod < nextreturnperiod_value) {
			nextreturnperiod_index++;
			if (nextreturnperiod_index < returnperiods_.size()) {
				nextreturnperiod_value = (OASIS_FLOAT)returnperiods_[nextreturnperiod_index];
			} else break;
			continue;
		} else {
			loss = getloss(nextreturnperiod_value, last_return_period, last_loss, current_return_period, current_loss);
		}

		tvar = tvar - ((tvar - loss) / counter);
		const int bufferSize = 4096;
		char buffer[bufferSize];
		int strLen;
		strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", summary_id, epcalc, eptype, (OASIS_FLOAT)nextreturnperiod_value, tvar);
		outputrows(buffer, strLen);

		nextreturnperiod_index++;
		if (nextreturnperiod_index < returnperiods_.size()) {
			nextreturnperiod_value = (OASIS_FLOAT)returnperiods_[nextreturnperiod_index];
			counter++;
		} else break;
	}

	if (current_return_period > 0) {
		last_return_period = current_return_period;
		last_loss = current_loss;
	}
}


void aggreports::wheatsheaf(int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss)
{
	if (fout_[handle] == nullptr) return;
	std::map<wheatkey, lossvec> items;

	for (auto x : out_loss) {
		wheatkey wk;;
		wk.sidx = x.first.sidx;
		wk.summary_id = x.first.summary_id;
		items[wk].push_back(x.second);
	}

	if (skipheader_ == false) {
		fprintf(fout_[handle], "summary_id,sidx,return_period,loss");
		if (ensembletosidx_.size() > 0) {
			fprintf(fout_[handle], ",ensemble_id");
		}
		fprintf(fout_[handle], "\n");
	}

	OASIS_FLOAT max_retperiod = (OASIS_FLOAT)totalperiods_;

	for (auto s : items) {
		if (s.first.sidx == -1) continue;		// skip -1 from wheatsheaf output
		lossvec &lpv = s.second;
		std::sort(lpv.rbegin(), lpv.rend());
		size_t nextreturnperiodindex = 0;
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		int i = 1;
		OASIS_FLOAT t = (OASIS_FLOAT)totalperiods_;
		for (auto lp : lpv) {
			OASIS_FLOAT retperiod = t / i;
			if (useReturnPeriodFile_) {
				if (nextreturnperiodindex == returnperiods_.size()) break;
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp, s.first.summary_id, s.first.sidx, max_retperiod);
			} else {
				const int bufferSize = 4096;
				char buffer[bufferSize];
				int strLen;
				strLen = snprintf(buffer, bufferSize, "%d,%d,%f,%f", s.first.summary_id, s.first.sidx, t / i, lp);
				if (ensembletosidx_.size() > 0)
					strLen += snprintf(buffer+strLen, bufferSize-strLen, ",0");
				strLen += snprintf(buffer+strLen, bufferSize-strLen, "\n");
				outputrows(handle, buffer, strLen);
			}
			i++;
		}
		if (useReturnPeriodFile_) {
			doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id, s.first.sidx, max_retperiod);
			while (nextreturnperiodindex < returnperiods_.size()) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id, s.first.sidx, max_retperiod);
			}
		}

	}

	// By ensemble ID
	if (ensembletosidx_.size() > 0) {
		for (auto ensemble : ensembletosidx_) {
			for (auto s : items) {
				if (std::find(ensemble.second.begin(), ensemble.second.end(), s.first.sidx) == ensemble.second.end()) continue;
				lossvec &lpv = s.second;
				std::sort(lpv.rbegin(), lpv.rend());
				size_t nextreturnperiodindex = 0;
				OASIS_FLOAT last_computed_rp = 0;
				OASIS_FLOAT last_computed_loss = 0;
				int i = 1;
				OASIS_FLOAT t = (OASIS_FLOAT)totalperiods_;
				for (auto lp : lpv) {
					OASIS_FLOAT retperiod = t / i;
					if (useReturnPeriodFile_) {
						if (nextreturnperiodindex == returnperiods_.size()) break;
						doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp, s.first.summary_id, s.first.sidx, max_retperiod, ensemble.first);
					} else {
						char buffer[4096];
						int strLen;
						strLen = sprintf(buffer, "%d,%d,%f,%f,%d\n", s.first.summary_id, s.first.sidx, t / i, lp, ensemble.first);
						outputrows(handle, buffer, strLen);
					}
					i++;
				}
				if (useReturnPeriodFile_) {
					doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id, s.first.sidx, max_retperiod, ensemble.first);
					while (nextreturnperiodindex < returnperiods_.size()) {
						doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id, s.first.sidx, max_retperiod, ensemble.first);
					}
				}

			}
		}
	}

}


void aggreports::wheatsheafwithweighting(int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss)
{
	if (fout_[handle] == nullptr) return;
	std::map<wheatkey, lossvec2> items;
	for (auto x : out_loss) {
		wheatkey wk;;
		wk.sidx = x.first.sidx;
		wk.summary_id = x.first.summary_id;
		lossval lv;
		lv.value = x.second;
		auto iter = periodstoweighting_.find(x.first.period_no);	// assume weighting is zero if not supplied
		if (iter != periodstoweighting_.end()) {
			lv.period_weighting = iter->second;
			lv.period_no = x.first.period_no;
			items[wk].push_back(lv);
		}
	}

	if (skipheader_ == false) {
		fprintf(fout_[handle], "summary_id,sidx,return_period,loss");
		if (ensembletosidx_.size() > 0) {
			fprintf(fout_[handle], ",ensemble_id");
		}
		fprintf(fout_[handle], "\n");
	}

	for (auto s : items) {
		// skip sidx = -1 from wheatsheaf output
		if (s.first.sidx == -1) continue;
		OASIS_FLOAT cummulative_weighting = 0;
		lossvec2 &lpv = s.second;

		std::sort(lpv.rbegin(), lpv.rend());
		size_t nextreturnperiodindex = 0;
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		OASIS_FLOAT max_retperiod = 0;
		bool largest_loss = false;
		for (auto lp : lpv) {
			cummulative_weighting += (OASIS_FLOAT)lp.period_weighting * samplesize_;
			if (lp.period_weighting) {
				OASIS_FLOAT retperiod = 1 / cummulative_weighting;
				if (!largest_loss) {
					max_retperiod = retperiod;
					largest_loss = true;
				}
				if (useReturnPeriodFile_) {
					if (nextreturnperiodindex == returnperiods_.size()) break;
					doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp.value, s.first.summary_id, s.first.sidx, max_retperiod);
				} else {
					const int bufferSize = 4096;
					char buffer[bufferSize];
					int strLen;
					strLen = snprintf(buffer, bufferSize, "%d,%d,%f,%f", s.first.summary_id, s.first.sidx, retperiod, lp.value);
					if (ensembletosidx_.size() > 0)
						strLen += snprintf(buffer+strLen, bufferSize-strLen, ",0");
					strLen += snprintf(buffer+strLen, bufferSize-strLen, "\n");
					outputrows(handle, buffer, strLen);
				}
			}
		}
		if (useReturnPeriodFile_) {
			doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id, s.first.sidx, max_retperiod);
			while (nextreturnperiodindex < returnperiods_.size()) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id, s.first.sidx, max_retperiod);
			}
		}

	}

	// By ensemble ID
	if (ensembletosidx_.size() > 0) {
		for (auto ensemble : ensembletosidx_) {
			for (auto s : items) {
				if (std::find(ensemble.second.begin(), ensemble.second.end(), s.first.sidx) == ensemble.second.end()) continue;
				OASIS_FLOAT cummulative_weighting = 0;
				lossvec2 &lpv = s.second;
				std::sort(lpv.rbegin(), lpv.rend());
				size_t nextreturnperiodindex = 0;
				OASIS_FLOAT last_computed_rp = 0;
				OASIS_FLOAT last_computed_loss = 0;
				OASIS_FLOAT max_retperiod = 0;
				bool largest_loss = false;
				for (auto lp : lpv) {
					cummulative_weighting += (OASIS_FLOAT)lp.period_weighting * samplesize_;
					if (lp.period_weighting) {
						OASIS_FLOAT retperiod = 1 / cummulative_weighting;
						if (!largest_loss) {
							max_retperiod = retperiod;
							largest_loss = true;
						}
						if (useReturnPeriodFile_) {
							if (nextreturnperiodindex == returnperiods_.size()) break;
							doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp.value, s.first.summary_id, s.first.sidx, max_retperiod, ensemble.first);
						} else {
							char buffer[4096];
							int strLen;
							strLen = sprintf(buffer, "%d,%d,%f,%f,%d\n",
									 s.first.summary_id, s.first.sidx, retperiod, lp.value, ensemble.first);
							outputrows(handle, buffer, strLen);
						}
					}
				}
				if (useReturnPeriodFile_) {
					doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id, s.first.sidx, max_retperiod, ensemble.first);
					while (nextreturnperiodindex < returnperiods_.size()) {
						doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id, s.first.sidx, max_retperiod, ensemble.first);
					}
				}
			}
		}
	}

}

void aggreports::outputOccWheatsheaf()
{
	if (periodstoweighting_.size() == 0) {
		wheatsheaf(OCC_WHEATSHEAF, max_out_loss_);
	}
	else {
		wheatsheafwithweighting(OCC_WHEATSHEAF, max_out_loss_);
	}

}
void aggreports::outputAggWheatsheaf()
{
	if (periodstoweighting_.size() == 0) {
		wheatsheaf(AGG_WHEATSHEAF, agg_out_loss_);
	}
	else {
		wheatsheafwithweighting(AGG_WHEATSHEAF, agg_out_loss_);
	}
}


void aggreports::writewheatSheafMeanwithweighting(const std::map<int, std::vector<lossval>> mean_map,
						  const int samplesize, const int handle,
						  const int ensemble_id)
{

	// Variables for ORD output
	int epcalc = PERSAMPLEMEAN;   // All type 2
	int eptype = 0;
	int eptype_tvar = 0;
	if (ord_out_) {
		if (handle == OCC_WHEATSHEAF_MEAN) {
			eptype = OEP;
			eptype_tvar = OEPTVAR;
		} else if (handle == AGG_WHEATSHEAF_MEAN) {
			eptype = AEP;
			eptype_tvar = AEPTVAR;
		}
	}

	for (auto m : mean_map) {

		std::vector<lossval> &lpv = m.second;
		std::vector<lossval>::reverse_iterator rit = lpv.rbegin();
		int maxindex = (int)lpv.size();
		while (rit != lpv.rend()) {
			if (rit->value != 0.0) break;
			maxindex--;
			rit++;
		}
		size_t nextreturnperiodindex = 0;
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		int i = 1;
		OASIS_FLOAT cummulative_weighting = 0;
		OASIS_FLOAT max_retperiod = 0;
		bool largest_loss = false;
		for (auto lp : lpv) {
			cummulative_weighting += (OASIS_FLOAT)(lp.period_weighting * samplesize_);
			if (lp.period_weighting) {
				OASIS_FLOAT retperiod = 1 / cummulative_weighting;
				if (!largest_loss) {
					max_retperiod = retperiod;
					largest_loss = true;
				}
				if (useReturnPeriodFile_) {
					OASIS_FLOAT loss = lp.value / samplesize;
					doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, loss, m.first, 2, max_retperiod, ensemble_id, epcalc, eptype);
				} else {
					const int bufferSize = 4096;
					char buffer[bufferSize];
					int strLen;
					strLen = snprintf(buffer, bufferSize, "%d,2,%f,%f", m.first, retperiod, lp.value / samplesize);
					if (ensembletosidx_.size() > 0)
						strLen += snprintf(buffer+strLen, bufferSize-strLen, ",%d", ensemble_id);
					strLen += snprintf(buffer+strLen, bufferSize-strLen, "\n");
					outputrows(handle, buffer, strLen);

					// ORD output
					if (ord_out_ != nullptr && ensemble_id == 0) {
						buffer[0] = 0;
						strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", m.first, epcalc, eptype, retperiod, lp.value / samplesize);
						outputrows(buffer, strLen);
					}
				}
			}
			i++;
			if (i > maxindex) break;
		}
		if (useReturnPeriodFile_) {
			do {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, m.first, 2, max_retperiod, ensemble_id, epcalc, eptype);
			} while (nextreturnperiodindex < returnperiods_.size());

		}
	}

	// Tail Value at Risk (TVaR) - ORD output only
	if (ord_out_ != nullptr && ensemble_id == 0) {
		for (auto m : mean_map) {

			std::vector<lossval> &lpv = m.second;
			std::vector<lossval>::reverse_iterator rit = lpv.rbegin();
			int maxindex = (int)lpv.size();
			while (rit != lpv.rend()) {
				if (rit->value != 0.0) break;
				maxindex--;
				rit++;
			}
			size_t nextreturnperiodindex = 0;
			OASIS_FLOAT last_computed_rp = 0;
			OASIS_FLOAT last_computed_loss = 0;
			int i = 1;
			OASIS_FLOAT tvar = 0;
			OASIS_FLOAT cummulative_weighting = 0;
			OASIS_FLOAT max_retperiod = 0;
			bool largest_loss = false;
			for (auto lp : lpv) {
				cummulative_weighting += (OASIS_FLOAT)(lp.period_weighting * samplesize_);
				if (lp.period_weighting) {
					OASIS_FLOAT retperiod = 1 / cummulative_weighting;
					if (!largest_loss) {
						max_retperiod = retperiod;
						largest_loss = true;
					}
					OASIS_FLOAT loss = lp.value / samplesize;
					if (useReturnPeriodFile_) {
						doreturnperiodout_tvar(nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, loss, tvar, i, m.first, epcalc, eptype_tvar, max_retperiod);
						tvar = tvar - ((tvar - loss) / i);
					} else {
						tvar = tvar - ((tvar - loss) / i);
						const int bufferSize = 4096;
						char buffer[bufferSize];
						int strLen;
						strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", m.first, epcalc, eptype_tvar, retperiod, tvar);
						outputrows(buffer, strLen);
					}
					i++;
					if (i > maxindex) break;
				}
				if (useReturnPeriodFile_) {
					do {
						doreturnperiodout_tvar(nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, tvar, i, m.first, epcalc, eptype_tvar, max_retperiod);
					} while (nextreturnperiodindex < returnperiods_.size());
				}
			}

		}
	}

}


void aggreports::wheatSheafMeanwithweighting(int samplesize, int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss)
{

	// Variables for ORD output
	int epcalc = MEANDR;   // Only Type 1 written here
	int eptype = 0;
	int eptype_tvar = 0;
	if (ord_out_) {
		if (handle == OCC_WHEATSHEAF_MEAN) {
			eptype = OEP;
			eptype_tvar = OEPTVAR;
		} else if (handle == AGG_WHEATSHEAF_MEAN) {
			eptype = AEP;
			eptype_tvar = AEPTVAR;
		}
	}

	if (fout_[handle] == nullptr) return;
	std::map<wheatkey, lossvec2> items;
	int maxPeriod = 0;
	for (auto x : out_loss) {
		wheatkey wk;;
		wk.sidx = x.first.sidx;
		wk.summary_id = x.first.summary_id;
		lossval lv;
		lv.value = x.second;
		auto iter = periodstoweighting_.find(x.first.period_no);	// assume weighting is zero if not supplied
		if (iter != periodstoweighting_.end()) {
			lv.period_weighting = iter->second;
			lv.period_no = x.first.period_no;
			if (lv.period_no > maxPeriod) maxPeriod = lv.period_no;
			items[wk].push_back(lv);
		}
	}

	std::map<int, std::vector<lossval>> mean_map;
	for (int i = 1; i <= maxsummaryid_; i++) {
		mean_map[i] = std::vector<lossval>(maxPeriod, lossval());
	}
	if (skipheader_ == false) {
		fprintf(fout_[handle], "summary_id,type,return_period,loss");
		if (ensembletosidx_.size() > 0) fprintf(fout_[handle], ",ensemble_id");
		fprintf(fout_[handle], "\n");
	}

	for (auto s : items) {
		size_t nextreturnperiodindex = 0;
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		lossvec2 &lpv = s.second;
		std::sort(lpv.rbegin(), lpv.rend());
		OASIS_FLOAT cummulative_weighting = 0;
		OASIS_FLOAT max_retperiod = 0;
		bool largest_loss = false;
		if (s.first.sidx != -1) {
			for (auto lp : lpv) {
				lossval &l = mean_map[s.first.summary_id][lp.period_no-1];
				l.period_no = lp.period_no;
				l.period_weighting = lp.period_weighting;
				l.value += lp.value;
			}
		}
		else {
			for (auto lp : lpv) {
				cummulative_weighting += (OASIS_FLOAT)(lp.period_weighting * samplesize_);
				if (lp.period_weighting) {
					OASIS_FLOAT retperiod = 1 / cummulative_weighting;
					if (!largest_loss) {
						max_retperiod = retperiod;
						largest_loss = true;
					}
					if (useReturnPeriodFile_) {
						doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp.value, s.first.summary_id, 1, max_retperiod, 0, epcalc, eptype);
					} else {
						const int bufferSize = 4096;
						char buffer[bufferSize];
						int strLen;
						strLen = snprintf(buffer, bufferSize, "%d,1,%f,%f", s.first.summary_id, retperiod, lp.value);
						if (ensembletosidx_.size() > 0)
							strLen += snprintf(buffer+strLen, bufferSize-strLen, ",0");
						strLen += snprintf(buffer+strLen, bufferSize-strLen, "\n");
						outputrows(handle, buffer, strLen);

						// ORD output
						if (ord_out_ != nullptr && meanDR_[eptype] == false) {
							buffer[0] = 0;
							strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", s.first.summary_id, epcalc, eptype, retperiod, lp.value);
							outputrows(buffer, strLen);
						}
					}
				}
			}
			if (useReturnPeriodFile_) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id, 1, max_retperiod, 0, epcalc, eptype);
				while (nextreturnperiodindex < returnperiods_.size()) {
					doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id, 1, max_retperiod, 0, epcalc, eptype);
				}
			}
		}
	}

	// Tail Value at Risk (TVaR) - Mean Disaster Recovery ORD output only
	if (ord_out_ != nullptr && meanDR_[eptype] == false) {
		for (auto s : items) {
			if (s.first.sidx != -1) continue;
			OASIS_FLOAT tvar = 0;
			OASIS_FLOAT cummulative_weighting = 0;
			lossvec2 &lpv = s.second;
			std::sort(lpv.rbegin(), lpv.rend());
			size_t nextreturnperiodindex = 0;
			OASIS_FLOAT last_computed_rp = 0;
			OASIS_FLOAT last_computed_loss = 0;
			OASIS_FLOAT max_retperiod = 0;
			bool largest_loss = false;
			int i = 1;
			for (auto lp : lpv) {
				cummulative_weighting += (lp.period_weighting * samplesize_);
				if (lp.period_weighting) {
					OASIS_FLOAT retperiod = 1 / cummulative_weighting;
					if (!largest_loss) {
						max_retperiod = retperiod;
						largest_loss = true;
					}
					if (useReturnPeriodFile_) {
						doreturnperiodout_tvar(nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp.value, tvar, i, s.first.summary_id, epcalc, eptype_tvar, max_retperiod);
						tvar = tvar - ((tvar - lp.value) / i);
					} else {
						tvar = tvar - ((tvar - lp.value) / i);
						const int bufferSize = 4096;
						char buffer[bufferSize];
						int strLen;
						strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", s.first.summary_id, epcalc, eptype_tvar, retperiod, tvar);
						outputrows(buffer, strLen);
					}
					i++;
				}
			}
			if (useReturnPeriodFile_) {
				do {
					doreturnperiodout_tvar(nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, tvar, i, s.first.summary_id, epcalc, eptype_tvar, max_retperiod);
				} while (nextreturnperiodindex < returnperiods_.size());
			}
		}
	}

	meanDR_[eptype] = true;   // Do not output Type 1 to ORD file again

	if (samplesize == 0) return; // avoid divide by zero error

	writewheatSheafMeanwithweighting(mean_map, samplesize, handle, 0);   // Type 2

	// By ensemble ID
	if (ensembletosidx_.size() > 0) {
		for (auto ensemble : ensembletosidx_) {

			maxPeriod = 0;
			for (auto x : items) {
				if (std::find(ensemble.second.begin(), ensemble.second.end(), x.first.sidx) == ensemble.second.end()) continue;
				lossvec2 &lpv = x.second;
				for(auto lp : lpv) {
					if (lp.period_no > maxPeriod) maxPeriod = lp.period_no;
				}
			}

			mean_map.clear();
			for (int i = 1; i <= maxsummaryid_; i++) {
				mean_map[i] = std::vector<lossval>(maxPeriod, lossval());
			}

			for (auto s : items) {
				if (std::find(ensemble.second.begin(), ensemble.second.end(), s.first.sidx) == ensemble.second.end()) continue;
				lossvec2 &lpv = s.second;
				std::sort(lpv.rbegin(), lpv.rend());
				for (auto lp : lpv) {
					lossval &l = mean_map[s.first.summary_id][lp.period_no-1];
					l.period_no = lp.period_no;
					l.period_weighting = lp.period_weighting;
					l.value += lp.value;
				}
			}
			
			writewheatSheafMeanwithweighting(mean_map, samplesize, handle, ensemble.first);

		}
	}

}


void aggreports::writewheatSheafMean(const std::map<int, std::vector<OASIS_FLOAT>> &mean_map,
				     const int samplesize, const int handle,
				     const OASIS_FLOAT max_retperiod, const int ensemble_id)
{

	// Variables for ORD output
	int epcalc = PERSAMPLEMEAN;   // All type 2
	int eptype = 0;
	int eptype_tvar = 0;
	if (ord_out_) {
		if (handle == OCC_WHEATSHEAF_MEAN) {
			eptype = OEP;
			eptype_tvar = OEPTVAR;
		} else if (handle == AGG_WHEATSHEAF_MEAN) {
			eptype = AEP;
			eptype_tvar = AEPTVAR;
		}
	}

	for (auto m : mean_map) {

		std::vector<OASIS_FLOAT> &lpv = m.second;
		std::vector<OASIS_FLOAT>::reverse_iterator rit = lpv.rbegin();
		int maxindex = (int)lpv.size();
		while (rit != lpv.rend()) {
			if (*rit != 0.0) break;
			maxindex--;
			rit++;
		}
		size_t nextreturnperiodindex = 0;
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		int i = 1;
		OASIS_FLOAT t = (OASIS_FLOAT)totalperiods_;
		for (auto lp : lpv) {
			OASIS_FLOAT retperiod = t / i;
			if (useReturnPeriodFile_) {
				OASIS_FLOAT loss = lp / samplesize;
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, loss, m.first, 2, max_retperiod, ensemble_id, epcalc, eptype);
			} else {
				const int bufferSize = 4096;
				char buffer[bufferSize];
				int strLen;
				strLen = snprintf(buffer, bufferSize, "%d,2,%f,%f", m.first, retperiod, lp / samplesize);
				if (ensembletosidx_.size() > 0)
					strLen += snprintf(buffer+strLen, bufferSize-strLen, ",%d", ensemble_id);
				strLen += snprintf(buffer+strLen, bufferSize-strLen, "\n");
				outputrows(handle, buffer, strLen);

				// ORD output
				if (ord_out_ != nullptr && ensemble_id == 0) {
					buffer[0] = 0;
					strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", m.first, epcalc, eptype, retperiod, lp / samplesize);
					outputrows(buffer, strLen);
				}
			}
			i++;
			if (i > maxindex) break;
		}
		if (useReturnPeriodFile_) {
			do {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, m.first, 2, max_retperiod, ensemble_id, epcalc, eptype);
			} while (nextreturnperiodindex < returnperiods_.size());
		}

	}

	// Tail Value at Risk (TVaR) - ORD output only
	if (ord_out_ != nullptr && ensemble_id == 0) {
		for (auto m : mean_map) {

			std::vector<OASIS_FLOAT> &lpv = m.second;
			std::vector<OASIS_FLOAT>::reverse_iterator rit = lpv.rbegin();
			int maxindex = (int)lpv.size();
			while (rit != lpv.rend()) {
				if (*rit != 0.0) break;
				maxindex--;
				rit++;
			}
			size_t nextreturnperiodindex = 0;
			OASIS_FLOAT last_computed_rp = 0;
			OASIS_FLOAT last_computed_loss = 0;
			int i = 1;
			OASIS_FLOAT tvar = 0;
			OASIS_FLOAT t = (OASIS_FLOAT)totalperiods_;
			for (auto lp : lpv) {
				OASIS_FLOAT retperiod = t / i;
				OASIS_FLOAT loss = lp / samplesize;
				if (useReturnPeriodFile_) {
					doreturnperiodout_tvar(nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, loss, tvar, i, m.first, epcalc, eptype_tvar, max_retperiod);
					tvar = tvar - ((tvar - loss) / i);
				} else {
					tvar = tvar - ((tvar - loss) / i);
					const int bufferSize = 4096;
					char buffer[bufferSize];
					int strLen;
					strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", m.first, epcalc, eptype_tvar, retperiod, tvar);
					outputrows(buffer, strLen);
				}
				i++;
				if (i > maxindex) break;
			}
			if (useReturnPeriodFile_) {
				do {
					doreturnperiodout_tvar(nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, tvar, i, m.first, epcalc, eptype_tvar, max_retperiod);
				} while (nextreturnperiodindex < returnperiods_.size());
			}

		}
	}

}


void aggreports::wheatSheafMean(int samplesize, int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss)
{

	// Variables for ORD output
	int epcalc = MEANDR;   // Only Type 1 written here
	int eptype = 0;
	int eptype_tvar = 0;
	if (ord_out_) {
		if (handle == OCC_WHEATSHEAF_MEAN) {
			eptype = OEP;
			eptype_tvar = OEPTVAR;
		} else if (handle == AGG_WHEATSHEAF_MEAN) {
			eptype = AEP;
			eptype_tvar = AEPTVAR;
		}
	}

	if (fout_[handle] == nullptr) return;
	std::map<wheatkey, lossvec> items;
	for (auto x : out_loss) {
		wheatkey wk;;
		wk.sidx = x.first.sidx;
		wk.summary_id = x.first.summary_id;
		items[wk].push_back(x.second);
	}

	size_t maxcount = 0;
	for (auto x : items) {
		if (x.second.size() > maxcount) maxcount = x.second.size();
	}

	std::map<int, std::vector<OASIS_FLOAT>> mean_map;
	for (int i = 1; i <= maxsummaryid_; i++) {
		mean_map[i] = std::vector<OASIS_FLOAT>(maxcount, 0);
	}
	if (skipheader_ == false) {
		fprintf(fout_[handle],"summary_id,type,return_period,loss");
		if (ensembletosidx_.size() > 0) fprintf(fout_[handle], ",ensemble_id");
		fprintf(fout_[handle], "\n");
	}

	OASIS_FLOAT max_retperiod = (OASIS_FLOAT)totalperiods_;

	for (auto s : items) {
		size_t nextreturnperiodindex = 0;
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		lossvec &lpv = s.second;
		std::sort(lpv.rbegin(), lpv.rend());
		if (s.first.sidx != -1) {
			int i = 0;
			for (auto lp : lpv) {
				mean_map[s.first.summary_id][i] += lp;
				i++;
			}
		}
		else {
			int i = 1;
			OASIS_FLOAT t = (OASIS_FLOAT) totalperiods_;
			for (auto lp : lpv) {
				OASIS_FLOAT retperiod = t / i;
				if (useReturnPeriodFile_) {
					doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp, s.first.summary_id, 1, max_retperiod, 0, epcalc, eptype);
				} else {
					const int bufferSize = 4096;
					char buffer[bufferSize];
					int strLen;
					strLen = snprintf(buffer, bufferSize, "%d,1,%f,%f", s.first.summary_id, retperiod, lp);
					if (ensembletosidx_.size() > 0)
						strLen += snprintf(buffer+strLen, bufferSize-strLen, ",0");
					strLen += snprintf(buffer+strLen, bufferSize-strLen, "\n");
					outputrows(handle, buffer, strLen);

					// ORD output
					if (ord_out_ != nullptr && meanDR_[eptype] == false) {
						buffer[0] = 0;
						strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", s.first.summary_id, epcalc, eptype, retperiod, lp);
						outputrows(buffer, strLen);
					}
				}

				i++;
			}
			if (useReturnPeriodFile_) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id, 1, max_retperiod, 0, epcalc, eptype);
				while (nextreturnperiodindex < returnperiods_.size()) {
					doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id, 1, max_retperiod, 0, epcalc, eptype);
				}
			}
		}
	}

	// Tail Value at Risk (TVaR) - Mean Disaster Recovery ORD output only
	if (ord_out_ != nullptr && meanDR_[eptype] == false) {
		for (auto s : items) {
			if (s.first.sidx != -1) continue;   // Type 1 only
			lossvec &lpv = s.second;
			std::sort(lpv.rbegin(), lpv.rend());
			OASIS_FLOAT tvar = 0;
			size_t nextreturnperiodindex = 0;
			OASIS_FLOAT last_computed_rp = 0;
			OASIS_FLOAT last_computed_loss = 0;
			int i = 1;
			OASIS_FLOAT t = (OASIS_FLOAT)totalperiods_;
			for (auto lp : lpv) {
				OASIS_FLOAT retperiod = t / i;
				if (useReturnPeriodFile_) {
					doreturnperiodout_tvar(nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp, tvar, i, s.first.summary_id, epcalc, eptype_tvar, max_retperiod);
					tvar = tvar - ((tvar - lp) / i);
				} else {
					tvar = tvar - ((tvar - lp) / i);
					const int bufferSize = 4096;
					char buffer[bufferSize];
					int strLen;
					strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", s.first.summary_id, epcalc, eptype_tvar, retperiod, tvar);
					outputrows(buffer, strLen);
				}
				i++;
			}
			if (useReturnPeriodFile_) {
				do {
					doreturnperiodout_tvar(nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, tvar, i, s.first.summary_id, epcalc, eptype_tvar, max_retperiod);
				} while (nextreturnperiodindex < returnperiods_.size());
			}
		}
	}

	meanDR_[eptype] = true;   // Do not output Type 1 to ORD file again

	if (samplesize == 0) return; // avoid divide by zero error

	writewheatSheafMean(mean_map, samplesize, handle, max_retperiod, 0);   // Type 2

	// By ensemble ID
	if (ensembletosidx_.size() > 0) {
		for (auto ensemble : ensembletosidx_) {

			maxcount = 0;
			for (auto x : items) {
				if (std::find(ensemble.second.begin(), ensemble.second.end(), x.first.sidx) == ensemble.second.end()) continue;
				if (x.second.size() > maxcount) maxcount = x.second.size();
			}
			
			mean_map.clear();
			for (int i = 1; i <= maxsummaryid_; i++) {
				mean_map[i] = std::vector<OASIS_FLOAT>(maxcount, 0);
			}

			for (auto s : items) {
				if (std::find(ensemble.second.begin(), ensemble.second.end(), s.first.sidx) == ensemble.second.end()) continue;
				lossvec &lpv = s.second;
				std::sort(lpv.rbegin(), lpv.rend());
				int i = 0;
				for (auto lp : lpv) {
					mean_map[s.first.summary_id][i] += lp;
					i++;
				}
			}

			writewheatSheafMean(mean_map, (int)ensemble.second.size(), handle, max_retperiod, ensemble.first);

		}

	}

}


void aggreports::outputOccWheatSheafMean(int samplesize)
{
	if (periodstoweighting_.size() == 0) {
		wheatSheafMean(samplesize, OCC_WHEATSHEAF_MEAN, max_out_loss_);
	}else {
		wheatSheafMeanwithweighting(samplesize, OCC_WHEATSHEAF_MEAN, max_out_loss_);
	}
}

void aggreports::outputAggWheatSheafMean(int samplesize)
{
	if (periodstoweighting_.size() == 0) {
		wheatSheafMean(samplesize, AGG_WHEATSHEAF_MEAN, agg_out_loss_);
	}
	else {
		wheatSheafMeanwithweighting(samplesize, AGG_WHEATSHEAF_MEAN, agg_out_loss_);
	}
}


inline void aggreports::writeSampleMeanwithweighting(const int handle,
	const int type, const std::map<summary_id_period_key, lossval> &items,
	const int ensemble_id) {

	// Variables for ORD output
	int epcalc = 0;
	int eptype = 0;
	int eptype_tvar = 0;
	if (ord_out_) {
		if (type == 1) {
			epcalc = MEANDR;
		} else if (type == 2) {
			epcalc = MEANSAMPLE;
		}

		if (handle == OCC_SAMPLE_MEAN) {
			eptype = OEP;
			eptype_tvar = OEPTVAR;
		} else if (handle == AGG_SAMPLE_MEAN) {
			eptype = AEP;
			eptype_tvar = AEPTVAR;
		}
	}

	std::map<int, std::vector<lossval>> mean_map;

	for (auto s : items) {
		mean_map[s.first.summary_id].push_back(s.second);
	}

	for (auto m : mean_map) {
		std::vector<lossval> &lpv = m.second;
		std::sort(lpv.rbegin(), lpv.rend());
		size_t nextreturnperiodindex = 0;
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		OASIS_FLOAT cummulative_weighting = 0;
		OASIS_FLOAT max_retperiod = 0;
		bool largest_loss = false;
		for (auto lp : lpv) {
			cummulative_weighting += (OASIS_FLOAT) (lp.period_weighting * samplesize_);
			if (lp.period_weighting) {
				OASIS_FLOAT retperiod = 1 / cummulative_weighting;
				if (!largest_loss) {
					max_retperiod = retperiod;
					largest_loss = true;
				}
				if (useReturnPeriodFile_) {
					doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp.value, m.first, type, max_retperiod, ensemble_id, epcalc, eptype);
				}
				else {
					const int bufferSize = 4096;
					char buffer[bufferSize];
					int strLen;
					strLen = snprintf(buffer, bufferSize, "%d,%d,%f,%f", m.first, type, retperiod, lp.value);
					if (ensembletosidx_.size() > 0)
						strLen += snprintf(buffer+strLen, bufferSize-strLen, ",%d", ensemble_id);
					strLen += snprintf(buffer+strLen, bufferSize-strLen, "\n");
					outputrows(handle, buffer, strLen);

					if (ord_out_ && ensemble_id == 0) {
						if (epcalc == MEANSAMPLE || (epcalc == MEANDR && meanDR_[eptype] == false)) {
							buffer[0] = 0;
							strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", m.first, epcalc, eptype, retperiod, lp.value);
							outputrows(buffer, strLen);
						}
					}
				}
			}
		}
		if (useReturnPeriodFile_) {
			doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, m.first, type, max_retperiod, ensemble_id, epcalc, eptype);
			while (nextreturnperiodindex < returnperiods_.size()) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, m.first, type, max_retperiod, ensemble_id, epcalc, eptype);
			}
		}
	}

}


void aggreports::sampleMeanwithweighting(int samplesize, int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss)
{
	if (fout_[handle] == nullptr) return;
	std::map<summary_id_period_key, lossval> itemsType1;
	std::map<summary_id_period_key, lossval> itemsType2;

	for (auto x : out_loss) {
		summary_id_period_key sk;
		auto iter = periodstoweighting_.find(x.first.period_no);	// assume weighting is zero if not supplied
		sk.period_no = x.first.period_no;
		sk.summary_id = x.first.summary_id;
		if (iter != periodstoweighting_.end()) {
			if (x.first.sidx == -1) {
				sk.type = 1;
				itemsType1[sk].period_weighting = iter->second;
				// For debugging
				itemsType1[sk].period_no = x.first.period_no;
				itemsType1[sk].value += x.second;
			}
			else if (samplesize > 0) {
				sk.type = 2;
				itemsType2[sk].period_weighting = iter->second;
				// For debugging
				itemsType2[sk].period_no = x.first.period_no;
				itemsType2[sk].value += (x.second / samplesize);
			}
		}
	}
	
	if (skipheader_ == false) {
		fprintf(fout_[handle], "summary_id,type,return_period,loss");
		if (ensembletosidx_.size() > 0) fprintf(fout_[handle], ",ensemble_id");
		fprintf(fout_[handle], "\n");
	}
	writeSampleMeanwithweighting(handle, 1, itemsType1);   // sidx = -1
	writeSampleMeanwithweighting(handle, 2, itemsType2);   // sidx > 0

	// By ensemble ID
	if (ensembletosidx_.size() > 0) {
		for (auto ensemble : ensembletosidx_) {
			itemsType2.clear();
			for (auto sidx : ensemble.second) {
				for (auto x : out_loss) {
					summary_id_period_key sk;
					auto iter = periodstoweighting_.find(x.first.period_no);
					sk.period_no = x.first.period_no;
					sk.summary_id = x.first.summary_id;
					sk.type = 2;
					if (iter != periodstoweighting_.end() && x.first.sidx == sidx) {
						itemsType2[sk].period_weighting = iter->second;
						itemsType2[sk].period_no = x.first.period_no;   // For debugging
						itemsType2[sk].value += (x.second / samplesize);
					}
				}
			}
			writeSampleMeanwithweighting(handle, 2, itemsType2, ensemble.first);
		}
	}

}

void aggreports::writesampleMean(const int handle,
				 const std::map<summary_id_type_key, std::vector<OASIS_FLOAT>> &mean_map,
				 const OASIS_FLOAT max_retperiod,
				 const int ensemble_id)
{

	// Variables for ORD output
	int epcalc = 0;   // Fail safe
	int eptype = 0;
	int eptype_tvar = 0;
	if (ord_out_) {
		if (handle == OCC_SAMPLE_MEAN) {
			eptype = OEP;
			eptype_tvar = OEPTVAR;
		} else if (handle == AGG_SAMPLE_MEAN) {
			eptype = AEP;
			eptype_tvar = AEPTVAR;
		}
	}

	for (auto m : mean_map) {
		std::vector<OASIS_FLOAT> &lpv = m.second;
		std::sort(lpv.rbegin(), lpv.rend());
		size_t nextreturnperiodindex = 0;
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		int i = 1;
		OASIS_FLOAT t = (OASIS_FLOAT)totalperiods_;
		for (auto lp : lpv) {
			OASIS_FLOAT retperiod = t / i;
			if (m.first.type == 1) epcalc = MEANDR;
			else if (m.first.type == 2) epcalc = MEANSAMPLE;
			else epcalc = 0;   // Should never get here

			if (useReturnPeriodFile_) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp, m.first.summary_id, m.first.type, max_retperiod, ensemble_id, epcalc, eptype);
			} else {
				const int bufferSize = 4096;
				char buffer[bufferSize];
				int strLen;
				strLen = snprintf(buffer, bufferSize, "%d,%d,%f,%f\n", m.first.summary_id, m.first.type, retperiod, lp);
				if (ensembletosidx_.size() > 0)
					strLen += snprintf(buffer+strLen, bufferSize-strLen, ",0");
				strLen += snprintf(buffer+strLen, bufferSize-strLen, "\n");
				outputrows(handle, buffer, strLen);

				// ORD output
				if (ord_out_ != nullptr && ensemble_id == 0) {
					if (epcalc == MEANSAMPLE || (epcalc == MEANDR && meanDR_[eptype] == false)) {
						buffer[0] = 0;
						strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", m.first.summary_id, epcalc, eptype, retperiod, lp);
						outputrows(buffer, strLen);
					}
				}
			}
			i++;
		}
		if (useReturnPeriodFile_) {
			do {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, m.first.summary_id, m.first.type, max_retperiod, ensemble_id, epcalc, eptype);
			} while (nextreturnperiodindex < returnperiods_.size());
		}
	}

}

void aggreports::sampleMean(int samplesize, int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss)
{

	if (fout_[handle] == nullptr) return;
	std::map<summary_id_period_key, OASIS_FLOAT> items;

	for (auto x : out_loss) {
		summary_id_period_key sk;
		sk.period_no = x.first.period_no;
		sk.summary_id = x.first.summary_id;
		if (x.first.sidx == -1) {
			sk.type = 1;
			items[sk] += x.second;
		}
		else {
			if (samplesize > 0) {
				sk.type = 2;
				items[sk] += (x.second / samplesize);
			}
		}
	}

	std::map<summary_id_type_key, std::vector<OASIS_FLOAT>> mean_map;

	if (skipheader_ == false) {
		fprintf(fout_[handle],"summary_id,type,return_period,loss");
		if (ensembletosidx_.size() > 0)
			fprintf(fout_[handle], ",ensemble_id");
		fprintf(fout_[handle], "\n");
	}

	for (auto s : items) {
		summary_id_type_key st;
		st.summary_id = s.first.summary_id;
		st.type = s.first.type;
		mean_map[st].push_back(s.second);
	}

	OASIS_FLOAT max_retperiod = (OASIS_FLOAT)totalperiods_;

	writesampleMean(handle, mean_map, max_retperiod);

	// By ensemble ID
	if (ensembletosidx_.size() > 0) {
		for (auto ensemble : ensembletosidx_) {
			items.clear();
			for (auto sidx : ensemble.second) {
				for (auto x : out_loss) {
					if (x.first.sidx == sidx) {
						summary_id_period_key sk;
						sk.period_no = x.first.period_no;
						sk.summary_id = x.first.summary_id;
						sk.type = 2;
						items[sk] += (x.second / ensemble.second.size());
					}
				}
			}

			mean_map.clear();
			for (auto s : items) {
				summary_id_type_key st;
				st.summary_id = s.first.summary_id;
				st.type = s.first.type;
				mean_map[st].push_back(s.second);
			}

			writesampleMean(handle, mean_map, max_retperiod, ensemble.first);

		}

	}

}


void aggreports::outputOccSampleMean(int samplesize)
{

	if (periodstoweighting_.size() == 0) {
		sampleMean(samplesize, OCC_SAMPLE_MEAN, max_out_loss_);
	}
	else {
		sampleMeanwithweighting(samplesize, OCC_SAMPLE_MEAN, max_out_loss_);
	}
}

void aggreports::outputAggSampleMean(int samplesize)
{

	if (periodstoweighting_.size() == 0) {
		sampleMean(samplesize, AGG_SAMPLE_MEAN, agg_out_loss_);
	}
	else {
		sampleMeanwithweighting(samplesize, AGG_SAMPLE_MEAN, agg_out_loss_);
	}
}
