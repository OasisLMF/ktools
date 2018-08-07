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

#include "fmcalc.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <fcntl.h>
#include <assert.h>
#include <unordered_set>

#ifdef __unix
    #include <unistd.h>
#endif

using namespace std;

struct pfmkey {
	int level_id;
	int item_id;
};

enum tc {			// tc = terms and conditions
	deductible,
	limit,
	share_prop_of_limit,
	deductible_prop_of_loss,
	limit_prop_of_loss,
	deductible_prop_of_tiv,
	limit_prop_of_tiv,
	deductible_prop_of_limit
};

//enum tc_new {			// tc = terms and conditions
//	deductible_1,
//	deductible_2,
//	deductible_3,
//	attachment_1,
//	limit_1,
//	share_1,
//	share_2,
//	share_3
//};

bool operator<(const fmlevelhdr& lhs, const fmlevelhdr& rhs)
{
	if (lhs.event_id != rhs.event_id)  return lhs.event_id < rhs.event_id;
	return lhs.output_id < rhs.output_id;		// should never reach here since these two are always equal 
}

struct fmxref_key
{
	int agg_id;
	int layer_id;
};

std::map<fmxref_key, int> fm_xrefmap;
bool operator<(const fmxref_key& lhs, const fmxref_key& rhs)
{
	if (lhs.agg_id != rhs.agg_id)  return lhs.agg_id < rhs.agg_id;
	return lhs.layer_id < rhs.layer_id;		
}

inline void fmcalc::dofmcalc_old(vector <LossRec> &agg_vec, int layer)
{
	for (LossRec &x : agg_vec) {
		if (x.agg_id == 0) break;
		x.original_loss = x.loss;
		if (x.agg_id > 0) {
			if (x.policytc_id > 0) {
				const profile_rec &profile = profile_vec_[x.policytc_id];
				//x.allocrule_id = profile.allocrule_id;
				x.allocrule_id = allocrule_;
				switch (profile.calcrule_id) {
					case 1:
					{
						OASIS_FLOAT ded = 0;
						OASIS_FLOAT lim = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible) ded = y.tc_val;
							if (y.tc_id == limit) lim = y.tc_val;
						}
						//Function1 = IIf(Loss < Ded, 0, IIf(Loss > Ded + Lim, Lim, Loss - Ded))
						OASIS_FLOAT loss = x.loss - ded;
						if (loss < 0) loss = 0;
						if (loss > lim) loss = lim;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 2:
					{
						OASIS_FLOAT ded = 0;
						OASIS_FLOAT lim = 0;
						OASIS_FLOAT share = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible) ded = y.tc_val;
							if (y.tc_id == limit) lim = y.tc_val;
							if (y.tc_id == share_prop_of_limit) share = y.tc_val;
						}
						//Function2 = IIf(Loss < Ded, 0, IIf(Loss > Ded + Lim, Lim, Loss - Ded)) * Share	
						OASIS_FLOAT loss = 0;
						if (x.loss > (ded + lim)) loss = lim;
						else loss = x.loss - ded;
						if (loss < 0) loss = 0;
						float floss = x.retained_loss + (x.loss - loss);
						loss = loss * share;
						x.retained_loss = x.retained_loss + (x.loss - loss);	
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;						
						x.loss = loss;

					}
					break;
					case 3:
					{
						OASIS_FLOAT ded = 0;
						OASIS_FLOAT lim = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible) ded = y.tc_val;
							if (y.tc_id == limit) lim = y.tc_val;
						}
						//Function3 = IIf(Loss < Ded, 0, IIf(Loss > Ded, Lim, Loss))
						OASIS_FLOAT loss = x.loss;
						if (loss < ded) loss = 0;
						else loss = loss;
						if (loss > lim) loss = lim;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 5:
					{
						OASIS_FLOAT ded = 0;
						OASIS_FLOAT lim = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_prop_of_loss) ded = y.tc_val;
							if (y.tc_id == limit_prop_of_loss) lim = y.tc_val;
						}
						//Function5 = Loss * (Lim - Ded)
						OASIS_FLOAT loss = x.loss * (lim - ded);
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 9:
					{
						OASIS_FLOAT ded = 0;
						OASIS_FLOAT lim = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_prop_of_limit) ded = y.tc_val;
							if (y.tc_id == limit) lim = y.tc_val;
						}
						//Function9 = IIf(Loss < (Ded * lim), 0, IIf(Loss > (ded* lim) + Lim, Lim, Loss - (Ded * lim))
						OASIS_FLOAT loss = x.loss - (ded * lim);
						if (loss < 0) loss = 0;
						if (loss > lim) loss = lim;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 10:
					{
						OASIS_FLOAT ded = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible) ded = y.tc_val;
						}
						// Function 10: Applies a cap on retained loss (maximum deductible)
						OASIS_FLOAT loss = 0;
						if (x.retained_loss > ded) {
							loss = x.loss + x.retained_loss - ded;
							if (loss < 0) loss = 0;
							x.retained_loss = x.retained_loss + (x.loss - loss);
						}
						else {
							loss = x.loss;
							// retained loss stays the same
						}
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 11:
					{
						OASIS_FLOAT ded = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible) ded = y.tc_val;
						}
						// Function 11: Applies a floor on retained loss (minimum deductible)
						OASIS_FLOAT loss = 0;
						if (x.retained_loss < ded) {
							loss = x.loss + x.retained_loss - ded;
							if (loss < 0) loss = 0;
							x.retained_loss = x.retained_loss + (x.loss - loss);
						}
						else {
							loss = x.loss;
							// retained loss stays the same
						}
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 12:
					{
						for (auto &z : profile.tc_vec) {
							if (z.tc_id == deductible) {
								OASIS_FLOAT loss = x.loss - z.tc_val;
								if (loss < 0) loss = 0;
								x.retained_loss = x.retained_loss + (x.loss - loss);
								x.loss = loss;
								break;
							}
						}
					}
					break;
					case 14:
					{
						OASIS_FLOAT lim = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == limit) lim = y.tc_val;
						}
						//Function14 =  IIf(Loss > lim, Lim, Loss)
						OASIS_FLOAT loss = x.loss;
						if (loss > lim) loss = lim;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;						
					}
					break;
					case 15:
					{
						OASIS_FLOAT lim = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == limit_prop_of_loss) lim = y.tc_val;
						}
						//Function15 =  Loss * lim
						OASIS_FLOAT loss = x.loss;
						loss = loss * lim;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 16:
					{
						OASIS_FLOAT ded = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_prop_of_loss) ded = y.tc_val;
						}
						//Function16 =  Loss - (loss * ded)
						OASIS_FLOAT loss = x.loss;
						loss = loss - (loss * ded);
						if (loss < 0) loss = 0;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					default:
					{
						fprintf(stderr, "Unknown calc rule %d\n", profile.calcrule_id);
					}
				}
			}
		}
	}
}

