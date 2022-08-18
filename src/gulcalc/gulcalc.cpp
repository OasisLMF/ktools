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
	Calculate the GUL
	Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/

#include "gulcalc.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <memory.h>
#include <chrono>
#include <thread>

#include <algorithm> 


#include "../include/oasis.h"



using namespace std;

bool operator<(const item_map_key& lhs, const item_map_key& rhs)
{
	if (lhs.areaperil_id != rhs.areaperil_id) {
		return lhs.areaperil_id < rhs.areaperil_id;
	}
	else {
		return lhs.vulnerability_id < rhs.vulnerability_id;
	}
}


bool operator<(const gulitemSampleslevel& lhs, const gulitemSampleslevel& rhs)
{
	if (lhs.item_id != rhs.item_id) {
		return lhs.item_id < rhs.item_id;
	}
	else {
		return lhs.sidx < rhs.sidx;
	}
}


bool operator<(const prob_mean & lhs, const prob_mean & rhs) {

	if (lhs.prob_to != rhs.prob_to) {
		return lhs.prob_to < rhs.prob_to;
	} else {
		return lhs.bin_mean < rhs.bin_mean;
	}

}

gulSampleslevelHeader lastitemheader;
gulSampleslevelHeader lastcorrelatedheader;
gulSampleslevelHeader lastcoverageheader;

// Benchmark memory usage
void gulcalc::outputbenchmark(const int event_id, bool header=false) {

	if (header) {

		fprintf(stderr, "event_id,cov,mode1,fullCorr,"
				"mode1UsedCoverageIDs,damagebindictionary_vec,"
				"item_map,coverages\n");
		return;
	}

	fprintf(stderr, "%d,", event_id);   // event_id

	// vector<vector<OASIS_FLOAT>> cov_
	size_t vec_size = 0;
	for (std::vector<std::vector<OASIS_FLOAT>>::const_iterator i = cov_.begin(); i < cov_.end(); i++) {
		vec_size += (*i).size();
	}
	fprintf(stderr, "%lu,",
		sizeof(std::vector<std::vector<OASIS_FLOAT>>) +
		  (sizeof(std::vector<OASIS_FLOAT>) * cov_.size()) +
		  (sizeof(OASIS_FLOAT) * vec_size));

	// vector<vector<vector<gulItemIDLoss>>> mode1_
	vec_size = 0;
	size_t vec2_size = 0;
	for (std::vector<std::vector<std::vector<gulItemIDLoss>>>::const_iterator i = mode1_.begin(); i < mode1_.end(); i++) {
		vec_size += (*i).size();
		for (std::vector<std::vector<gulItemIDLoss>>::const_iterator ii = (*i).begin(); ii < (*i).end(); ii++) {
			vec2_size += (*ii).size();
		}
	}
	fprintf(stderr, "%lu,",
		sizeof(std::vector<std::vector<std::vector<gulItemIDLoss>>>) +
		  (sizeof(std::vector<std::vector<gulItemIDLoss>>) * mode1_.size()) +
		  (sizeof(std::vector<gulItemIDLoss>) * vec_size) +
		  (sizeof(gulItemIDLoss) * vec2_size));

	// vector<vector<vector<gulItemIDLoss>>> fullCorr_
	vec_size = 0;
	vec2_size = 0;
	for (std::vector<std::vector<std::vector<gulItemIDLoss>>>::const_iterator i = fullCorr_.begin(); i < fullCorr_.end(); i++) {
		vec_size += (*i).size();
		for (std::vector<std::vector<gulItemIDLoss>>::const_iterator ii = (*i).begin(); ii < (*i).end(); ii++) {
			vec2_size += (*ii).size();
		}
	}
	fprintf(stderr, "%lu,",
		sizeof(std::vector<std::vector<std::vector<gulItemIDLoss>>>) +
		  (sizeof(std::vector<std::vector<gulItemIDLoss>>) * fullCorr_.size()) +
		  (sizeof(std::vector<gulItemIDLoss>) * vec_size) +
		  (sizeof(gulItemIDLoss) * vec2_size));

	// vector<int> mode1UsedCoverageIDs_
	fprintf(stderr, "%lu,",
		sizeof(std::vector<int>) +
		  (sizeof(int) * mode1UsedCoverageIDs_.size()));

	// vector<damagebindictionary> *damagebindictionary_vec_
	fprintf(stderr, "%lu,",
		sizeof(std::vector<damagebindictionary>) +
		  (sizeof(damagebindictionary) * (*damagebindictionary_vec_).size()));

	// map<item_map_key, vector<item_map_rec>> *iutem_map_
	vec_size = 0;
	for (std::map<item_map_key, std::vector<item_map_rec>>::const_iterator i = (*item_map_).begin(); i != (*item_map_).end(); i++) {
		vec_size += i->second.size();
	}
	fprintf(stderr, "%lu,",
		sizeof(std::map<item_map_key, vector<item_map_rec>>) +
		  (sizeof(item_map_key) * (*item_map_).size()) +
		  (sizeof(item_map_rec) * vec_size));

	// vector<OASIS_FLOAT> *coverages_
	fprintf(stderr, "%lu\n",
		sizeof(std::vector<OASIS_FLOAT>) +
		  (sizeof(OASIS_FLOAT) * (*coverages_).size()));

	return;

}


