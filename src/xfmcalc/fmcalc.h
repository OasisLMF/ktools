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
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/

#ifndef FMCALC_H_
#define FMCALC_H_

#include "../include/oasis.h"
#include "fmstructs.h"

#include <vector>
#include <map>


class fmcalc {
public:
	void setmaxlevel(int maxlevel) { if (maxlevel > -1) maxLevel_ = maxlevel; }
	void dofm(int event_id_, const std::vector<int> &items_, std::vector<std::vector<OASIS_FLOAT>> &event_guls_);
	fmcalc(int maxRunLevel) { init(maxRunLevel); }
	inline OASIS_FLOAT gettiv(int item_id) { return item_to_tiv_[item_id]; }
private:
	std::vector<std::vector<std::vector<int>>> policy_tc_vec_vec_vec_; // policy_tc_vec_vec_vec_[level][agg_id][layer_id]
	std::vector<int> level_to_maxagg_id_;
	int max_layer_ = 0;		// initialized from policy_tc
	int max_agg_id_ = 0;	// initialized from policy_tc
	std::vector <profile_rec> profile_vec_;
	int maxLevel_ = 0;
	std::vector<std::vector<int>> pfm_vec_vec_;  // initialized from fm/programme.bin  pfm_vec_vec[level_id][item_id] returns agg_id 
	std::vector<OASIS_FLOAT> item_to_tiv_;	
	void init_programme(int maxrunLevel);
	void init_profile();
	void init_itemtotiv();
	void init_fmxref();
	void init(int MaxRunLevel);
	void init_policytc(int MaxRunLevel);
	bool loadcoverages(std::vector<OASIS_FLOAT> &coverages);
	inline void dofmcalc_r(std::vector<std::vector<int>>  &aggid_to_vectorlookups_, std::vector<std::vector <LossRec>> &agg_vecs_, 
		int level_, int max_level_,	std::map<fmlevelhdr, std::vector<fmlevelrec> > &outmap_, 
		fmlevelhdr &fmhdr_, int sidx_, const std::vector<std::vector<std::vector<policytcvidx>>> &avxs_, int layer_, 
		const std::vector<int> &items_, const std::vector<OASIS_FLOAT> &guls_);
	inline void dofmcalc(std::vector <LossRec> &agg_vec_);	
};

#endif  // FMCALC_H_
