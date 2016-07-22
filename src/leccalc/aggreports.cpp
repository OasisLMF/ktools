
#include <vector>
#include <algorithm>    // std::sort

#include "aggreports.hpp"


struct line_points {
	float from_x;
	float from_y;
	float to_x;
	float to_y;
};
inline float linear_interpolate(line_points lp, float xpos)
{
	return ((xpos - lp.to_x) * (lp.from_y - lp.to_y) / (lp.from_x - lp.to_x)) + lp.to_y;
}

bool operator<(const summary_id_period_key& lhs, const summary_id_period_key& rhs)
{
	if (lhs.summary_id != rhs.summary_id) {
		return lhs.summary_id < rhs.summary_id;
	}
	if (lhs.period_no != rhs.period_no) {
		return lhs.period_no < rhs.period_no;
	}
	else {
		return lhs.type < rhs.type;
	}
}


bool operator<(const summary_id_type_key& lhs, const summary_id_type_key& rhs)
{
	if (lhs.summary_id != rhs.summary_id) {
		return lhs.summary_id < rhs.summary_id;
	}
	else {
		return lhs.type < rhs.type;
	}
}


bool operator<(const wheatkey& lhs, const wheatkey& rhs)
{
	if (lhs.summary_id != rhs.summary_id) {
		return lhs.summary_id < rhs.summary_id;
	}
	else {
		return lhs.sidx < rhs.sidx;
	}
}

aggreports::aggreports(int totalperiods, int maxsummaryid, std::map<outkey2, float> &agg_out_loss, std::map<outkey2, float> &max_out_loss, FILE **fout, bool useReturnPeriodFile) :
	totalperiods_(totalperiods), maxsummaryid_(maxsummaryid), agg_out_loss_(agg_out_loss), max_out_loss_(max_out_loss), fout_(fout), useReturnPeriodFile_(useReturnPeriodFile)
{
	loadreturnperiods();
};

void aggreports::loadreturnperiods()
{
	if (useReturnPeriodFile_ == false) return;

	FILE *fin = fopen(RETURNPERIODS_FILE, "rb");
	if (fin == NULL) {
		std::cerr << "loadreturnperiods: Unable to open " << RETURNPERIODS_FILE << "\n";
		exit(-1);
	}

	int return_period;

	while (fread(&return_period, sizeof(return_period), 1, fin) == 1) {
		returnperiods_.push_back(return_period);
	}

	fclose(fin);

	if (returnperiods_.size() == 0) {
		useReturnPeriodFile_ = false;
		fprintf(stderr, "No return periods loaded - running without defined return periods option\n");
	}
}

void aggreports::fulluncertainty(int handle,const std::map<outkey2, float> &out_loss)
{
	if (fout_[handle] == nullptr) return;
	std::map<int, lossvec> items;
	for (auto x : out_loss) {
		items[x.first.summary_id].push_back(x.second);
	}

	fprintf(fout_[handle], "Summary_id, return_period, Loss\n");

	for (auto s : items) {
		lossvec &lpv = s.second;
		std::sort(lpv.rbegin(), lpv.rend());
		int i = 1;
		float t = (float) totalperiods_;
		for (auto lp : lpv) {
			fprintf(fout_[handle], "%d, %f,%f\n", s.first, t / i, lp);
			i++;
		}
	}
}
void aggreports::outputOccFulluncertainty()
{
	fulluncertainty(OCC_FULL_UNCERTAINTY, max_out_loss_);
}

// Full uncertainity output
void aggreports::outputAggFulluncertainty()
{
	fulluncertainty(AGG_FULL_UNCERTAINTY, agg_out_loss_);
}

