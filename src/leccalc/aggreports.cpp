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
#include "aggreports.h"


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
	std::map<outkey2, OASIS_FLOAT> &max_out_loss, FILE **fout, bool useReturnPeriodFile, int samplesize) :
	totalperiods_(totalperiods), maxsummaryid_(maxsummaryid), agg_out_loss_(agg_out_loss), max_out_loss_(max_out_loss),
	fout_(fout), useReturnPeriodFile_(useReturnPeriodFile), samplesize_(samplesize)
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
		//printf("%d, %f\n", p.period_no, p.weighting);
		total_weighting += p.weighting;
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
		fprintf(stderr, "%s: Error opening file %s\n", __func__, RETURNPERIODS_FILE);
		exit(-1);
	}

	int return_period;

	while (fread(&return_period, sizeof(return_period), 1, fin) == 1) {
		returnperiods_.push_back(return_period);
	}

	fclose(fin);

	if (returnperiods_.size() == 0) {
		useReturnPeriodFile_ = false;
		std::cerr << __func__ << ": No return periods loaded - running without defined return periods option\n";
	}
}

// this one should run using period table 
void aggreports::fulluncertaintywithweighting(int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss)
{
	if (fout_[handle] == nullptr) return;
	std::map<int, lossvec2> items;
	for (auto x : out_loss) {
		lossval lv;
		lv.value = x.second;
		auto iter = periodstoweighting_.find(x.first.period_no);	// assume weighting is zero if not supplied
		if (iter != periodstoweighting_.end()) {
			lv.period_weighting = iter->second;
			lv.period_no = x.first.period_no;		// for debugging
			items[x.first.summary_id].push_back(lv);
		}
	}

	fprintf(fout_[handle], "summary_id,return_period,loss\n");	

	for (auto s : items) {
		OASIS_FLOAT cummulative_weigthing = 0;
		lossvec2 &lpv = s.second;
		std::sort(lpv.rbegin(), lpv.rend());
		int nextreturnperiodindex = 0;
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		for (auto lp : lpv) {
			cummulative_weigthing += lp.period_weighting;
			if (cummulative_weigthing) {
				OASIS_FLOAT retperiod = 1 / cummulative_weigthing;
				//if (samplesize_) retperiod = retperiod * samplesize_;
				//retperiod = retperiod / i;
				if (useReturnPeriodFile_) {
					doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp.value, s.first, 0);
				}
				else {
					fprintf(fout_[handle], "%d,%f,%f\n", s.first, retperiod, lp.value);
				}
				//fprintf(fout_[handle], "%d,%f,%f\n", s.first, f, lp.value);
			}
		}
	}
}

void aggreports::fulluncertainty(int handle,const std::map<outkey2, OASIS_FLOAT> &out_loss)
{
	if (fout_[handle] == nullptr) return;
	std::map<int, lossvec> items;
	for (auto x : out_loss) {
		items[x.first.summary_id].push_back(x.second);
	}

	fprintf(fout_[handle], "summary_id,return_period,loss\n");

	for (auto s : items) {
		lossvec &lpv = s.second;
		std::sort(lpv.rbegin(), lpv.rend());
		int nextreturnperiodindex = 0;
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		int i = 1;
		OASIS_FLOAT t = (OASIS_FLOAT) totalperiods_;
		if (samplesize_) t = (OASIS_FLOAT) (totalperiods_ * samplesize_ );
		for (auto lp : lpv) {
			OASIS_FLOAT retperiod = t / i;
			if (useReturnPeriodFile_) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp, s.first, 0);
			}else{
				fprintf(fout_[handle],"%d,%f,%f\n", s.first, retperiod, lp);
			}
			i++;
		}
		if (useReturnPeriodFile_) {
			doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first, 0);
			while (nextreturnperiodindex < returnperiods_.size()) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first, 0);
			}
		}

	}
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

