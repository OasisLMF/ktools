#pragma once

#include <map>
#include <vector>


#define MAX_SUMMARY_SETS 10

struct loss_exp {
	float loss;			// gul total
						//float exposure;		// tiv total
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
	float *sse[MAX_SUMMARY_SETS] = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };         // s[summary_set][summary_id] to exposure
	//bool *bsummary;
	std::vector<float> coverages_;
	std::vector<float> outputs_;
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
	float *alloc_sse_arrays(int summary_set);
	void outputsamplesize(int samplesize);
	void outputsummary(int sample_size, int event_id);
	void processsummeryset(int summaryset, int event_id, int coverage_id, int sidx, float gul);
	void dosummary(int sample_size, int event_id, int coverage_id, int sidx, float gul, float expval);
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