// fairly recent old used for reproducing old runs
void fmcalc::compute_item_proportionsx(std::vector<std::vector<std::vector <LossRec>>> &agg_vecs, const std::vector<OASIS_FLOAT> &guls, int level_, int layer_, int previous_layer_)
{

	if (layer_ == 1) {
		std::vector<OASIS_FLOAT> items_prop;
		items_prop.resize(guls.size(), 0);

		for (int level = 1; level < agg_vecs.size(); level++) {
			vector <LossRec> &agg_vec = agg_vecs[level][layer_];
			auto iter = agg_vec.begin();
			while (iter != agg_vec.end()) {
				iter->item_prop.resize(guls.size(), 0);	// this is wrong should be same size as item_idx unless we are using the item_idx as an index into this
				iter++;
			}
		}
		for (int level = 1; level < agg_vecs.size(); level++) {
			vector <LossRec> &agg_vec = agg_vecs[level][layer_];
			OASIS_FLOAT loss_total = 0;
			auto iter = agg_vec.begin();
			while (iter != agg_vec.end()) {
				OASIS_FLOAT total = 0;
				for (int idx : *(iter->item_idx)) {	// because this is the first level there should only be one item_idx
					total += guls[idx];
				}
				iter->gul_total = total;
				loss_total += iter->loss;
				iter++;
			}
			iter = agg_vec.begin();		// loop thru again
			while (iter != agg_vec.end()) {
				if (iter->loss > 0 && loss_total > 0) {
					iter->proportion = iter->loss / loss_total;
				}
				else {
					iter->proportion = 0;
				}
				if (level == 1) {
					//iter->item_prop.resize((iter->item_idx)->size(), 0);
					OASIS_FLOAT total = 0;
					for (int idx : *(iter->item_idx)) {
						total += guls[idx];
					}
					int i = 0;
					if (total > 0) {
						for (int idx : *(iter->item_idx)) {
							iter->item_prop[idx] = guls[idx] / iter->gul_total;
							items_prop[idx] = (guls[idx] / total) * iter->proportion;
							//items_prop[idx] = (guls[idx] / iter->gul_total);
							i++;
						}
					}
				}
				else {
					OASIS_FLOAT prop_total = 0;
					for (int idx : *(iter->item_idx)) {
						prop_total += items_prop[idx];
					}
					int i = 0;
					// iter->item_prop.resize((iter->item_idx)->size(), 0);
					if (prop_total > 0) {
						for (int idx : *(iter->item_idx)) {
							iter->item_prop[idx] = items_prop[idx] / prop_total;
							items_prop[idx] = (items_prop[idx] / prop_total) * iter->proportion;
							i++;
						}
					}
				}
				iter++;
			}
		}
	}
	else {
		if (previous_layer_ < layer_) {
			std::vector<OASIS_FLOAT> items_prop;
			items_prop.resize(guls.size(), 0);
			vector <LossRec> &prev_agg_vec = agg_vecs[level_][1];
			vector <LossRec> &current_agg_vec = agg_vecs[level_][layer_];
			current_agg_vec[0].proportion = prev_agg_vec[0].proportion;
			current_agg_vec[0].item_prop = prev_agg_vec[0].item_prop;
		}
		else {
			vector <LossRec> &prev_agg_vec = agg_vecs[level_ - 1][previous_layer_];
			vector <LossRec> &prev_agg_vec_base = agg_vecs[level_ - 1][1];
			for (int i = 0; i < prev_agg_vec.size(); i++) {
				if (prev_agg_vec[i].item_prop.size() == 0) {
					//prev_agg_vec[i].loss = prev_agg_vec_base[i].loss;
					prev_agg_vec[i].item_prop = prev_agg_vec_base[i].item_prop;
				}
			}
			//

			std::vector<int> v;
			v.resize(guls.size(), -1);
			auto iter = prev_agg_vec.begin();
			int j = 0;

			while (iter != prev_agg_vec.end()) {
				auto it = iter->item_idx->begin();
				while (it != iter->item_idx->end()) {
					v[*it] = j;
					it++;
				}
				j++;
				iter++;
			}
			for (int y = 0; y < agg_vecs[level_][layer_].size(); y++) {
				auto it = agg_vecs[level_][layer_][y].item_idx->begin();
				// 1 create an item id  to prev_agg_vec index
				// 2 use below loop to create a set which will then be used to iterate and get previous_gul_total
				std::unordered_set<int> s;
				while (it != agg_vecs[level_][layer_][y].item_idx->end()) {
					s.insert(v[*it]);
					//prev_gul_total += prev_agg_vec[*it].loss;
					it++;
				}

				OASIS_FLOAT prev_gul_total = 0;
				auto s_iter = s.begin();
				while (s_iter != s.end()) {
					prev_gul_total += prev_agg_vec[*s_iter].loss;
					s_iter++;
				}

				// agg_vecs[level_][layer_][y].loss = prev_gul_total;
				it = agg_vecs[level_][layer_][y].item_idx->begin();
				while (it != agg_vecs[level_][layer_][y].item_idx->end()) {
					if (prev_gul_total > 0) {
						// this is recomputing the first layers proportions
						// only the first three have values the rest are zero for prev_agg_vec[v[*it]].item_prop						
						agg_vecs[level_][layer_][y].item_prop.push_back(prev_agg_vec[v[*it]].loss * prev_agg_vec[v[*it]].item_prop[*it] / prev_gul_total); 
					}
					else {
						agg_vecs[level_][layer_][y].item_prop.push_back(0);
					}
					it++;
				}
			}
		}
		//for (int level = 1; level < agg_vecs.size(); level++) {
		//	vector <LossRec> &agg_vec = agg_vecs[level][layer_];
		//	for (int i = 0; level < agg_vec.size(); i++) {
		//		agg_vec[i].item_prop = prev_agg_vec[i].item_prop;
		//	}
		//	//auto iter = agg_vec.begin();
		//	//while (iter != agg_vec.end()) {
		//	//	iter->item_prop.resize(guls.size(), 0);	// this is wrong should be same size as item_idx unless we are using the item_idx as an index into this
		//	//	iter++;
		//	//}
		//}

		return;

	}
}

