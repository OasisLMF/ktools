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

#ifdef _MSC_VER
#include <fcntl.h>
#include <io.h>
#endif 

#include <vector>
#include <map>
using namespace std;
#include "../include/oasis.h"

const int mean_idx = 1 << 24;

struct fmdata {
	int item_id;
	int agg_id;
	int prog_id;
	int level_id;
	int policytc_id;
	int layer_id;
	int calcrule_id;
	float deductible;
	float limits;
	float share_prop_of_lim;
};

struct fmrec {
	int event_id;
	int sidx;
	int prog_id;
	int output_id;
	int layer_id;
	float loss;
};

struct fmlevel {
	int event_id;
	int prog_id;
	int agg_id;
	int layer_id;
	int output_id;
	int sidx;
	float loss;
};

struct fmlevelhdr {
	int event_id;
	int prog_id;
	int layer_id;
	int output_id;
};
struct fmlevelrec {
	int sidx;
	float loss;
};

struct gkey {
	int sidx;
	int agg_id;
	int prog_id;
	int layer_id;
};
struct grec {
	float total;
	float deductible;
	float limits;
	float share_prop_of_lim;
};
bool operator<(const gkey& lhs, const gkey& rhs)
{
	if (lhs.sidx != rhs.sidx)  return lhs.sidx < rhs.sidx;
	if (lhs.agg_id != rhs.agg_id)  return lhs.agg_id < rhs.agg_id;
	if (lhs.prog_id != rhs.prog_id)  return lhs.prog_id < rhs.prog_id;
	return lhs.layer_id < rhs.layer_id;
}
void genLevel2(int event_id_, std::vector<fmlevel> &fm_level2_, std::vector<fmlevel> &fm_level1_,
	std::map<int, fmdata> &fmd_level2_, std::map<int, float> &back_alloc_)
{
	std::map<gkey, grec> agg_sum;
	std::vector<fmlevel>::iterator iter = fm_level1_.begin();
	while (iter != fm_level1_.end()){
		std::map<int, fmdata>::iterator pos = fmd_level2_.find(iter->output_id);
		if (pos != fmd_level2_.end()) {
			gkey g;
			g.sidx = iter->sidx;
			g.agg_id = pos->second.agg_id;
			g.prog_id = pos->second.prog_id;
			g.layer_id = pos->second.layer_id;
			if (agg_sum.find(g) != agg_sum.end()) {
				agg_sum[g].total += iter->loss;

			}
			else {
				agg_sum[g].total = iter->loss;
				agg_sum[g].deductible = pos->second.deductible;
				agg_sum[g].limits = pos->second.limits;
				agg_sum[g].share_prop_of_lim = pos->second.share_prop_of_lim;

			}
		}
		iter++;
	}

	std::map<gkey, grec>::iterator aiter = agg_sum.begin();

	iter = fm_level1_.begin();
	while (iter != fm_level1_.end()){
		gkey g;
		g.sidx = iter->sidx;
		g.prog_id = 0;
		g.layer_id = 0;
		g.agg_id = 0;
		std::map<int, fmdata>::iterator pos = fmd_level2_.find(iter->output_id);
		if (pos != fmd_level2_.end()) {
			g.agg_id = pos->second.agg_id;
			g.prog_id = pos->second.prog_id;
			g.layer_id = pos->second.layer_id;
		}
		aiter = agg_sum.find(g);
		if (aiter != agg_sum.end()) {
			fmlevel f;
			f.agg_id = g.agg_id;
			f.event_id = event_id_;
			f.layer_id = g.layer_id;
			f.output_id = iter->output_id;
			f.prog_id = g.prog_id;
			f.sidx = g.sidx;
			float loss_prop = 0;
			if (iter->loss > 0 ) {
				loss_prop = iter->loss / aiter->second.total;
			}
			f.loss = aiter->second.total - aiter->second.deductible;
			if (f.loss < 0) f.loss = 0;
			if (f.loss > aiter->second.limits) f.loss = aiter->second.limits;
			f.loss = f.loss * aiter->second.share_prop_of_lim;
			f.loss = f.loss * loss_prop;
			fm_level2_.push_back(f);
		}

		iter++;
	}

}