void aggreports::wheatsheaf(int handle, const std::map<outkey2, float> &out_loss)
{
	if (fout_[handle] == nullptr) return;
	std::map<wheatkey, lossvec> items;

	for (auto x : out_loss) {
		wheatkey wk;;
		wk.sidx = x.first.sidx;
		wk.summary_id = x.first.summary_id;
		items[wk].push_back(x.second);
	}

	fprintf(fout_[handle], "Summary_id, sidx, return_period, Loss\n");
	for (auto s : items) {
		lossvec &lpv = s.second;
		std::sort(lpv.rbegin(), lpv.rend());
		int i = 1;
		float t = (float)totalperiods_;
		for (auto lp : lpv) {
			fprintf(fout_[handle], "%d,%d, %f,%f\n", s.first.summary_id, s.first.sidx, t / i, lp);
			i++;
		}
	}
}

void aggreports::outputOccWheatsheaf()
{
	wheatsheaf(OCC_WHEATSHEAF, max_out_loss_);
}
void aggreports::outputAggWheatsheaf()
{
	wheatsheaf(AGG_WHEATSHEAF, agg_out_loss_);
}

void aggreports::wheatSheafMean(int samplesize, int handle, const std::map<outkey2, float> &out_loss)
{
	if (fout_[handle] == nullptr) return;
	std::map<wheatkey, lossvec> items;
	for (auto x : out_loss) {
		wheatkey wk;;
		wk.sidx = x.first.sidx;
		wk.summary_id = x.first.summary_id;
		items[wk].push_back(x.second);
	}

	int maxcount = 0;
	for (auto x : items) {
		if (x.second.size() > maxcount) maxcount = x.second.size();
	}

	std::map<int, std::vector<float>> mean_map;
	for (int i = 1; i <= maxsummaryid_; i++) {
		mean_map[i] = std::vector<float>(maxcount, 0);
	}
	fprintf(fout_[handle], "Summary_id,type, return_period, Loss\n");
	for (auto s : items) {
		int nextreturnperiodindex = 0;
		int nextreturnperiodvalue = 0;
		if (useReturnPeriodFile_) nextreturnperiodvalue = returnperiods_[nextreturnperiodindex];
		float last_computed_rp = 0;	
		float last_computed_loss = 0;
		lossvec &lpv = s.second;
		std::sort(lpv.rbegin(), lpv.rend());
		if (s.first.sidx != -1) {
			int i = 0;
			for (auto lp : lpv) {
				mean_map[s.first.summary_id][i] += lp;
				i++;
			}
		}
		else {
			int i = 1;
			float t = (float) totalperiods_;
			for (auto lp : lpv) {
				if (useReturnPeriodFile_) {
					float retperiod = t / i;
					if (retperiod == nextreturnperiodvalue) {
						//fprintf(stderr, "%d, 1, %f,%f\n", s.first.summary_id, retperiod, lp);
						fprintf(fout_[handle], "%d, 1, %f,%f\n", s.first.summary_id, retperiod, lp);
						nextreturnperiodindex++;
						nextreturnperiodvalue = returnperiods_[nextreturnperiodindex];
					} else{
						if (retperiod < nextreturnperiodvalue) {
							line_points lpt;
							lpt.from_x = last_computed_rp;
							lpt.from_y = last_computed_loss;
							lpt.to_x = retperiod;
							lpt.to_y = lp;
							float zz = linear_interpolate(lpt, nextreturnperiodvalue);
							//fprintf(stderr, "%d, 1, %f,%f\n", s.first.summary_id, (float)nextreturnperiodvalue, zz);
							fprintf(fout_[handle], "%d, 1, %f,%f\n", s.first.summary_id, (float)nextreturnperiodvalue, zz);
							nextreturnperiodindex++;
							nextreturnperiodvalue = returnperiods_[nextreturnperiodindex];
						}
					}
					//fprintf(fout_[handle], "%d, 1, %f,%f\n", s.first.summary_id, t / i, lp);					
					last_computed_rp = retperiod;
					last_computed_loss = lp;
				}else {
					fprintf(fout_[handle], "%d, 1, %f,%f\n", s.first.summary_id, t / i, lp);
				}
				
				i++;
			}
			if (useReturnPeriodFile_) {

				line_points lpt;
				lpt.from_x = last_computed_rp;
				lpt.from_y = last_computed_loss;
				lpt.to_x = 0;
				lpt.to_y = 0;
				float zz = linear_interpolate(lpt, nextreturnperiodvalue);
				//fprintf(stderr, "%d, 1, %f,%f\n", s.first.summary_id, (float)nextreturnperiodvalue, zz);
				fprintf(fout_[handle], "%d, 1, %f,%f\n", s.first.summary_id, (float)nextreturnperiodvalue, zz);
				nextreturnperiodindex++;
				while (nextreturnperiodindex < returnperiods_.size()) {
					nextreturnperiodvalue = returnperiods_[nextreturnperiodindex];
					zz = linear_interpolate(lpt, nextreturnperiodvalue);
					fprintf(fout_[handle], "%d, 1, %f,%f\n", s.first.summary_id, (float)nextreturnperiodvalue, zz);
					//fprintf(stderr, "%d, 1, %f,%f\n", s.first.summary_id, (float)nextreturnperiodvalue, zz);
					nextreturnperiodindex++;
				}
			}
		}
	}

	if (samplesize == 0) return; // avoid divide by zero error

	for (auto m : mean_map) {
		std::vector<float> &lpv = m.second;
		std::vector<float>::reverse_iterator rit = lpv.rbegin();
		int maxindex = lpv.size();
		while (rit != lpv.rend()) {
			if (*rit != 0.0) break;
			maxindex--;
			rit++;
		}
		int i = 1;
		float t = (float)totalperiods_;
		for (auto lp : lpv) {
			fprintf(fout_[handle], "%d, 2, %f,%f\n", m.first, t / i, lp / samplesize);
			i++;
			if (i > maxindex) break;
		}
	}

}

