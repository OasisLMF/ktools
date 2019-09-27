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
#include <set>
#include <unordered_set>
#include <algorithm>

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

struct layer_agg {
	int layer_id;
	int agg_id;
};

bool operator<(const layer_agg& lhs, const layer_agg& rhs)
{
	if (lhs.layer_id != rhs.layer_id)  return lhs.layer_id < rhs.layer_id;
	return lhs.agg_id < rhs.agg_id;		// should never reach here since these two are always equal 
}

bool operator<(const fm_policyTC& lhs, const fm_policyTC& rhs)
{
	if (lhs.layer_id != rhs.layer_id) return lhs.layer_id < rhs.layer_id;
	if (lhs.level_id != rhs.level_id) return lhs.level_id < rhs.level_id;
	return lhs.agg_id < rhs.agg_id;
}

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

//
// 
void fmcalc::compute_item_proportions(std::vector<std::vector<std::vector <LossRec>>> &agg_vecs, const std::vector<OASIS_FLOAT> &guls, unsigned int level_, unsigned int layer_,unsigned int previous_layer_)
{

	if (layer_ == 1) {
		std::vector<OASIS_FLOAT> items_prop (guls.size(), 0);

		for (unsigned int level = 1; level < agg_vecs.size(); level++) {
			vector <LossRec> &agg_vec = agg_vecs[level][layer_];
			vector <LossRec>& previous_agg_vec = agg_vecs[level-1][layer_];
			OASIS_FLOAT loss_total = 0;
			OASIS_FLOAT retained_loss_total = 0;
			OASIS_FLOAT previous_proportions_total = 0;
			auto p_iter = previous_agg_vec.begin();
			while (p_iter != previous_agg_vec.end()) {
				previous_proportions_total += p_iter->proportion;
				p_iter++;
			}
			auto iter = agg_vec.begin();
			while (iter != agg_vec.end()) {
				OASIS_FLOAT total = 0;
				if (iter->item_idx) {
					if (iter->item_prop == nullptr) {
						iter->item_prop = std::make_shared<std::vector<OASIS_FLOAT>>(std::vector<OASIS_FLOAT>((iter->item_idx)->size(), 0));
					}else {
						(iter->item_prop)->resize((iter->item_idx)->size(), 0);
					}
					for (int idx : *(iter->item_idx)) {	// because this is the first level there should only be one item_idx
						total += guls[idx];
					}
				}
				iter->gul_total = total;
				loss_total += iter->loss;
				retained_loss_total += iter->retained_loss;
				iter++;
			}
			if (loss_total <= 0 && previous_proportions_total > 0.01) {
				// if there is loss_total loss just copy previous level proportions
				auto iter = agg_vec.begin();		// loop thru again to work out proportion
				auto previous_iter = previous_agg_vec.begin();		// loop thru again to work out proportion
				while (iter != agg_vec.end()) {
					if (iter->item_idx) {
						for (int idx : *(iter->item_idx)) {
							std::vector<OASIS_FLOAT>& v = *(iter->item_prop);
							iter->proportion = iter->proportion + items_prop[idx];
						}
					}
					iter++;
				}
			}
			else {
				iter = agg_vec.begin();		// loop thru again to work out proportion
				while (iter != agg_vec.end()) {
					if (iter->loss > 0 && loss_total > 0) {
						iter->proportion = iter->loss / loss_total;
					}
					else {
						iter->proportion = 0;
					}
					if (level == 1) {
						OASIS_FLOAT total = 0;
						if (iter->item_idx) {
							for (int idx : *(iter->item_idx)) {
								total += guls[idx];
							}
						}
						int i = 0;
						if (total > 0) {
							for (int idx : *(iter->item_idx)) {
								std::vector<OASIS_FLOAT>& v = *(iter->item_prop);
								v[i] = guls[idx] / iter->gul_total;
								items_prop[idx] = (guls[idx] / total) * iter->proportion;
								i++;
							}
						}
					}
					else {
						OASIS_FLOAT prop_total = 0;
						if (iter->item_idx) {
							for (int idx : *(iter->item_idx)) {
								prop_total += items_prop[idx];
							}
						}
						int i = 0;
						if (prop_total > 0) {
							if (iter->item_idx) {
								for (int idx : *(iter->item_idx)) {
									std::vector<OASIS_FLOAT>& v = *(iter->item_prop);
									v[i] = items_prop[idx] / prop_total;
									items_prop[idx] = (items_prop[idx] / prop_total) * iter->proportion;
									i++;
								}
							}
						}
					}
					iter++;
				}
			}			
		}
	}
	else {
		if (previous_layer_ < layer_) {
			vector <LossRec> &prev_agg_vec = agg_vecs[level_][1];
			vector <LossRec> &current_agg_vec = agg_vecs[level_][layer_];
			size_t iMax =  prev_agg_vec.size();
			if (current_agg_vec.size() > iMax) iMax = current_agg_vec.size();
			for (int i = 0; i < iMax; i++) {
				current_agg_vec[i].proportion = prev_agg_vec[i].proportion;
				current_agg_vec[i].item_prop = prev_agg_vec[i].item_prop;
			}			
		}
		else {			
			vector <LossRec> &prev_agg_vec = agg_vecs[level_ - 1][previous_layer_];
			vector <LossRec> &prev_agg_vec_base = agg_vecs[level_ - 1][1];
			for (int i = 0; i < prev_agg_vec.size(); i++) {
				//if (prev_agg_vec[i].item_prop->size() == 0) {
				if (prev_agg_vec[i].item_prop == nullptr) {
					//prev_agg_vec[i].loss = prev_agg_vec_base[i].loss;
					prev_agg_vec[i].item_prop = prev_agg_vec_base[i].item_prop;
				}
			}
			//

			std::vector<int> v(guls.size(), -1);
			//v.resize(guls.size(), -1);
			auto iter = prev_agg_vec.begin();
			int j = 0;

			while (iter != prev_agg_vec.end()) {
				if (iter->item_idx) {
					auto it = iter->item_idx->begin();
					while (it != iter->item_idx->end()) {
						v[*it] = j;
						it++;
					}
				}
				j++;
				iter++;
			}
			for (int y = 0; y < agg_vecs[level_][layer_].size(); y++) {
				if (agg_vecs[level_][layer_][y].item_idx != nullptr) {
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
						if (agg_vecs[level_][layer_][y].item_prop == nullptr) {
							agg_vecs[level_][layer_][y].item_prop = std::make_shared<std::vector<OASIS_FLOAT>>(std::vector<OASIS_FLOAT>());
						}
						if (prev_gul_total > 0) {
							int j = -1;
							const std::vector<int>& z = *(prev_agg_vec[v[*it]].item_idx);
							for (int i = 0; i < z.size(); i++) {
								if (z[i] == *it) {
									j = i;
									break;
								}
							}
							// this is recomputing the first layers proportions
							if (j > -1) {
								agg_vecs[level_][layer_][y].item_prop->push_back(prev_agg_vec[v[*it]].loss * prev_agg_vec[v[*it]].item_prop->at(j) / prev_gul_total);
							}
							else {
								agg_vecs[level_][layer_][y].item_prop->push_back(0);
							}

						}
						else {
							agg_vecs[level_][layer_][y].item_prop->push_back(0);
						}
						it++;
					}
				}
			}
		}		
		return;

	}
}