// new
void fmcalc::compute_item_proportions(std::vector<std::vector<std::vector <LossRec>>> &agg_vecs, const std::vector<OASIS_FLOAT> &guls, int level_, int layer_, int previous_layer_)
{

	if (layer_ == 1) {
		std::vector<OASIS_FLOAT> items_prop;
		items_prop.resize(guls.size(), 0);

		for (int level = 1; level < agg_vecs.size(); level++) {
			vector <LossRec> &agg_vec = agg_vecs[level][layer_];
			auto iter = agg_vec.begin();
			while (iter != agg_vec.end()) {
				iter->item_prop.resize((iter->item_idx)->size(), 0);
				iter++;
			}
		}
		for (int level = 1; level < agg_vecs.size(); level++) {
			vector <LossRec> &agg_vec = agg_vecs[level][layer_];
			OASIS_FLOAT loss_total = 0;
			auto iter = agg_vec.begin();
			while (iter != agg_vec.end()) {
				OASIS_FLOAT total = 0;
				for (int idx : *(iter->item_idx)) {	// because this is the first level there should only be one item_idx
					total += guls[idx];
				}
				iter->gul_total = total;
				loss_total += iter->loss;
				iter++;
			}
			iter = agg_vec.begin();		// loop thru again
			while (iter != agg_vec.end()) {
				if (iter->loss > 0 && loss_total > 0) {
					iter->proportion = iter->loss / loss_total;
				}
				else {
					iter->proportion = 0;
				}
				if (level == 1) {
					OASIS_FLOAT total = 0;
					for (int idx : *(iter->item_idx)) {
						total += guls[idx];
					}
					int i = 0;
					if (total > 0) {
						for (int idx : *(iter->item_idx)) {
							iter->item_prop[i] = guls[idx] / iter->gul_total;
							items_prop[idx] = (guls[idx] / total) * iter->proportion;
							i++;
						}
					}
				}
				else {
					OASIS_FLOAT prop_total = 0;
					for (int idx : *(iter->item_idx)) {
						prop_total += items_prop[idx];
					}
					int i = 0;
					if (prop_total > 0) {
						for (int idx : *(iter->item_idx)) {
							iter->item_prop[i] = items_prop[idx] / prop_total;
							items_prop[idx] = (items_prop[idx] / prop_total) * iter->proportion;
							i++;
						}
					}
				}
				iter++;
			}
		}
	}
	else {
		if (previous_layer_ < layer_) {
			std::vector<OASIS_FLOAT> items_prop;
			items_prop.resize(guls.size(), 0);
			vector <LossRec> &prev_agg_vec = agg_vecs[level_][1];
			vector <LossRec> &current_agg_vec = agg_vecs[level_][layer_];
			current_agg_vec[0].proportion = prev_agg_vec[0].proportion;
			current_agg_vec[0].item_prop = prev_agg_vec[0].item_prop;
		}
		else {			
			vector <LossRec> &prev_agg_vec = agg_vecs[level_ - 1][previous_layer_];
			vector <LossRec> &prev_agg_vec_base = agg_vecs[level_ - 1][1];
			for (int i = 0; i < prev_agg_vec.size(); i++) {
				if (prev_agg_vec[i].item_prop.size() == 0) {
					//prev_agg_vec[i].loss = prev_agg_vec_base[i].loss;
					prev_agg_vec[i].item_prop = prev_agg_vec_base[i].item_prop;
				}
			}
			//

			std::vector<int> v;
			v.resize(guls.size(), -1);
			auto iter = prev_agg_vec.begin();
			int j = 0;

			while (iter != prev_agg_vec.end()) {
				auto it = iter->item_idx->begin();
				while (it != iter->item_idx->end()) {
					v[*it] = j;
					it++;
				}
				j++;
				iter++;
			}
			for (int y = 0; y < agg_vecs[level_][layer_].size(); y++) {
				auto it = agg_vecs[level_][layer_][y].item_idx->begin();
				// 1 create an item id  to prev_agg_vec index
				// 2 use below loop to create a set which will then be used to iterate and get previous_gul_total
				std::unordered_set<int> s;
				while (it != agg_vecs[level_][layer_][y].item_idx->end()) {
					s.insert(v[*it]);
					//prev_gul_total += prev_agg_vec[*it].loss;
					it++;
				}

				OASIS_FLOAT prev_gul_total = 0;
				auto s_iter = s.begin();
				while (s_iter != s.end()) {
					prev_gul_total += prev_agg_vec[*s_iter].loss;
					s_iter++;
				}

				// agg_vecs[level_][layer_][y].loss = prev_gul_total;
				it = agg_vecs[level_][layer_][y].item_idx->begin();
				while (it != agg_vecs[level_][layer_][y].item_idx->end()) {
					if (prev_gul_total > 0) {
						int j = -1;
						const std::vector<int> &z = *(prev_agg_vec[v[*it]].item_idx);
						for (int i = 0; i < z.size(); i++) {							
							if (z[i] == *it) {
								j = i;
								break;
							}
						}
						// this is recomputing the first layers proportions
						if (j > -1) {
							agg_vecs[level_][layer_][y].item_prop.push_back(prev_agg_vec[v[*it]].loss * prev_agg_vec[v[*it]].item_prop[j] / prev_gul_total);
						}
						else {
							agg_vecs[level_][layer_][y].item_prop.push_back(0);
						}

					}
					else {
						agg_vecs[level_][layer_][y].item_prop.push_back(0);
					}
					it++;
				}
			}
		}		
		return;

	}
}
/*



*/
//
// The item prop should not be needed so see if we can remove it all together - still used in function calc
void fmcalc::compute_item_proportions_old(std::vector<std::vector<std::vector <LossRec>>> &agg_vecs, const std::vector<OASIS_FLOAT> &guls,int level_, int layer_, int previous_layer_)
{

	if (layer_ == 1) {	// Level 1 is always item_id to agg_id so there will always be only 1 item_idx  and therefore the allcated percentage should be 1
		std::vector<OASIS_FLOAT> items_prop;
		items_prop.resize(guls.size(), 0);
		const int layer_id = 1;

		for (int level = 1; level < agg_vecs.size(); level++) {
			vector <LossRec> &agg_vec = agg_vecs[level][layer_id];
			auto iter = agg_vec.begin();
			while (iter != agg_vec.end()) {
				iter->item_prop.resize(guls.size(), 0);	// this is wrong should be same size as item_idx unless we are using the item_idx as an index into this
				iter++;
			}
		}
		for (int level = 1; level < agg_vecs.size(); level++) {
			vector <LossRec> &agg_vec = agg_vecs[level][layer_id];
			OASIS_FLOAT loss_total = 0;
			auto iter = agg_vec.begin();
			while (iter != agg_vec.end()) {
				OASIS_FLOAT total = 0;
				for (int idx : *(iter->item_idx)) {	// because this is the first level there should only be one item_idx
					total += guls[idx];
				}
				iter->gul_total = total;
				loss_total += iter->loss;
				iter++;
			}
			iter = agg_vec.begin();		// loop thru again
			while (iter != agg_vec.end()) {
				if ( iter->loss > 0 && loss_total > 0) {
					iter->proportion = iter->loss / loss_total;
				}
				else {
					iter->proportion = 0;
				}
				if (level == 1) {
					//iter->item_prop.resize((iter->item_idx)->size(), 0);
					OASIS_FLOAT total = 0;
					for (int idx : *(iter->item_idx)) {
						total += guls[idx];
					}
					int i = 0;
					if (total > 0) {
						for (int idx : *(iter->item_idx)) {
							iter->item_prop[idx] = guls[idx] / iter->gul_total;
							items_prop[idx] = (guls[idx] / total) * iter->proportion;
							//items_prop[idx] = (guls[idx] / iter->gul_total);
							i++;
						}
					}
				}
				else {
					OASIS_FLOAT prop_total = 0;
					for (int idx : *(iter->item_idx)) {
						prop_total += items_prop[idx];
					}
					int i = 0;
					// iter->item_prop.resize((iter->item_idx)->size(), 0);
					if (prop_total > 0) {
						for (int idx : *(iter->item_idx)) {
							iter->item_prop[idx] = items_prop[idx] / prop_total;
							items_prop[idx] = (items_prop[idx] / prop_total) * iter->proportion;
							i++;
						}
					}
				}
				iter++;
			}
		}
	}
	else {
		// This code is broken for fm22 - works for other examples except probably fm28
		vector <LossRec> &prev_agg_vec = agg_vecs[level_ - 1][previous_layer_];

		for (int y = 0; y < agg_vecs[level_][layer_].size(); y++) {
			auto it = agg_vecs[level_][layer_][y].item_idx->begin();
			OASIS_FLOAT prev_gul_total = 0;
			while (it != agg_vecs[level_][layer_][y].item_idx->end()) {
				prev_gul_total += prev_agg_vec[*it].loss;
				it++;
			}
			it = agg_vecs[level_][layer_][y].item_idx->begin();
			while (it != agg_vecs[level_][layer_][y].item_idx->end()) {
				if (prev_gul_total > 0) {
					agg_vecs[level_][layer_][y].item_prop.push_back(prev_agg_vec[*it].loss / prev_gul_total);
				}else{
					agg_vecs[level_][layer_][y].item_prop.push_back(0);
				}
				it++;
			}
		}
	}
}
inline void fmcalc::dofmcalc_r(std::vector<std::vector<int>>  &aggid_to_vectorlookups, vector<vector<vector <LossRec>>> &agg_vecs, int level, int max_level,
	std::map<fmlevelhdr, std::vector<fmlevelrec> > &outmap, fmlevelhdr &fmhdr, int gul_idx, const std::vector<std::vector<std::vector<policytcvidx>>> &avxs, int layer, 
	const std::vector<int> &items, std::vector<vector<OASIS_FLOAT>> &event_guls, int previous_level, int previous_layer)
{

	int previous_layer_id = 1;
	
	if (layer <= level_to_max_layer_[level - 1]) {
		previous_layer_id = layer;	// Only if the previous level has this layer id then use it otherwise use the first layer
	}

	//vector <LossRec> &prev_agg_vec = agg_vecs[level - 1][previous_layer_id];
	vector <LossRec> &agg_vec_previous_level = agg_vecs[level - 1][previous_layer_id];	// this will be either same layer as this level or layer 1
	vector <LossRec> &agg_vec_previous_layer = agg_vecs[level][previous_layer];	// current level but previous layer
	vector <LossRec> &agg_vec = agg_vecs[level][layer];
	const std::vector<std::vector<policytcvidx>> &avx = avxs[level];

	std::vector<float> previous_layer_retained_losses;
	
	if (layer > 1) {
		// on this level get the previous layers retained losses
		previous_layer_retained_losses.resize(agg_vec_previous_layer.size());
		for (unsigned int i = 0; i < agg_vec_previous_layer.size(); i++) {
			previous_layer_retained_losses[i] = agg_vec_previous_layer[i].retained_loss;
		}
	}

	agg_vec.clear();

	std::vector<int> &aggid_to_vectorlookup = aggid_to_vectorlookups[level];


	if (agg_vec.size() == 0) {
        int size = aggid_to_vectorlookup.size();
        agg_vec.resize(size);
	}
	else {
		for (unsigned int i = 0; i < agg_vec.size(); i++) {
			if (layer == 1) {
				agg_vec[i].original_loss = 0;
				agg_vec[i].loss = 0;
			}else{
				agg_vec[i].loss = agg_vec[i].retained_loss;
				agg_vec[i].original_loss = agg_vec[i].loss;
				agg_vec[i].retained_loss = 0;
			}
		}
	}

	for (unsigned int i = 0;i < agg_vec_previous_level.size();i++){ // loop through previous levels of agg_vec
		if (agg_vec_previous_level[i].agg_id != 0) {
			int agg_id = pfm_vec_vec_[level][agg_vec_previous_level[i].agg_id];
			if (layer < policy_tc_vec_vec_vec_[level][agg_id].size()) {
				if (agg_vec_previous_level[i].next_vec_idx == -1) {	// next_vec_idx can be zero
					agg_vec_previous_level[i].next_vec_idx = aggid_to_vectorlookup[agg_id - 1];
				}
				int vec_idx = agg_vec_previous_level[i].next_vec_idx;
				if (agg_vec[vec_idx].policytc_id != avx[layer][vec_idx].policytc_id) { // policytc_id cannot be zero - first position in lookup vector not used
					agg_vec[vec_idx].policytc_id = avx[layer][vec_idx].policytc_id;
					agg_vec[vec_idx].agg_id = agg_id;
					agg_vec[vec_idx].item_idx = &avx[layer][vec_idx].item_idx;
				}

				agg_vec[vec_idx].loss += agg_vec_previous_level[i].loss;
				agg_vec[vec_idx].retained_loss += agg_vec_previous_level[i].retained_loss;				
				agg_vec[vec_idx].accumulated_tiv += agg_vec_previous_level[i].accumulated_tiv;
				agg_vec[vec_idx].effective_deductible += agg_vec_previous_level[i].effective_deductible;
			}	
		}
		else {
			// this is valid when full array was not populated in previous level
			//fprintf(stderr, "Missing agg_id for index %d policytc_id: %d\n", i, prev_agg_vec[i].policytc_id);

		}
	}
	
	if (layer > 1) {
		if (agg_vec.size() == previous_layer_retained_losses.size()) {
			for (unsigned int i = 0; i < agg_vec.size(); i++) {
				agg_vec[i].previous_layer_retained_loss = previous_layer_retained_losses[i];
			}
		}
	}

	
	if (oldFMProfile_ == true) {
		dofmcalc_old(agg_vec, layer);
	}
	else {
		dofmcalc_new(agg_vec, layer);
	}

	if (level == max_level) {
		int sidx = gul_idx - 1;
		if (sidx == -1) sidx = tiv_idx;
		if (sidx == 0) sidx = mean_idx;		
		fmlevelrec rec;
        rec.loss = 0;
		rec.sidx = sidx;		
		if (netvalue_ && layer > 1) {
			for (int i = 0; i < agg_vec.size(); i++) {
				agg_vec[i].item_net = agg_vec_previous_layer[i].item_net;
			}
		}
		for (LossRec &x : agg_vec) {
			if (netvalue_ && layer == 1) {
				const std::vector<OASIS_FLOAT> &guls = event_guls[gul_idx];
				x.item_net.resize(x.item_idx->size(), 0);
				const std::vector<int> &z = *(x.item_idx);
				for (int i = 0; i < x.item_net.size(); i++) {					
					x.item_net[i]= guls[z[i]];
				}								
			}			
			if (x.allocrule_id == -1 || x.allocrule_id == 0 ) { // no back allocation
				if (x.agg_id > 0) {	// agg id cannot be zero
					if (netvalue_) { // get net gul value							
						rec.loss = x.retained_loss;
						rec.loss = x.net_loss;
					}else {
						rec.loss = x.loss;
					}
					if (rec.loss > 0.0 || rec.sidx < 0) {
						fmxref_key k;
						k.layer_id = layer;
						k.agg_id = x.agg_id;
						auto it = fm_xrefmap.find(k);
						if (it == fm_xrefmap.end()) {
							fmhdr.output_id = k.agg_id;
						}
						else {
							fmhdr.output_id = it->second;
						}						
						if (netvalue_) {
							if (layer == max_layer_) outmap[fmhdr].push_back(rec);			// neglible cost
						}else {
							outmap[fmhdr].push_back(rec);			// neglible cost
						}
					}
				}
			}			
			// at level 1 allocrule 2 and 1 are the same
			if (x.allocrule_id == 2 && level == 1) x.allocrule_id = 1; 

			if (x.allocrule_id == 1 ) {	// back allocate as a proportion of the total of the original guls	
				OASIS_FLOAT gultotal = 0;
				int index = gul_idx;	// default index top level
				if (sidx == tiv_idx) index = 0;
				const std::vector<OASIS_FLOAT> &guls = event_guls[index];
				int vec_idx = aggid_to_vectorlookup[x.agg_id-1];		// Same index applies to avx as to agg_vec
				for (int idx : avx[layer][vec_idx].item_idx) {
					gultotal += guls[idx];
				}
				for (int idx : avx[layer][vec_idx].item_idx) {
                    OASIS_FLOAT prop = 0;
                    if (gultotal > 0) prop = guls[idx] / gultotal;
					//fmhdr.output_id = items[idx];					
					if (netvalue_) { // get net gul value
						if (level == 1 && layer==1) {
							// because no calc rules have been applied yet there is no retained loss yet
							rec.loss = guls[idx] - (x.loss * prop);
						}else {							
							rec.loss = x.net_loss * prop;
						}

					}else {
						rec.loss = x.loss * prop;
					}
					if (rec.loss > 0.0 || rec.sidx < 0) {
						fmxref_key k;
						k.layer_id = layer;
						k.agg_id = items[idx];
						auto it = fm_xrefmap.find(k);
						if (it == fm_xrefmap.end()) {
							fmhdr.output_id = k.agg_id;
						}
						else {
							fmhdr.output_id = it->second;
						}

						if (netvalue_) {
							if (layer == max_layer_) outmap[fmhdr].push_back(rec);			// neglible cost
						}
						else {
							outmap[fmhdr].push_back(rec);			// neglible cost
						}
					}
				}				
			}

			if (x.allocrule_id == 2) {		// back allocate as a proportion of the total of the previous losses			
				int vec_idx = aggid_to_vectorlookup[x.agg_id - 1];																				
				const std::vector<OASIS_FLOAT> &guls = event_guls[gul_idx];
				compute_item_proportions(agg_vecs, guls, level, layer, previous_layer_id);
				if (x.item_prop.size() > 0) {
					for (int i=0;i < avx[layer][vec_idx].item_idx.size();i++){
						int idx = avx[layer][vec_idx].item_idx[i];
					//for (int idx : avx[layer][vec_idx].item_idx) {
						OASIS_FLOAT prop = x.item_prop[i];		// this points to the final level and  current layer of agg_vecs breaks on layer id 2
						if (netvalue_) { // get net gul value							
							x.item_net[i] = x.item_net[i] - (x.loss * prop);
							if (x.item_net[i] < 0) x.item_net[i] = 0;
							rec.loss = x.item_net[i];
						}
						else {
							rec.loss = x.loss * prop;
						}
						if (rec.loss > 0.0 || rec.sidx < 0) {
							fmxref_key k;
							k.layer_id = layer;
							k.agg_id = items[idx];
							auto it = fm_xrefmap.find(k);
							if (it == fm_xrefmap.end()) {
								fmhdr.output_id = k.agg_id;
							}
							else {
								fmhdr.output_id = it->second;
							}
							if (netvalue_) {
								if (layer == max_layer_) outmap[fmhdr].push_back(rec);			// neglible cost
							}
							else {
								outmap[fmhdr].push_back(rec);			// neglible cost
							}
						}
					}
				}
				else {
					fprintf(stderr, "Error: item_prop is zero!!");
				}

			}
		}
		if (layer < max_layer_) {
			int previous_level = level;
			int previous_layer = layer;
			layer++;
			dofmcalc_r(aggid_to_vectorlookups, agg_vecs, level, max_level, outmap, fmhdr, gul_idx, avxs, layer,items, event_guls, previous_level, previous_layer);
		}
	}
	else {		
		if (layer < level_to_max_layer_[level]) {
			int previous_level = level;
			int previous_layer = layer;
			layer++;
			dofmcalc_r(aggid_to_vectorlookups, agg_vecs, level, max_level, outmap, fmhdr, gul_idx, avxs, layer, items, event_guls, previous_level, previous_layer);
		}
		else {
			int previous_level = level;
			int previous_layer = layer;
			level++;
			layer = 1;
			dofmcalc_r(aggid_to_vectorlookups, agg_vecs, level, max_level, outmap, fmhdr, gul_idx, avxs, layer, items, event_guls, previous_level, previous_layer);
		}		
	}
}

