#pragma once

#include "../include/oasis.hpp"

#include <vector>
#include <map>


struct LossRec {
	float loss;
	float retained_loss;
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


class fmcalc {
public:
	void setmaxlevel(int maxlevel) { if (maxlevel > -1) maxLevel_ = maxlevel; }
	void dofm(int event_id_, const std::vector<int> &items_, std::vector<std::vector<float>> &event_guls_);
	fmcalc(int maxRunLevel) { init(maxRunLevel); }
private:
	int maxLevel_ = 0;
	void init_programme(int maxrunLevel);
	void init_profile();
	void init(int MaxRunLevel);
	void init_policytc(int MaxRunLevel);
	inline void dofmcalc_r(std::vector<std::vector<int>>  &aggid_to_vectorlookups_, std::vector<std::vector <LossRec>> &agg_vecs_, 
		int level_, int max_level_,	std::map<fmlevelhdr, std::vector<fmlevelrec> > &outmap_, 
		fmlevelhdr &fmhdr_, int sidx_, const std::vector<std::vector<std::vector<policytcvidx>>> &avxs_, int layer_, 
		const std::vector<int> &items_, const std::vector<float> &guls_);
	inline void dofmcalc(std::vector <LossRec> &agg_vec_);
	
};