inline void fmcalc::dofmcalc_r(std::vector<std::vector<int>>  &aggid_to_vectorlookups, vector<vector<vector <LossRec>>> &agg_vecs, int level, int max_level,
	std::map<int, std::vector<fmlevelrec> > &outmap, int gul_idx, const std::vector<std::vector<std::vector<policytcvidx>>> &avxs, int layer, 
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

	std::vector<int> *aggid_to_vectorlookup = &aggid_to_vectorlookups[level];
	//agg_vec.clear();
	
	auto iter = agg_vec.begin();
	LossRec l;
	while (iter != agg_vec.end()) {
		if (iter->policytc_id == 0) break;
		*iter = l;
		iter++;
	}

	if (agg_vec.size() != aggid_to_vectorlookup->size()) {
		agg_vec.resize(aggid_to_vectorlookup->size());
	}


	for (unsigned int i = 0;i < agg_vec_previous_level.size();i++){ // loop through previous levels of agg_vec
		if (agg_vec_previous_level[i].agg_id != 0) {
			int agg_id = pfm_vec_vec_[level][agg_vec_previous_level[i].agg_id];
			if (layer < policy_tc_vec_vec_vec_[level][agg_id].size()) {
				if (agg_vec_previous_level[i].next_vec_idx == -1) {	// next_vec_idx can be zero
					agg_vec_previous_level[i].next_vec_idx = (*aggid_to_vectorlookup)[agg_id - 1];
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
				agg_vec[vec_idx].limit_surplus += agg_vec_previous_level[i].limit_surplus;
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
	
	dofmcalc(agg_vec, layer);

	if (allocrule_ == 2) {
		const std::vector<OASIS_FLOAT>& guls = event_guls[gul_idx];
		compute_item_proportions(agg_vecs, guls, level, layer, previous_layer_id);
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
				if (x.item_idx) {
					x.item_net = std::make_shared<std::vector<OASIS_FLOAT>>(std::vector<OASIS_FLOAT>());
					x.item_net->resize(x.item_idx->size(), 0);
					const std::vector<int> &z = *(x.item_idx);
					for (int i = 0; i < x.item_net->size(); i++) {					
						x.item_net->at(i)= guls[z[i]];
					}
				}								
			}

			int allocrule_id = allocrule_;
			if (x.loss > 0.0 || rec.sidx < 0 || netvalue_ || debug_) {
				if (allocrule_id == 0) { // no back allocation
					if (x.agg_id > 0) {	// agg id cannot be zero
						if (netvalue_) { // get net gul value							
							rec.loss = x.retained_loss;							
						}
						else {
							rec.loss = x.loss;
						}
						if (rec.loss > 0.0 || rec.sidx < 0 || debug_) {
							fmxref_key k;
							k.layer_id = layer;
							k.agg_id = x.agg_id;
							auto it = fm_xrefmap.find(k);
							int output_id = 0;
							if (it == fm_xrefmap.end()) {
								output_id = k.agg_id;
							}
							else {
								output_id = it->second;
								if (netvalue_) {
									if (layer == max_layer_) outmap[output_id].push_back(rec);			// negligible cost
								}
								else {
									outmap[output_id].push_back(rec);			// negligible cost
								}
							}							
						}
					}
				}
				// at level 1 allocrule 2 and 1 are the same
				if (allocrule_id == 2 && level == 1) allocrule_id = 1;

				if (allocrule_id == 1 && x.agg_id > 0) {	// back allocate as a proportion of the total of the original guls	
					OASIS_FLOAT gultotal = 0;
					int index = gul_idx;	// default index top level
					if (sidx == tiv_idx) index = 0;
					const std::vector<OASIS_FLOAT> &guls = event_guls[index];
					int vec_idx = (*aggid_to_vectorlookup)[x.agg_id - 1];		// Same index applies to avx as to agg_vec
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
								rec.loss = x.retained_loss * prop;
							}

						}
						else {
							rec.loss = x.loss * prop;
						}
						if (rec.loss > 0.0 || rec.sidx < 0 || debug_) {
							fmxref_key k;
							k.layer_id = layer;
							k.agg_id = items[idx];
							auto it = fm_xrefmap.find(k);
							int output_id = 0;
							if (it == fm_xrefmap.end()) {
								output_id = k.agg_id;
							}
							else {
								output_id = it->second;
								if (netvalue_) {
									if (layer == max_layer_) outmap[output_id].push_back(rec);			// negligible cost
								}
								else {
									outmap[output_id].push_back(rec);			// negligible cost
								}
							}							
						}
					}
				}

				if (allocrule_id == 2 && x.agg_id > 0) {		// back allocate as a proportion of the total of the previous losses			
					//if (item_proportions_computed == false) {
					//	const std::vector<OASIS_FLOAT> &guls = event_guls[gul_idx];
					//	compute_item_proportions(agg_vecs, guls, level, layer, previous_layer_id);
					//	if (allocruleOptimizationOff_ == false) item_proportions_computed = true;
					//}

					if (x.item_prop && x.item_prop->size() > 0) {
						int vec_idx = (*aggid_to_vectorlookup)[x.agg_id - 1];
						for (int i = 0; i < avx[layer][vec_idx].item_idx.size(); i++) {
							int idx = avx[layer][vec_idx].item_idx[i];
							//for (int idx : avx[layer][vec_idx].item_idx) {
							OASIS_FLOAT prop = x.item_prop->at(i);
							if (netvalue_) { // get net gul value							
								x.item_net->at(i) = x.item_net->at(i) - (x.loss * prop);
								if (x.item_net->at(i) < 0) x.item_net->at(i) = 0;
								rec.loss = x.item_net->at(i);
							}
							else {
								rec.loss = x.loss * prop;
							}
							if (rec.loss > 0.0 || rec.sidx < 0 || debug_) {
								fmxref_key k;
								k.layer_id = layer;
								k.agg_id = items[idx];
								auto it = fm_xrefmap.find(k);
								int output_id = 0;
								if (it == fm_xrefmap.end()) {
									output_id = k.agg_id;
								}
								else {
									output_id = it->second;
									if (netvalue_) {
										if (layer == max_layer_) outmap[output_id].push_back(rec);			// neglible cost
									}
									else {
										outmap[output_id].push_back(rec);			// neglible cost
									}
								}								
							}
						}
					}
					else {
						fprintf(stderr, "Error: item_prop is zero !! layer = %d agg_id = %d previous_layer_id = %d level = %d\nItem set: \n",layer, x.agg_id, previous_layer_id,level);
						auto iter = x.item_idx->begin();
						while (iter != x.item_idx->end()) {
							fprintf(stderr, "%d ", *iter); iter++;
						}
						fprintf(stderr, "\n");
					}

				}
			}
		}
		if (layer < max_layer_) {
			int previous_level = level;
			int previous_layer = layer;
			layer++;
			dofmcalc_r(aggid_to_vectorlookups, agg_vecs, level, max_level, outmap, gul_idx, avxs, layer,items, event_guls, previous_level, previous_layer);
		}
	}
	else {		
		if (layer < level_to_max_layer_[level]) {
			int previous_level = level;
			int previous_layer = layer;
			layer++;
			dofmcalc_r(aggid_to_vectorlookups, agg_vecs, level, max_level, outmap, gul_idx, avxs, layer, items, event_guls, previous_level, previous_layer);
		}
		else {
			int previous_level = level;
			int previous_layer = layer;
			level++;
			layer = 1;
			dofmcalc_r(aggid_to_vectorlookups, agg_vecs, level, max_level, outmap, gul_idx, avxs, layer, items, event_guls, previous_level, previous_layer);
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
				aggid_to_vectorlookup[agg_id-1] = (int) avx[layer_id].size() - 1;
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
						zzaggid_to_vectorlookup[agg_id-1] = (int) next[layer].size() - 1; // temp use for getting 
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
	std::map<int, std::vector<fmlevelrec>> outmap;
	
	for (unsigned int gul_idx = 0; gul_idx < event_guls.size(); gul_idx++) {	// loop sample + 1 times
		const std::vector<OASIS_FLOAT> &guls = event_guls[gul_idx];
		if (gul_idx < 2 || gulhasvalue(guls)) {
			agg_vec.resize(total_loss_items);			
			//fmlevelhdr fmhdr;
			//fmhdr.event_id = event_id;
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
				if (isGULStreamType_ == true)	agg_vec[vec_idx].accumulated_tiv = item_to_tiv_[avx[1][vid].item_idx[0]+1];
				else agg_vec[vec_idx].accumulated_tiv = 0;
				agg_vec[vec_idx].policytc_id = avx[1][vid].policytc_id;
			}

//			dofmcalc(agg_vec);					
			
			dofmcalc_r(aggid_to_vectorlookups, agg_vecs, level + 1, maxLevel_, outmap, gul_idx, avxs, 1, items, event_guls,0,1);

			agg_vec.clear();
		}
	}
	aggid_to_vectorlookups.clear();
	// Do output
	for (const auto &x : outmap) {
		fwrite(&event_id, sizeof(event_id), 1, stdout);
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
		policy_tc_vec_vec_vec_[f.level_id][f.agg_id][f.layer_id] = f.profile_id;
	}

}
void fmcalc::parse_policytc(std::vector< fm_policyTC> &p)
{
	FILE* fin = NULL;
	std::string file = FMPOLICYTC_FILE;
	if (inputpath_.length() > 0) {
		file = inputpath_ + file.substr(5);
	}
	fin = fopen(file.c_str(), "rb");

	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, file.c_str());
		exit(EXIT_FAILURE);
	}
	fm_policyTC f;
	
	int max_layer_id = 0;
	int max_level_id = 0;
	int i = fread(&f, sizeof(f), 1, fin);
	while (i != 0) {
		p.push_back(f);		
		if (f.level_id > max_level_id) max_level_id = f.level_id;			
		i = fread(&f, sizeof(f), 1, fin);
	}
	fclose(fin);
	std::set<layer_agg> la;
	auto iter = p.begin();
	int max_agg_id = 0;

	while (iter != p.end()) {
		if (iter->level_id == max_level_id) {
			if (iter->agg_id > max_agg_id) max_agg_id = iter->agg_id;
			if (iter->layer_id > max_layer_id) max_layer_id = iter->layer_id;
			layer_agg l;
			l.agg_id = iter->agg_id;
			l.layer_id = iter->layer_id;
			la.insert(l);
		}
		iter++;
	}


	for (int i = 1;i <= max_layer_id;i++) {
		for (int j = 1;j <= max_agg_id;j++) {
			layer_agg l;
			l.layer_id = i;
			l.agg_id = j;
			auto iter = la.find(l);
			if (iter == la.end()) {
				fm_policyTC f;
				f.agg_id = l.agg_id;
				f.layer_id = l.layer_id;
				f.level_id = max_level_id;
				f.profile_id = noop_profile_id;
				p.push_back(f);
			}
		}
	}


	std::sort(p.begin(), p.end());

}
void fmcalc::init_policytc(int maxRunLevel)
{
	if (maxRunLevel != -1) maxRunLevel_ = maxRunLevel;

	std::vector< fm_policyTC> p;
	parse_policytc(p);

	fm_policyTC f;
	f.layer_id = 1;
	f.level_id = 0;
	f.profile_id = -1;
	for (int i = 1; i <= level_to_maxagg_id_[0]; i++) {		
		f.agg_id = i;		
		addtcrow(f);
	}


	auto iter = p.begin();
	while (iter != p.end()) {
		addtcrow(*iter);
		iter++;
	}

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
	size_t i = fread(&tiv, sizeof(tiv), 1, fin);
	while (i != 0) {
		coverage_id++;
		coverages[coverage_id] = tiv;
		i = fread(&tiv, sizeof(tiv), 1, fin);
	}

	fclose(fin);
	return true;

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
	unsigned int nrec = (unsigned int) sz / (unsigned int)sizeof(item);
	item_to_tiv_.resize(nrec+1,0.0);

	item itm;
	size_t i = fread(&itm, sizeof(itm), 1, fin);
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
	size_t i = fread(&f, sizeof(f), 1, fin);
	while (i != 0) {
		fmxref_key k;
		k.agg_id = f.agg_id;
		k.layer_id = f.layer_id;
/*
		if (k.agg_id == f.output_id && k.layer_id == 1) {
			// skip it
		}else {
			fm_xrefmap[k] = f.output_id;
		}
*/
		fm_xrefmap[k] = f.output_id;
		i = fread(&f, sizeof(f),  1, fin);
	}
	fclose(fin);	
}

void fmcalc::init(int MaxRunLevel)
{    
    init_programme(MaxRunLevel);
	init_profile();
	init_policytc(MaxRunLevel);
	init_fmxref();	
	
}