inline void fmcalc::dofmcalc_r_old(std::vector<std::vector<int>>  &aggid_to_vectorlookups, vector<vector<vector <LossRec>>> &agg_vecs, int level, int max_level,
	std::map<fmlevelhdr, std::vector<fmlevelrec> > &outmap, fmlevelhdr &fmhdr, int gul_idx, const std::vector<std::vector<std::vector<policytcvidx>>> &avxs, int layer,
	const std::vector<int> &items, std::vector<vector<OASIS_FLOAT>> &event_guls, int previous_level, int previous_layer)
{
	int previous_layer_id = 1;

	if (layer <= level_to_max_layer_[level - 1]) {
		previous_layer_id = layer;	// Only if the previous layer has this layer id then use it otherwise use the first layer
	}

	vector <LossRec> &prev_agg_vec = agg_vecs[level - 1][previous_layer_id];
	vector <LossRec> &agg_vec_previous_layer = agg_vecs[level][previous_layer_id];	// current level but previous layer
	vector <LossRec> &agg_vec = agg_vecs[level][layer];
	const std::vector<std::vector<policytcvidx>> &avx = avxs[level];

	std::vector<float> previous_retained_losses;

	previous_retained_losses.resize(agg_vec_previous_layer.size());
	if (layer > 1) {
		for (unsigned int i = 0; i < agg_vec_previous_layer.size(); i++) {
			previous_retained_losses[i] = agg_vec_previous_layer[i].retained_loss;
		}
	}

	agg_vec.clear();

	std::vector<int> &aggid_to_vectorlookup = aggid_to_vectorlookups[level];


	if (agg_vec.size() == 0) {
		int size = aggid_to_vectorlookup.size();
		agg_vec.resize(size);
	}
	else {
		for (unsigned int i = 0; i < agg_vec.size(); i++) {
			if (layer == 1) {
				agg_vec[i].original_loss = 0;
				agg_vec[i].loss = 0;
			}
			else {
				agg_vec[i].loss = agg_vec[i].retained_loss;
				agg_vec[i].original_loss = agg_vec[i].loss;
				agg_vec[i].retained_loss = 0;
			}
		}
	}

	for (unsigned int i = 0; i < prev_agg_vec.size(); i++) { // loop through previous levels of agg_vec
		if (prev_agg_vec[i].agg_id != 0) {
			int agg_id = pfm_vec_vec_[level][prev_agg_vec[i].agg_id];
			if (layer < policy_tc_vec_vec_vec_[level][agg_id].size()) {
				if (prev_agg_vec[i].next_vec_idx == -1) {	// next_vec_idx can be zero
					prev_agg_vec[i].next_vec_idx = aggid_to_vectorlookup[agg_id - 1];
				}
				int vec_idx = prev_agg_vec[i].next_vec_idx;
				if (agg_vec[vec_idx].policytc_id != avx[layer][vec_idx].policytc_id) { // policytc_id cannot be zero - first position in lookup vector not used
					agg_vec[vec_idx].policytc_id = avx[layer][vec_idx].policytc_id;
					agg_vec[vec_idx].agg_id = agg_id;
					agg_vec[vec_idx].item_idx = &avx[layer][vec_idx].item_idx;
				}

				agg_vec[vec_idx].loss += prev_agg_vec[i].loss;
				agg_vec[vec_idx].retained_loss += prev_agg_vec[i].retained_loss;
			}
		}
		else {
			// this is valid when full array was not populated in previous level
			//fprintf(stderr, "Missing agg_id for index %d policytc_id: %d\n", i, prev_agg_vec[i].policytc_id);

		}
	}

	if (layer > 1) {
		if (agg_vec.size() == previous_retained_losses.size()) {
			for (unsigned int i = 0; i < agg_vec.size(); i++) {
				agg_vec[i].previous_layer_retained_loss = previous_retained_losses[i];
			}
		}
	}

	dofmcalc_old(agg_vec, layer);

	if (level == max_level) {
		int sidx = gul_idx - 1;
		if (sidx == -1) sidx = tiv_idx;
		if (sidx == 0) sidx = mean_idx;
		fmlevelrec rec;
		rec.loss = 0;
		rec.sidx = sidx;
		for (LossRec &x : agg_vec) {
			if (x.allocrule_id == -1 || x.allocrule_id == 0) { // no back allocation
				if (x.agg_id > 0) {	// agg id cannot be zero
					if (netvalue_) { // get net gul value							
						rec.loss = x.retained_loss;
						rec.loss = x.net_loss;
					}
					else {
						rec.loss = x.loss;
					}
					if (rec.loss > 0.0 || rec.sidx < 0) {
						fmxref_key k;
						k.layer_id = layer;
						k.agg_id = x.agg_id;
						auto it = fm_xrefmap.find(k);
						if (it == fm_xrefmap.end()) {
							fmhdr.output_id = k.agg_id;
						}
						else {
							fmhdr.output_id = it->second;
						}
						if (netvalue_) {
							if (layer == max_layer_) outmap[fmhdr].push_back(rec);			// neglible cost
						}
						else {
							outmap[fmhdr].push_back(rec);			// neglible cost
						}
					}
				}
			}
			// at level 1 allocrule 2 and 1 are the same
			if (x.allocrule_id == 2 && level == 1) x.allocrule_id = 1;

			if (x.allocrule_id == 1) {	// back allocate as a proportion of the total of the original guls	
				OASIS_FLOAT gultotal = 0;
				int index = gul_idx;	// default index top level
				if (sidx == tiv_idx) index = 0;
				const std::vector<OASIS_FLOAT> &guls = event_guls[index];
				int vec_idx = aggid_to_vectorlookup[x.agg_id - 1];		// Same index applies to avx as to agg_vec
				for (int idx : avx[layer][vec_idx].item_idx) {
					gultotal += guls[idx];
				}
				for (int idx : avx[layer][vec_idx].item_idx) {
					OASIS_FLOAT prop = 0;
					if (gultotal > 0) prop = guls[idx] / gultotal;
					//fmhdr.output_id = items[idx];					
					if (netvalue_) { // get net gul value
						if (level == 1 && layer == 1) {
							// because no calc rules have been applied yet there is no retained loss yet
							rec.loss = guls[idx] - (x.loss * prop);
						}
						else {
							rec.loss = x.net_loss * prop;
						}

					}
					else {
						rec.loss = x.loss * prop;
					}
					if (rec.loss > 0.0 || rec.sidx < 0) {
						fmxref_key k;
						k.layer_id = layer;
						k.agg_id = items[idx];
						auto it = fm_xrefmap.find(k);
						if (it == fm_xrefmap.end()) {
							fmhdr.output_id = k.agg_id;
						}
						else {
							fmhdr.output_id = it->second;
						}

						if (netvalue_) {
							if (layer == max_layer_) outmap[fmhdr].push_back(rec);			// neglible cost
						}
						else {
							outmap[fmhdr].push_back(rec);			// neglible cost
						}
					}
				}
			}

			if (x.allocrule_id == 2) {		// back allocate as a proportion of the total of the previous losses			
				int vec_idx = aggid_to_vectorlookup[x.agg_id - 1];
				const std::vector<OASIS_FLOAT> &guls = event_guls[gul_idx];
				compute_item_proportions(agg_vecs, guls, level, layer, previous_layer_id);
				if (x.item_prop.size() > 0) {
					for (int idx : avx[layer][vec_idx].item_idx) {
						OASIS_FLOAT prop = x.item_prop[idx];		// this points to the final level and  current layer of agg_vecs breaks on layer id 2
						if (netvalue_) { // get net gul value							
							rec.loss = x.retained_loss * prop;
						}
						else {
							rec.loss = x.loss * prop;
						}
						if (rec.loss > 0.0 || rec.sidx < 0) {
							fmxref_key k;
							k.layer_id = layer;
							k.agg_id = items[idx];
							auto it = fm_xrefmap.find(k);
							if (it == fm_xrefmap.end()) {
								fmhdr.output_id = k.agg_id;
							}
							else {
								fmhdr.output_id = it->second;
							}
							if (netvalue_) {
								if (layer == max_layer_) outmap[fmhdr].push_back(rec);			// neglible cost
							}
							else {
								outmap[fmhdr].push_back(rec);			// neglible cost
							}
						}
					}
				}
				else {
					fprintf(stderr, "Error: item_prop is zero!!");
				}

			}
		}
		if (layer < max_layer_) {
			int previous_level = level;
			int previous_layer = layer;
			layer++;
			dofmcalc_r_old(aggid_to_vectorlookups, agg_vecs, level, max_level, outmap, fmhdr, gul_idx, avxs, layer, items, event_guls, previous_level, previous_layer);
		}
	}
	else {
		if (layer < level_to_max_layer_[level]) {
			int previous_level = level;
			int previous_layer = layer;
			layer++;
			dofmcalc_r_old(aggid_to_vectorlookups, agg_vecs, level, max_level, outmap, fmhdr, gul_idx, avxs, layer, items, event_guls, previous_level, previous_layer);
		}
		else {
			int previous_level = level;
			int previous_layer = layer;
			level++;
			layer = 1;
			dofmcalc_r_old(aggid_to_vectorlookups, agg_vecs, level, max_level, outmap, fmhdr, gul_idx, avxs, layer, items, event_guls, previous_level, previous_layer);
		}
	}
}


