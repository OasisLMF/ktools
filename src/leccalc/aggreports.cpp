
#include <vector>
#include <algorithm>    // std::sort

#include "leccalc.hpp"


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


void fulluncertainty(int handle,const std::map<outkey2, float> &out_loss)
{
	if (fout[handle] == nullptr) return;
	std::map<int, lossvec> items;
	for (auto x : out_loss) {
		items[x.first.summary_id].push_back(x.second);
	}

	fprintf(fout[handle], "Summary_id, return_period, Loss\n");

	for (auto s : items) {
		lossvec &lpv = s.second;
		std::sort(lpv.rbegin(), lpv.rend());
		int i = 1;
		float t = (float) totalperiods;
		for (auto lp : lpv) {
			fprintf(fout[handle], "%d, %f,%f\n", s.first, t / i, lp);
			i++;
		}
	}
}
void outputOccFulluncertainty()
{
	fulluncertainty(OCC_FULL_UNCERTAINTY, max_out_loss);
}

// Full uncertainity output
void outputAggFulluncertainty()
{
	fulluncertainty(AGG_FULL_UNCERTAINTY, agg_out_loss);

}

void wheatsheaf(int handle, const std::map<outkey2, float> &out_loss)
{
	if (fout[handle] == nullptr) return;
	std::map<wheatkey, lossvec> items;

	for (auto x : out_loss) {
		wheatkey wk;;
		wk.sidx = x.first.sidx;
		wk.summary_id = x.first.summary_id;
		items[wk].push_back(x.second);
	}

	fprintf(fout[handle], "Summary_id, sidx, return_period, Loss\n");
	for (auto s : items) {
		lossvec &lpv = s.second;
		std::sort(lpv.rbegin(), lpv.rend());
		int i = 1;
		float t = (float)totalperiods;
		for (auto lp : lpv) {
			fprintf(fout[handle], "%d,%d, %f,%f\n", s.first.summary_id, s.first.sidx, t / i, lp);
			i++;
		}
	}

}

void outputOccWheatsheaf()
{
	wheatsheaf(OCC_WHEATSHEAF, max_out_loss);
}
void outputAggWheatsheaf()
{
	wheatsheaf(AGG_WHEATSHEAF, agg_out_loss);
}

void wheatSheafMean(int samplesize, int handle, const std::map<outkey2, float> &out_loss)
{
	if (fout[handle] == nullptr) return;
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
	for (int i = 1; i <= maxsummaryid; i++) {
		mean_map[i] = std::vector<float>(maxcount, 0);
	}
	fprintf(fout[handle], "Summary_id,type, return_period, Loss\n");

	for (auto s : items) {
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
			float t = (float) totalperiods;
			for (auto lp : lpv) {
				fprintf(fout[handle], "%d, 1, %f,%f\n", s.first.summary_id, t / i, lp);
				i++;
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
		float t = (float)totalperiods;
		for (auto lp : lpv) {
			fprintf(fout[handle], "%d, 2, %f,%f\n", m.first, t / i, lp / samplesize);
			i++;
			if (i > maxindex) break;
		}
	}

}

void outputOccWheatSheafMean(int samplesize)
{
	wheatSheafMean(samplesize, OCC_WHEATSHEAF_MEAN, max_out_loss);
}

void outputAggWheatSheafMean(int samplesize)
{
	wheatSheafMean(samplesize, AGG_WHEATSHEAF_MEAN, agg_out_loss);
}

void sampleMean(int samplesize, int handle, const std::map<outkey2, float> &out_loss)
{
	if (fout[handle] == nullptr) return;
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

	fprintf(fout[handle], "Summary_id,type, return_period, Loss_mean\n");
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
		float t = (float) totalperiods;
		for (auto lp : lpv) {
			fprintf(fout[handle], "%d, %d, %f,%f\n", m.first.summary_id, m.first.type, t / i, lp);
			i++;
		}
	}
}
void outputOccSampleMean(int samplesize)
{
	sampleMean(samplesize, OCC_SAMPLE_MEAN, max_out_loss);
}

void outputAggSampleMean(int samplesize)
{
	sampleMean(samplesize, AGG_SAMPLE_MEAN, agg_out_loss);
}