OASIS_FLOAT gulcalc::getgul(damagebindictionary &b, gulGulSamples &g)
{
	OASIS_FLOAT gul = 0;
	if (b.bin_from == b.bin_to) {
		gul = b.bin_to * g.tiv;
		return gul;
	}

	double x = (g.bin_mean - b.bin_from) / (b.bin_to - b.bin_from);
	int z = (int) (round(x * 100000));
	if (z == 50000) {
		gul = (b.bin_from + ((g.rval - g.prob_from) / (g.prob_to - g.prob_from) * (b.bin_to - b.bin_from))) * g.tiv;
		return gul;
	}

	double bin_width = b.bin_to - b.bin_from;
	double bin_height = g.prob_to - g.prob_from;

	double aa = (3 * bin_height / (bin_width * bin_width)) * ((2 * x) - 1);

	double bb = (2 * bin_height / bin_width) * (2 - (3* x)) ;

	double cc = g.prob_from - g.rval;
	gul = (b.bin_from + (sqrt(bb*bb - (4 * aa*cc)) - bb) / (2 * aa)) * g.tiv;

	return gul;
}
void split_tiv_classic(std::vector<gulItemIDLoss> &gulitems, OASIS_FLOAT tiv)
{
// if the total loss exceeds the tiv 
// then split tiv in the same proportions to the losses
	if (tiv > 0) {
		OASIS_FLOAT total_loss = 0;
		auto iter = gulitems.begin();
		while (iter != gulitems.end()) {
			total_loss += iter->loss;
			iter++;
		}
		if (total_loss > tiv) {
			iter = gulitems.begin();
			while (iter != gulitems.end()) {
				double percentage = iter->loss / total_loss;
				iter->loss = percentage * tiv;
				iter++;
			}
		}
	}
}


void split_tiv_multiplicative(std::vector<gulItemIDLoss> &gulitems, OASIS_FLOAT tiv){
//Split the total insured value (TIV) using a multiplicative formula for the
//total loss as tiv * (1 - (1-A)*(1-B)*(1-C)...), where A, B, C are damage ratios
//computed as the ratio between a sub-peril loss and the tiv. Sub-peril losses
//in gulitems are always back-allocated proportionally to the losses.
    OASIS_FLOAT sum_loss = 0;
    double undamaged_value = 1;
    double multiplicative_loss = 0;

    auto iter = gulitems.begin();
	while (iter != gulitems.end()) {
	    undamaged_value *= 1 - iter->loss / tiv;
        sum_loss += iter->loss;
        iter++;
	}
    multiplicative_loss = tiv * (1. - undamaged_value);

    if (sum_loss > 0) {
        double percentage = multiplicative_loss / sum_loss;
        iter = gulitems.begin();
        while (iter != gulitems.end()) {
            iter->loss *= percentage;
            iter++;
        }
    }
}

void gulcalc::writemode0output(const item_map_rec &er, const OASIS_FLOAT tiv,
			       const int event_id, const int bin_index,
			       const OASIS_FLOAT rval, const probrec &p,
			       const int sample_id,
			       const bool correlated=false) {

	gulGulSamples g;
	g.event_id = event_id;
	g.item_id = er.item_id;
	g.tiv = tiv;
	g.bin_index = bin_index;
	g.prob_from = p.prob_from;
	g.prob_to = p.prob_to;
	g.bin_mean = p.bin_mean;
	g.rval = rval;
	g.sidx = sample_id + 1;

	gulitemSampleslevel gg;
	damagebindictionary b = (*damagebindictionary_vec_)[g.bin_index];
	if (debug_) gg.loss = rval;
	else gg.loss = getgul(b, g);

	gg.sidx = g.sidx;
	gg.event_id = g.event_id;
	gg.item_id = g.item_id;
	if (gg.loss >= loss_threshold_) {

		if (correlated) correlatedoutputgul(gg);
		else itemoutputgul(gg);

		if (!correlatedWriter_) {
			gulcoverageSampleslevel ggc;
			ggc.loss = gg.loss;
			ggc.sidx = g.sidx;
			ggc.coverage_id = er.coverage_id;
			ggc.event_id = g.event_id;
			gencovoutput(ggc);
		}

	}

}

// As this is sparsely populated, if we keep a list of the populated fields in
// another vector then we do not need to iterate all the rows to clear it
void gulcalc::clearmode1_data() {

	auto iter = mode1UsedCoverageIDs_.begin();
	while (iter != mode1UsedCoverageIDs_.end()) {
		mode1_stats_[*iter].clear();
		iter++;
	}
	mode1UsedCoverageIDs_.clear();
}


