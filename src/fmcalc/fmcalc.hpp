#pragma once

#include "../include/oasis.hpp"
#include "fmstructs.hpp"

#include <vector>
#include <map>


class fmcalc {
public:
	void setmaxlevel(int maxlevel) { if (maxlevel > -1) maxLevel_ = maxlevel; }
	void dofm(int event_id_, const std::vector<int> &items_, std::vector<std::vector<float>> &event_guls_);
	fmcalc(int maxRunLevel) { init(maxRunLevel); }
	inline float gettiv(int item_id) { return item_to_tiv_[item_id]; }
private:
	std::vector<std::vector<std::vector<int>>> policy_tc_vec_vec_vec;
	std::vector<int> level_to_maxagg_id;
	int max_layer_ = 0;		// initialized from policy_tc
	int max_agg_id_ = 0;	// initialized from policy_tc
	std::vector <profile_rec> profile_vec_;
	int maxLevel_ = 0;
	std::vector<std::vector<int>> pfm_vec_vec_;  // initialized from fm/programme.bin  pfm_vec_vec[level_id][item_id] returns agg_id 
	std::vector<float> item_to_tiv_;	
	void init_programme(int maxrunLevel);
	void init_profile();
	void init_itemtotiv();
	void init_fmxref();
	void init(int MaxRunLevel);
	void init_policytc(int MaxRunLevel);
	bool loadcoverages(std::vector<float> &coverages);
	inline void dofmcalc_r(std::vector<std::vector<int>>  &aggid_to_vectorlookups_, std::vector<std::vector <LossRec>> &agg_vecs_, 
		int level_, int max_level_,	std::map<fmlevelhdr, std::vector<fmlevelrec> > &outmap_, 
		fmlevelhdr &fmhdr_, int sidx_, const std::vector<std::vector<std::vector<policytcvidx>>> &avxs_, int layer_, 
		const std::vector<int> &items_, const std::vector<float> &guls_);
	inline void dofmcalc(std::vector <LossRec> &agg_vec_);	
};