void aggreports::doreturnperiodout(int handle, int &nextreturnperiod_index, OASIS_FLOAT &last_return_period, OASIS_FLOAT &last_loss,
			OASIS_FLOAT current_return_period, OASIS_FLOAT current_loss, int summary_id, int type)
{
	if (nextreturnperiod_index >= returnperiods_.size()) {
		return;
	}
	OASIS_FLOAT nextreturnperiod_value = returnperiods_[nextreturnperiod_index];
	while (current_return_period <= nextreturnperiod_value) {
		OASIS_FLOAT loss = getloss(nextreturnperiod_value, last_return_period, last_loss, current_return_period, current_loss);
		if(type) fprintf(fout_[handle],"%d,%d,%f,%f\n", summary_id, type, (OASIS_FLOAT)nextreturnperiod_value, loss);
		else fprintf(fout_[handle],"%d,%f,%f\n", summary_id, (OASIS_FLOAT)nextreturnperiod_value, loss);
		nextreturnperiod_index++;
		if (nextreturnperiod_index < returnperiods_.size()) {
			nextreturnperiod_value = returnperiods_[nextreturnperiod_index];
		}else{
			break;
		}
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

	fprintf(fout_[handle], "summary_id,sidx,return_period,loss\n");
	for (auto s : items) {
		lossvec &lpv = s.second;
		std::sort(lpv.rbegin(), lpv.rend());
		int nextreturnperiodindex = 0;
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		int i = 1;
		OASIS_FLOAT t = (OASIS_FLOAT)totalperiods_;
		for (auto lp : lpv) {
			OASIS_FLOAT retperiod = t / i;
			if (useReturnPeriodFile_) {
				if (nextreturnperiodindex == returnperiods_.size()) break;
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp, s.first.summary_id, s.first.sidx);
			}else {
				fprintf(fout_[handle],"%d,%d,%f,%f\n", s.first.summary_id, s.first.sidx, t / i, lp);
			}			
			i++;
		}
		if (useReturnPeriodFile_) {
			doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id, s.first.sidx);
			while (nextreturnperiodindex < returnperiods_.size()) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id, s.first.sidx);
			}
		}

	}
}
void aggreports::wheatsheafwithweighting(int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss)
{
	fprintf(stderr, "TODO:: wheatsheafwithweighting\n");
}

void aggreports::outputOccWheatsheaf()
{
	if (periodstoweighting_.size() == 0) {
		wheatsheaf(OCC_WHEATSHEAF, max_out_loss_);
	}
	else {
		wheatsheafwithweighting(OCC_FULL_UNCERTAINTY, max_out_loss_);
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

void aggreports::wheatSheafMeanwithweighting(int samplesize, int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss)
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
			lv.period_no = x.first.period_no;		// for debugging
			items[wk].push_back(lv);
		}		
	}

	int maxcount = 0;
	for (auto x : items) {
		if (x.second.size() > maxcount) maxcount = x.second.size();
	}

	std::map<int, std::vector<lossval>> mean_map;
	for (int i = 1; i <= maxsummaryid_; i++) {		
		mean_map[i] = std::vector<lossval>(maxcount, lossval());
	}
	fprintf(fout_[handle], "summary_id,type,return_period,loss\n");

	for (auto s : items) {
		int nextreturnperiodindex = 0;
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		lossvec2 &lpv = s.second;
		std::sort(lpv.rbegin(), lpv.rend());
		OASIS_FLOAT cummulative_weigthing = 0;
		if (s.first.sidx != -1) {
			int i = 0;
			for (auto lp : lpv) {
				lossval &l = mean_map[s.first.summary_id][i];
				l.period_no = lp.period_no;
				l.period_weighting = lp.period_weighting;
				l.value += lp.value;
				i++;
			}
		}
		else {			
			int i = 1;
			OASIS_FLOAT t = (OASIS_FLOAT)totalperiods_;
			for (auto lp : lpv) {
				cummulative_weigthing += lp.period_weighting;
				if (cummulative_weigthing) {
					OASIS_FLOAT retperiod = 1 / cummulative_weigthing;
					if (useReturnPeriodFile_) {
						doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp.value, s.first.summary_id, 1);
					}
					else {
						fprintf(fout_[handle], "%d,1,%f,%f\n", s.first.summary_id, retperiod, lp.value);
					}
				}

				i++;
			}
			if (useReturnPeriodFile_) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id, 1);
				while (nextreturnperiodindex < returnperiods_.size()) {
					doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id, 1);
				}
			}
		}
	}
	
	if (samplesize == 0) return; // avoid divide by zero error

	for (auto m : mean_map) {
		std::vector<lossval> &lpv = m.second;
		std::vector<lossval>::reverse_iterator rit = lpv.rbegin();
		int maxindex = lpv.size();
		while (rit != lpv.rend()) {
			if (rit->value != 0.0) break;
			maxindex--;
			rit++;
		}
		int nextreturnperiodindex = 0;
		int nextreturnperiodvalue = 0;
		if (useReturnPeriodFile_) nextreturnperiodvalue = returnperiods_[nextreturnperiodindex];
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		int i = 1;
		OASIS_FLOAT t = (OASIS_FLOAT)totalperiods_;
		OASIS_FLOAT cummulative_weigthing = 0;
		for (auto lp : lpv) {
			cummulative_weigthing += lp.period_weighting;
			if (cummulative_weigthing) {
				OASIS_FLOAT retperiod = 1 / cummulative_weigthing;
				if (useReturnPeriodFile_) {
					OASIS_FLOAT loss = lp.value / samplesize;
					doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, loss, m.first, 2);
				}
				else {
					fprintf(fout_[handle], "%d,2,%f,%f\n", m.first, retperiod, lp.value / samplesize);
				}
			}
			i++;
			if (i > maxindex) break;
		}
		if (useReturnPeriodFile_) {
			doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, m.first, 2);
			while (nextreturnperiodindex < returnperiods_.size()) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, m.first, 2);
			}
		}
	}
}

