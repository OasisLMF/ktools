// TODO
#include "aalcalc.h"

#if defined(_MSC_VER)
#include "../include/dirent.h"
#else
#include <dirent.h>
#endif
#include <math.h>


bool operator<(const period_sidx_map_key& lhs, const period_sidx_map_key& rhs)
{
	if (lhs.period_no != rhs.period_no) {
		return lhs.period_no < rhs.period_no;
	}
	else {
		if (lhs.sidx != rhs.sidx) {
			return lhs.sidx < rhs.sidx;
		}else {
			return lhs.summary_id < rhs.summary_id;
		}
	}
}


// Load and normalize weigthting table 
// we must have entry for every return period!!!
// otherwise no way to pad missing ones
// Weightings should be between zero and 1 and should sum to one 
void aalcalc::loadperiodtoweigthing()
{
	FILE *fin = fopen(PERIODS_FILE, "rb");
	if (fin == NULL) return;
	Periods p;
	double total_weighting = 0;
	size_t i = fread(&p, sizeof(Periods), 1, fin);
	while (i != 0) {
		total_weighting += p.weighting;
		periodstoweighting_[p.period_no] = (OASIS_FLOAT)p.weighting;
		i = fread(&p, sizeof(Periods), 1, fin);
	}
	// If we are going to have weightings we should have them for all periods
	//	if (periodstowighting_.size() != no_of_periods_) {
	//		fprintf(stderr, "Total number of periods in %s does not match the number of periods in %s\n", PERIODS_FILE, OCCURRENCE_FILE);
	//		exit(-1);
	//	}
	// Weighting already normalzed just split over samples...
	auto iter = periodstoweighting_.begin();
	while (iter != periodstoweighting_.end()) {
		// iter->second = iter->second / total_weighting; // no need sinece already normalized 
		if (samplesize_ == -1) { 
			fprintf(stderr, "Sample size not initialzed\n"); 
			exit(EXIT_FAILURE);
		}
		if (samplesize_) iter->second = iter->second / samplesize_;   // split weighting over samples
		iter++;
	}
}

void aalcalc::loadoccurrence()
{

	int date_algorithm_ = 0;
	FILE *fin = fopen(OCCURRENCE_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, OCCURRENCE_FILE);
		exit(-1);
	}

	size_t i = fread(&date_algorithm_, sizeof(date_algorithm_), 1, fin);	// discard date algorithm
	i = fread(&no_of_periods_, sizeof(no_of_periods_), 1, fin);
	occurrence occ;
	i = fread(&occ, sizeof(occ), 1, fin);
	while (i != 0) {
		event_count_[occ.event_id] = event_count_[occ.event_id] + 1;
		event_to_period_[occ.event_id] = occ.period_no;
		i = fread(&occ, sizeof(occ), 1, fin);
	}

	fclose(fin);
}
void aalcalc::applyweightings(int event_id, const std::map <int, double> &periodstoweighting, std::vector<sampleslevelRec> &vrec)
{
	if (periodstoweighting.size() == 0) return;
	double factor = periodstoweighting.size();
	auto it = event_to_period_.find(event_id);
	if (it != event_to_period_.end()) {
		auto iter = periodstoweighting.find(it->second);
		if (iter != periodstoweighting.end()) {
			for (int i = 0; i < vrec.size(); i++) {
				vrec[i].loss = vrec[i].loss * iter->second * factor;
			}
		}
		else {
			// Event not found in periods.bin so no weighting i.e zero 
			for (int i = 0; i < vrec.size(); i++) vrec[i].loss = 0;
			//	fprintf(stderr, "Event %d not found in periods.bin\n", event_id);
			//		exit(-1);
		}
	}
	else {
		fprintf(stderr, "Event ID %d not found\n", event_id);
	}
}

