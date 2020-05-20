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

gulSampleslevelHeader lastitemheader;
gulSampleslevelHeader lastcorrelatedheader;
gulSampleslevelHeader lastcoverageheader;

// Benchmark memory usage
void gulcalc::OutputBenchmark(const int event_id, bool header=false) {

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
void splittiv(std::vector<gulItemIDLoss> &gulitems, OASIS_FLOAT tiv)
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

// because this is sparsely populated - if we keep a list of the populated fields in another vector then we do not need to iterate all the rows to clear it.
// std::vector<std::vector<std::vector<gulItemIDLoss>>> mode1_;
void gulcalc::clearmode1_data()
{
	auto iter = mode1UsedCoverageIDs_.begin();
	while (iter != mode1UsedCoverageIDs_.end()) {
		mode1_[*iter].clear();
		//auto iter2 = mode1_[*iter].begin()	;
		//while (iter2 != mode1_[*iter].end()) {
		//	iter2->clear();
		//	iter2++;
		//}
		iter++;
	}
	mode1UsedCoverageIDs_.clear();
}
void gulcalc::clearfullCorr_data()
{
	auto iter = fullCorr_.begin();
	while (iter != fullCorr_.end()) {
		auto iter2 = iter->begin();
		while (iter2 != iter->end()) {
			iter2->clear();
			iter2++;
		}
		iter++;
	}
}
void gulcalc::outputmode1data(int event_id)
{
	
	//fprintf(stderr, "event_id = %d\n",event_id);
	//for (int j = 1; j < mode1_.size(); j++) {	// iterate over coverage id
	auto cov_iter = mode1UsedCoverageIDs_.begin();
	while(cov_iter != mode1UsedCoverageIDs_.end()){
		// map of item to a vector of
		std::map<int, std::vector< gulSampleslevelRec> > gxi;
		OASIS_FLOAT tiv = 0;
		bool hasData = false;
		int j = *cov_iter;
		if (mode1_[j].size() > 0) {
			tiv = (*coverages_)[j];		
			hasData = true;
			for (size_t i = 0; i < mode1_[j].size(); i++) {	// now the sidx loop				
					splittiv(mode1_[j][i], tiv);
					auto iter = mode1_[j][i].begin();
					while (iter != mode1_[j][i].end()) {	// now iterate over valid item_id,losses
						gulSampleslevelRec gg;
						gg.sidx = i - 2;
						gg.loss = iter->loss;						
						gxi[iter->item_id].push_back(gg);
						iter++;
					}

			}
		}
		// We have completed the loop on that coverage id so all items associated with that coverage ID can now be outputted
		// Output the rows here
		if (hasData == true) {
			//fprintf(stderr, "event_id = %d we have data !!!!\n", event_id);
			auto iter = gxi.begin();
			std::vector<gulitemSampleslevel> gulrows;
			while (iter != gxi.end()) {
				gulitemSampleslevel g;
				g.event_id = event_id;
				g.item_id = iter->first;	// thats the coverage id						
				auto iter2 = iter->second.begin();
				while (iter2 != iter->second.end()) {
					g.sidx = iter2->sidx;
					g.loss = iter2->loss;
					gulrows.push_back(g);
					if (g.sidx == mean_idx) {
						g.sidx = tiv_idx;
						g.loss = tiv / gxi.size();
						gulrows.push_back(g);
					}
					iter2++;
				}
				iter++;
			}


			sort(gulrows.begin(), gulrows.end());
			auto v_iter = gulrows.begin();
			while (v_iter != gulrows.end()) {
				itemoutputgul(*v_iter);
				v_iter++;
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

void gulcalc::outputcorrelateddata(int event_id)
{

	for (size_t j = 1; j < fullCorr_.size(); j++) {
		std::map<int, std::vector<gulSampleslevelRec> > gxi;
		OASIS_FLOAT tiv =0;
		if(fullCorr_[j].size() > 0) {
			tiv = (*coverages_)[j];
			for (size_t i = 0; i < fullCorr_[j].size(); i++) {   // sidx loop
				splittiv(fullCorr_[j][i], tiv);
				auto iter = fullCorr_[j][i].begin();
				while (iter != fullCorr_[j][i].end()) {   // valid item_id, losses loop
					gulSampleslevelRec gg;
					gg.sidx = i - 2;
					gg.loss = iter->loss;

					gxi[iter->item_id].push_back(gg);
					iter++;
				}
			}
		}

		// Output rows here

		auto iter = gxi.begin();
		std::vector<gulitemSampleslevel> gulrows;
		while (iter != gxi.end()) {
			gulitemSampleslevel g;
			g.event_id = event_id;
			g.item_id = iter->first;

			auto iter2 = iter->second.begin();
			while (iter2 != iter->second.end()) {

				g.sidx = iter2->sidx;
				g.loss = iter2->loss;
				gulrows.push_back(g);
				if(g.sidx == mean_idx) {
					g.sidx = tiv_idx;
					g.loss = tiv / gxi.size();
					gulrows.push_back(g);

				}
				iter2++;
			}
			iter++;
		}

		sort(gulrows.begin(), gulrows.end());
		auto v_iter = gulrows.begin();
		while (v_iter != gulrows.end()) {
			correlatedoutputgul(*v_iter);
			v_iter++;
		}

	}

	clearfullCorr_data();
}

void gulcalc::gencovoutput(gulcoverageSampleslevel &gg)
{
	if (coverageWriter_ == 0)  return;
	if (cov_[gg.coverage_id].size() == 0) {
		cov_[gg.coverage_id].resize(samplesize_ + 3, 0);
	}
	cov_[gg.coverage_id][gg.sidx + 2] += gg.loss;
}

void gulcalc::genmode1output(gulitemSampleslevel& gg,int coverage_id)
{	
	if (mode1_[coverage_id].size() == 0) {
		mode1UsedCoverageIDs_.push_back(coverage_id);
		mode1_[coverage_id].resize(samplesize_ + 3);
	}
	gulItemIDLoss gi;
	gi.item_id = gg.item_id;
	gi.loss = gg.loss;
	mode1_[coverage_id][gg.sidx + 2].push_back(gi);
}

void gulcalc::gencorrelatedoutput(gulitemSampleslevel& gg, int coverage_id)
{
	if (fullCorr_[coverage_id].size() == 0) {
		fullCorr_[coverage_id].resize(samplesize_ + 3);
	}
	gulItemIDLoss gi;
	gi.item_id = gg.item_id;
	gi.loss = gg.loss;
	fullCorr_[coverage_id][gg.sidx + 2].push_back(gi);
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

void gulcalc::setupandgenoutput(const item_map_rec &er, const OASIS_FLOAT tiv,
				const int event_id, const int bin_index,
				const OASIS_FLOAT rval, const probrec &p,
				const int sample_id, const bool correlated=false)
{
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
		if (correlated) gencorrelatedoutput(gg, er.coverage_id);
		else genmode1output(gg, er.coverage_id);
	}
}

void gulcalc::output_mean(OASIS_FLOAT tiv, prob_mean *pp, int bin_count, OASIS_FLOAT &gul_mean,  OASIS_FLOAT &std_dev)
{
	OASIS_FLOAT last_prob_to = 0;
	gul_mean = 0;
	std_dev = 0;
	OASIS_FLOAT ctr_var = 0;

	for (int bin_index = 0; bin_index < bin_count; bin_index++){
		probrec p;
		if (bin_index == 0) p.prob_from = 0;
		else p.prob_from = last_prob_to;
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
}

void gulcalc::processrec_mode1(char* rec, int recsize)
{
	damagecdfrec* d = (damagecdfrec*)rec;
	char* endofRec = rec + recsize;

	int rnd_count = rnd_->rdxmax();

	item_map_key k;
	k.areaperil_id = d->areaperil_id;
	k.vulnerability_id = d->vulnerability_id;

	auto pos = item_map_->find(k);
	if (pos != item_map_->end()) {
		auto iter = pos->second.begin();
		while (iter != pos->second.end()) {
			gulitemSampleslevel gx;
			gx.event_id = d->event_id;
			gx.item_id = iter->item_id;
			char* b = rec + sizeof(damagecdfrec);
			int* bin_count = (int*)b;
			b = b + sizeof(int);
			prob_mean* pp = (prob_mean*)b;
			OASIS_FLOAT std_dev;
			OASIS_FLOAT gul_mean;
			OASIS_FLOAT tiv = (*coverages_)[iter->coverage_id];
			output_mean(tiv, pp, *bin_count, gul_mean, std_dev);
			gx.loss = gul_mean;
			gx.sidx = mean_idx;			
			genmode1output(gx, iter->coverage_id);
			if (correlatedWriter_) gencorrelatedoutput(gx, iter->coverage_id);
			gx.loss = std_dev;
			gx.sidx = std_dev_idx;
			genmode1output(gx, iter->coverage_id);
			if (correlatedWriter_) gencorrelatedoutput(gx, iter->coverage_id);
			int ridx = 0; // dummy value
			int ridx0 = 0;
			switch (rndopt_) {
			case rd_option::userandomnumberfile:
				ridx = ((iter->group_id * p1_ * p3_) + (d->event_id * p2_)) % rnd_count;
				ridx0 = ((p1_ * p3_) + (d->event_id *p2_)) % rnd_count;
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

			prob_mean* pp_max = pp + (*bin_count) - 1;
			for (int i = 0; i < samplesize_; i++) {
				OASIS_FLOAT  rval;
				OASIS_FLOAT rval0;
				if (rndopt_ == rd_option::usehashedseed) {
					rval = rnd_->nextrnd();
					if (correlatedWriter_) rval0 = rnd0_->nextrnd();
				}
				else {
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
				pp = (prob_mean*)b;
				bool hit_rval = false;
				bool hit_rval0 = false;
				for (int bin_index = 0; bin_index < *bin_count; bin_index++) {
					if ((char*)pp > endofRec) {
						cerr << "FATAL: Reached end of record"
							; // this is an error condition
						pp--;
					}

					probrec p;
					if (bin_index == 0) p.prob_from = 0;
					else p.prob_from = last_prob_to;
					p.prob_to = pp->prob_to;
					p.bin_mean = pp->bin_mean;
					last_prob_to = pp->prob_to;
					if (rval < p.prob_to && !hit_rval) {
						setupandgenoutput(*iter, tiv, d->event_id, bin_index, rval, p, i);
						if (!correlatedWriter_) break; // break the for loop
						else hit_rval = true;
					}
					if (correlatedWriter_ && !hit_rval0 && rval0 < p.prob_to) {
						setupandgenoutput(*iter, tiv, d->event_id, bin_index, rval0, p, i, true);
						hit_rval0 = true;
					}
					if (hit_rval && hit_rval0) break;   // break the for loop

					pp++;
				}
			}
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
			OASIS_FLOAT std_dev;
			OASIS_FLOAT gul_mean;
			OASIS_FLOAT tiv = (*coverages_)[iter->coverage_id];
			output_mean(tiv, pp, *bin_count, gul_mean, std_dev);
			gx.sidx = tiv_idx;
			gx.loss = tiv;
			itemoutputgul(gx);			
			gx.loss = std_dev;
			gc.loss = std_dev;
			gx.sidx = std_dev_idx;
			gc.sidx = std_dev_idx;
			itemoutputgul(gx);
			gencovoutput(gc);
			gx.loss = gul_mean;
			gc.loss = gul_mean;
			gx.sidx = mean_idx;
			gc.sidx = mean_idx;
			itemoutputgul(gx);
			gencovoutput(gc);
			int ridx = 0; // dummy value		
			switch (rndopt_) {
			case rd_option::userandomnumberfile:
				ridx = ((iter->group_id * p1_*p3_) + (d->event_id * p2_)) % rnd_count;
				break;
			case rd_option::usecachedvector:
				ridx = iter->group_id * samplesize_;
				break;
			case rd_option::usehashedseed:
				{
					unsigned long long s1 = (iter->group_id * 1543270363L) % 2147483648L;		// hash group_id and event_id to seed random number
					unsigned long long s2 = (d->event_id * 1943272559L) % 2147483648L;
					s1 = (s1 + s2 + rand_seed_) % 2147483648L;
					rnd_->seedRands(s1);
				}
				break;
			default:
				fprintf(stderr, "FATAL: %s: Unknown random number option\n", __func__);
				exit(-1);
			}

			prob_mean *pp_max = pp + (*bin_count) -1;
			for (int i = 0; i < samplesize_; i++){
				OASIS_FLOAT  rval;
				if (rndopt_ == rd_option::usehashedseed) rval = rnd_->nextrnd();
				else rval = rnd_->rnd(ridx + i);
				if (rval >= pp_max->prob_to) {
					rval = pp_max->prob_to - 0.00000003;	// set value to just under max value (which should be 1)
				}
				
                OASIS_FLOAT last_prob_to = 0;
				pp = (prob_mean *)b;
				for (int bin_index = 0; bin_index < *bin_count; bin_index++){
					if ((char *)pp > endofRec) {
						cerr << "FATAL: Reached end of record"
							; // this is an error condition
						pp--;
					}

					probrec p;
					if (bin_index == 0) p.prob_from = 0;
					else p.prob_from = last_prob_to;
					p.prob_to = pp->prob_to;
					p.bin_mean = pp->bin_mean;
					last_prob_to = pp->prob_to;
					if (rval < p.prob_to){
						gulGulSamples g;
						g.event_id = d->event_id;
						g.item_id = iter->item_id;
						g.tiv = tiv;
						g.bin_index = bin_index;
						g.prob_from = p.prob_from;
						g.prob_to = p.prob_to;
						g.bin_mean = p.bin_mean;
						g.rval = rval;
						g.sidx = i + 1;
						gulitemSampleslevel gg;
						gulcoverageSampleslevel ggc;
						damagebindictionary b = (*damagebindictionary_vec_)[g.bin_index];
                        if (debug_) gg.loss = rval;
                        else gg.loss = getgul(b, g);
						ggc.loss = gg.loss;
						gg.sidx = g.sidx;
						ggc.sidx = g.sidx;
						gg.event_id = g.event_id;
						gg.item_id = g.item_id;
						ggc.coverage_id = iter->coverage_id;
						ggc.event_id = g.event_id;
						if (gg.loss >= loss_threshold_) {
							itemoutputgul(gg);
							//covoutputgul(ggc);
							gencovoutput(ggc);
						}
						break; // break the for loop
					}

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

	if (benchmark_) OutputBenchmark(0, true);

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
				if (benchmark_) OutputBenchmark(last_event_id);
				outputmode1data(last_event_id);
				if (correlatedWriter_) outputcorrelateddata(last_event_id);
			}
			last_event_id = d->event_id;
			if (rndopt_ == rd_option::usecachedvector) rnd_->clearvec();
		}

		processrec_mode1(rec, recsize);
	}
	if (benchmark_) OutputBenchmark(d->event_id);
	outputmode1data(d->event_id);
	if (correlatedWriter_) outputcorrelateddata(d->event_id);

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

	if (benchmark_) OutputBenchmark(0, true);

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
				if (benchmark_) OutputBenchmark(last_event_id);
				outputcoveragedata(last_event_id);
			}
			last_event_id = d->event_id;						
			if (rndopt_ == rd_option::usecachedvector) rnd_->clearvec();
		}

		processrec(rec, recsize);
	}
	if (benchmark_) OutputBenchmark(d->event_id);
	outputcoveragedata(d->event_id);
	if (itemWriter_)  itemWriter_(ibuf_, sizeof(unsigned char), itembufoffset_);
	if (lossWriter_)  lossWriter_(ibuf_, sizeof(unsigned char), itembufoffset_);
	if (coverageWriter_) coverageWriter_(cbuf_, sizeof(unsigned char), covbufoffset_);

	delete[] rec;
}
