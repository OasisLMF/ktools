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
#ifndef GULCALC_H_
#define GULCALC_H_

#include "../include/oasis.h"

#include <vector>
#include <map>
#include "getrands.h"

struct exposure_rec {
	int item_id;
	int group_id;
	OASIS_FLOAT tiv;
};

struct item_map_key {
	AREAPERIL_INT areaperil_id;
	int vulnerability_id;
};

bool operator<(const item_map_key& lhs, const item_map_key& rhs);

struct item_map_rec {
	int item_id;
	int coverage_id;
	int group_id;
};


struct prob_mean {
	OASIS_FLOAT prob_to;
	OASIS_FLOAT bin_mean;
};

struct gulGulSamples {
	int event_id;
	int item_id;
	OASIS_FLOAT tiv;
	int bin_index;
	OASIS_FLOAT prob_from;
	OASIS_FLOAT prob_to;
	OASIS_FLOAT bin_mean;
	int sidx;
	double rval;
};

const int gularraysize = 1000;
const int bufsize = sizeof(gulitemSampleslevel)* gularraysize;

struct gulcalcopts {
	rd_option rndopt = rd_option::usehashedseed;
	int rand_vector_size = 1000000;
	int rand_seed = 0;
	std::string item_output;
	std::string coverage_output;
	std::string correlated_output;
	bool itemLevelOutput = false;
	bool coverageLevelOutput = false;
	bool correlatedLevelOutput = false;
	int samplesize = -1;
	double loss_threshold = 0.0;
	bool debug = false;
	FILE *itemout = stdout;
	FILE *covout = stdout;
	FILE *corrout = stdout;
	int mode = 0;		// default mode = 0 
};

struct gulItemIDLoss {
	int item_id;
	double loss;
};
struct probrec {
      OASIS_FLOAT prob_from;
      OASIS_FLOAT prob_to;
      OASIS_FLOAT bin_mean;
};
class gulcalc  {
private:
	getRands *rnd_;
	getRands *rnd0_;
	int rand_seed_ = 0;
	gulcalcopts opt_;
	const std::map<item_map_key, std::vector<item_map_rec> > *item_map_;
	const std::vector<OASIS_FLOAT> *coverages_;
	const std::vector<damagebindictionary> *damagebindictionary_vec_;
	void gencovoutput(gulcoverageSampleslevel &gc);
	void genmode1output(gulitemSampleslevel& gc, int coverage_id);
	void gencorrelatedoutput(gulitemSampleslevel& gc, int coverage_id);
	std::vector<std::vector<OASIS_FLOAT>> cov_;
	std::vector<std::vector<std::vector<gulItemIDLoss>>> mode1_;
	std::vector<std::vector<std::vector<gulItemIDLoss>>> fullCorr_;
	std::vector<int> mode1UsedCoverageIDs_;
	void covoutputgul(gulcoverageSampleslevel &gc);
	void outputcoveragedata(int event_id);
	void outputmode1data(int event_id);
	void outputcorrelateddata(int event_id);
	void itemoutputgul(gulitemSampleslevel &gg);
	void correlatedoutputgul(gulitemSampleslevel &gg);
	void setupandgenoutput(const item_map_rec &er, const OASIS_FLOAT tiv, const int event_id, const int bin_index, const OASIS_FLOAT rval, const probrec &p, const int sample_id, const bool correlated);
	void(*itemWriter_)(const void *ibuf, int size, int count);
	void(*coverageWriter_)(const void *ibuf, int size, int count);
    void (*lossWriter_)(const void *ibuf, int size, int count);	// loss stream writer
    	void(*correlatedWriter_)(const void *ibuf, int size, int count);
	bool(*iGetrec_)(char *rec, int recsize);
	OASIS_FLOAT getgul(damagebindictionary &b, gulGulSamples &g);
	void output_mean(const item_map_rec &er, OASIS_FLOAT tiv, prob_mean *pp, int bin_count, OASIS_FLOAT &gul_mean, OASIS_FLOAT &std_dev);
	void init();
	unsigned char *ibuf_;	// item level buffer
	unsigned char *corrbuf_;   // correlated level buffer
	unsigned char *cbuf_;	// coverage level buffer
	int itembufoffset_ = 0;
	int correlatedbufoffset_ = 0;
	int covbufoffset_ = 0;
	double loss_threshold_;
	rd_option rndopt_;
	bool debug_;
	int samplesize_;
	bool isFirstItemEvent_;
	bool isFirstCorrelatedEvent_;
	bool isFirstCovEvent_;
	long long p1_;
	long long p2_;
	long long p3_;
	void clearmode1_data();
	void clearfullCorr_data();

public:	
	void processrec(char *rec, int recsize);
	void processrec_mode1(char* rec, int recsize);
	 gulcalc(const std::vector<damagebindictionary> &damagebindictionary_vec,const std::vector<OASIS_FLOAT> &tivs,
		const std::map<item_map_key, std::vector<item_map_rec> > &item_map, getRands &rnd, getRands &rnd0, 		
		void (*itemWriter)(const void *ibuf,int size, int count),
		void(*coverageWriter)(const void *ibuf,int size, int count),
        void (*lossWriter)(const void *ibuf, int size, int count),
		void(*correlatedWriter)(const void *ibuf, int size, int count),
		bool(*iGetrec)(char *rec, int recsize),
		const gulcalcopts& opt
		): opt_(opt) {
		damagebindictionary_vec_ = &damagebindictionary_vec;
		coverages_ = &tivs;
		cov_.resize(coverages_->size());
		if (opt.mode == 1) {
			mode1_.resize(coverages_->size());
		}
		if (opt.correlatedLevelOutput == true) {
			fullCorr_.resize(coverages_->size());
		}
		item_map_ = &item_map;
		rnd_ = &rnd;
		rnd0_ = &rnd0;

		itemWriter_ = itemWriter;
		coverageWriter_ = coverageWriter;
        lossWriter_ = lossWriter;
		correlatedWriter_ = correlatedWriter;
		iGetrec_ = iGetrec;
		ibuf_ = new unsigned char[bufsize + sizeof(gulitemSampleslevel)]; // make the allocation bigger by 1 record to avoid overrunning
		corrbuf_ = new unsigned char[bufsize + sizeof(gulitemSampleslevel)];   // make the allocation bigger by 1 record to avoid overrunning
		cbuf_ = new unsigned char[bufsize + sizeof(gulitemSampleslevel)]; // make the allocation bigger by 1 record to avoid overrunning
		//TODO: when using double precision, these buffers above are overflowing,
		//the hack fix is to replace sizeof(gulitemSampleslevel) with 2*sizeof(gulitemSampleslevel)
		loss_threshold_ = opt.loss_threshold;
		rndopt_ = opt.rndopt;				
		debug_ = opt.debug;
		samplesize_ = opt.samplesize;
		isFirstItemEvent_ = true;
		isFirstCorrelatedEvent_ = true;
		isFirstCovEvent_ = true;
		p1_ = rnd_->getp1();	// prime p1	make these long to force below expression to not have sign problem
		p2_ = rnd_->getp2((unsigned int)p1_);  // prime no p2
		p3_ = rnd_->getp2((unsigned int)samplesize_);	// use as additional offset to stop overlapping of random numbers 
		rand_seed_ = opt.rand_seed;
	}	
	~gulcalc() {
		delete [] ibuf_;
		delete [] corrbuf_;
		delete [] cbuf_;
	}
	void mode0();
	void mode1();
};

void doit(const gulcalcopts &g);
#endif // GULCALC_H_
