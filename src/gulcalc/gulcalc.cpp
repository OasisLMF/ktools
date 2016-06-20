/*
* Copyright (c)2015 Oasis LMF Limited 
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

#include "gulcalc.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <memory.h>

#include "../include/oasis.hpp"

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

struct probrec {
	float prob_from;
	float prob_to;
	float bin_mean;
};

gulSampleslevelHeader lastitemheader;
gulSampleslevelHeader lastcoverageheader;

float gulcalc::getgul(damagebindictionary &b, gulGulSamples &g)
{
	float gul = 0;
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
	if (itemWriter_ == 0)  return;
	if (itembufoffset_ >= bufsize) {
		itemWriter_(ibuf_, sizeof(unsigned char), itembufoffset_);
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

void gulcalc::output_mean(const item_map_rec &er, float tiv, prob_mean *pp, int bin_count, float &gul_mean,  float &std_dev)
{
	float last_prob_to = 0;
	gul_mean = 0;
	std_dev = 0;
	float ctr_var = 0;

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
	float g2 = gul_mean * gul_mean;
	std_dev = ctr_var - g2;
    if (std_dev < 0) std_dev  = 0;
	std_dev = sqrt(std_dev);
}

void gulcalc::processrec(char *rec, int recsize)
{
damagecdfrec *d = (damagecdfrec *)rec;
	char *endofRec = rec + recsize;
	//long long p1 = rnd_->getp1();	// prime p1	make these long to force below expression to not have sign problem
	//long long p2 = rnd_->getp2((unsigned int)p1);  // prime no p2
	//long long p3 = rnd_->getp2((unsigned int)samplesize_);	// use as additional offset to stop collision
	int rnd_count = rnd_->rdxmax();

	item_map_key k;
	k.areaperil_id = d->areaperil_id;
	k.vulnerability_id = d->vulnerabilty_id;

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
			float std_dev;
			float gul_mean;
//			coveragerec cr=	(*coverages_)[iter->coverage_id];
			float tiv = (*coverages_)[iter->coverage_id];
			output_mean(*iter, tiv, pp, *bin_count, gul_mean, std_dev);			
			gx.loss = gul_mean;
			gc.loss = gul_mean;
			gx.sidx = mean_idx;
			gc.sidx = mean_idx;
			itemoutputgul(gx);
			covoutputgul(gc);			
			gx.loss = std_dev;
			gc.loss = std_dev;
			gx.sidx = std_dev_idx;
			gc.sidx = std_dev_idx;
			itemoutputgul(gx);
			covoutputgul(gc);
			int ridx = 0; // dummy value		
            if (userandomtable_) ridx = ((iter->group_id * p1_*p3_) + (d->event_id * p2_)) % rnd_count;
            else ridx = iter->group_id * samplesize_;
			for (int i = 0; i < samplesize_; i++){
				float  rval;
				rval = rnd_->rnd(ridx + i);
                float last_prob_to = 0;
				pp = (prob_mean *)b;
				for (int bin_index = 0; bin_index < *bin_count; bin_index++){
					if ((char *)pp > endofRec) {
						cerr << "Reached end of record"
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
						if (gg.loss >= gul_limit_) {
							itemoutputgul(gg);
							covoutputgul(ggc);
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

	gulstream_type = gul_coverage_stream | gulstream_id;

	if (coverageWriter_ != 0) {
		coverageWriter_(&gulstream_type, sizeof(gulstream_type), 1);
		coverageWriter_(&samplesize_, sizeof(samplesize_), 1);
	}
}
void gulcalc::doit()
{
	init();

	int total_bins = damagebindictionary_vec_->size();
	int max_recsize = (int)(total_bins * sizeof(prob_mean)) + sizeof(damagecdfrec) + sizeof(int);
	int last_event_id = -1;

	char *rec = new char[max_recsize];
	damagecdfrec *d = (damagecdfrec *)rec;

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
			last_event_id = d->event_id;
			if (userandomtable_ == false) rnd_->clearvec();
		}

		processrec(rec, recsize);
	}

	if (itemWriter_)  itemWriter_(ibuf_, sizeof(unsigned char), itembufoffset_);
	if (coverageWriter_) coverageWriter_(cbuf_, sizeof(unsigned char), covbufoffset_);
}
