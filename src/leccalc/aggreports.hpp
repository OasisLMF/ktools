#pragma once

#include "leccalc.hpp"

class aggreports {
private:
	FILE **fout_;
	int totalperiods_;
	int maxsummaryid_;
	std::map<outkey2, float> &agg_out_loss_;
	std::map<outkey2, float> &max_out_loss_;
	bool useReturnPeriodFile_;
	std::vector<int> returnperiods_;

//
	void fulluncertainty(int handle, const std::map<outkey2, float> &out_loss);	
	void wheatsheaf(int handle, const std::map<outkey2, float> &out_loss);	
	void wheatSheafMean(int samplesize, int handle, const std::map<outkey2, float> &out_loss);	
	void sampleMean(int samplesize, int handle, const std::map<outkey2, float> &out_loss);	
	void loadreturnperiods();
	float getloss(float nextreturnperiod, float last_return_period, float last_loss, float current_return_period, float current_loss) const;
	void doreturnperiodout(int handle, int &nextreturnperiod_index, float &last_return_period, float &last_loss, float current_return_period, float current_loss, int summary_id, int type);
public:

	void outputOccFulluncertainty();
	void outputAggFulluncertainty();
	void outputOccWheatsheaf();
	void outputAggWheatsheaf();
	void outputOccWheatSheafMean(int samplesize);
	void outputAggWheatSheafMean(int samplesize);
	void outputOccSampleMean(int samplesize);
	void outputAggSampleMean(int samplesize);
	aggreports(int totalperiods, int maxsummaryid, std::map<outkey2, float> &agg_out_loss, std::map<outkey2, float> &max_out_loss, FILE **fout, bool useReturnPeriodFile) ;
};
