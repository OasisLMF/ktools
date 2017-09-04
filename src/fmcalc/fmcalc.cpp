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
Example implmentation for FM with back allocation

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


#ifdef __unix
    #include <unistd.h>
#endif

using namespace std;

// NOTE: we use multui dimensional arrays rather tham map with keys for performance resons

struct fm_policyTC {
	int level_id;
	int agg_id;
	int layer_id;
	int PolicyTC_id;
};

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

inline void fmcalc::dofmcalc(vector <LossRec> &agg_vec)
{
	for (LossRec &x : agg_vec) {
		if (x.agg_id > 0) {
			const profile_rec &profile = profile_vec_[x.policytc_id];
			x.allocrule_id = profile.allocrule_id;
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
					x.retained_loss = x.retained_loss + (x.loss - loss);
					x.loss = loss * share;

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

inline void fmcalc::dofmcalc_r(std::vector<std::vector<int>>  &aggid_to_vectorlookups, vector<vector <LossRec>> &agg_vecs, int level, int max_level,
	std::map<fmlevelhdr, std::vector<fmlevelrec> > &outmap, fmlevelhdr &fmhdr, int sidx, const std::vector<std::vector<std::vector<policytcvidx>>> &avxs, int layer, 
	const std::vector<int> &items, const std::vector<OASIS_FLOAT> &guls)
{
	vector <LossRec> &prev_agg_vec = agg_vecs[level - 1];
	vector <LossRec> &agg_vec = agg_vecs[level];
	const std::vector<std::vector<policytcvidx>> &avx = avxs[level];
	agg_vec.clear();
	std::vector<int> &aggid_to_vectorlookup = aggid_to_vectorlookups[level];

	if (agg_vec.size() == 0) {
        int size = aggid_to_vectorlookup.size();
        agg_vec.resize(size);
	}
	else {
		for (unsigned int i = 0;i < agg_vec.size();i++) agg_vec[i].loss = 0;
	}

	for (unsigned int i = 0;i < prev_agg_vec.size();i++){ // loop through previous levels of agg_vec
		if (prev_agg_vec[i].agg_id != 0) {
			int agg_id = pfm_vec_vec_[level][prev_agg_vec[i].agg_id];			
			if (layer < policy_tc_vec_vec_vec_[level][agg_id].size()) {
				if (prev_agg_vec[i].next_vec_idx == -1) {	// next_vec_idx can be zero
					prev_agg_vec[i].next_vec_idx = aggid_to_vectorlookup[agg_id - 1];
				}
				int vec_idx = prev_agg_vec[i].next_vec_idx;

				if (agg_vec[vec_idx].policytc_id == 0) { // policytc_id cannot be zero - first position in lookup vector not used
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
	
	if (sidx == 3) {
		//fprintf(stderr, "We're here");
	}

	dofmcalc(agg_vec);

	if (level == max_level) {
		fmlevelrec rec;
        rec.loss = 0;
		rec.sidx = sidx;		
		for (auto x : agg_vec) {
			if (x.allocrule_id == -1 || x.allocrule_id == 0 ) { // no back allocation
				fmxref_key k;
				k.layer_id = layer;
				k.agg_id = x.agg_id;
				fmhdr.output_id = fm_xrefmap[k];
				rec.loss = x.loss;
				outmap[fmhdr].push_back(rec);			// neglible cost
			}			
			if (x.allocrule_id == 1) {	// back allocate as a proportion of the total of the original guls	
				OASIS_FLOAT gultotal = 0;
				int vec_idx = aggid_to_vectorlookup[x.agg_id-1];		// Same index applies to avx as to agg_vec
				for (int idx : avx[layer][vec_idx].item_idx) {
					gultotal += guls[idx];
				}
				for (int idx : avx[layer][vec_idx].item_idx) {
                    OASIS_FLOAT prop = 0;
                    if (gultotal > 0) prop = guls[idx] / gultotal;
					//fmhdr.output_id = items[idx];
					fmxref_key k;
					k.layer_id = layer;
					k.agg_id = items[idx];
					fmhdr.output_id = fm_xrefmap[k];
					rec.loss = x.loss * prop;
					outmap[fmhdr].push_back(rec);			// neglible cost
				}				
			}
			if (x.allocrule_id == 2) {		// back allocate as a proportion of the total of the previous losses
				OASIS_FLOAT prev_gul_total = 0;
				int vec_idx = aggid_to_vectorlookup[x.agg_id - 1];		// Same index applies to avx as to agg_vec
				for (int idx : avx[layer][vec_idx].item_idx) {
					prev_gul_total += prev_agg_vec[idx].loss;
				}
				for (int idx : avx[layer][vec_idx].item_idx) {
					OASIS_FLOAT prop = 0;
					if (prev_gul_total > 0) prop = prev_agg_vec[idx].loss / prev_gul_total;
					fmxref_key k;
					k.layer_id = layer;
					k.agg_id = items[idx];
					fmhdr.output_id = fm_xrefmap[k];
					rec.loss = x.loss * prop;
					outmap[fmhdr].push_back(rec);			// neglible cost
				}
			}			
		}
		if (layer < max_layer_) {
			layer++;
			dofmcalc_r(aggid_to_vectorlookups, agg_vecs, level, max_level, outmap, fmhdr, sidx, avxs, layer,items, guls);
		}
	}
	else {
		level++;
		dofmcalc_r(aggid_to_vectorlookups, agg_vecs, level, max_level, outmap, fmhdr, sidx, avxs, layer, items, guls);
	}
}

void fmcalc::dofm(int event_id, const std::vector<int> &items, std::vector<vector<OASIS_FLOAT>> &event_guls)
{
	
	const int level = 1;
	std::vector<std::vector<int>>  aggid_to_vectorlookups(maxLevel_ + 1);

	std::vector<int> &aggid_to_vectorlookup = aggid_to_vectorlookups[level];	
	aggid_to_vectorlookup.resize(level_to_maxagg_id_[level], -2);

	const int layer_id = 1;
	std::vector<std::vector<std::vector<policytcvidx>>> avxs;
	int total_loss_items = 0;
    avxs.resize(maxLevel_ + 1);
	if (event_guls.size() > 0) {
		auto &guls = event_guls[0];		// pick any set of events they'll all have the same size we just want the size
		std::vector<std::vector<policytcvidx>> &avx = avxs[1];

		avx.resize(max_layer_+1);
		for (unsigned int i = 0;i < guls.size();i++) {
			int agg_id = pfm_vec_vec_[level][items[i]];

			int current_idx = aggid_to_vectorlookup[agg_id-1];
			if (current_idx == -2) {
				policytcvidx a;
				a.agg_id = agg_id;
				a.policytc_id = policy_tc_vec_vec_vec_[level][agg_id][layer_id];
				avx[1].push_back(a);			// populate the first layer
				avx[1][avx[1].size() - 1].item_idx.push_back(i);
				aggid_to_vectorlookup[agg_id-1] = avx[1].size() - 1;
				total_loss_items++;
			}
			else {
				avx[1][current_idx].item_idx.push_back(i);
			}
			
		}
		       
		for (unsigned int zzlevel = 2;zzlevel < avxs.size();zzlevel++) {
			std::vector<std::vector<policytcvidx>>  &prev = avxs[zzlevel-1];
			std::vector<std::vector<policytcvidx>>  &next = avxs[zzlevel];
			next.resize(max_layer_ + 1);

			std::vector<int> &zzaggid_to_vectorlookup = aggid_to_vectorlookups[zzlevel]; // 
			zzaggid_to_vectorlookup.resize(level_to_maxagg_id_[zzlevel] , -2);
			int previous_layer = 1; // previous layer is always one becuase only the last layer can be greater than 1

			for (unsigned int i = 0;i < prev[1].size();i++) {
				int agg_id = pfm_vec_vec_[zzlevel][prev[previous_layer][i].agg_id];
				int current_idx = zzaggid_to_vectorlookup[agg_id-1];
				if (current_idx == -2)
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

	vector<vector <LossRec>>agg_vecs(maxLevel_ + 1);
	vector <LossRec> &agg_vec = agg_vecs[level];
	std::map<fmlevelhdr, std::vector<fmlevelrec>> outmap;
	
	
	for (unsigned int idx = 0; idx < event_guls.size(); idx++) {	// loop sample + 1 times
		agg_vec.resize(total_loss_items);
		fmlevelhdr fmhdr;
		fmhdr.event_id = event_id;
		const std::vector<OASIS_FLOAT> &guls = event_guls[idx];
		const std::vector<std::vector<policytcvidx>> &avx = avxs[1];
		for (unsigned int i = 0;i < agg_vec.size(); i++) agg_vec[i].loss = 0;
		int last_agg_id = -1;
		int vec_idx = 0;
		for (unsigned int i = 0;i < guls.size();i++) {
			int agg_id = pfm_vec_vec_[level][items[i]];
			if (last_agg_id != agg_id) {
				vec_idx = aggid_to_vectorlookup[agg_id-1];
				last_agg_id = agg_id;				 
			}			
			agg_vec[vec_idx].loss += guls[i];
            int vid = aggid_to_vectorlookup[agg_id-1];
            agg_vec[vec_idx].agg_id = avx[1][vid].agg_id;
            agg_vec[vec_idx].item_idx = &avx[1][vid].item_idx;
            agg_vec[vec_idx].policytc_id = avx[1][vid].policytc_id;
		}

		dofmcalc(agg_vec);

		int sidx = idx-1;
		if (sidx == -1) sidx = tiv_idx;
		if (sidx == 0) sidx = mean_idx;
		

        if (level == maxLevel_) {
            int layer = 1;
            fmlevelrec rec;
            rec.loss = 0;
            rec.sidx = sidx;
            //fmhdr.layer_id = layer;
            for (auto x : agg_vec) {
                if (x.allocrule_id == -1 || x.allocrule_id == 0 ) { // no back allocation
					fmxref_key k;
					k.layer_id = layer;
					k.agg_id = x.agg_id;
					fmhdr.output_id = fm_xrefmap[k];
                    rec.loss = x.loss;
                    outmap[fmhdr].push_back(rec);			// neglible cost
                }
                if (x.allocrule_id == 1 || x.allocrule_id == 2) {	// back allocate as a proportion of the total of the original guls
                    OASIS_FLOAT gultotal = 0;
					const std::vector<OASIS_FLOAT> &guls = event_guls[event_guls.size()-1];	
                    // int vec_idx = aggid_to_vectorlookup[x.agg_id];		// Same index applies to avx as to agg_vec
					for (int idx = 0; idx < guls.size(); idx++) {
                        gultotal += guls[idx];
                    }
					for (int idx = 0; idx < guls.size(); idx++) {
                        OASIS_FLOAT prop = 0;
                        if (gultotal > 0) prop = guls[idx] / gultotal;
                        fmhdr.output_id = items[idx];
                        rec.loss = x.loss * prop;
						fmxref_key k;
						k.layer_id = layer;
						k.agg_id = items[idx];
                        outmap[fmhdr].push_back(rec);			// neglible cost
                    }

                }				
            }
            if (layer < max_layer_) {
                layer++;
                dofmcalc_r(aggid_to_vectorlookups, agg_vecs, level, maxLevel_, outmap, fmhdr, sidx, avxs, layer,items, guls);
            }
        }else {
            dofmcalc_r(aggid_to_vectorlookups, agg_vecs, level + 1, maxLevel_, outmap, fmhdr, sidx,avxs,1, items, guls);
        }
		agg_vec.clear();
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


void fmcalc::init_policytc(int maxRunLevel)
{
    if (maxRunLevel == -1) maxRunLevel = 10000;

	FILE *fin = fopen(FMPOLICYTC_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, FMPOLICYTC_FILE);
		exit(EXIT_FAILURE);
	}

	fm_policyTC f;
	int max_level = 0;
	max_layer_ = 0;
	max_agg_id_ = 0;
	int i = fread(&f, sizeof(f), 1, fin);
	while (i != 0) {
        if 	(f.level_id <= maxRunLevel){
            if (f.level_id > max_level) max_level = f.level_id;
            if (f.layer_id > max_layer_) max_layer_ = f.layer_id;
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
		i = fread(&f, sizeof(f), 1, fin);
	}
	fclose(fin);

}

void fmcalc::init_programme(int maxRunLevel)
{
	FILE *fin = fopen(FMPROGRAMME_FILE, "rb");
	if (fin == NULL){
		fprintf(stderr, "%s: cannot open %s\n", __func__, FMPROGRAMME_FILE);
		exit(EXIT_FAILURE);
	}

	fm_programme f;
    if (maxRunLevel == -1) maxRunLevel = 10000;

	int i = fread(&f, sizeof(f), 1, fin);
	while (i != 0) {
        if (f.level_id <= maxRunLevel){
			if (maxLevel_ < f.level_id) {
				maxLevel_ = f.level_id;
				level_to_maxagg_id_.resize(maxLevel_+1, -1);
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

inline void add_tc(unsigned char tc_id, OASIS_FLOAT &tc_val, std::vector<tc_rec> &tc_vec)
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
	FILE *fin = fopen(COVERAGES_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, COVERAGES_FILE);
		exit(-1);
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

void fmcalc::init_itemtotiv()
{
	std::vector<OASIS_FLOAT> coverages;
	loadcoverages(coverages);

	FILE *fin = fopen(ITEMS_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, ITEMS_FILE);
		exit(-1);
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
void fmcalc::init_profile()
{
	FILE *fin = fopen(FMPROFILE_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, FMPROFILE_FILE);
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
	FILE *fin = fopen(FMXREF_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, FMXREF_FILE);
		exit(EXIT_FAILURE);
	}
	fmXref f;
	int i = fread(&f, sizeof(f), 1, fin);
	while (i != 0) {
		fmxref_key k;
		k.agg_id = f.agg_id;
		k.layer_id = f.layer_id;
		fm_xrefmap[k] = f.output_id;
		i = fread(&f, sizeof(f), 1, fin);
	}
	fclose(fin);	
}

void fmcalc::init(int MaxRunLevel)
{
    init_policytc(MaxRunLevel);	
    init_programme(MaxRunLevel);
	init_fmxref();
	init_profile();
	init_itemtotiv();
}