// Set losses for alloc rule where total peril loss = maximum subperil loss
void gulcalc::setmaxloss(std::vector<std::vector<gulItemIDLoss>> &gilv) {

	for (size_t i = 3; i < gilv.size(); i++) {

		OASIS_FLOAT max_loss = 0.0;
		int max_loss_count = 0;

		// First loop: find maximum loss and count occurrences
		auto iter = gilv[i].begin();
		while (iter != gilv[i].end()) {
			if (iter->loss > max_loss) {
				max_loss = iter->loss;
				max_loss_count = 0;
			}
			if (iter->loss == max_loss) max_loss_count++;
			iter++;
		}

		// Second loop: distribute maximum losses evenly among highest
		// contributing subperils and set other losses to 0
		iter = gilv[i].begin();
		while (iter != gilv[i].end()) {
			if (iter->loss == max_loss) iter->loss /= max_loss_count;
			else iter->loss = 0;
			iter++;
		}

	}

}


void gulcalc::writemode1output(const int event_id, const OASIS_FLOAT tiv,
			       std::vector<std::vector<gulItemIDLoss>> &gilv,
			       const bool correlated=false) {

	// Set losses for alloc rule where
	// total peril loss = maximum subperil loss
	if (alloc_rule_ == 2) setmaxloss(gilv);

	std::map<int, std::vector<gulSampleslevelRec>> gxi;

    // select tiv split method depending on alloc_rule_
    void (*split_tiv)(std::vector<gulItemIDLoss> &, OASIS_FLOAT);
    if (alloc_rule_==3) split_tiv = &split_tiv_multiplicative;
    else split_tiv = &split_tiv_classic;
	// Check whether the sum of losses per sample exceed TIV
	// If so, split TIV in proportion to losses
	for (size_t i = 0; i < gilv.size(); i++) {

		split_tiv(gilv[i], tiv);
		auto iter = gilv[i].begin();
		while (iter != gilv[i].end()) {

			gulSampleslevelRec gg;
			gg.sidx = i - num_idx_;
			gg.loss = iter->loss;

			gxi[iter->item_id].push_back(gg);
			iter++;

		}

	}

	// Output all items associated with current coverage ID
	// Output the rows here
	auto iter = gxi.begin();
	while (iter != gxi.end()) {
		gulitemSampleslevel g;
		g.event_id = event_id;
		g.item_id = iter->first;
		auto iter2 = iter->second.begin();
		while (iter2 != iter->second.end()) {
			g.sidx = iter2->sidx;
			g.loss = iter2->loss;
			if (correlated) correlatedoutputgul(g);
			else itemoutputgul(g);
			iter2++;
		}
		iter++;
	}

}

