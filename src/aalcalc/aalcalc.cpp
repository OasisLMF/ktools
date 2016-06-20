#include "../include/oasis.hpp"
#include <map>
#include <vector>
#include <math.h>

std::map<int, int> event_count_;	// count of events in occurence table used to create cartisian effect on event_id
int no_of_periods_ = 0;
std::map<int, aal_rec> map_analytical_aal_;
std::map<int, aal_rec> map_sample_aal_;

void loadoccurrence()
{
	
	int date_algorithm_ = 0;
	FILE *fin = fopen(OCCURRENCE_FILE, "rb");
	if (fin == NULL) {
		std::cerr << "loadoccurrence: Unable to open " << OCCURRENCE_FILE << "\n";
		exit(-1);
	}

	int i = fread(&date_algorithm_, sizeof(date_algorithm_), 1, fin);
	i = fread(&no_of_periods_, sizeof(no_of_periods_), 1, fin);
	occurrence occ;
	i = fread(&occ, sizeof(occ), 1, fin);
	while (i != 0) {
		event_count_[occ.event_id] = event_count_[occ.event_id] + 1;
		i = fread(&occ, sizeof(occ), 1, fin);
	}

	fclose(fin);
}


void outputresultscsv()
{
	printf("summary_id,type,mean, mean_squared\n");

	for (auto x : map_analytical_aal_) {
		float mean = x.second.mean;
		float sd_dev = sqrt((x.second.mean_squared - (x.second.mean * x.second.mean / no_of_periods_)) / (no_of_periods_ - 1));
		printf("%d,%d, %f, %f, %f \n", x.first, x.second.type,mean, sd_dev, x.second.max_exposure_value);
	}

	for (auto x : map_sample_aal_) {
		float mean = x.second.mean;
		float sd_dev = sqrt((x.second.mean_squared - (x.second.mean * x.second.mean / no_of_periods_)) / (no_of_periods_ - 1));
		printf("%d,%d, %f, %f, %f \n", x.first,x.second.type, mean, sd_dev, x.second.max_exposure_value);
	}

}

void outputsummarybin()
{
	//fwrite(&no_of_periods_, sizeof(no_of_periods_), 1, stdout);

	for (auto x : map_analytical_aal_) {
		fwrite(&x.second, sizeof(aal_rec), 1, stdout);
	}

	for (auto x : map_sample_aal_) {
		fwrite(&x.second, sizeof(aal_rec), 1, stdout);
	}
}

void do_analytical_calc(const summarySampleslevelHeader &sh,  float mean_loss)
{	
	float mean_squared = mean_loss*mean_loss;
	int count = event_count_[sh.event_id];		// do the cartesian
	mean_loss = mean_loss * count;
	mean_squared = mean_squared * count;
	auto iter = map_analytical_aal_.find(sh.summary_id);
	if (iter != map_analytical_aal_.end()) {
		aal_rec &a = iter->second;
		if (a.max_exposure_value < sh.expval) a.max_exposure_value = sh.expval;
		a.mean += mean_loss;
		a.mean_squared += mean_squared;
	}
	else {
		aal_rec a;
		a.summary_id = sh.summary_id;
		a.type = 1;
		a.max_exposure_value = sh.expval;
		a.mean = mean_loss;
		a.mean_squared = mean_squared;
		map_analytical_aal_[sh.summary_id] = a;
	}
}
void do_sample_calc(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec, int samplesize)
{
	float mean_loss=0;
	for (auto x : vrec) {
		mean_loss += x.loss;
	}
	mean_loss = mean_loss / samplesize;	
	float mean_squared = mean_loss*mean_loss;
	int count = event_count_[sh.event_id];		// do the cartesian
	mean_loss = mean_loss * count;
	mean_squared = mean_squared * count;
	auto iter = map_sample_aal_.find(sh.summary_id);
	if (iter != map_sample_aal_.end()) {
		aal_rec &a = iter->second;
		if (a.max_exposure_value < sh.expval) a.max_exposure_value = sh.expval;
		a.mean += mean_loss;
		a.mean_squared += mean_squared;
	}
	else {
		aal_rec a;
		a.summary_id = sh.summary_id;
		a.type = 2;
		a.max_exposure_value = sh.expval;
		a.mean = mean_loss;
		a.mean_squared = mean_squared;
		map_sample_aal_[sh.summary_id] = a;
	}
	
}

void doaalcalc(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec, float mean_loss,int samplesize)
{
	do_analytical_calc(sh, mean_loss);
	do_sample_calc(sh, vrec, samplesize);
}

void doit()
{
	loadoccurrence();
	int summarycalcstream_type = 0;
	int i = fread(&summarycalcstream_type, sizeof(summarycalcstream_type), 1, stdin);
	int stream_type = summarycalcstream_type & summarycalc_id;

	if (stream_type != summarycalc_id) {
		std::cerr << "Not a summarycalc stream type\n";
		exit(-1);
	}
	stream_type = streamno_mask & summarycalcstream_type;
	
	if (stream_type == 1) {
		int samplesize;
		int summary_set = 0;
		i = fread(&samplesize, sizeof(samplesize), 1, stdin);
		if (i != 0) i = fread(&summary_set, sizeof(summary_set), 1, stdin);
		std::vector<sampleslevelRec> vrec;
		summarySampleslevelHeader sh;
		int j = 0;
		float mean_loss = 0;
		while (i != 0) {
			i = fread(&sh, sizeof(sh), 1, stdin);
			while (i != 0) {
				sampleslevelRec sr;
				i = fread(&sr, sizeof(sr), 1, stdin);
				if (i == 0 || sr.sidx == 0) {
					doaalcalc(sh, vrec, mean_loss, samplesize);
					vrec.clear();
					break;
				}
				if (sr.sidx == -1) mean_loss = sr.loss;
				if (sr.sidx >=0) vrec.push_back(sr);
			}

			j++;
		}
		doaalcalc(sh, vrec,mean_loss, samplesize);
	}
	// outputresultscsv();
	outputsummarybin();
}

int main(int argc, char* argv[])
{

	initstreams();
	doit();
	fprintf(stderr, "\n");
	return 0;

}
