// Implemention of aalcalc using the welford method
#include "aalcalc.h"
#if defined(_MSC_VER)
#include "../include/dirent.h"
#else
#include <dirent.h>
#endif


float I = 0;
float K = 0;
float L = 0;
// standard deviation implementation using welford method
void aalcalc::do_analytical_calcw(const summarySampleslevelHeader &sh, double mean_loss)
{
	I = I + 1;
	float J = mean_loss;
	float k = K + (J - K) / I;
	L = L + (J-k)*(J-K);
	//fprintf(stderr,"I=%f J=%f K=%f L=%f\n", I, J, K, L);
	fprintf(stderr, "%f\n",J);
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
		do_sample_calc_end();
		do_analytical_calc_end();
		//outputsummarybin();
		//getnumberofperiods();
		outputresultscsv();
	}
	else {
		fprintf(stderr, "Unable to open directory %s\n", path.c_str());
		exit(-1);
	}
}