void aggreports::wheatSheafMean(int samplesize, int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss)
{
	if (fout_[handle] == nullptr) return;
	std::map<wheatkey, lossvec> items;
	for (auto x : out_loss) {
		wheatkey wk;;
		wk.sidx = x.first.sidx;
		wk.summary_id = x.first.summary_id;
		items[wk].push_back(x.second);
	}

	int maxcount = 0;
	for (auto x : items) {
		if (x.second.size() > maxcount) maxcount = x.second.size();
	}

	std::map<int, std::vector<OASIS_FLOAT>> mean_map;
	for (int i = 1; i <= maxsummaryid_; i++) {
		mean_map[i] = std::vector<OASIS_FLOAT>(maxcount, 0);
	}
	fprintf(fout_[handle],"summary_id,type,return_period,loss\n");
	for (auto s : items) {
		int nextreturnperiodindex = 0;
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
					doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp, s.first.summary_id,1);
				}else {
					fprintf(fout_[handle],"%d,1,%f,%f\n", s.first.summary_id, retperiod, lp);
				}
				
				i++;
			}
			if (useReturnPeriodFile_) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id,1);
				while (nextreturnperiodindex < returnperiods_.size()) {					
					doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, s.first.summary_id,1);
				}
			}
		}
	}

	if (samplesize == 0) return; // avoid divide by zero error

	for (auto m : mean_map) {
		std::vector<OASIS_FLOAT> &lpv = m.second;
		std::vector<OASIS_FLOAT>::reverse_iterator rit = lpv.rbegin();
		int maxindex = lpv.size();
		while (rit != lpv.rend()) {
			if (*rit != 0.0) break;
			maxindex--;
			rit++;
		}
		int nextreturnperiodindex = 0;
		int nextreturnperiodvalue = 0;
		if (useReturnPeriodFile_) nextreturnperiodvalue = returnperiods_[nextreturnperiodindex];
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		int i = 1;
		OASIS_FLOAT t = (OASIS_FLOAT)totalperiods_;
		for (auto lp : lpv) {
			OASIS_FLOAT retperiod = t / i;
			if (useReturnPeriodFile_) {
				OASIS_FLOAT loss = lp / samplesize;
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, loss, m.first,2);
			}else {
				fprintf(fout_[handle],"%d,2,%f,%f\n", m.first, retperiod, lp / samplesize);
			}
			i++;
			if (i > maxindex) break;
		}
		if (useReturnPeriodFile_) {
			doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, m.first, 2);
			while (nextreturnperiodindex < returnperiods_.size()) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, m.first, 2);
			}
		}
	}
}