bool fmcalc::gulhasvalue(const std::vector<OASIS_FLOAT> &gul) const
{
	for (OASIS_FLOAT i : gul) if (i > 0) return true;
	return false;
}
void fmcalc::dofm(int event_id, const std::vector<int> &items, std::vector<vector<OASIS_FLOAT>> &event_guls)
{
	
	const int level = 0;
	std::vector<std::vector<int>>  aggid_to_vectorlookups(maxLevel_ + 1);

	std::vector<int> &aggid_to_vectorlookup = aggid_to_vectorlookups[level];	
	int size = level_to_maxagg_id_[level];
	if (size == -1) {
		fprintf(stderr, "Error: Possible level %d not initialized\n", level);
		exit(-1);
	}
	else {
		aggid_to_vectorlookup.resize(size, -2);
	}

	const int layer_id = 1;
	std::vector<std::vector<std::vector<policytcvidx>>> avxs;
	int total_loss_items = 0;
    avxs.resize(maxLevel_ + 1);
	if (event_guls.size() > 0) {
		auto &guls = event_guls[0];		// pick any set of events they'll all have the same size we just want the size
		std::vector<std::vector<policytcvidx>> &avx = avxs[level];

		avx.resize(max_layer_+1);
		for (unsigned int i = 0;i < guls.size();i++) {
			int agg_id = pfm_vec_vec_[level][items[i]];

			int current_idx = aggid_to_vectorlookup[agg_id-1];
			if (current_idx == -2) {
				policytcvidx a;
				a.agg_id = agg_id;
				a.policytc_id = policy_tc_vec_vec_vec_[level][agg_id][layer_id];
				avx[layer_id].push_back(a);			// populate the first layer
				avx[layer_id][avx[layer_id].size() - 1].item_idx.push_back(i);
				aggid_to_vectorlookup[agg_id-1] = avx[layer_id].size() - 1;
				total_loss_items++;
			}
			else {
				avx[layer_id][current_idx].item_idx.push_back(i);
			}
			
		}
		       
		for (unsigned int zzlevel = 1;zzlevel < avxs.size();zzlevel++) {
			std::vector<std::vector<policytcvidx>>  &prev = avxs[zzlevel-1];
			std::vector<std::vector<policytcvidx>>  &next = avxs[zzlevel];
			next.resize(max_layer_ + 1);

			std::vector<int> &zzaggid_to_vectorlookup = aggid_to_vectorlookups[zzlevel]; // 
			int size = level_to_maxagg_id_[zzlevel];
			if (size == -1) {
				fprintf(stderr, "Error: Possible level %d not initialized\n", zzlevel);
				exit(-1);
			}
			else {
				zzaggid_to_vectorlookup.resize(size, -2);
			}
			
			int previous_layer = 1; // previous layer is always one because only the last layer can be greater than 1

			for (unsigned int i = 0;i < prev[previous_layer].size();i++) {
				int agg_id = pfm_vec_vec_[zzlevel][prev[previous_layer][i].agg_id];
				int current_idx = zzaggid_to_vectorlookup[agg_id-1];
				if (current_idx == -2)		// field not initialized
				{

					for (unsigned int layer = 1; layer < policy_tc_vec_vec_vec_[zzlevel][agg_id].size() ; layer++ ){ // loop through layers
						struct policytcvidx a;
						const int policytc_id = policy_tc_vec_vec_vec_[zzlevel][agg_id][layer];

						a.agg_id = agg_id;
						a.policytc_id = policytc_id;

						next[layer].push_back(a);
						
						int current_idx = next[layer].size() - 1;
						for (int x : prev[previous_layer][i].item_idx) {
							next[layer][current_idx].item_idx.push_back(x);	
						}
						zzaggid_to_vectorlookup[agg_id-1] = next[layer].size() - 1; // temp use for getting 
					}						
				}
				else {
					for (unsigned int layer = 1; layer < policy_tc_vec_vec_vec_[zzlevel][agg_id].size(); layer++) { // loop through layers
						const int policytc_id = policy_tc_vec_vec_vec_[zzlevel][agg_id][layer];
						for (int x : prev[previous_layer][i].item_idx) {
							next[layer][current_idx].item_idx.push_back(x);	
						}
					}

				}
			}
		}
	}

	vector<vector<vector <LossRec>>>agg_vecs(maxLevel_ + 1);
	for (int i = 0; i < maxLevel_ + 1; i++) {
		agg_vecs[i].resize(max_layer_ + 1);
	}
	vector <LossRec> &agg_vec = agg_vecs[level][layer_id];
	std::map<fmlevelhdr, std::vector<fmlevelrec>> outmap;
	
	for (unsigned int gul_idx = 0; gul_idx < event_guls.size(); gul_idx++) {	// loop sample + 1 times
		const std::vector<OASIS_FLOAT> &guls = event_guls[gul_idx];
		if (gul_idx < 2 || gulhasvalue(guls)) {
			agg_vec.resize(total_loss_items);
			fmlevelhdr fmhdr;
			fmhdr.event_id = event_id;
			const std::vector<std::vector<policytcvidx>> &avx = avxs[level];
			for (unsigned int i = 0; i < agg_vec.size(); i++) agg_vec[i].loss = 0;
			int last_agg_id = -1;
			int vec_idx = 0;
			for (unsigned int i = 0; i < guls.size(); i++) {
				int agg_id = pfm_vec_vec_[level][items[i]];
				if (last_agg_id != agg_id) {
					vec_idx = aggid_to_vectorlookup[agg_id - 1];
					last_agg_id = agg_id;
				}
				agg_vec[vec_idx].loss += guls[i];
				int vid = aggid_to_vectorlookup[agg_id - 1];
				agg_vec[vec_idx].agg_id = avx[1][vid].agg_id;
				agg_vec[vec_idx].item_idx = &avx[1][vid].item_idx;
				agg_vec[vec_idx].accumulated_tiv = item_to_tiv_[avx[1][vid].item_idx[0]+1];
				agg_vec[vec_idx].policytc_id = avx[1][vid].policytc_id;
			}

//			dofmcalc(agg_vec);					
			
			dofmcalc_r(aggid_to_vectorlookups, agg_vecs, level + 1, maxLevel_, outmap, fmhdr, gul_idx, avxs, 1, items, event_guls,0,1);

			agg_vec.clear();
		}
	}
	aggid_to_vectorlookups.clear();
	// Do output
	for (const auto &x : outmap) {
		fwrite(&x.first, sizeof(x.first), 1, stdout);
		for (const auto &y : x.second) {
			fwrite(&y, sizeof(y), 1, stdout);
		}
		fmlevelrec r;
		r.sidx = 0;
		r.loss = 0;
		fwrite(&r, sizeof(r), 1, stdout); // terminate rows
	}
}

