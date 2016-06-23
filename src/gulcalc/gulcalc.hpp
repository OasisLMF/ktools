#pragma once

#include "../include/oasis.hpp"

#include <vector>
#include <map>
#include "getrands.hpp"

struct exposure_rec {
	int item_id;
	int group_id;
	float tiv;
};

struct item_map_key {
	int areaperil_id;
	int vulnerability_id;
};

bool operator<(const item_map_key& lhs, const item_map_key& rhs);

struct item_map_rec {
	int item_id;
	int coverage_id;
	int group_id;
};


struct prob_mean {
	float prob_to;
	float bin_mean;
};

struct gulGulSamples {
	int event_id;
	int item_id;
	float tiv;
	int bin_index;
	float prob_from;
	float prob_to;
	float bin_mean;
	int sidx;
	double rval;
};

const int gularraysize = 1000;
const int bufsize = sizeof(gulitemSampleslevel)* gularraysize;

class gulcalc  {
private:
	getRands *rnd_;
	const std::map<item_map_key, std::vector<item_map_rec> > *item_map_;
	const std::vector<float> *coverages_;
	const std::vector<damagebindictionary> *damagebindictionary_vec_;
	void covoutputgulx(gulcoverageSampleslevel &gc);
	std::map<int, std::vector<float>> cov_;
	void covoutputgul(gulcoverageSampleslevel &gc);
	void outputcoveragedata(int event_id);
	void itemoutputgul(gulitemSampleslevel &gg);
	void(*itemWriter_)(const void *ibuf, int size, int count);
	void(*coverageWriter_)(const void *ibuf, int size, int count);
	bool(*iGetrec_)(char *rec, int recsize);
	float getgul(damagebindictionary &b, gulGulSamples &g);
	void output_mean(const item_map_rec &er, float tiv, prob_mean *pp, int bin_count, float &gul_mean, float &std_dev);
	void init();
	unsigned char *ibuf_;	// item level buffer
	unsigned char *cbuf_;	// coverage level buffer
	int itembufoffset_ = 0;
	int covbufoffset_ = 0;
	double gul_limit_;
	bool userandomtable_;
	bool debug_;
	int samplesize_;
	bool isFirstItemEvent_;
	bool isFirstCovEvent_;
	long long p1_;
	long long p2_;
	long long p3_;
public:	
	void processrec(char *rec, int recsize);
	gulcalc(const std::vector<damagebindictionary> &damagebindictionary_vec,const std::vector<float> &tivs,
		const std::map<item_map_key, std::vector<item_map_rec> > &item_map, getRands &rnd, 
		double gul_limit,
		bool userandomtable,
		bool debug,
		int samplesize,
		void (*itemWriter)(const void *ibuf,int size, int count),
		void(*coverageWriter)(const void *ibuf,int size, int count),
		bool(*iGetrec)(char *rec, int recsize)
		) {
		damagebindictionary_vec_ = &damagebindictionary_vec;
		coverages_ = &tivs;
		item_map_ = &item_map;
		rnd_ = &rnd;
		itemWriter_ = itemWriter;
		coverageWriter_ = coverageWriter;
		iGetrec_ = iGetrec;
		ibuf_ = new unsigned char[bufsize + sizeof(gulitemSampleslevel)]; // make the allocation bigger by 1 record to avoid overrunning
		cbuf_ = new unsigned char[bufsize + sizeof(gulitemSampleslevel)]; // make the allocation bigger by 1 record to avoid overrunning
		gul_limit_ = gul_limit;
		userandomtable_ = userandomtable;
		debug_ = debug;
		samplesize_ = samplesize;
		isFirstItemEvent_ = true;
		isFirstCovEvent_ = true;
		p1_ = rnd_->getp1();	// prime p1	make these long to force below expression to not have sign problem
		p2_ = rnd_->getp2((unsigned int)p1_);  // prime no p2
		p3_ = rnd_->getp2((unsigned int)samplesize_);	// use as additional offset to stop overlapping of random numbers 
	}
	~gulcalc() {
		delete [] ibuf_;
		delete [] cbuf_;
	}
	void doit();
};