void aggreports::outputOccWheatSheafMean(int samplesize)
{
	wheatSheafMean(samplesize, OCC_WHEATSHEAF_MEAN, max_out_loss_);
}

void aggreports::outputAggWheatSheafMean(int samplesize)
{
	wheatSheafMean(samplesize, AGG_WHEATSHEAF_MEAN, agg_out_loss_);
}

void aggreports::sampleMean(int samplesize, int handle, const std::map<outkey2, float> &out_loss)
{
	if (fout_[handle] == nullptr) return;
	std::map<summary_id_period_key, float> items;

	for (auto x : out_loss) {
		summary_id_period_key sk;
		sk.period_no = x.first.period_no;
		sk.summary_id = x.first.summary_id;
		if (x.first.sidx == -1) {
			sk.type = 1;
			items[sk] += x.second;
		}
		else {
			if (samplesize > 0) {
				sk.type = 2;
				items[sk] += (x.second / samplesize);
			}
		}
	}

	std::map<summary_id_type_key, std::vector<float>> mean_map;

	fprintf(fout_[handle], "Summary_id,type, return_period, Loss_mean\n");
	for (auto s : items) {
		summary_id_type_key st;
		st.summary_id = s.first.summary_id;
		st.type = s.first.type;
		mean_map[st].push_back(s.second);
	}

	for (auto m : mean_map) {
		std::vector<float> &lpv = m.second;
		std::sort(lpv.rbegin(), lpv.rend());
		int i = 1;
		float t = (float) totalperiods_;
		for (auto lp : lpv) {
			fprintf(fout_[handle], "%d, %d, %f,%f\n", m.first.summary_id, m.first.type, t / i, lp);
			i++;
		}
	}
}
void aggreports::outputOccSampleMean(int samplesize)
{
	sampleMean(samplesize, OCC_SAMPLE_MEAN, max_out_loss_);
}

void aggreports::outputAggSampleMean(int samplesize)
{
	sampleMean(samplesize, AGG_SAMPLE_MEAN, agg_out_loss_);
}