void fmcalc::addtcrow(const fm_policyTC &f)
{
	if (f.level_id <= maxRunLevel_) {
		if (f.level_id > max_level_) max_level_ = f.level_id;
		if (f.layer_id > max_layer_) max_layer_ = f.layer_id;
		if (f.level_id >= (int) level_to_max_layer_.size() ) {	
			level_to_max_layer_.resize(f.level_id + 1);
			level_to_max_layer_[f.level_id] = 0;
		}
		if (level_to_max_layer_[f.level_id] < f.layer_id) level_to_max_layer_[f.level_id] = f.layer_id;

		if (f.agg_id > max_agg_id_) max_agg_id_ = f.agg_id;

		if (policy_tc_vec_vec_vec_.size() < f.level_id + 1) {
			policy_tc_vec_vec_vec_.resize(f.level_id + 1);
		}
		if (policy_tc_vec_vec_vec_[f.level_id].size() < f.agg_id + 1) {
			policy_tc_vec_vec_vec_[f.level_id].resize(f.agg_id + 1);
		}

		if (policy_tc_vec_vec_vec_[f.level_id][f.agg_id].size() < f.layer_id + 1) {
			policy_tc_vec_vec_vec_[f.level_id][f.agg_id].resize(f.layer_id + 1);
		}
		policy_tc_vec_vec_vec_[f.level_id][f.agg_id][f.layer_id] = f.PolicyTC_id;
	}

}
void fmcalc::init_policytc(int maxRunLevel)
{
    if (maxRunLevel != -1) maxRunLevel_ = maxRunLevel;
	FILE *fin = NULL;
	std::string file = FMPOLICYTC_FILE;
	if (inputpath_.length() > 0) {
		file = inputpath_ + file.substr(5);
	}
	fin = fopen(file.c_str(), "rb");

	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, file.c_str());
		exit(EXIT_FAILURE);
	}
	max_level_ = 0;
	max_layer_ = 0;
	max_agg_id_ = 0;
	fm_policyTC f;
	f.layer_id = 1;
	f.level_id = 0;
	f.PolicyTC_id = -1;
	for (int i = 1; i <= level_to_maxagg_id_[0]; i++) {		
		f.agg_id = i;		
		addtcrow(f);
	}
	
	int i = fread(&f, sizeof(f), 1, fin);
	while (i != 0) {
		addtcrow(f);   
		i = fread(&f, sizeof(f), 1, fin);
	}
	fclose(fin);

}