void aalcalc::applyweightingstomap(std::map<int, aal_rec> &m, int i)
{
	auto iter = m.begin();
	while (iter != m.end()) {
		//iter->second.mean = iter->second.mean * i;
		//iter->second.mean_squared = iter->second.mean_squared * i * i;
		iter++;
	}
}
void aalcalc::applyweightingstomaps()
{
	int i = periodstoweighting_.size();
	if (i == 0) return;
	applyweightingstomap(map_analytical_aal_, i);
	applyweightingstomap(map_sample_aal_, i);
}
int analytic_count = 0;
void aalcalc::do_analytical_calc(const summarySampleslevelHeader &sh, double mean_loss)
{

	if (periodstoweighting_.size() > 0) {
		double factor = (double)periodstoweighting_.size();
		auto it = event_to_period_.find(sh.event_id);
		if (it != event_to_period_.end()) {
			auto iter = periodstoweighting_.find(it->second);
			if (iter != periodstoweighting_.end()) {
				mean_loss = mean_loss * iter->second * factor;
			}
			else {
				// no weighting so assume its zero
				mean_loss = 0;
			}
		}
	}
	double mean_squared = mean_loss * mean_loss;
	int count = event_count_[sh.event_id];		// do the cartesian
	mean_loss = mean_loss * count;
	mean_squared = mean_squared * count;
	auto iter = map_analytical_aal_.find(sh.summary_id);
	if (iter != map_analytical_aal_.end()) {
		aal_rec &a = iter->second;
		if (a.max_exposure_value < sh.expval) a.max_exposure_value = sh.expval;
		a.mean += mean_loss;
		a.mean_squared += mean_squared;
		//if (mean_loss > 0) {
		//	analytic_count++;
		//	//if (analytic_count == 14461) {
		//	//	fprintf(stderr, "Got here\n");
		//	//}
		//	//printf("%d: %d,%d, %f, %f, %f \n", analytic_count, a.summary_id, a.type, a.mean, a.mean_squared, a.max_exposure_value);
		//}
	}
	else {
		aal_rec a;
		a.summary_id = sh.summary_id;
		a.type = 1;
		a.max_exposure_value = sh.expval;
		a.mean = mean_loss;
		a.mean_squared = mean_squared;
		map_analytical_aal_[sh.summary_id] = a;
		//if (mean_loss > 0) {
		//	analytic_count++;
		//	//printf("%d: %d,%d, %f, %f, %f \n", analytic_count, a.summary_id, a.type, a.mean, a.mean_squared, a.max_exposure_value);
		//}
	}
}
void aalcalc::do_sample_calc_end(){

	auto iter = map_sample_sum_loss_.begin();


	while (iter != map_sample_sum_loss_.end()) {
		auto a_iter = map_sample_aal_.find(iter->first.summary_id);
		if (a_iter != map_sample_aal_.end()) {
			aal_rec &a = a_iter->second;
			if (a.max_exposure_value < iter->second.max_exposure_value) a.max_exposure_value = iter->second.max_exposure_value;
			a.mean += iter->second.sum_of_loss;
			a.mean_squared += iter->second.sum_of_loss * iter->second.sum_of_loss;
			//a.mean_squared += iter->second.sum_of_loss_squared;
		}
		else {
			aal_rec a;
			a.summary_id = iter->first.summary_id;
			a.type = 2;
			a.max_exposure_value = iter->second.max_exposure_value;
			a.mean = iter->second.sum_of_loss;
			a.mean_squared = iter->second.sum_of_loss * iter->second.sum_of_loss;
			//a.mean_squared = iter->second.sum_of_loss_squared;
			map_sample_aal_[iter->first.summary_id] = a;
		}
		iter++;
	}



	//int p1 = no_of_periods_ * samplesize_;
	//int p2 = p1 - 1;

	//iter = map_sample_sum_loss_.begin();
	//double mean = 0;
	//double sum_of_square_losses = 0;
	//while (iter != map_sample_sum_loss_.end()) {
	//	mean += iter->second.sum_of_loss;
	//	sum_of_square_losses += iter->second.sum_of_loss_squared;
	//	iter++;
	//}
	//mean = mean / samplesize_;
	//double mean_squared = mean * mean;

	//double s1 = sum_of_square_losses - mean_squared / p1;
	//double s2 = s1 / p2;
	//double sd_dev = sqrt(s2);
	//fprintf(stderr, "");
}
void aalcalc::do_sample_calc(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec){

	period_sidx_map_key k;
	k.period_no = event_to_period_[sh.event_id];
	k.summary_id = sh.summary_id;

	if (k.period_no == 0) return;

	//if (k.summary_id != 1) return;

	for (auto x : vrec) {
		k.sidx = x.sidx;
		auto iter = map_sample_sum_loss_.find(k);
		if (iter != map_sample_sum_loss_.end()) {
			loss_rec &a = iter->second;
			if (a.max_exposure_value < sh.expval) a.max_exposure_value = sh.expval;
			a.sum_of_loss += x.loss;
			//a.sum_of_loss_squared += x.loss * x.loss;
		}
		else {
			loss_rec l;
			l.sum_of_loss = x.loss;
			//l.sum_of_loss_squared = x.loss * x.loss;
			l.max_exposure_value = sh.expval;
			map_sample_sum_loss_[k] = l;
		}

	}


}
void aalcalc::do_sample_calc_old(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec)
{
	OASIS_FLOAT mean_loss = 0;
	OASIS_FLOAT mean_squared = 0;
	for (auto x : vrec) {
		mean_loss += x.loss;
		mean_squared += x.loss * x.loss;
	}
	//mean_loss = mean_loss / samplesize_;	
	//mean_squared = mean_squared / (samplesize_*samplesize_);
	//OASIS_FLOAT mean_squared = mean_loss*mean_loss;
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


void aalcalc::doaalcalc(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec, OASIS_FLOAT mean_loss)
{
	do_analytical_calc(sh, mean_loss);
	if (samplesize_) do_sample_calc(sh, vrec);
}

void aalcalc::process_summaryfile(const std::string &filename)
{
	FILE *fin= fopen(filename.c_str(), "rb");
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
					applyweightings(sh.event_id, periodstoweighting_, vrec);
					doaalcalc(sh, vrec, mean_loss);
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
			applyweightings(sh.event_id, periodstoweighting_, vrec);
			doaalcalc(sh, vrec, mean_loss);
		}
	}
	
	fclose(fin);
}
void aalcalc::outputresultscsv()
{
	if (skipheader_ == false) printf("summary_id,type,mean,standard_deviation,exposure_value\n");
	int p1 = no_of_periods_ * samplesize_;
	int p2 = p1 - 1;

	for (auto x : map_analytical_aal_) {
		double mean = x.second.mean;
		double mean_squared = x.second.mean * x.second.mean;
		double s1 = x.second.mean_squared - mean_squared / p1;
		double s2 = s1 / p2;
		double sd_dev = sqrt(s2);
		//double sd_dev = sqrt((x.second.mean_squared - (x.second.mean * x.second.mean / no_of_periods_)) / (no_of_periods_ - 1));
		mean = mean / no_of_periods_;
		printf("%d,%d,%f,%f,%f\n", x.first, x.second.type, mean, sd_dev, x.second.max_exposure_value);
	}

	p1 = no_of_periods_ * samplesize_;
	p2 = p1 - 1;

	for (auto x : map_sample_aal_) {
		double mean = x.second.mean / samplesize_;
		double mean_squared = x.second.mean * x.second.mean;
		double s1 = x.second.mean_squared - mean_squared / p1;
		double s2 = s1 / p2;
		double sd_dev = sqrt(s2);
		//double sd_dev = sqrt((x.second.mean_squared - (x.second.mean * x.second.mean / no_of_periods_)) / (no_of_periods_ - 1));
		mean = mean / no_of_periods_;
		printf("%d,%d,%f,%f,%f\n", x.first, x.second.type, mean, sd_dev, x.second.max_exposure_value);
	}

}

