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


#ifndef AALCALC_H_
#define AALCALC_H_

#include "../include/oasis.h"

#include <string>
#include <map>
#include <vector>
// key for analytic data
struct period_map_key {
	int summary_id;
	int period_no;
};
// key for sample data
struct period_sidx_map_key {
	int summary_id;
	int period_no;
	int sidx;
};

// key for sample data
struct period_sidx_map_key_new {
//	int summary_id;
	int period_no;
	int sidx;
};

struct event_offset_rec{
	int event_id;	
	int fileindex;
	long long offset;
};
//bool operator<(const period_sidx_map_key& lhs, const period_sidx_map_key& rhs);
bool operator<(const period_map_key& lhs, const period_map_key& rhs);

class aalcalc {
private:
	std::map<int, std::vector<event_offset_rec>> summary_id_to_event_offset_;
	std::map<int, int> event_count_;	// count of events in occurrence table used to create cartesian effect on event_id
	std::map<int, std::vector<int>> event_to_period_;	// Mapping of event to period no
	int no_of_periods_ = 0;
	int max_ensemble_id_ = 0;
	int max_period_no_=0;
	int samplesize_ = -1;
	std::vector<std::vector<double>> max_exposure_val_ = std::vector<std::vector<double>>(2);
	std::map<int, aal_rec> map_analytical_aal_;
	std::map<int, aal_rec> map_analytical_aal_w_;
	std::map<int, aal_rec> map_sample_aal_;
	std::vector<double> vec_sample_sum_loss_;
	std::vector<aal_rec> vec_sample_aal_;
	std::vector<aal_rec_ensemble> vec_ensemble_aal_;
	std::vector<aal_rec> vec_analytical_aal_;
	int max_summary_id_ = 0;

	int current_summary_id_ = 0;
	std::map <int, double> periodstoweighting_;
	std::vector<int> sidxtoensemble_;
	std::vector<int> ensemblecount_;
	bool skipheader_ = false;
	bool ord_output_ = false;
// private functions
	template<typename T>
	void loadoccurrence(T &occ, FILE * fin);
	void loadoccurrence();
	void indexevents(const std::string& fullfilename, std::string& filename);
	void load_event_to_summary_index(const std::string& subfolder);
	void initsameplsize(const std::string &path);
	void loadperiodtoweigthing();
	void loadensemblemapping();
	void process_summaryfilew(const std::string &filename);
	void debug_process_summaryfile(const std::string &filename);
	void do_calc_by_period(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec);
	void do_calc_end(int period_no);
	void do_sample_calc_newx(const summarySampleslevelHeader& sh, const std::vector<sampleslevelRec>& vrec);
	void do_sample_calc_new(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec);
	inline void calculatemeansddev(const aal_rec_ensemble &record,
				       const int sample_size, const int p1,
				       const int p2, const int periods,
				       double &mean, double &sd_dev);
	void outputresultscsv();
	void outputresultscsv_new(std::vector<aal_rec> &vec_aal, int periods, int sample_size);
	void outputresultscsv_new(const std::vector<aal_rec_ensemble> &vec_aal, const int periods);
	void outputresultscsv_new();
	inline void outputrows(const char * buffer, int strLen);
	void getmaxsummaryid(std::string& path);
public:
	aalcalc(bool skipheader, bool ord_output) : skipheader_(skipheader), ord_output_(ord_output) {};
	void doit(const std::string& subfolder);		// experimental
	void debug(const std::string &subfolder);
};

#endif  // AALCALC_H_

