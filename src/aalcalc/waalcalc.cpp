// Implemention of aalcalc using the welford method
#include "aalcalc.h"
#if defined(_MSC_VER)
#include "../include/dirent.h"
#else
#include <dirent.h>
#endif
#include <math.h>       /* sqrt */


void aalcalc::do_sample_calc_endw()
{

}

double total_samples = 0;
double I = 0;
double K = 0;
double L = 0;

void aalcalc::do_analytical_calc_endw()
{

	auto iter = map_analytical_sum_loss_w_.begin();
	while (iter != map_analytical_sum_loss_w_.end()) {
		auto a_iter = map_analytical_aal_w_.find(iter->first.summary_id);
		if (a_iter != map_analytical_aal_w_.end()) {
			aal_rec &a = a_iter->second;
		}
		else {
			aal_rec a;
			a.summary_id = iter->first.summary_id;
			a.type = 1;
			a.max_exposure_value = iter->second.max_exposure_value;
			map_analytical_aal_[iter->first.summary_id] = a;
		}
		iter++;
	}

	double v = L / (total_samples - 1);
	v = sqrt(v);
	fprintf(stderr, "Welford standard deviation: %f\n", v);

}
void aalcalc::do_analytical_calcw(const summarySampleslevelHeader &sh, double mean_loss)
{
	period_map_key k;
	k.period_no = 0;
	//k.period_no = event_to_period_[sh.event_id];
	k.summary_id = sh.summary_id;

	if (k.period_no == 0) return;
	auto iter = map_analytical_sum_loss_w_.find(k);

	if (iter != map_analytical_sum_loss_w_.end()) {
		loss_rec_w &l = iter->second;
		if (l.max_exposure_value < sh.expval) l.max_exposure_value = sh.expval;
		l.w.I = l.w.I + 1;
		double J = mean_loss;
		double k1 = l.w.K + (J - l.w.K) / l.w.I;
		l.w.L = L + (J - k1)*(J - l.w.K);
		l.max_exposure_value = sh.expval;
		l.w.K = k1;
	}
	else {
		loss_rec_w l;
		l.w.I = 0;
		l.w.K = 0;
		l.w.L = 0;		
		l.w.I = l.w.I + 1;
		double J = mean_loss;
		double k1 = l.w.K + (J - l.w.K) / l.w.I;
		l.w.L = L + (J - k1)*(J - l.w.K);
		l.max_exposure_value = sh.expval;
		l.w.K = k1;
		map_analytical_sum_loss_w_[k] = l;
	}
	;
	I = I + 1;
	double J = mean_loss;
	double k1 = K + (J - K) / I;
	L = L + (J-k1)*(J-K);
	K = k1;
	total_samples++;
	//fprintf(stderr,"I=%f J=%f K=%f L=%f\n", I, J, K, L);
	//fprintf(stderr, "%f\n",J);
}
void aalcalc::do_sample_calcw(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec)
{

}
void aalcalc::doaalcalcw(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec, OASIS_FLOAT mean_loss)
{
	
	do_analytical_calcw(sh, mean_loss);
	if (samplesize_) do_sample_calcw(sh, vrec);
}
void aalcalc::process_summaryfilew(const std::string &filename)
{
	FILE *fin = fopen(filename.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, filename.c_str());
		exit(EXIT_FAILURE);
	}

	int summarycalcstream_type = 0;
	size_t i = fread(&summarycalcstream_type, sizeof(summarycalcstream_type), 1, fin);
	int stream_type = summarycalcstream_type & summarycalc_id;

	if (stream_type != summarycalc_id) {
		fprintf(stderr, "%s: Not a summarycalc stream type %d\n", __func__, stream_type);
		exit(-1);
	}
	stream_type = streamno_mask & summarycalcstream_type;
	bool haveData = false;

	if (stream_type == 1) {
		int summary_set = 0;
		i = fread(&samplesize_, sizeof(samplesize_), 1, fin);
		if (i != 0) i = fread(&summary_set, sizeof(summary_set), 1, fin);
		std::vector<sampleslevelRec> vrec;
		summarySampleslevelHeader sh;
		int j = 0;
		OASIS_FLOAT mean_loss = 0;
		while (i != 0) {
			i = fread(&sh, sizeof(sh), 1, fin);
			while (i != 0) {
				haveData = true;
				sampleslevelRec sr;
				i = fread(&sr, sizeof(sr), 1, fin);
				if (i == 0 || sr.sidx == 0) {
					doaalcalcw(sh, vrec, mean_loss);
					vrec.clear();
					break;
				}
				if (sr.sidx == -1) mean_loss = sr.loss;
				if (sr.sidx >= 0) vrec.push_back(sr);
			}
			haveData = false;
			j++;
		}
		if (haveData == true) {
			doaalcalcw(sh, vrec, mean_loss);
		}
	}

	fclose(fin);
}

void aalcalc::doitw(const std::string &subfolder)
{
	std::string path = "work/" + subfolder;
	if (path.substr(path.length() - 1, 1) != "/") {
		path = path + "/";
	}
	initsameplsize(path);
	loadoccurrence();
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			std::string s = ent->d_name;
			if (s.length() > 4 && s.substr(s.length() - 4, 4) == ".bin") {
				s = path + ent->d_name;				
				process_summaryfilew(s);
				//setinputstream(s);
				//processinputfile(samplesize, event_to_periods, maxsummaryid, agg_out_loss, max_out_loss);
			}


		}
		applyweightingstomaps();
		do_sample_calc_endw();
		do_analytical_calc_endw();
		//outputsummarybin();
		//getnumberofperiods();
		outputresultscsv();
	}
	else {
		fprintf(stderr, "Unable to open directory %s\n", path.c_str());
		exit(-1);
	}
}