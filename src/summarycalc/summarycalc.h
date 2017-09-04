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

struct loss_exp {
	OASIS_FLOAT loss;			// gul total
						//OASIS_FLOAT exposure;		// tiv total
};

#define MAX_SUMMARY_ID 2147483647

typedef std::vector<int> coverage_id_or_output_id_to_Summary_id;	// will turn into vectors once code is working
//typedef std::map<int, int> coverage_id_or_output_id_to_Summary_id;	// will turn into vectors once code is working
//typedef std::map<int, int> output_id_to_Summary_id;		// will turn into vectors once code is working

class summarycalc {
	enum input_type {UNKNOWN,GUL_STREAM,FM_STREAM};
private:
	int min_summary_id_[MAX_SUMMARY_SETS] = { MAX_SUMMARY_ID ,MAX_SUMMARY_ID , MAX_SUMMARY_ID , MAX_SUMMARY_ID , MAX_SUMMARY_ID , MAX_SUMMARY_ID , MAX_SUMMARY_ID , MAX_SUMMARY_ID , MAX_SUMMARY_ID  ,MAX_SUMMARY_ID };  // min should be equal to one											  
	int max_summary_id_[MAX_SUMMARY_SETS] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
	FILE *fout[MAX_SUMMARY_SETS] = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
	coverage_id_or_output_id_to_Summary_id *co_to_s[MAX_SUMMARY_SETS] = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
	//output_id_to_Summary_id *o_to_s[MAX_SUMMARY_SETS] = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
	loss_exp **sssl[MAX_SUMMARY_SETS] = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr }; // three dimensional array sssl[summary_set][summary_id][sidx] to loss exposure
	OASIS_FLOAT *sse[MAX_SUMMARY_SETS] = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };         // s[summary_set][summary_id] to exposure
	//bool *bsummary;
	std::vector<OASIS_FLOAT> coverages_;
	std::vector<OASIS_FLOAT> outputs_;
	input_type inputtype_ = UNKNOWN;
// functions
	void reset_sssl_array(int sample_size);
	void alloc_sssl_array(int sample_size);
	void reset_sse_array();
	void alloc_sse_array();
	void init_c_to_s();
	void init_o_to_s();
	void loadfmsummaryxref();
	void loadgulsummaryxref();
	void outputsummaryset(int sample_size, int summary_set, int event_id);	
	void outputstreamtype(int summary_set);
	void outputstreamtype();
	void outputsamplesizeandsummaryset(int summary_set, int sample_size);
	void reset_ssl_array(int summary_set, int sample_size, loss_exp **ssl);
	loss_exp **alloc_ssl_arrays(int summary_set, int sample_size);
	OASIS_FLOAT *alloc_sse_arrays(int summary_set);
	void outputsamplesize(int samplesize);
	void outputsummary(int sample_size, int event_id);
	void processsummeryset(int summaryset, int event_id, int coverage_id, int sidx, OASIS_FLOAT gul);
	void dosummary(int sample_size, int event_id, int coverage_id, int sidx, OASIS_FLOAT gul, OASIS_FLOAT expval);
	bool loadcoverages();

public:
	summarycalc();
	void dogulsummary();
	void dofmsummary();
	void doit();
	void openpipe(int summary_id, const std::string &pipe);
	void setgulmode() { inputtype_ = GUL_STREAM; };

	void setfmmode() { inputtype_ = FM_STREAM; };

};

#endif // SUMMARYCALC_H_