void fmcalc::init_programme(int maxRunLevel)
{

	FILE *fin = NULL;
	std::string file = FMPROGRAMME_FILE;
	if (inputpath_.length() > 0) {
		file = inputpath_ + file.substr(5);
	}
	fin = fopen(file.c_str(), "rb");
	if (fin == NULL){
		fprintf(stderr, "%s: cannot open %s\n", __func__, file.c_str());
		exit(EXIT_FAILURE);
	}

	fm_programme f;
    if (maxRunLevel != -1) maxRunLevel_ = maxRunLevel;

	if (level_to_maxagg_id_.size() == 0) {
		level_to_maxagg_id_.resize(1, -1);
	}
	int i = fread(&f, sizeof(f), 1, fin);
	while (i != 0) {
        if (f.level_id <= maxRunLevel_){
			if (maxLevel_ < f.level_id) {
				maxLevel_ = f.level_id;
				level_to_maxagg_id_.resize(maxLevel_ + 1, -1);
			}
			if (f.level_id == 1) {	
				if (f.from_agg_id > level_to_maxagg_id_[0])level_to_maxagg_id_[0] = f.from_agg_id;
				if (pfm_vec_vec_.size() < 1) {
					pfm_vec_vec_.resize(1);
				}
				if (pfm_vec_vec_[0].size() < f.from_agg_id + 1) {
					pfm_vec_vec_[0].resize(f.from_agg_id + 1);
				}
				pfm_vec_vec_[0][f.from_agg_id] = f.from_agg_id;
			}					
			if (f.to_agg_id > level_to_maxagg_id_[f.level_id])level_to_maxagg_id_[f.level_id] = f.to_agg_id;
            if (pfm_vec_vec_.size() < f.level_id + 1) {
                pfm_vec_vec_.resize(f.level_id + 1);
            }
            if (pfm_vec_vec_[f.level_id].size() < f.from_agg_id + 1) {
                pfm_vec_vec_[f.level_id].resize(f.from_agg_id + 1);
            }
            pfm_vec_vec_[f.level_id][f.from_agg_id] = f.to_agg_id;
            if (f.to_agg_id == 0) {
                fprintf(stderr, "Invalid agg id from fm_programme.bin\n");
            }
        }
		i = fread(&f, sizeof(f), 1, fin);
	}
	fclose(fin);

}