void genLevel1(int event_id_, std::vector<fmlevel> &fm_level1_, std::map<int, float> &back_alloc_,
	std::vector<gulGulSampeslevel2> &event_guls_, std::map<int, fmdata> &fmd_level1_)
{
	std::vector<gulGulSampeslevel2>::iterator iter = event_guls_.begin();
	while (iter != event_guls_.end()){
		fmlevel f;
		f.sidx = iter->sidx;
		f.output_id = iter->item_id;
		f.prog_id = 1;
		f.layer_id = 1;
		f.agg_id = iter->item_id;
		f.loss = iter->gul;
		std::map<int, fmdata>::iterator pos = fmd_level1_.find(iter->item_id);
		if (pos != fmd_level1_.end()) {
			f.prog_id = pos->second.prog_id;
			f.agg_id = pos->second.agg_id;
			f.layer_id = pos->second.layer_id;
			f.output_id = pos->second.item_id;
			f.loss = iter->gul - pos->second.deductible;
			if (f.loss < 0) f.loss = 0;
		}
		fm_level1_.push_back(f);
		back_alloc_[f.sidx] += f.loss;
		iter++;
	}
};


void outputfm(std::vector<fmlevel> fm_level_)
{
	std::vector<fmlevel>::iterator iter = fm_level_.begin();
	bool firstrec = true;
	fmlevelhdr f;
	f.layer_id = 0;
	while (iter != fm_level_.end()){


		if (f.layer_id != iter->layer_id ||
			f.output_id != iter->output_id ||
			f.prog_id != iter->prog_id
			) {

			if (firstrec == false) {
				fmlevelrec frec;
				frec.sidx = 0;
				frec.loss = 0.0;
				fwrite(&frec, sizeof(fmlevelrec), 1, stdout);
			}
			else {
				firstrec = false;
			}
			f.event_id = iter->event_id;
			f.layer_id = iter->layer_id;
			f.output_id = iter->output_id;
			f.prog_id = iter->prog_id;
			
			fwrite(&f, sizeof(fmlevelhdr), 1, stdout);
		}
		
		fmlevelrec frec;
		frec.sidx = iter->sidx;
		if (frec.sidx == 0) frec.sidx = mean_idx;
		frec.loss = iter->loss;
		fwrite(&frec, sizeof(fmlevelrec), 1, stdout);
		
		iter++;
	}

	fmlevelrec frec;
	frec.sidx = 0;
	frec.loss = 0.0;
	fwrite(&frec, sizeof(fmlevelrec), 1, stdout);

}
void dofm(int event_id_, std::vector<gulGulSampeslevel2> &event_guls_,
	std::map<int, fmdata> &fmd_level1_, std::map<int, fmdata> &fmd_level2_)
{
	std::vector<fmlevel> fm_level1;
	std::vector<fmlevel> fm_level2;
	std::map<int, float> back_alloc;
	genLevel1(event_id_, fm_level1, back_alloc, event_guls_, fmd_level1_);
	genLevel2(event_id_, fm_level2, fm_level1, fmd_level2_, back_alloc);
	outputfm(fm_level2);
}

void doit(std::map<int, fmdata> &fmd_level1_, std::map<int, fmdata> &fmd_level3_)
{

#ifdef _MSC_VER
	_setmode(_fileno(stdout), O_BINARY);
	_setmode(_fileno(stdin), O_BINARY);
#endif

#ifdef __unix
	freopen(NULL, "rb", stdin);
	freopen(NULL, "wb", stdout);
#endif

	
	int stream_type = 0;
	int i = fread(&stream_type, sizeof(stream_type), 1, stdin);
	if (stream_type != 2) {
		std::cerr << "invalid stream time";
		exit(-1);
	}
	std::vector<gulGulSampeslevel2> event_guls;
	gulGulSampeslevel2 p;
	i = fread(&p, sizeof(gulGulSampeslevel2), 1, stdin);
	int last_event_id = p.event_id;

	while (i != 0) {
		if (p.event_id != last_event_id) {
			dofm(last_event_id, event_guls, fmd_level1_, fmd_level3_);
			event_guls.clear();
			last_event_id = p.event_id;
		}
		if (p.sidx >= 0) event_guls.push_back(p);

		i = fread(&p, sizeof(gulGulSampeslevel2), 1, stdin);
	}

}


void init(std::map<int, fmdata> &fmd_level1_, std::map<int, fmdata> &fmd_level2_)
{
	std::ostringstream oss;
	oss << "fm/fm_data.bin";

	FILE *fin = fopen(oss.str().c_str(), "rb");
	if (fin == NULL){
		cerr << "Error opening " << oss.str() << "\n";
		exit(EXIT_FAILURE);
	}

	fmdata f;

	int i = fread(&f, sizeof(fmdata), 1, fin);
	while (i != 0) {
		if (f.level_id == 1) fmd_level1_[f.item_id] = f;
		if (f.level_id == 2) fmd_level2_[f.item_id] = f;
		i = fread(&f, sizeof(fmdata), 1, fin);
	}
	fclose(fin);
}

int main()
{
	std::map<int, fmdata> fmd_level1;
	std::map<int, fmdata> fmd_level2;
	init(fmd_level1, fmd_level2);
	doit(fmd_level1, fmd_level2);
	return 0;
}
