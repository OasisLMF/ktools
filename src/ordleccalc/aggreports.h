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

#include <map>
#include <vector>
#include "leccalc.h"

enum { MEANDR = 1, FULL, PERSAMPLEMEAN, MEANSAMPLE };
enum { OEP = 1, OEPTVAR, AEP, AEPTVAR };
enum { MEANS = 0, SAMPLES };
enum { WHEATSHEAF = 0, WHEATSHEAF_MEAN };

struct line_points{
	OASIS_FLOAT from_x;
	OASIS_FLOAT from_y;
	OASIS_FLOAT to_x;
	OASIS_FLOAT to_y;
};

class aggreports {
private:
	const int totalperiods_;
	const int maxsummaryid_;
	std::vector<std::map<outkey2, OutLosses>> &out_loss_;
	const bool *outputFlags_;
	FILE **fout_;
	std::map<int, bool> meanDR_ = {
		{ 0, true }, { OEP, false }, { AEP, false }
	};   // key 0 for fail safe
	bool useReturnPeriodFile_;
	const int samplesize_;
	std::vector<int> returnperiods_;
	std::map <int, double> periodstoweighting_;
	std::map<int, std::vector<int>> ensembletosidx_;
	const bool skipheader_;
	const bool ordFlag_;

	OASIS_FLOAT (OutLosses::*GetAgg)() = &OutLosses::GetAggOutLoss;
	OASIS_FLOAT (OutLosses::*GetMax)() = &OutLosses::GetMaxOutLoss;

