#pragma once

#include <map>
#include "../include/oasis.hpp"

enum { AGG_FULL_UNCERTAINTY = 0, AGG_WHEATSHEAF, AGG_SAMPLE_MEAN, AGG_WHEATSHEAF_MEAN, OCC_FULL_UNCERTAINTY, OCC_WHEATSHEAF, OCC_SAMPLE_MEAN, OCC_WHEATSHEAF_MEAN };


struct outkey2 {
	int summary_id;
	int period_no;
	int sidx;
};

struct wheatkey {
	int summary_id;
	int sidx;
};

struct summary_id_period_key {
	int summary_id;
	int period_no;
	int type;
};


typedef  std::vector<float> lossvec;

extern FILE *fout[];
extern int totalperiods;
extern int maxsummaryid;

extern std::map<outkey2, float> agg_out_loss;
extern std::map<outkey2, float> max_out_loss;

void outputAggFulluncertainty();
void outputAggWheatsheaf();
void outputAggWheatSheafMean(int samplesize);
void outputAggSampleMean(int samplesize);
void outputOccFulluncertainty();
void outputOccWheatsheaf();
void outputOccWheatSheafMean(int samplesize);
void outputOccSampleMean(int samplesize);
