/*
* Copyright (c)2015 Oasis LMF Limited
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
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <fcntl.h>
#include <assert.h>

#include <fenv.h>
#ifdef __unix
    #include <unistd.h>
#endif

#include <vector>
#include <map>

using namespace std;
#include "../include/oasis.hpp"

// NOTE: we use multui dimensional arrays rather tham map with keys for performance resons
struct fm_programme {
	int prog_id;
	int level_id;
	int agg_id;
	int item_id;
};

struct fm_policyTC {
	int prog_id;
	int level_id;
	int agg_id;
	int layer_id;
	int PolicyTC_id;
};

struct fm_profile {
	int policytc_id;
	int calcrule_id;
	int allocrule_id;
	int sourcerule_id;
	int levelrule_id;
	int ccy_id;
	float deductible;
	float limits;
	float share_prop_of_lim;
	float deductible_prop_of_loss;
	float limit_prop_of_loss;
	float deductible_prop_of_tiv;
	float limit_prop_of_tiv;
	float deductible_prop_of_limit;
};

struct pfmkey {
	//	int prog_id;		// This is not really used at the momement - not passed through the guls will hardcode output to 1
	int level_id;
	int item_id;
};

int max_layer = 0;

std::vector<std::vector<int>> pfm_vec_vec;  // initialized from fm/programme.bin  pfm_vec_vec[level_id][item_id] returns agg_id [we ignore prog_id not passed through the system]

std::vector<vector<vector<int>>> policy_tc_vec_vec_vec;

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

struct tc_rec {
	float tc_val;
	unsigned char tc_id;
};

struct profile_rec {
	int calcrule_id;
	int allocrule_id;
	int sourcerule_id;
	int levelrule_id;
	int ccy_id;
	std::vector<tc_rec> tc_vec;

};

std::vector <profile_rec> profile_vec;

bool operator<(const fmlevelhdr& lhs, const fmlevelhdr& rhs)
{
	if (lhs.event_id != rhs.event_id)  return lhs.event_id < rhs.event_id;
	if (lhs.prog_id != rhs.prog_id)  return lhs.prog_id < rhs.prog_id;
	if (lhs.layer_id != rhs.layer_id)  return lhs.layer_id < rhs.layer_id;	
	return lhs.output_id < rhs.output_id;
}

struct gulRec {
	int agg_id;		// This has be stored for thresholds cannot be implied
	float gul;		// may want to cut down to singe this causes 4 byte padding for allignment
};

struct policytcvidx {
	int policytc_id;
	int agg_id;
	int next_vidx = -1;
	std::vector<int> item_idx;
};

struct LossRec {
	float loss;
	float retained_loss;
	int agg_id;
	int policytc_id;
	int allocrule_id;
	int next_vec_idx = -1;
	const std::vector<int> *item_idx;
};


inline void dofmcalc(vector <LossRec> &agg_vec_)
{
	for (LossRec &x : agg_vec_) {
		const profile_rec &profile = profile_vec[x.policytc_id];		
		x.allocrule_id = profile.allocrule_id;
		switch (profile.calcrule_id) {
			case 1:
			{				
				float ded = 0;
				float lim = 0;
				for (auto y : profile.tc_vec) {
					if (y.tc_id == deductible) ded = y.tc_val;
					if (y.tc_id == limit) lim = y.tc_val;
				}
				//Function1 = IIf(Loss < Ded, 0, IIf(Loss > Ded + Lim, Lim, Loss - Ded))
				float loss = x.loss - ded;
				if (loss < 0) loss = 0;
				if (loss > lim) loss = lim;
				x.retained_loss = x.retained_loss + ( x.loss - loss);
				x.loss = loss;
			}
			break;
			case 2:
			{
				float ded = 0;
				float lim = 0;
				float share = 0;
				for (auto y : profile.tc_vec) {
					if (y.tc_id == deductible) ded = y.tc_val;
					if (y.tc_id == limit) lim = y.tc_val;
					if (y.tc_id == share_prop_of_limit) share = y.tc_val;
				}
				//Function2 = IIf(Loss < Ded, 0, IIf(Loss > Ded + Lim, Lim, Loss - Ded)) * Share	
				float loss = 0;
				if (x.loss > (ded + lim)) loss = limit;
				else loss = x.loss - ded;
				if (loss < 0) loss = 0;
				x.retained_loss = x.retained_loss + ( x.loss - loss);
				x.loss = loss * share;

			}
			break;
			case 11:
				{
					float ded = 0;
					for (auto y : profile.tc_vec) {
						if (y.tc_id == deductible) ded = y.tc_val;					
					}
					float loss = 0;
					if (x.retained_loss < ded) {
						loss = x.loss - (ded - x.retained_loss);
						if (loss < 0) loss = 0;
						x.retained_loss = x.retained_loss + ( x.loss - loss);
					}else {
						loss = x.loss;
						// retained loss stays the same
					}

					x.loss = loss ;					
				}
				break;
			case 12:
			{
				for (auto &z : profile.tc_vec) {
					if (z.tc_id == deductible) {
						float loss = x.loss - z.tc_val;
						if (loss < 0) loss = 0;
						x.retained_loss = x.retained_loss + ( x.loss - loss);
						x.loss = loss;
						break;
					}
				}
			}
			break;
			default:
			{
				fprintf(stderr, "Unknown calc rule %d\n", profile.calcrule_id);
			}
		}
	}
}

inline void dofmcalc_r(std::vector<std::map<int, int>>  &aggid_to_vectorlookups_, vector<vector <LossRec>> &agg_vecs_, int level_, int max_level_,
	std::map<fmlevelhdr, std::vector<fmlevelrec> > &outmap_, fmlevelhdr &fmhdr_, int sidx_, const std::vector<std::vector<std::vector<policytcvidx>>> &avxs_, int layer_, 
	const std::vector<int> &items_, const std::vector<float> &guls_)
{
	vector <LossRec> &prev_agg_vec = agg_vecs_[level_ - 1];
	vector <LossRec> &agg_vec = agg_vecs_[level_];
	const std::vector<std::vector<policytcvidx>> &avx = avxs_[level_];
	agg_vec.clear();
	std::map<int, int> &aggid_to_vectorlookup = aggid_to_vectorlookups_[level_];

	if (agg_vec.size() == 0) {
        int size = aggid_to_vectorlookup.size();
        agg_vec.resize(size);
	}
	else {
		for (unsigned int i = 0;i < agg_vec.size();i++) agg_vec[i].loss = 0;
	}

	for (unsigned int i = 0;i < prev_agg_vec.size();i++){ // loop through previous levels of agg_vec
		if (prev_agg_vec[i].agg_id != 0) {
			int agg_id = pfm_vec_vec[level_][prev_agg_vec[i].agg_id];
			if (prev_agg_vec[i].next_vec_idx == -1) {	// next_vec_idx can be zero
				prev_agg_vec[i].next_vec_idx = aggid_to_vectorlookup[agg_id];
			}
			int vec_idx = prev_agg_vec[i].next_vec_idx;
			if (agg_vec[vec_idx].policytc_id == 0) { // policytc_id cannot be zero - first position in lookup vector not used
				agg_vec[vec_idx].policytc_id = avx[layer_][vec_idx].policytc_id;
				agg_vec[vec_idx].agg_id = agg_id;
				agg_vec[vec_idx].item_idx = &avx[layer_][vec_idx].item_idx;
			}			
			agg_vec[vec_idx].loss += prev_agg_vec[i].loss;						
		}
		else {
			fprintf(stderr, "Missing agg_id for index %d policytc_id: %d\n", i, prev_agg_vec[i].policytc_id);

		}
	}

	dofmcalc(agg_vec);

	if (level_ == max_level_) {
		fmlevelrec rec;
        rec.loss = 0;
		rec.sidx = sidx_;
		fmhdr_.layer_id = layer_;
		for (auto x : agg_vec) {
			if (x.allocrule_id == -1 || x.allocrule_id == 0 ) { // no back allocation
				fmhdr_.output_id = x.agg_id;				
				rec.loss = x.loss;
				outmap_[fmhdr_].push_back(rec);			// neglible cost
			}
			if (x.allocrule_id == 1) {	// back allocate as a proportion of the total of the original guls	
				float gultotal = 0;
				int vec_idx = aggid_to_vectorlookup[x.agg_id];		// Same index applies to avx as to agg_vec
				for (int idx : avx[layer_][vec_idx].item_idx) {
					gultotal += guls_[idx];
				}
				for (int idx : avx[layer_][vec_idx].item_idx) {
                    float prop = 0;
                    if (gultotal > 0) prop = guls_[idx] / gultotal;
					fmhdr_.output_id = items_[idx];
					rec.loss = x.loss * prop;
					outmap_[fmhdr_].push_back(rec);			// neglible cost
				}				

			}
		}
		if (layer_ < max_layer) {
			layer_++;
			dofmcalc_r(aggid_to_vectorlookups_, agg_vecs_, level_, max_level_, outmap_, fmhdr_, sidx_, avxs_, layer_,items_, guls_);
		}
	}
	else {
		level_++;
		dofmcalc_r(aggid_to_vectorlookups_, agg_vecs_, level_, max_level_, outmap_, fmhdr_, sidx_, avxs_, layer_, items_, guls_);
	}
}

void dofm(int event_id_, const std::vector<int> &items_, std::vector<vector<float>> &event_guls_, int max_level_)
{
	
	const int level = 1;
	std::vector<std::map<int, int>>  aggid_to_vectorlookups(max_level_ + 1);

	std::map<int, int> &aggid_to_vectorlookup = aggid_to_vectorlookups[level];	

	std::map<fmlevelhdr, std::vector<fmlevelrec>> outmap;
	fmlevelhdr fmhdr;
	fmhdr.event_id = event_id_;
	fmhdr.prog_id = 1;
	fmhdr.layer_id = 1;

	const int layer_id = 1;
	std::vector<std::vector<std::vector<policytcvidx>>> avxs;

    avxs.resize(max_level_ + 1);
	if (event_guls_.size() > 0) {
		auto &guls = event_guls_[0];		// pick any set of events they'll all have the same size we just want teh size
		std::vector<std::vector<policytcvidx>> &avx = avxs[1];

		avx.resize(max_layer+1);
		for (unsigned int i = 0;i < guls.size();i++) {
			int agg_id = pfm_vec_vec[level][items_[i]];
			auto iter = aggid_to_vectorlookup.find(agg_id);
			if (iter == aggid_to_vectorlookup.end()) {
				policytcvidx a;
				a.agg_id = agg_id;
				a.policytc_id = policy_tc_vec_vec_vec[level][agg_id][layer_id];
				avx[1].push_back(a);			// populate the first layer
				avx[1][avx[1].size() - 1].item_idx.push_back(i);
				aggid_to_vectorlookup[agg_id] = avx[1].size() - 1;
			}
			else {
				avx[1][iter->second].item_idx.push_back(i);
			}
			
		}
		
        // int i = 0;
        //for (auto &x : aggid_to_vectorlookup) {
        //    fprintf(stderr,"aggid = %d for avx[1][%d] \n",x.second,i);
        //	x.second = i;
        //	i++;
        //}

		for (unsigned int zzlevel = 2;zzlevel < avxs.size();zzlevel++) {
			std::vector<std::vector<policytcvidx>>  &prev = avxs[zzlevel-1];
			std::vector<std::vector<policytcvidx>>  &next = avxs[zzlevel];
			next.resize(max_layer + 1);
			//const vector <int> &r = policy_tc_vec_vec_vec[ptk.layer_id][zzlevel];
			std::map<int, int> &aggid_to_vectorlookup = aggid_to_vectorlookups[zzlevel]; // 
			int previous_layer = 1; // previous layer is always one becuase only the last layer can be greater than 1
			// genaggvectorlookup(aggid_to_vectorlookup, prev[previous_layer], zzlevel);
			for (unsigned int i = 0;i < prev[1].size();i++) {
				int agg_id = pfm_vec_vec[zzlevel][prev[previous_layer][i].agg_id];
				auto iter = aggid_to_vectorlookup.find(agg_id);
				
				if (iter == aggid_to_vectorlookup.end())
				{
					// vec_idx = aggid_to_vectorlookup[agg_id];
					for (unsigned int layer = 1; layer < policy_tc_vec_vec_vec[zzlevel][agg_id].size() ; layer++ ){ // loop through layers
						struct policytcvidx a;
						const int policytc_id = policy_tc_vec_vec_vec[zzlevel][agg_id][layer];
						//if (next[layer].size() < (agg_id + 1)) {
							//next[layer].resize(agg_id + 1);
						//}
						a.agg_id = agg_id;
						a.policytc_id = policytc_id;
						//a.vidx = vec_idx;
						next[layer].push_back(a);
						
						int current_idx = next[layer].size() - 1;
						for (int x : prev[previous_layer][i].item_idx) {
							next[layer][current_idx].item_idx.push_back(x);	
						}
						aggid_to_vectorlookup[agg_id] = next[layer].size() - 1; // temp use for getting 
					}						
				}
				else {
					for (unsigned int layer = 1; layer < policy_tc_vec_vec_vec[zzlevel][agg_id].size(); layer++) { // loop through layers
						const int policytc_id = policy_tc_vec_vec_vec[zzlevel][agg_id][layer];
						next[layer][iter->second].agg_id;
						int current_idx = aggid_to_vectorlookup[agg_id];
						for (int x : prev[previous_layer][i].item_idx) {
							next[layer][current_idx].item_idx.push_back(x);	
						}
					}

				}
			}
            //int i = 0;
            //for (auto &x : aggid_to_vectorlookup) {		// setup aggid_to_vectorlookup for indexing agg_vec
            //	x.second = i;
            //	i++;
            //}
		}
	}

	vector<vector <LossRec>>agg_vecs(max_level_ + 1);
	vector <LossRec> &agg_vec = agg_vecs[level];

	agg_vec.resize(aggid_to_vectorlookup.size());

	for (unsigned int idx = 0; idx < event_guls_.size(); idx++) {	// loop sample + 1 times
		const std::vector<float> &guls = event_guls_[idx];
		const std::vector<std::vector<policytcvidx>> &avx = avxs[1];
		for (unsigned int i = 0;i < agg_vec.size(); i++) agg_vec[i].loss = 0;
		int last_agg_id = -1;
		int vec_idx = 0;
		for (unsigned int i = 0;i < guls.size();i++) {
			int agg_id = pfm_vec_vec[level][items_[i]];
			if (last_agg_id != agg_id) {
				vec_idx = aggid_to_vectorlookup[agg_id];
				last_agg_id = agg_id;				 
			}			
			agg_vec[vec_idx].loss += guls[i];
            int vid = aggid_to_vectorlookup[agg_id];
            agg_vec[vec_idx].agg_id = avx[1][vid].agg_id;
            agg_vec[vec_idx].item_idx = &avx[1][vid].item_idx;
            agg_vec[vec_idx].policytc_id = avx[1][vid].policytc_id;


		}

		dofmcalc(agg_vec);
		int sidx = idx;
		if (sidx == 0) sidx = -1;

        if (level == max_level_) {
            int layer = 1;
            fmlevelrec rec;
            rec.loss = 0;
            rec.sidx = sidx;
            fmhdr.layer_id = layer;
            for (auto x : agg_vec) {
                if (x.allocrule_id == -1 || x.allocrule_id == 0 ) { // no back allocation
                    fmhdr.output_id = x.agg_id;
                    rec.loss = x.loss;
                    outmap[fmhdr].push_back(rec);			// neglible cost
                }
                if (x.allocrule_id == 1) {	// back allocate as a proportion of the total of the original guls
                    float gultotal = 0;
                    int vec_idx = aggid_to_vectorlookup[x.agg_id];		// Same index applies to avx as to agg_vec
                    for (int idx : avx[layer][vec_idx].item_idx) {
                        gultotal += guls[idx];
                    }
                    for (int idx : avx[layer][vec_idx].item_idx) {
                        float prop = 0;
                        if (gultotal > 0) prop = guls[idx] / gultotal;
                        fmhdr.output_id = items_[idx];
                        rec.loss = x.loss * prop;
                        outmap[fmhdr].push_back(rec);			// neglible cost
                    }

                }
            }
            if (layer < max_layer) {
                layer++;
                dofmcalc_r(aggid_to_vectorlookups, agg_vecs, level, max_level_, outmap, fmhdr, sidx, avxs, layer,items_, guls);
            }
        }else {
            dofmcalc_r(aggid_to_vectorlookups, agg_vecs, level + 1, max_level_, outmap, fmhdr, sidx,avxs,1, items_, guls);
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


void doit(int &maxLevel_)
{

	unsigned int fmstream_type = 1 | fmstream_id;

	fwrite(&fmstream_type, sizeof(fmstream_type), 1, stdout);

	int gulstream_type = 0;
	int i = fread(&gulstream_type, sizeof(gulstream_type), 1, stdin);

	int stream_type = gulstream_type & gulstream_id;

	if (stream_type != gulstream_id) {
		std::cerr << "Not a gul stream type\n";
		exit(-1);
	}
	stream_type = streamno_mask &gulstream_type;
	if (stream_type != 1) {
		std::cerr << "Unsupported gul stream type\n";
		exit(-1);
	}

	int last_event_id = -1;
	if (stream_type == 1) {
		int samplesize = 0;
		i = fread(&samplesize, sizeof(samplesize), 1, stdin);
		fwrite(&samplesize, sizeof(samplesize), 1, stdout);
		std::vector<vector<float>> event_guls(samplesize + 1);
		std::vector<int> items;
		items.reserve(500000);
		while (i != 0) {
			gulSampleslevelHeader gh;
			i = fread(&gh, sizeof(gh), 1, stdin);
			if (gh.event_id != last_event_id && i == 1) {
				if (last_event_id != -1) {
					dofm(last_event_id, items, event_guls, maxLevel_);
				}

				items.clear();
				for (unsigned int i = 0;i < event_guls.size();i++) event_guls[i].clear();
				last_event_id = gh.event_id;
			}
			if (i == 0) {
				if (last_event_id != -1) {
					dofm(last_event_id, items, event_guls, maxLevel_);
				}
			}
			while (i != 0) {
				gulSampleslevelRec gr;
				i = fread(&gr, sizeof(gr), 1, stdin);
				if (i == 0) {
					dofm(last_event_id, items, event_guls, maxLevel_);
					break;
				}
				if (gr.sidx == 0) break;
				//				if (gr.sidx == mean_idx) gr.sidx = 0;
				gulSampleslevelEventRec gs;
				gs.item_id = gh.item_id;
				gs.sidx = gr.sidx;
				gs.gul = gr.gul;
				if (gs.sidx >= -1) {
					int sidx = gs.sidx;
					if (sidx == -1) sidx = 0;
					event_guls[sidx].push_back(gs.gul);
				}
				if (gr.sidx == -1)  items.push_back(gh.item_id);
			}
		}
	}
}

void init_policytc()
{
	std::ostringstream oss;
	oss << "fm/fm_policytc.bin";

	FILE *fin = fopen(oss.str().c_str(), "rb");
	if (fin == NULL) {
		cerr << "Error opening " << oss.str() << "\n";
		exit(EXIT_FAILURE);
	}

	fm_policyTC f;
	int max_level = 0;
	int i = fread(&f, sizeof(f), 1, fin);
	while (i != 0) {
	
		if (f.level_id > max_level) max_level = f.level_id;
		if (f.layer_id > max_layer) max_layer = f.layer_id;
	
		if (policy_tc_vec_vec_vec.size() < f.level_id + 1) {
			policy_tc_vec_vec_vec.resize(f.level_id + 1);
		}
		if (policy_tc_vec_vec_vec[f.level_id].size() < f.agg_id + 1) {
			policy_tc_vec_vec_vec[f.level_id].resize(f.agg_id + 1);
		}

		if (policy_tc_vec_vec_vec[f.level_id][f.agg_id].size() < f.layer_id + 1) {
			policy_tc_vec_vec_vec[f.level_id][f.agg_id].resize(f.layer_id + 1);
		}
		policy_tc_vec_vec_vec[f.level_id][f.agg_id][f.layer_id] = f.PolicyTC_id;

		i = fread(&f, sizeof(f), 1, fin);
	}
	fclose(fin);

}

void init_programme(int &maxLevel_)
{
	std::ostringstream oss;
	oss << "fm/fm_programme.bin";

	FILE *fin = fopen(oss.str().c_str(), "rb");
	if (fin == NULL){
		cerr << "Error opening " << oss.str() << "\n";
		exit(EXIT_FAILURE);
	}

	fm_programme f;

	int i = fread(&f, sizeof(f), 1, fin);
	while (i != 0) {
		if (maxLevel_ < f.level_id) maxLevel_ = f.level_id;

		if (pfm_vec_vec.size() < f.level_id + 1) {
			pfm_vec_vec.resize(f.level_id + 1);
		}
		if (pfm_vec_vec[f.level_id].size() < f.item_id + 1) {
			pfm_vec_vec[f.level_id].resize(f.item_id + 1);
		}
		pfm_vec_vec[f.level_id][f.item_id] = f.agg_id;
		if (f.agg_id == 0) {
			fprintf(stderr, "Invalid agg id from fm_programme.bin\n");
		}

		i = fread(&f, sizeof(f), 1, fin);
	}
	fclose(fin);

}

inline void add_tc(unsigned char tc_id, float &tc_val, std::vector<tc_rec> &tc_vec)
{
	if (tc_val > -1) {
		tc_rec t;
		t.tc_id = tc_id;
		t.tc_val = tc_val;
		tc_vec.push_back(t);
	}
}
void init_profile()
{

	std::ostringstream oss;
	oss << "fm/fm_profile.bin";

	FILE *fin = fopen(oss.str().c_str(), "rb");
	if (fin == NULL) {
		cerr << "Error opening " << oss.str() << "\n";
		exit(EXIT_FAILURE);
	}

	fm_profile f;
	int i = fread(&f, sizeof(f), 1, fin);
	while (i != 0) {
		profile_rec p;
		p.allocrule_id = f.allocrule_id;
		p.calcrule_id = f.calcrule_id;
		p.ccy_id = f.ccy_id;
		p.levelrule_id = f.levelrule_id;
		p.sourcerule_id = f.sourcerule_id;

		add_tc(deductible, f.deductible, p.tc_vec);
		add_tc(limit, f.limits, p.tc_vec);
		add_tc(share_prop_of_limit, f.share_prop_of_lim, p.tc_vec);		
		add_tc(deductible_prop_of_loss, f.deductible_prop_of_loss, p.tc_vec);
		add_tc(limit_prop_of_loss, f.limit_prop_of_loss, p.tc_vec);
		add_tc(deductible_prop_of_tiv, f.deductible_prop_of_tiv, p.tc_vec);
		add_tc(limit_prop_of_tiv, f.limit_prop_of_tiv, p.tc_vec);
		add_tc(deductible_prop_of_limit, f.deductible_prop_of_limit, p.tc_vec);

		if (profile_vec.size() < f.policytc_id+1) {
			profile_vec.resize(f.policytc_id + 1);
		}
		profile_vec[f.policytc_id] = p;
	
		i = fread(&f, sizeof(f), 1, fin);
	}
	fclose(fin);
}

void init(int &maxLevel_)
{
	init_policytc();
	init_programme(maxLevel_);
	init_profile();
}

void help()
{

    cerr << "-I inputfilename\n"
        << "-O outputfielname\n"
        ;
}


int main(int argc, char* argv[])
{

    std::string inFile;
    std::string outFile;

#ifdef __unix
	int opt;
    while ((opt = getopt(argc, argv, "hI:O:")) != -1) {
        switch (opt) {
        case 'I':
            inFile = optarg;
            break;
         case 'O':
            outFile = optarg;
            break;
        case 'h':
           help();
           exit(EXIT_FAILURE);
        default:
            help();
            exit(EXIT_FAILURE);
        }
    }
#endif

   initstreams(inFile, outFile);
   
#ifdef __unix
   // posix_fadvise(fileno(stdin), 0, 0, POSIX_FADV_SEQUENTIAL);
#endif
     
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
	int maxlevel = 0;
	init(maxlevel);
	doit(maxlevel);
	return 0;
}
