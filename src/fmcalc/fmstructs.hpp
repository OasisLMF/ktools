#pragma once
#include <vector>

struct LossRec {
	float loss = 0;
	float retained_loss = 0;
	int agg_id;
	int policytc_id;
	int allocrule_id;
	int next_vec_idx = -1;
	const std::vector<int> *item_idx;
};

struct policytcvidx {
	int policytc_id;
	int agg_id;
	int next_vidx = -1;
	std::vector<int> item_idx;
};

struct tc_rec {
	float tc_val;
	unsigned char tc_id;
};

struct profile_rec {
	int calcrule_id;
	int allocrule_id;
	int ccy_id;
	std::vector<tc_rec> tc_vec;

};