void fmcalc::add_tc(unsigned char tc_id, OASIS_FLOAT &tc_val, std::vector<tc_rec> &tc_vec)
{
	if (tc_val > -1) {
		tc_rec t;
		t.tc_id = tc_id;
		t.tc_val = tc_val;
		tc_vec.push_back(t);
	}
}

bool fmcalc::loadcoverages(std::vector<OASIS_FLOAT> &coverages)
{

	FILE *fin = NULL;
	std::string file = COVERAGES_FILE;
	if (inputpath_.length() > 0) {
		file = inputpath_ + file.substr(5);
	}
	fin = fopen(file.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, file.c_str());
		exit(EXIT_FAILURE);
	}
	
	flseek(fin, 0L, SEEK_END);
	long long sz = fltell(fin);
	flseek(fin, 0L, SEEK_SET);

	OASIS_FLOAT tiv;
	unsigned int nrec = sz / sizeof(tiv);

	coverages.resize(nrec + 1);
	int coverage_id = 0;
	int i = fread(&tiv, sizeof(tiv), 1, fin);
	while (i != 0) {
		coverage_id++;
		coverages[coverage_id] = tiv;
		i = fread(&tiv, sizeof(tiv), 1, fin);
	}

	fclose(fin);
	return true;

}
int fmcalc::getmaxnoofitems()
{

	FILE *fin = NULL;
	std::string file = ITEMS_FILE;
	if (inputpath_.length() > 0) {
		file = inputpath_ + file.substr(5);
	}
	fin = fopen(file.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, file.c_str());
		exit(EXIT_FAILURE);
	}

	flseek(fin, 0L, SEEK_END);
	long long p = fltell(fin);
	p = p / sizeof(item);
	fclose(fin);
	return int(p);

}

void fmcalc::init_itemtotiv()
{
	std::vector<OASIS_FLOAT> coverages;
	loadcoverages(coverages);

	FILE *fin = NULL;
	std::string file = ITEMS_FILE;
	if (inputpath_.length() > 0) {
		file = inputpath_ + file.substr(5);
	}
	fin = fopen(file.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, file.c_str());
		exit(EXIT_FAILURE);
	}

	flseek(fin, 0L, SEEK_END);
	long long sz = fltell(fin);
	flseek(fin, 0L, SEEK_SET);
	int last_item_id = 0;
	unsigned int nrec = sz / sizeof(item);
	item_to_tiv_.resize(nrec+1,0.0);

	item itm;
	int i = fread(&itm, sizeof(itm), 1, fin);
	while (i != 0) {
		last_item_id++;
		if (itm.id != last_item_id) {
			fprintf(stderr, "Item ids are not contiguous or do not start from one");
			exit(-1);
		}
		last_item_id = itm.id;
		item_to_tiv_[itm.id] = coverages[itm.coverage_id];
		i = fread(&itm, sizeof(itm), 1, fin);
	}
	fclose(fin);

}

void fmcalc::init_profile_old()
{

	FILE *fin = NULL;
	std::string file = FMPROFILE_FILE_OLD;
	if (inputpath_.length() > 0) {
		file = inputpath_ + file.substr(5);
	}
	fin = fopen(file.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, file.c_str());
		exit(EXIT_FAILURE);
	}

	fm_profile f;
	int i = fread(&f, sizeof(f), 1, fin);
	while (i != 0) {
		profile_rec p;
		p.allocrule_id = f.allocrule_id;
		p.calcrule_id = f.calcrule_id;
		p.ccy_id = f.ccy_id;

		add_tc(deductible, f.deductible, p.tc_vec);
		add_tc(limit, f.limits, p.tc_vec);
		add_tc(share_prop_of_limit, f.share_prop_of_lim, p.tc_vec);		
		add_tc(deductible_prop_of_loss, f.deductible_prop_of_loss, p.tc_vec);
		add_tc(limit_prop_of_loss, f.limit_prop_of_loss, p.tc_vec);
		add_tc(deductible_prop_of_tiv, f.deductible_prop_of_tiv, p.tc_vec);
		add_tc(limit_prop_of_tiv, f.limit_prop_of_tiv, p.tc_vec);
		add_tc(deductible_prop_of_limit, f.deductible_prop_of_limit, p.tc_vec);

		if (profile_vec_.size() < f.policytc_id+1) {
			profile_vec_.resize(f.policytc_id + 1);
		}
		profile_vec_[f.policytc_id] = p;
	
		i = fread(&f, sizeof(f), 1, fin);
	}
	fclose(fin);
}

void fmcalc::init_fmxref()
{
	FILE *fin = NULL;
	std::string file = FMXREF_FILE;
	if (inputpath_.length() > 0) {
		file = inputpath_ + file.substr(5);
	}
	fin = fopen(file.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, file.c_str());
		exit(EXIT_FAILURE);
	}

	fmXref f;
	int i = fread(&f, sizeof(f), 1, fin);
	while (i != 0) {
		fmxref_key k;
		k.agg_id = f.agg_id;
		k.layer_id = f.layer_id;
		if (k.agg_id == f.output_id && k.layer_id == 1) {
			// skip it
		}else {
			fm_xrefmap[k] = f.output_id;
		}
		
		i = fread(&f, sizeof(f), 1, fin);
	}
	fclose(fin);	
}

void fmcalc::init(int MaxRunLevel)
{    
    init_programme(MaxRunLevel);
	init_policytc(MaxRunLevel);
	init_fmxref();
	if (oldFMProfile_ == true) {
		init_profile_old();
	}
	else {
		init_profile_new();
	}
	init_itemtotiv();
}

