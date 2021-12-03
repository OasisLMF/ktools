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
#ifndef SUMMARYCALC_H_
#define SUMMARYCALC_H_


#include <map>
#include <vector>


#define MAX_SUMMARY_SETS 10
#define MAX_SUMMARY_ID 2147483647

typedef std::vector<int> coverage_id_or_output_id_to_Summary_id;	// will turn into vectors once code is working
//typedef std::map<int, int> coverage_id_or_output_id_to_Summary_id;	// will turn into vectors once code is working
//typedef std::map<int, int> output_id_to_Summary_id;		// will turn into vectors once code is working

class summarycalc {
	enum input_type {UNKNOWN,GUL_COVERAGE_STREAM,GUL_ITEM_STREAM,FM_STREAM,GUL_ITEMX_STREAM};
private:
	int min_summary_id_[MAX_SUMMARY_SETS] = { MAX_SUMMARY_ID ,MAX_SUMMARY_ID , MAX_SUMMARY_ID , MAX_SUMMARY_ID , MAX_SUMMARY_ID , MAX_SUMMARY_ID , MAX_SUMMARY_ID , MAX_SUMMARY_ID , MAX_SUMMARY_ID  ,MAX_SUMMARY_ID };  // min should be equal to one											  
	int max_summary_id_[MAX_SUMMARY_SETS] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
	FILE *fout[MAX_SUMMARY_SETS] = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
	FILE *idxout[MAX_SUMMARY_SETS] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	coverage_id_or_output_id_to_Summary_id *co_to_s_[MAX_SUMMARY_SETS] = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
	//output_id_to_Summary_id *o_to_s[MAX_SUMMARY_SETS] = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
	OASIS_FLOAT **sssl[MAX_SUMMARY_SETS] = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr }; // three dimensional array sssl[summary_set][summary_id][sidx] to loss exposure
	OASIS_FLOAT *sse[MAX_SUMMARY_SETS] = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };         // s[summary_set][summary_id] to exposure
	//bool *bsummary;
	std::vector<OASIS_FLOAT> coverages_;
	std::vector<OASIS_FLOAT> outputs_;
	std::vector<int> item_to_coverage_;
	input_type inputtype_ = UNKNOWN;
	std::string inputpath_;
	bool zerooutput_ = false;
	std::map<int, std::string> indexFiles;
	long long offset_[MAX_SUMMARY_SETS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };   // for index file
	const int num_idx_ = 5;
	int first_idx_ = 2;
	int last_event_id_ = -1;
	int last_coverage_or_output_id_ = -1;
// functions
	void reset_sssl_array(int sample_size);
	void alloc_sssl_array(int sample_size);
	void reset_sse_array();
	void alloc_sse_array();
	void init_c_to_s();
	void init_o_to_s();
	void loadsummaryxref(const std::string &filename);
	void loadgulsummaryxref();
	void outputsummaryset(int sample_size, int summary_set, int event_id);	
	void outputstreamtype(int summary_set);
	void outputstreamtype();
	void outputsamplesizeandsummaryset(int summary_set, int sample_size);
	void reset_ssl_array(int summary_set, int sample_size, OASIS_FLOAT **ssl);
	OASIS_FLOAT **alloc_ssl_arrays(int summary_set, int sample_size);
	OASIS_FLOAT *alloc_sse_arrays(int summary_set);
	void outputsamplesize(int samplesize);
	void outputsummary(int sample_size, int event_id);
	OASIS_FLOAT add_losses(const OASIS_FLOAT loss, const OASIS_FLOAT gul);
	OASIS_FLOAT calculate_chanceofloss(const OASIS_FLOAT loss, const OASIS_FLOAT gul);
	OASIS_FLOAT (summarycalc::*PropagateLosses)(const OASIS_FLOAT, const OASIS_FLOAT);
	void processsummaryset(int summaryset, int coverage_id, int sidx, OASIS_FLOAT gul, OASIS_FLOAT (summarycalc::*PropagateLosses)(const OASIS_FLOAT, const OASIS_FLOAT));
	inline void reset_for_new_event(const int sample_size, const int event_id);
	inline void processsummarysets(const int coverage_or_output_id, const int sidx, const OASIS_FLOAT gul, OASIS_FLOAT (summarycalc::*PropagateLosses)(const OASIS_FLOAT, const OASIS_FLOAT));
	void dosummary_chanceofloss_maxloss(int sample_size, int event_id, int coverage_or_output_id, int sidx, OASIS_FLOAT gul);
	void dosummary(int sample_size, int event_id, int coverage_id, int sidx, OASIS_FLOAT gul, OASIS_FLOAT expval);
	bool loadcoverages();
	void loaditemtocoverage();
	bool isGulStream(unsigned int stream_type);
	bool isFMStream(unsigned int stream_type);

public:
	summarycalc();
	void dosummaryprocessing(int samplesize);
	void dogulcoveragesummary();
	void dogulitemsummary();
	void dogulitemxsummary();
	void dofmsummary();
	void doit();
	void openpipe(int summary_id, const std::string &pipe);
	void openindexfiles();
	void setgulcoveragemode() { inputtype_ = GUL_COVERAGE_STREAM; };
	void setgulitemmode() { inputtype_ = GUL_ITEM_STREAM; };
	void setgulitemxmode() { inputtype_ = GUL_ITEMX_STREAM; };
	void setfmmode() { inputtype_ = FM_STREAM; };
	void setinputpath(const std::string &s) { inputpath_ = s; }
	void enablezerooutput() { zerooutput_ = true; }
};

#endif // SUMMARYCALC_H_