void gulcalc::outputmode1data(int event_id) {

	int rnd_count = rnd_->rdxmax();

	auto cov_iter = mode1UsedCoverageIDs_.begin();
	while (cov_iter != mode1UsedCoverageIDs_.end()) {

		std::vector<std::vector<gulItemIDLoss>> gilv;
		std::vector<std::vector<gulItemIDLoss>> gilv0;
		gilv.resize(samplesize_ + num_idx_ + 1);
		if (correlatedWriter_) gilv0.resize(samplesize_ + num_idx_ + 1);
		int j = *cov_iter;
		OASIS_FLOAT tiv = 0.0;
		OASIS_FLOAT exposureValue = 0.0;
		bool hasData = false;
		if (mode1_stats_[j].size() > 0) {
			tiv = (*coverages_)[j];
			exposureValue = tiv / mode1_stats_[j].size();
			hasData = true;
		}

		for (size_t i = 0; i < mode1_stats_[j].size(); i++) {

			gulItemIDLoss gi;
			gi.item_id = mode1_stats_[j][i].item_id;

			// maximum loss
			gi.loss = mode1_stats_[j][i].max_loss;
			gilv[max_loss_idx + num_idx_].push_back(gi);
			if (correlatedWriter_)
				gilv0[max_loss_idx + num_idx_].push_back(gi);

			// exposure value record
			// tiv / count(tuple(event_id-coverage_id))
			gi.loss = exposureValue;
			gilv[tiv_idx + num_idx_].push_back(gi);
			if (correlatedWriter_)
				gilv0[tiv_idx + num_idx_].push_back(gi);

			// standard deviation
			gi.loss = mode1_stats_[j][i].std_dev;
			gilv[std_dev_idx + num_idx_].push_back(gi);
			if (correlatedWriter_)
				gilv0[std_dev_idx + num_idx_].push_back(gi);

			// mean
			gi.loss = mode1_stats_[j][i].gul_mean;
			gilv[mean_idx + num_idx_].push_back(gi);
			if (correlatedWriter_)
				gilv0[mean_idx + num_idx_].push_back(gi);

			int ridx = 0;
			int ridx0 = 0;
			switch (rndopt_) {
				case rd_option::userandomnumberfile:
					ridx = ((mode1_stats_[j][i].group_id * p1_ * p3_) + (event_id * p2_)) % rnd_count;
					ridx0 = ((p1_ * p3_) + (event_id * p2_)) % rnd_count;
					break;
				case rd_option::usecachedvector:
					ridx = mode1_stats_[j][i].group_id * samplesize_;
					ridx0 = samplesize_;
					break;
				case rd_option::usehashedseed:
				{
					unsigned long long s1 = (mode1_stats_[j][i].group_id * 1543270363L) % 2147483648L;   // hash group_id and event_id to seed random number
					unsigned long long s2 = (event_id * 1943272559L) % 2147483648L;
					s1 = (s1 + s2 + rand_seed_) % 2147483648L;
					rnd_->seedRands(s1);
					if (correlatedWriter_) {
						s2 = (s2 + rand_seed_) % 2147483648L;
						rnd0_->seedRands(s2);
					}
				}
				break;
				default:
					fprintf(stderr, "FATAL: %s: Unknown random number option\n", __func__);
					exit(-1);
			}

			// Generate losses for each sample
			for (int k = 0; k < samplesize_; k++) {

				OASIS_FLOAT rval;
				OASIS_FLOAT rval0 = 0;

				if (rndopt_ == rd_option::usehashedseed) {
					rval = rnd_->nextrnd();
					if (correlatedWriter_) rval0 = rnd0_->nextrnd();
				} else {
					rval = rnd_->rnd(ridx + k);
					if (correlatedWriter_) rval0 = rnd0_->rnd(ridx0 + k);
				}

				// If required set random number to just under
				// maximum prob_to (which should be 1)
				OASIS_FLOAT prob_to_max = bin_lookup_[mode1_stats_[j][i].bin_map_ids.back()].prob_to;
				if (rval >= prob_to_max) {
					rval = prob_to_max - 0.00000003;
				}
				if (correlatedWriter_ && rval0 >= prob_to_max) {
					rval0 = prob_to_max - 0.00000003;
				}

				OASIS_FLOAT last_prob_to = 0;
				bool hit_rval = false;
				bool hit_rval0 = false;
				int max_bin_end = (int)mode1_stats_[j][i].bin_map_ids.size();
				for (int bin_index = 0; bin_index < max_bin_end; bin_index++) {

					prob_mean pp = bin_lookup_[mode1_stats_[j][i].bin_map_ids[bin_index]];
					probrec p;
					p.prob_from = last_prob_to;
					p.prob_to = pp.prob_to;
					p.bin_mean = pp.bin_mean;
					last_prob_to = pp.prob_to;

					if (rval < p.prob_to && !hit_rval) {
						fillgulitemloss(gi.item_id, tiv, event_id, bin_index, rval, p, k+1, gilv);
						if (!correlatedWriter_) break;
						else hit_rval = true;
					}
					if (correlatedWriter_ && rval0 < p.prob_to && !hit_rval0) {
						fillgulitemloss(gi.item_id, tiv, event_id, bin_index, rval0, p, k+1, gilv0);
						hit_rval0 = true;
					}
					if (hit_rval && hit_rval0) break;

				}

			}

		}

		if (hasData) {

			writemode1output(event_id, tiv, gilv);
			if (correlatedWriter_) {
				writemode1output(event_id, tiv, gilv0, true);
			}

		}

		cov_iter++;

	}

	clearmode1_data();

}

void gulcalc::outputcoveragedata(int event_id)
{
	if (opt_.coverageLevelOutput == false) return;
	for (size_t j = 1; j < cov_.size(); j++) {
		if (cov_[j].size() > 0) {
			gulcoverageSampleslevel gc;
			gc.event_id = event_id;
			gc.coverage_id = j;
			for (size_t i = 1; i < cov_[j].size(); i++) {
				gc.sidx = i - 2;
				gc.loss = cov_[j][i];
				OASIS_FLOAT tiv = (*coverages_)[gc.coverage_id];
				if (gc.loss > tiv) gc.loss = tiv;
				if (gc.sidx) {
					if (gc.sidx == -1) {
						covoutputgul(gc);		// always output the mean	
					}
					else {
						if (gc.loss >= loss_threshold_) {
							covoutputgul(gc);
						}
					}
				}
			}

		}
	}
	cov_.clear();
	cov_.resize(coverages_->size());
}

void gulcalc::gencovoutput(gulcoverageSampleslevel &gg)
{
	if (coverageWriter_ == 0)  return;
	if (cov_[gg.coverage_id].size() == 0) {
		cov_[gg.coverage_id].resize(samplesize_ + 3, 0);
	}
	cov_[gg.coverage_id][gg.sidx + 2] += gg.loss;
}

