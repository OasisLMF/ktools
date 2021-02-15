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

#ifndef AGGREPORTS_H_
#define AGGREPORTS_H_

#include<vector>
#include "leccalc.h"

enum { FULL = 1, PERSAMPLEMEAN, MEANDR, MEANSAMPLE };
enum { OEP = 1, AEP, OEPTVAR, AEPTVAR };

class aggreports {
private:
	int totalperiods_;
	int maxsummaryid_;
	std::map<outkey2, OASIS_FLOAT> &agg_out_loss_;
	std::map<outkey2, OASIS_FLOAT> &max_out_loss_;
	FILE **fout_;
	FILE *ord_out_;
	bool useReturnPeriodFile_;
	int samplesize_ = 0;
	std::vector<int> returnperiods_;
	std::map <int, double> periodstoweighting_;
	std::map<int, std::vector<int>> ensembletosidx_;
	bool skipheader_ = false;

//
	inline void outputrows(const int handle, const char * buffer, int strLen);
	inline void outputrows(const char * buffer, int strLen);
	void fulluncertainty(int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss);
	void writefulluncertainty(const int handle, const int type, const std::map<outkey2, OASIS_FLOAT> &out_loss, const int ensemble_id=0);
	void fulluncertaintywithweighting(int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss);
	void writefulluncertaintywithweighting(const int handle, const int type, const std::map<outkey2, OASIS_FLOAT> &out_loss, const int ensemble_id=0);
	void wheatsheaf(int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss);
	void wheatsheafwithweighting(int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss); 
	void wheatSheafMean(int samplesize, int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss);	
	void wheatSheafMeanwithweighting(int samplesize, int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss);
	void sampleMean(int samplesize, int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss);
	void sampleMeanwithweighting(int samplesize, int handle, const std::map<outkey2, OASIS_FLOAT> &out_loss);
	inline void writeSampleMean(const int handle, const int type, const std::map<summary_id_period_key, lossval> &items, const int ensemble_id=0);
	void loadreturnperiods();
	OASIS_FLOAT getloss(OASIS_FLOAT nextreturnperiod, OASIS_FLOAT last_return_period, OASIS_FLOAT last_loss, 
		OASIS_FLOAT current_return_period, OASIS_FLOAT current_loss) const;
	void doreturnperiodout(int handle, size_t &nextreturnperiod_index,
			       OASIS_FLOAT &last_return_period,
			       OASIS_FLOAT &last_loss,
			       OASIS_FLOAT current_return_period,
			       OASIS_FLOAT current_loss, int summary_id,
			       int type, OASIS_FLOAT max_retperiod,
			       int ensemble_id=0, int epcalc=0, int eptype=0);
	void doreturnperiodout_tvar(size_t &nextreturnperiod_index,
				    OASIS_FLOAT &last_return_period,
				    OASIS_FLOAT &last_loss,
				    const OASIS_FLOAT current_return_period,
				    const OASIS_FLOAT current_loss,
				    OASIS_FLOAT tvar, int counter,
				    const int summary_id, const int epcalc,
				    const int eptype,
				    const OASIS_FLOAT max_retperiod);

public:
	void loadperiodtoweigthing();
	void loadensemblemapping();
	void initordout();
	void outputOccFulluncertainty();
	void outputAggFulluncertainty();
	void outputOccWheatsheaf();
	void outputAggWheatsheaf();
	void outputOccWheatSheafMean(int samplesize);
	void outputAggWheatSheafMean(int samplesize);
	void outputOccSampleMean(int samplesize);
	void outputAggSampleMean(int samplesize);
	aggreports(int totalperiods, int maxsummaryid, std::map<outkey2, OASIS_FLOAT> &agg_out_loss, std::map<outkey2, OASIS_FLOAT> &max_out_loss,
		FILE **fout, bool useReturnPeriodFile, int samplesize, bool skipheader, FILE *ord_out) ;
};
#endif // AGGREPORTS_H_