//void aggreports::wheatSheafMeanwithweighting(int samplesize, int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss)
//{
//	fprintf(stderr, "TODO!!! wheatsheafwithweighting \n");
//	wheatSheafMean(samplesize, handle, out_loss);
//}

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


void aggreports::sampleMeanwithweighting(int samplesize, int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss)
{
	if (fout_[handle] == nullptr) return;
	std::map<summary_id_period_key,lossval> items;

	for (auto x : out_loss) {
		summary_id_period_key sk;
		lossval lv;
		lv.value = x.second;
		auto iter = periodstoweighting_.find(x.first.period_no);	// assume weighting is zero if not supplied
		sk.period_no = x.first.period_no;
		sk.summary_id = x.first.summary_id;
		if (iter != periodstoweighting_.end()) {
			if (x.first.sidx == -1) {
				sk.type = 1;
				items[sk].period_weighting = iter->second;
				items[sk].period_no = x.first.period_no;		// for debugging
				items[sk].value += x.second;
			}
			else {
				if (samplesize > 0) {
					sk.type = 2;
					items[sk].period_no = x.first.period_no;		// for debugging
					items[sk].period_weighting = iter->second;
					items[sk].value += (x.second / samplesize);
				}
			}
		}
	}

	std::map<summary_id_type_key, std::vector<lossval>> mean_map;

	fprintf(fout_[handle], "summary_id,type,return_period,loss\n");
	for (auto s : items) {
		summary_id_type_key st;
		st.summary_id = s.first.summary_id;
		st.type = s.first.type;
		mean_map[st].push_back(s.second);
	}

	for (auto m : mean_map) {
		std::vector<lossval> &lpv = m.second;
		std::sort(lpv.rbegin(), lpv.rend());
		int nextreturnperiodindex = 0;
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		OASIS_FLOAT cummulative_weigthing = 0;
		for (auto lp : lpv) {
			cummulative_weigthing += (lp.period_weighting * samplesize_);
			if (cummulative_weigthing) {
				OASIS_FLOAT retperiod = 1 / cummulative_weigthing;
				if (useReturnPeriodFile_) {
					doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp.value, m.first.summary_id, m.first.type);
				}
				else {
					fprintf(fout_[handle], "%d,%d,%f,%f\n", m.first.summary_id, m.first.type, retperiod, lp.value);
				}
			}
		}
		if (useReturnPeriodFile_) {
			doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, m.first.summary_id, m.first.type);
			while (nextreturnperiodindex < returnperiods_.size()) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, m.first.summary_id, m.first.type);
			}
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

	fprintf(fout_[handle],"summary_id,type,return_period,loss\n");
	for (auto s : items) {
		summary_id_type_key st;
		st.summary_id = s.first.summary_id;
		st.type = s.first.type;
		mean_map[st].push_back(s.second);
	}

	for (auto m : mean_map) {
		std::vector<OASIS_FLOAT> &lpv = m.second;
		std::sort(lpv.rbegin(), lpv.rend());
		int nextreturnperiodindex = 0;
		OASIS_FLOAT last_computed_rp = 0;
		OASIS_FLOAT last_computed_loss = 0;
		int i = 1;
		OASIS_FLOAT t = (OASIS_FLOAT) totalperiods_;
		for (auto lp : lpv) {
			OASIS_FLOAT retperiod = t / i;
			if (useReturnPeriodFile_) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, retperiod, lp, m.first.summary_id, m.first.type);
			}else {
				fprintf(fout_[handle],"%d,%d,%f,%f\n", m.first.summary_id, m.first.type, retperiod, lp);
			}
			i++;
		}
		if (useReturnPeriodFile_) {
			doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, m.first.summary_id, m.first.type);
			while (nextreturnperiodindex < returnperiods_.size()) {
				doreturnperiodout(handle, nextreturnperiodindex, last_computed_rp, last_computed_loss, 0, 0, m.first.summary_id, m.first.type);
			}
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