void gulcalc::covoutputgul(gulcoverageSampleslevel &gg)
{
	if (coverageWriter_ == 0)  return;
	if (covbufoffset_ >= bufsize) {
		coverageWriter_(cbuf_,sizeof(unsigned char), covbufoffset_);
		covbufoffset_ = 0;
	}

	if (gg.event_id != lastcoverageheader.event_id || gg.coverage_id != lastcoverageheader.item_id) {
		if (isFirstCovEvent_ == false) {
			gulSampleslevelRec r;
			r.sidx = 0;
			r.loss = 0;
			memcpy(cbuf_ + covbufoffset_, &r, sizeof(gulSampleslevelRec));
			covbufoffset_ += sizeof(gulSampleslevelRec);	// null terminate list
		}
		else {
			isFirstCovEvent_ = false;
		}
		lastcoverageheader.event_id = gg.event_id;
		lastcoverageheader.item_id = gg.coverage_id;
		memcpy(cbuf_ + covbufoffset_, &lastcoverageheader, sizeof(lastcoverageheader));
		covbufoffset_ += sizeof(lastcoverageheader);
	}

	gulSampleslevelRec r;
	r.sidx = gg.sidx;
	r.loss = gg.loss;
	memcpy(cbuf_ + covbufoffset_, &r, sizeof(gulSampleslevelRec));
	covbufoffset_ += sizeof(gulSampleslevelRec);

}

void gulcalc::itemoutputgul(gulitemSampleslevel &gg)
{
	if (itemWriter_ == 0 && lossWriter_ == 0)  return;
	if (itembufoffset_ >= bufsize) {
		if (itemWriter_) itemWriter_(ibuf_, sizeof(unsigned char), itembufoffset_);
		if (lossWriter_) lossWriter_(ibuf_, sizeof(unsigned char), itembufoffset_);
		itembufoffset_ = 0;
	}

	if (gg.event_id != lastitemheader.event_id || gg.item_id != lastitemheader.item_id) {
		if (isFirstItemEvent_ == false){
			gulSampleslevelRec r;
			r.sidx = 0;
			r.loss = 0;
			memcpy(ibuf_ + itembufoffset_, &r, sizeof(gulSampleslevelRec));
			itembufoffset_ += sizeof(gulSampleslevelRec);	// null terminate list
		}
		else {
			isFirstItemEvent_ = false;
		}
		lastitemheader.event_id = gg.event_id;
		lastitemheader.item_id = gg.item_id;
		memcpy(ibuf_ + itembufoffset_, &lastitemheader, sizeof(lastitemheader));
		itembufoffset_ += sizeof(lastitemheader);
	}

	gulSampleslevelRec r;
	r.sidx = gg.sidx;
	r.loss = gg.loss;    
	memcpy(ibuf_ + itembufoffset_, &r, sizeof(gulSampleslevelRec));
	itembufoffset_ += sizeof(gulSampleslevelRec);

}

void gulcalc::correlatedoutputgul(gulitemSampleslevel &gg)
{
	if (correlatedWriter_ == 0) return;
	if (correlatedbufoffset_ >= bufsize) {
		correlatedWriter_(corrbuf_, sizeof(unsigned char), correlatedbufoffset_);
		correlatedbufoffset_ = 0;
	}

	if(gg.event_id != lastcorrelatedheader.event_id || gg.item_id != lastcorrelatedheader.item_id) {
		if (isFirstCorrelatedEvent_ == false) {
			gulSampleslevelRec r;
			r.sidx = 0;
			r.loss = 0;
			memcpy(corrbuf_ + correlatedbufoffset_, &r, sizeof(gulSampleslevelRec));
			correlatedbufoffset_ += sizeof(gulSampleslevelRec);
		}
		else {
			isFirstCorrelatedEvent_ = false;
		}
		lastcorrelatedheader.event_id = gg.event_id;
		lastcorrelatedheader.item_id = gg.item_id;
		memcpy(corrbuf_ + correlatedbufoffset_, &lastcorrelatedheader, sizeof(lastcorrelatedheader));
		correlatedbufoffset_ += sizeof(lastcorrelatedheader);
	}

	gulSampleslevelRec r;
	r.sidx = gg.sidx;
	r.loss = gg.loss;
	memcpy(corrbuf_ + correlatedbufoffset_, &r, sizeof(gulSampleslevelRec));
	correlatedbufoffset_ += sizeof(gulSampleslevelRec);

}

void gulcalc::fillgulitemloss(const int item_id, const OASIS_FLOAT tiv,
			      const int event_id, const int bin_index,
			      const OASIS_FLOAT rval, const probrec &p,
			      const int sample_id,
			      std::vector<std::vector<gulItemIDLoss>> &gilv) {

	gulGulSamples g;
	g.event_id = event_id;
	g.item_id = item_id;
	g.tiv = tiv;
	g.bin_index = bin_index;
	g.prob_from = p.prob_from;
	g.prob_to = p.prob_to;
	g.bin_mean = p.bin_mean;
	g.rval = rval;
	g.sidx = sample_id + num_idx_;

	damagebindictionary b = (*damagebindictionary_vec_)[g.bin_index];
	gulItemIDLoss gg;
	gg.item_id = g.item_id;
	if (debug_) gg.loss = rval;
	else gg.loss = getgul(b, g);

	// Only record losses greater than loss threshold
	if (gg.loss >= loss_threshold_) gilv[g.sidx].push_back(gg);

}