void aalcalc::doit(const std::string &subfolder)
{
	loadoccurrence();
	loadperiodtoweigthing();	// move this to after the samplesize_ variable has been set i.e.  after reading the first 8 bytes of the first summary file
	std::string path = "work/" + subfolder;
	if (path.substr(path.length() - 1, 1) != "/") {
		path = path + "/";
	}

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			std::string s = ent->d_name;
			if (s.length() > 4 && s.substr(s.length() - 4, 4) == ".bin") {
				s = path + ent->d_name;				
				process_summaryfile(s);
				//setinputstream(s);
				//processinputfile(samplesize, event_to_periods, maxsummaryid, agg_out_loss, max_out_loss);
			}


		}
		applyweightingstomaps();
		do_sample_calc_end();
		//outputsummarybin();
		//getnumberofperiods();
		outputresultscsv();
	}
	else {
		fprintf(stderr, "Unable to open directory %s\n", path.c_str());
		exit(-1);
	}	
}
void aalcalc::debug_process_summaryfile(const std::string &filename)
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
		printf("event_id,period_no,summary_id,sidx,loss\n");
		summarySampleslevelHeader sh;
		int j = 0;
		OASIS_FLOAT mean_loss = 0;
		while (i != 0) {
			i = fread(&sh, sizeof(sh), 1, fin);
			while (i != 0) {
				haveData = true;
				sampleslevelRec sr;
				i = fread(&sr, sizeof(sr), 1, fin);
				if (i == 0) break;
				if (sr.sidx == 0) break;
				printf("%d,%d,%d,%d,%f\n", sh.event_id, event_to_period_[sh.event_id], sh.summary_id, sr.sidx, sr.loss);
			}
			j++;
		}		
	}

	fclose(fin);
}


void aalcalc::debug(const std::string &subfolder)
{
	loadoccurrence();
	std::string path = "work/" + subfolder;
	if (path.substr(path.length() - 1, 1) != "/") {
		path = path + "/";
	}

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			std::string s = ent->d_name;
			if (s.length() > 4 && s.substr(s.length() - 4, 4) == ".bin") {
				s = path + ent->d_name;
				debug_process_summaryfile(s);
			}
		}
	}
	else {
		fprintf(stderr, "Unable to open directory %s\n", path.c_str());
		exit(-1);
	}
}