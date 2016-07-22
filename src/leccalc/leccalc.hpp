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

//extern FILE *fout[];