void gulcalc::output_mean_mode1(const OASIS_FLOAT tiv, prob_mean *pp,
				const int bin_count, OASIS_FLOAT &gul_mean,
				OASIS_FLOAT &std_dev, OASIS_FLOAT &max_loss,
				std::vector<int> &bin_ids) {

	OASIS_FLOAT last_prob_to = 0;
	gul_mean = 0;
	std_dev = 0;
	OASIS_FLOAT ctr_var = 0;

	for (int bin_index = 0; bin_index < bin_count; bin_index++) {

		// Map prob_mean to integer for later lookup
		auto pos = bin_map_.find(*pp);
		if (pos == bin_map_.end()) {
			bin_map_[*pp] = bin_map_.size();
			bin_lookup_.push_back(*pp);
		}
		bin_ids.push_back(bin_map_[*pp]);

		probrec p;
		if (bin_index == 0) {
			p.prob_from = 0;
		} else {
			p.prob_from = last_prob_to;
		}
		p.prob_to = pp->prob_to;
		p.bin_mean = pp->bin_mean;
		last_prob_to = pp->prob_to;
		gul_mean = gul_mean + ((p.prob_to - p.prob_from) * p.bin_mean * tiv);
		ctr_var = ctr_var + ((p.prob_to - p.prob_from) * p.bin_mean * p.bin_mean * tiv * tiv);
		pp++;
	}

	OASIS_FLOAT g2 = gul_mean * gul_mean;
	std_dev = ctr_var - g2;
	if (std_dev < 0) std_dev = 0;
	std_dev = sqrt(std_dev);

	max_loss = tiv * (*damagebindictionary_vec_)[bin_count - 1].bin_to;
}

void gulcalc::output_mean(OASIS_FLOAT tiv, prob_mean *pp, int bin_count,
			  OASIS_FLOAT &gul_mean,  OASIS_FLOAT &std_dev,
			  OASIS_FLOAT &max_loss)
{
	OASIS_FLOAT last_prob_to = 0;
	gul_mean = 0;
	std_dev = 0;
	OASIS_FLOAT ctr_var = 0;

	for (int bin_index = 0; bin_index < bin_count; bin_index++)
	{
		probrec p;
		if (bin_index == 0) {
			p.prob_from = 0;
		} else {
			p.prob_from = last_prob_to;
		}
		p.prob_to = pp->prob_to;
		p.bin_mean = pp->bin_mean;
		last_prob_to = pp->prob_to;
		gul_mean = gul_mean + ((p.prob_to - p.prob_from) *p.bin_mean * tiv);
		ctr_var = ctr_var + ((p.prob_to - p.prob_from) *p.bin_mean*p.bin_mean * tiv * tiv);
		pp++;
	}
	OASIS_FLOAT g2 = gul_mean * gul_mean;
	std_dev = ctr_var - g2;
	if (std_dev < 0) std_dev  = 0;
	std_dev = sqrt(std_dev);

	max_loss = tiv * (*damagebindictionary_vec_)[bin_count - 1].bin_to;
}

void gulcalc::processrec_mode1(char* rec, int recsize) {

	damagecdfrec * d = (damagecdfrec*)rec;

	item_map_key k;
	k.areaperil_id = d->areaperil_id;
	k.vulnerability_id = d->vulnerability_id;

	auto pos = item_map_->find(k);
	if (pos != item_map_->end()) {

		auto iter = pos->second.begin();
		while(iter != pos->second.end()) {

			processrecData recData;
			recData.item_id = iter->item_id;
			recData.group_id = iter->group_id;
			char * b = rec + sizeof(damagecdfrec);
			int * bin_count = (int*)b;
			b = b + sizeof(int);
			prob_mean* pp = (prob_mean*)b;
			OASIS_FLOAT gul_mean;
			OASIS_FLOAT std_dev;
			OASIS_FLOAT max_loss;
			OASIS_FLOAT tiv = (*coverages_)[iter->coverage_id];
			vector<int> bin_ids;
			output_mean_mode1(tiv, pp, *bin_count, gul_mean,
					  std_dev, max_loss, bin_ids);
			recData.gul_mean = gul_mean;
			recData.std_dev = std_dev;
			recData.max_loss = max_loss;
			recData.bin_map_ids = bin_ids;

			if (mode1_stats_[iter->coverage_id].size() == 0) {
				mode1UsedCoverageIDs_.push_back(iter->coverage_id);
			}
			mode1_stats_[iter->coverage_id].push_back(recData);

			iter++;

		}

	}

}