	void LoadReturnPeriods();
	void LoadPeriodsToWeighting();
	void LoadEnsembleMapping();
	OASIS_FLOAT GetLoss(const OASIS_FLOAT next_return_period,
			    const OASIS_FLOAT last_return_period,
			    const OASIS_FLOAT last_loss,
			    const OASIS_FLOAT current_return_period,
			    const OASIS_FLOAT current_loss) const;
	void FillTVaR(std::map<int, std::vector<TVaR>> &tail,
		      const int summary_id, const int epcalc,
		      const OASIS_FLOAT nextreturnperiod_value,
		      const OASIS_FLOAT tvar);
	void FillTVaR(std::map<wheatkey, std::vector<TVaR>> &tail,
		      const int summary_id, const int sidx,
		      const OASIS_FLOAT nextreturnperiod_value,
		      const OASIS_FLOAT tvar);
	template<typename T>
	void WriteReturnPeriodOut(const std::vector<int> fileIDs,
		size_t &nextreturnperiod_index, OASIS_FLOAT &last_return_period,
		OASIS_FLOAT &last_loss, const OASIS_FLOAT current_return_period,
		const OASIS_FLOAT current_loss, const int summary_id,
		const int eptype, const int epcalc,
		const OASIS_FLOAT max_retperiod, int counter, OASIS_FLOAT tvar,
		T &tail,
		void (aggreports::*WriteOutput)(const std::vector<int>,
						const int, const int, const int,
						const OASIS_FLOAT,
						const OASIS_FLOAT));
	inline void OutputRows(const std::vector<int> fileIDs,
			       const char * buffer, int strLen);
	void WriteLegacyOutput(const std::vector<int> fileIDs,
			       const int summary_id, const int type,
			       const int ensemble_id,
			       const OASIS_FLOAT retperiod,
			       const OASIS_FLOAT loss);
	void WriteORDOutput(const std::vector<int> fileIDs,
			    const int summary_id, const int epcalc,
			    const int eptype, const OASIS_FLOAT retperiod,
			    const OASIS_FLOAT loss);
	void WriteTVaR(const std::vector<int> fileIDs, const int epcalc,
		       const int eptype_tvar,
		       const std::map<int, std::vector<TVaR>> &tail);
	void WriteTVaR(const std::vector<int> fileIDs, const int eptype_tvar,
		       const std::map<wheatkey, std::vector<TVaR>> &tail);
	inline void DoSetUp(int &eptype, int &eptype_tvar, int &epcalc,
		const int ensemble_id,
		void (aggreports::*&WriteOutput)(const std::vector<int>,
						 const int, const int,
						 const int, const OASIS_FLOAT,
						 const OASIS_FLOAT));
	void WriteExceedanceProbabilityTable(const std::vector<int> fileIDs,
					     std::map<int, lossvec> &items,
					     const OASIS_FLOAT max_retperiod,
					     int epcalc, int eptype,
					     int samplesize=1,
					     int ensemble_id=0);
	void WriteExceedanceProbabilityTable(const std::vector<int> fileIDs,
		std::map<int, lossvec2> &items,
		const OASIS_FLOAT cum_weight_constant, int epcalc, int eptype,
		int samplesize=1, int ensemble_id=0);
	void DoSetUpWheatsheaf(int &eptype, int &eptype_tvar,
		const int ensemble_id,
		void (aggreports::*&WriteOutput)(const std::vector<int>,
						 const int, const int,
						 const int, const OASIS_FLOAT,
						 const OASIS_FLOAT));
	void WritePerSampleExceedanceProbabilityTable(
		const std::vector<int> fileIDs,
		std::map<wheatkey, lossvec> &items, int eptype,
		int ensemble_id=0);
	void WritePerSampleExceedanceProbabilityTable(
		const std::vector<int> fileIDs,
		std::map<wheatkey, lossvec2> &items, int eptype,
		int ensemble_id=0);
	void MeanDamageRatio(const std::vector<int> fileIDs,
			     OASIS_FLOAT (OutLosses::*GetOutLoss)(),
			     const int eptype);
	void MeanDamageRatioWithWeighting(const std::vector<int> fileIDs,
		OASIS_FLOAT (OutLosses::*GetOutLoss)(), const int eptype);
	inline std::vector<int> GetFileIDs(const int handle, int table=EPT);
	void FullUncertainty(const std::vector<int> fileIDs,
			     OASIS_FLOAT (OutLosses::*GetOutLoss)(),
			     const int eptype);
	void FullUncertaintyWithWeighting(const std::vector<int> fileIDs,
		OASIS_FLOAT (OutLosses::*GetOutLoss)(), const int eptype);
	inline void FillWheatsheafItems(const outkey2 key,
					std::map<wheatkey, lossvec> &items,
					const OASIS_FLOAT loss);
	inline void FillWheatsheafItems(const outkey2 key,
					std::map<wheatkey, lossvec2> &items,
					const OASIS_FLOAT loss,
					std::vector<int> &maxPeriodNo);
	void WheatsheafAndWheatsheafMean(const std::vector<int> handles,
					 OASIS_FLOAT (OutLosses::*GetOutLoss)(),
					 const int eptype,
					 const int ensemble_id=0);
	void WheatsheafAndWheatsheafMeanWithWeighting(
		const std::vector<int> handles,
		OASIS_FLOAT (OutLosses::*GetOutLoss)(), const int eptype,
		const int ensemble_id=0);
	void SampleMean(const std::vector<int> fileIDs,
			OASIS_FLOAT (OutLosses::*GetOutLoss)(),
			const int eptype);
	void SampleMeanWithWeighting(const std::vector<int> fileIDs,
				     OASIS_FLOAT (OutLosses::*GetOutLoss)(),
				     const int eptype);

public:
	aggreports(const int totalperiods, const int maxsummaryid, std::vector<std::map<outkey2, OutLosses>> &out_loss, FILE **fout, const bool useReturnPeriodFile, const int samplesize, const bool skipheader, const bool *outputFlags, const bool ordFlag);
	void OutputAggMeanDamageRatio();
	void OutputOccMeanDamageRatio();
	void OutputAggFullUncertainty();
	void OutputOccFullUncertainty();
	void OutputAggWheatsheafAndWheatsheafMean();
	void OutputOccWheatsheafAndWheatsheafMean();
	void OutputAggSampleMean();
	void OutputOccSampleMean();
};
#endif // AGGREPORTS_H_