void gulcalc::processrec(char *rec, int recsize)
{
damagecdfrec *d = (damagecdfrec *)rec;
	char *endofRec = rec + recsize;

	int rnd_count = rnd_->rdxmax();

	item_map_key k;
	k.areaperil_id = d->areaperil_id;
	k.vulnerability_id = d->vulnerability_id;

	auto pos = item_map_->find(k);
	if (pos != item_map_->end()){
		auto iter = pos->second.begin();
		while (iter != pos->second.end()){
			gulitemSampleslevel gx;
			gulcoverageSampleslevel gc;
			gx.event_id = d->event_id;
			gx.item_id = iter->item_id;
			gc.event_id = gx.event_id;
			gc.coverage_id = iter->coverage_id;
			char *b = rec + sizeof(damagecdfrec);
			int *bin_count = (int *)b;
			b = b + sizeof(int);
			prob_mean *pp = (prob_mean *)b;
			OASIS_FLOAT max_loss;
			OASIS_FLOAT std_dev;
			OASIS_FLOAT gul_mean;
			OASIS_FLOAT tiv = (*coverages_)[iter->coverage_id];
			output_mean(tiv, pp, *bin_count, gul_mean, std_dev,
				    max_loss);
			gx.sidx = max_loss_idx;
			gx.loss = max_loss;
			itemoutputgul(gx);
			if (correlatedWriter_) correlatedoutputgul(gx);
			gx.sidx = tiv_idx;
			gx.loss = tiv;
			itemoutputgul(gx);
			if (correlatedWriter_) correlatedoutputgul(gx);
			gx.loss = std_dev;
			gc.loss = std_dev;
			gx.sidx = std_dev_idx;
			gc.sidx = std_dev_idx;
			itemoutputgul(gx);
			if (correlatedWriter_) correlatedoutputgul(gx);
			gencovoutput(gc);
			gx.loss = gul_mean;
			gc.loss = gul_mean;
			gx.sidx = mean_idx;
			gc.sidx = mean_idx;
			itemoutputgul(gx);
			if (correlatedWriter_) correlatedoutputgul(gx);
			gencovoutput(gc);

			int ridx = 0; // dummy value
			int ridx0 = 0;
			switch (rndopt_) {
			case rd_option::userandomnumberfile:
				ridx = ((iter->group_id * p1_*p3_) + (d->event_id * p2_)) % rnd_count;
				ridx0 = ((p1_ * p3_) + (d->event_id * p2_)) % rnd_count;
				break;
			case rd_option::usecachedvector:
				ridx = iter->group_id * samplesize_;
				ridx0 = samplesize_;
				break;
			case rd_option::usehashedseed:
				{
					unsigned long long s1 = (iter->group_id * 1543270363L) % 2147483648L;		// hash group_id and event_id to seed random number
					unsigned long long s2 = (d->event_id * 1943272559L) % 2147483648L;
					s1 = (s1 + s2 + rand_seed_) % 2147483648L;
					rnd_->seedRands(s1);
					if (correlatedWriter_) {
						s2 = (s2 + rand_seed_) % 2147483648L;
						rnd0_->seedRands(s2);
					}
				}
				break;
			default:
				fprintf(stderr, "FATAL: %s: Unknown random number option\n", __func__);
				exit(-1);
			}

			prob_mean *pp_max = pp + (*bin_count) -1;
			for (int i = 0; i < samplesize_; i++) {

				OASIS_FLOAT  rval;
				OASIS_FLOAT rval0 = 0;

				if (rndopt_ == rd_option::usehashedseed) {
					rval = rnd_->nextrnd();
					if (correlatedWriter_) rval0 = rnd0_->nextrnd();
				} else {
					rval = rnd_->rnd(ridx + i);
					if (correlatedWriter_) rval0 = rnd0_->rnd(ridx0 + i);
				}

				if (rval >= pp_max->prob_to) {
					rval = pp_max->prob_to - 0.00000003;	// set value to just under max value (which should be 1)
				}
				if (correlatedWriter_ && rval0 >= pp_max->prob_to) {
					rval0 = pp_max->prob_to - 0.00000003;
				}

                		OASIS_FLOAT last_prob_to = 0;
				bool hit_rval = false;
				bool hit_rval0 = false;
				pp = (prob_mean *)b;
				for (int bin_index = 0; bin_index < *bin_count; bin_index++){
					if ((char *)pp > endofRec) {
						cerr << "FATAL: Reached end of record"
							; // this is an error condition
						pp--;
					}

					probrec p;
					p.prob_from = last_prob_to;
					p.prob_to = pp->prob_to;
					p.bin_mean = pp->bin_mean;
					last_prob_to = pp->prob_to;

					if (rval < p.prob_to && !hit_rval) {
						writemode0output(*iter, tiv, d->event_id, bin_index, rval, p, i);
						if (!correlatedWriter_) break;
						else hit_rval = true;
					}
					if (correlatedWriter_ && rval0 < p.prob_to && !hit_rval0) {
						writemode0output(*iter, tiv, d->event_id, bin_index, rval0, p, i, true);
						hit_rval0 = true;
					}
					if (hit_rval && hit_rval0) break;
					pp++;
				}
			}
			iter++;
		}
	}

}
void gulcalc::init()
{
	int	gulstream_type = gul_item_stream | gulstream_id;

	if (itemWriter_ != 0) {
		itemWriter_(&gulstream_type, sizeof(gulstream_type), 1);
		itemWriter_(&samplesize_, sizeof(samplesize_), 1);
	}
	
	if (coverageWriter_ != 0) {
        gulstream_type = gul_coverage_stream | gulstream_id;
		coverageWriter_(&gulstream_type, sizeof(gulstream_type), 1);
		coverageWriter_(&samplesize_, sizeof(samplesize_), 1);
	}

	if (lossWriter_ != 0) {
            gulstream_type = gul_item_stream | loss_stream_id;
            lossWriter_(&gulstream_type, sizeof(gulstream_type), 1);
            lossWriter_(&samplesize_, sizeof(samplesize_), 1);
    }

	if(correlatedWriter_ != 0) {
	    gulstream_type = gul_item_stream | loss_stream_id;
	    correlatedWriter_(&gulstream_type, sizeof(gulstream_type), 1);
	    correlatedWriter_(&samplesize_, sizeof(samplesize_), 1);
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(PIPE_DELAY));
}
void gulcalc::mode1()
{
	// Change this 
	// collate the data as coverage_id,sidx,item_id,loss
	// then when the items are on the same coverage split the loss proportionally
	// process each event and output the data
	// 
	init();

	size_t total_bins = damagebindictionary_vec_->size();
	int max_recsize = (int)(total_bins * sizeof(prob_mean)) + sizeof(damagecdfrec) + sizeof(int);
	int last_event_id = -1;

	char* rec = new char[max_recsize];
	damagecdfrec* d = (damagecdfrec*)rec;

	if (benchmark_) outputbenchmark(0, true);

	for (;;)
	{
		char* p = rec;
		bool bSuccess = iGetrec_(p, sizeof(damagecdfrec));
		if (bSuccess == false) break;
		p = p + sizeof(damagecdfrec);
		bSuccess = iGetrec_(p, sizeof(int)); // we now have bin count
		int* q = (int*)p;
		p = p + sizeof(int);
		int recsize = (*q) * sizeof(prob_mean);
		// we should now have damagecdfrec in memory
		bSuccess = iGetrec_(p, recsize);
		recsize += sizeof(damagecdfrec) + sizeof(int);
		if (d->event_id != last_event_id) {
			if (last_event_id > 0) {
				if (benchmark_) outputbenchmark(last_event_id);
				outputmode1data(last_event_id);
			}
			last_event_id = d->event_id;
			if (rndopt_ == rd_option::usecachedvector) rnd_->clearvec();
		}

		processrec_mode1(rec, recsize);
	}
	if (benchmark_) outputbenchmark(d->event_id);
	outputmode1data(d->event_id);

	if (itemWriter_) itemWriter_(ibuf_, sizeof(unsigned char), itembufoffset_);
	if (lossWriter_) lossWriter_(ibuf_, sizeof(unsigned char), itembufoffset_);
	if (correlatedWriter_) correlatedWriter_(corrbuf_, sizeof(unsigned char), correlatedbufoffset_);
	delete[] rec;
}
void gulcalc::mode0()
{
	init();

	size_t total_bins = damagebindictionary_vec_->size();
	const int max_recsize = (int)(total_bins * sizeof(prob_mean)) + sizeof(damagecdfrec) + sizeof(int);
	int last_event_id = -1;

	char *rec = new char[max_recsize];
	// char rec[max_recsize]; -- not portable will not compile with Microsoft c++ 
	
	damagecdfrec *d = (damagecdfrec *)rec;

	if (benchmark_) outputbenchmark(0, true);

	for (;;)
	{
		char *p = rec;
		bool bSuccess = iGetrec_(p, sizeof(damagecdfrec));
		if (bSuccess == false) break;
		p = p + sizeof(damagecdfrec);
		bSuccess = iGetrec_(p, sizeof(int)); // we now have bin count
		int *q = (int *)p;
		p = p + sizeof(int);
		int recsize = (*q) * sizeof(prob_mean);
		// we should now have damagecdfrec in memory
		bSuccess = iGetrec_(p, recsize);
		recsize += sizeof(damagecdfrec) + sizeof(int);
		if (d->event_id != last_event_id) {
			if (last_event_id > 0) {
				if (benchmark_) outputbenchmark(last_event_id);
				outputcoveragedata(last_event_id);
			}
			last_event_id = d->event_id;						
			if (rndopt_ == rd_option::usecachedvector) rnd_->clearvec();
		}

		processrec(rec, recsize);
	}
	if (benchmark_) outputbenchmark(d->event_id);
	outputcoveragedata(d->event_id);
	if (itemWriter_)  itemWriter_(ibuf_, sizeof(unsigned char), itembufoffset_);
	if (lossWriter_)  lossWriter_(ibuf_, sizeof(unsigned char), itembufoffset_);
	if (coverageWriter_) coverageWriter_(cbuf_, sizeof(unsigned char), covbufoffset_);
	if (correlatedWriter_) correlatedWriter_(corrbuf_, sizeof(unsigned char), correlatedbufoffset_);

	delete[] rec;
}
