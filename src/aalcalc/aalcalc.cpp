
#include "aalcalc.h"

#if defined(_MSC_VER)
#include "../include/dirent.h"
#else
#include <dirent.h>
#endif
#include <math.h>
#include <string.h>
#include <set>



namespace summaryindex {
	void doit(const std::string& subfolder);
}

bool operator<(const period_sidx_map_key& lhs, const period_sidx_map_key& rhs)
{

	if (lhs.sidx != rhs.sidx) {
		return lhs.sidx < rhs.sidx;
	}
	else {
		if (lhs.period_no != rhs.period_no) {
			return lhs.period_no < rhs.period_no;
		}
		else {
			return lhs.summary_id < rhs.summary_id;
		}
	}

	
}

bool operator<(const period_map_key& lhs, const period_map_key& rhs)
{
	if (lhs.summary_id != rhs.summary_id) {
		return lhs.summary_id < rhs.summary_id;
	}
	else {
		return lhs.period_no < rhs.period_no;
	}

}


// Load and normalize weighting table 
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

}

void aalcalc::loadoccurrence()
{

	int date_algorithm_ = 0;
	FILE *fin = fopen(OCCURRENCE_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "FATAL: %s: cannot open %s\n", __func__, OCCURRENCE_FILE);
		exit(-1);
	}
	std::set<int>  periods;
	size_t i = fread(&date_algorithm_, sizeof(date_algorithm_), 1, fin);	// discard date algorithm
	i = fread(&no_of_periods_, sizeof(no_of_periods_), 1, fin);
	occurrence occ;
	i = fread(&occ, sizeof(occ), 1, fin);
	while (i != 0) {
		event_count_[occ.event_id] = event_count_[occ.event_id] + 1;
		event_to_period_[occ.event_id].push_back(occ.period_no);
		if (max_period_no_ < occ.period_no) max_period_no_ = occ.period_no;
		periods.insert(occ.period_no);
		i = fread(&occ, sizeof(occ), 1, fin);
	}
	
	if (max_period_no_ > no_of_periods_) {
		fprintf(stderr, "FATAL: Period numbers are not contigious\n");
		exit(-1);
	}
	fclose(fin);
}

void aalcalc::indexevents(const std::string& fullfilename, std::string& filename) {
	FILE* fin = fopen(fullfilename.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "FATAL: %s: cannot open %s\n", __func__, fullfilename.c_str());
		exit(EXIT_FAILURE);
	}
	std::string ss = filename.substr(1, filename.length() - 5);
	int fileindex = atoi(ss.c_str());
	long long offset = 0;
	int summarycalcstream_type = 0;
	size_t i = fread(&summarycalcstream_type, sizeof(summarycalcstream_type), 1, fin);
	if (i != 0) offset += sizeof(summarycalcstream_type);
	int stream_type = summarycalcstream_type & summarycalc_id;

	if (stream_type != summarycalc_id) {
		fprintf(stderr, "FATAL: %s: Not a summarycalc stream type %d\n", __func__, stream_type);
		exit(-1);
	}

	stream_type = streamno_mask & summarycalcstream_type;
	int samplesize;
	i = fread(&samplesize, sizeof(samplesize), 1, fin);
	if (i != 0) offset += sizeof(samplesize);
	int summary_set = 0;
	if (i != 0) i = fread(&summary_set, sizeof(summary_set), 1, fin);
	if (i != 0) offset += sizeof(summary_set);
	summarySampleslevelHeader sh;
	int last_event_id = -1;
	int last_summary_id = -1;
	long long last_offset = -1;
	bool skiprecord = false;
	while (i != 0) {
		i = fread(&sh, sizeof(sh), 1, fin);
		if (i != 0) {
			if (last_event_id != sh.event_id || last_summary_id != sh.summary_id) {				
				//fprintf(stdout, "%d, %s, %lld\n", sh.event_id, filename.c_str(), offset);
				if (skiprecord == false && last_event_id > 0) {
					event_offset_rec s;
					//s.filename = filename;
					s.offset = last_offset;
					s.event_id = last_event_id;
					s.fileindex = fileindex;
					summary_id_to_event_offset_[last_summary_id].push_back(s);
				}
				else {
					skiprecord = false;
				}
				last_event_id = sh.event_id;
				last_summary_id = sh.summary_id;
				last_offset = offset;
			}
			offset += sizeof(sh);
		}

		while (i != 0) {
			sampleslevelRec sr;
			i = fread(&sr, sizeof(sr), 1, fin);
			if (i != 0) offset += sizeof(sr);
			if (i == 0) break;
			if (sr.sidx == 0) break;
			if (sr.sidx == -1 && sr.loss == 0.0) skiprecord = true;
		}
	}

	fclose(fin);

}
struct ind_rec {
	int summary_id;
	int event_id;
	int fileindex;
	long long offset;
};
void savesummaryIndex(const std::string& subfolder,const std::map<int, std::vector<event_offset_rec>> &summary_id_to_event_offset)
{
	std::string path = "work/" + subfolder;
	if (path.substr(path.length() - 1, 1) != "/") {
		path = path + "/";
	}

	path = path + "index.idx";
	FILE* fout = fopen(path.c_str(), "wb");
	auto iter = summary_id_to_event_offset.begin();
	while (iter != summary_id_to_event_offset.end()) {
		auto iter2 = iter->second.begin();
		while (iter2 != iter->second.end()) {
			ind_rec xx;
			xx.summary_id = iter->first;
			xx.event_id = iter2->event_id;
			xx.fileindex = iter2->fileindex;
			xx.offset = iter2->offset;
			//fprintf(fout, "%d,%d,%d,%lld\n", iter->first, iter2->event_id,iter2->fileindex,iter2->offset);
			fwrite(&xx, sizeof(xx),1, fout);
			iter2++;
		}
		iter++;
	}
	fclose(fout);
}
void loadsummaryindex(const std::string& subfolder, std::map<int, std::vector<event_offset_rec>>& summary_id_to_event_offset)
{
	std::string path = "work/" + subfolder;
	if (path.substr(path.length() - 1, 1) != "/") {
		path = path + "/";
	}
	path = path + "index.idx";
	FILE* fin = fopen(path.c_str(), "rb");
	if (fin == nullptr) {
		fprintf(stderr, "FATAL: File %s not  found\n", path.c_str());
		exit(-1);
	}
	ind_rec idx;
	size_t i = fread(&idx, sizeof(idx), 1, fin);
	while (i > 0) {
		event_offset_rec r;
		r.event_id = idx.event_id;
		r.fileindex = idx.fileindex;
		r.offset = idx.offset;
		summary_id_to_event_offset[idx.summary_id].push_back(r);
		i = fread(&idx, sizeof(idx), 1, fin);
	}
	fclose(fin);
}
void aalcalc::load_event_to_summary_index(const std::string& subfolder)
{
	
	std::string path = "work/" + subfolder;
	if (path.substr(path.length() - 1, 1) != "/") {
		path = path + "/";
	}

	DIR* dir;
	struct dirent* ent;
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			std::string s = ent->d_name;
			if (s.length() > 4 && s.substr(s.length() - 4, 4) == ".bin") {
				std::string s2 = path + ent->d_name;				
				indexevents(s2, s);
			}
		}
	}
	else {
		fprintf(stderr, "FATAL: Unable to open directory %s\n", path.c_str());
		exit(-1);
	}
	// Save summaryIndex
	savesummaryIndex(subfolder,summary_id_to_event_offset_);	
	exit(-1);
}


void aalcalc::applyweightingstomap(std::map<int, aal_rec> &m)
{
	auto iter = m.begin();
	while (iter != m.end()) {
		iter++;
	}
}
void aalcalc::applyweightingstomaps()
{
	int i =(int) periodstoweighting_.size();
	if (i == 0) return;
	applyweightingstomap(map_analytical_aal_);
	applyweightingstomap(map_sample_aal_);
}
void aalcalc::do_calc_end_new()
{
	auto iter = set_periods_.begin();
	while (iter != set_periods_.end()) {
		int vidx = (samplesize_ + 1) * (*iter);
		for (int sidx = 0; sidx < samplesize_ + 1; sidx++) {
			loss_rec& aa = vec_sample_sum_loss_[vidx+sidx];
			if (sidx > 0) {
				aal_rec& a = vec_sample_aal_[current_summary_id_];
				a.type = 2;
				if (a.summary_id == 0) a.summary_id = current_summary_id_;
				if (a.max_exposure_value < aa.max_exposure_value) a.max_exposure_value = aa.max_exposure_value;
				a.mean += aa.sum_of_loss * aa.weighting;
				a.mean_squared += aa.sum_of_loss * aa.sum_of_loss * aa.weighting;
			}
			else {
				aal_rec& a = vec_analytical_aal_[current_summary_id_];
				a.type = 1;
				if (a.max_exposure_value < aa.max_exposure_value) a.max_exposure_value = aa.max_exposure_value;
				a.mean += aa.sum_of_loss * aa.weighting;
				a.mean_squared += aa.sum_of_loss * aa.sum_of_loss * aa.weighting;
				if (a.summary_id == 0) a.summary_id = current_summary_id_;
			}
			aa.max_exposure_value = 0;
			aa.sum_of_loss = 0;
			aa.weighting = 0;
		}
		iter++;
	}
	set_periods_.clear();
}

void aalcalc::do_sample_calc_newx(const summarySampleslevelHeader& sh, const std::vector<sampleslevelRec>& vrec) {

	// k.summary_id = sh.summary_id;
	auto p_iter = event_to_period_.find(sh.event_id);
	if (p_iter == event_to_period_.end()) return;

	double factor = (double)periodstoweighting_.size();

	for (auto period_no : p_iter->second) {

		set_periods_.emplace(period_no);
		double weighting = 1;
		auto iter = periodstoweighting_.find(period_no);
		if (factor > 0 && iter != periodstoweighting_.end()) {
			weighting = iter->second * factor;
		}
		else {
			if (factor > 0) {
				weighting = 0;
			}
		}

		int vidx = 0;
		for (auto x : vrec) {
			if (x.loss > 0) {
				int sidx = (x.sidx == -1) ? 0 : x.sidx;
				vidx = (samplesize_ + 1) * period_no + sidx;
				loss_rec & a = vec_sample_sum_loss_[vidx];
				if (a.max_exposure_value < sh.expval) a.max_exposure_value = sh.expval;
				a.sum_of_loss += x.loss;
				a.weighting = weighting;
			}
		}
	}
}


void aalcalc::do_sample_calc_new(const summarySampleslevelHeader& sh, const std::vector<sampleslevelRec>& vrec) {
	do_sample_calc_newx(sh, vrec);
}


void aalcalc::doaalcalc_new(const summarySampleslevelHeader& sh, const std::vector<sampleslevelRec>& vrec)
{
	do_sample_calc_new(sh, vrec);
}

void aalcalc::outputresultscsv_new(std::vector<aal_rec>& vec_aal, int periods,int sample_size)
{
	int p1 = periods * sample_size;
	int p2 = p1 - 1;

	auto v_iter = vec_aal.begin();
	while (v_iter != vec_aal.end()) {
		if (v_iter->summary_id > 0) {
			double mean = v_iter->mean / sample_size;
			double mean_squared = v_iter->mean * v_iter->mean;
			double s1 = v_iter->mean_squared - mean_squared / p1;
			double s2 = s1 / p2;
			double sd_dev = sqrt(s2);
			mean = mean / periods;
			printf("%d,%d,%f,%f,%f\n", v_iter->summary_id, v_iter->type, mean, sd_dev, v_iter->max_exposure_value);
		}
		v_iter++;
	}
}
void aalcalc::outputresultscsv_new()
{
	if (skipheader_ == false) printf("summary_id,type,mean,standard_deviation,exposure_value\n");

	outputresultscsv_new(vec_analytical_aal_, no_of_periods_,1);
	outputresultscsv_new(vec_sample_aal_, no_of_periods_ , samplesize_);

}
void aalcalc::outputresultscsv()
{
	if (skipheader_ == false) printf("summary_id,type,mean,standard_deviation,exposure_value\n");
	int p1 = no_of_periods_ ;
	int p2 = p1 - 1;

	for (auto x : map_analytical_aal_) {
		double mean = x.second.mean;
		double mean_squared = x.second.mean * x.second.mean;
		double s1 = x.second.mean_squared - mean_squared / p1;
		double s2 = s1 / p2;
		double sd_dev = sqrt(s2);
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
		mean = mean / no_of_periods_;
		printf("%d,%d,%f,%f,%f\n", x.first, x.second.type, mean, sd_dev, x.second.max_exposure_value);
	}

}
void aalcalc::initsameplsize(const std::string &path)
{
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			std::string s = ent->d_name;
			if (s.length() > 4 && s.substr(s.length() - 4, 4) == ".bin") {
				s = path + ent->d_name;
				FILE *fin = fopen(s.c_str(), "rb");
				if (fin == NULL) {
					fprintf(stderr, "FATAL:%s: cannot open %s\n", __func__, s.c_str());
					exit(EXIT_FAILURE);
				}
				int summarycalcstream_type = 0;
				fread(&summarycalcstream_type, sizeof(summarycalcstream_type), 1, fin);
				fread(&samplesize_, sizeof(samplesize_), 1, fin);
				fclose(fin);
				break;
			}

		}
	}
}

void aalcalc::getmaxsummaryid(std::string &path)
{
	std::string filename = path + "max_summary_id.idx";
	FILE *fin = fopen(filename.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "FATAL:%s: cannot open %s\n", __func__, filename.c_str());
		exit(EXIT_FAILURE);
	}

	char line[4096];
	if (fgets(line, sizeof(line), fin) != 0)
	{
		int ret = sscanf(line, "%d", &max_summary_id_);
		if (ret != 1) {
			fprintf(stderr, "FATAL:Invalid data in line %d:\n%s %d", 1, line, ret);
			exit(-1);
		}		
	}

	fclose(fin);
}
void aalcalc::doit(const std::string& subfolder)
{
	std::string path = "work/" + subfolder;
	if (path.substr(path.length() - 1, 1) != "/") {
		path = path + "/";
	}
	summaryindex::doit(subfolder);
	initsameplsize(path);
	getmaxsummaryid(path);
	loadoccurrence();
	loadperiodtoweigthing();	// move this to after the samplesize_ variable has been set i.e.  after reading the first 8 bytes of the first summary file
	char line[4096];
	vec_sample_sum_loss_.resize(((no_of_periods_+1) * (samplesize_+1)) + 1);
	vec_sample_aal_.resize(max_summary_id_ + 1);
	vec_analytical_aal_.resize(max_summary_id_ + 1);
	std::vector<std::string> filelist;
	std::vector<FILE *> filehandles;
	std::string filename = path + "filelist.idx";
	FILE* fin = fopen(filename.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "FATAL:%s: cannot open %s\n", __func__, filename.c_str());
		exit(EXIT_FAILURE);
	}
	
	while (fgets(line, sizeof(line), fin) != 0) {
		char *pos;
		if ((pos = strchr(line, '\n')) != NULL) *pos = '\0';   // remove newline from buffer		
		std::string s = line;
		filename = path + s;
		FILE* in = fopen(filename.c_str(), "rb");
		if (in == NULL) {
			fprintf(stderr, "FATAL:%s: cannot open %s\n", __func__, s.c_str());
			exit(EXIT_FAILURE);
		}
		filelist.push_back(s);
		filehandles.push_back(in);
	}

	fclose(fin);

	int lineno = 1;
	filename = path + "summaries.idx";
	fin = fopen(filename.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "FATAL:%s: cannot open %s\n", __func__, filename.c_str());
		exit(EXIT_FAILURE);
	}

	int summary_id;
	int file_index;
	long long file_offset;
	int last_summary_id = -1;
	int last_file_index = -1;
	FILE* summary_fin = nullptr;
	while (fgets(line, sizeof(line), fin) != 0)
	{
		int ret = sscanf(line, "%d, %d, %lld", &summary_id, &file_index, &file_offset);
		if (ret != 3) {
			fprintf(stderr, "FATAL:Invalid data in line %d:\n%s %d", lineno, line, ret);
			exit(-1);
		}
		else
		{
			if (last_summary_id != summary_id) {				
				current_summary_id_ = last_summary_id;
				applyweightingstomaps();
				do_calc_end_new();
				last_summary_id = summary_id;
			}
			
			if (last_file_index != file_index) {
				last_file_index = file_index;
				summary_fin = filehandles[file_index];
			}
			if (summary_fin != nullptr) {
				std::vector<sampleslevelRec> vrec;
				flseek(summary_fin, file_offset, SEEK_SET);
				summarySampleslevelHeader sh;
				size_t i = fread(&sh, sizeof(sh), 1, summary_fin);
				while (i != 0) {
					sampleslevelRec sr;
					i = fread(&sr, sizeof(sr), 1, summary_fin);
					if (i == 0 || sr.sidx == 0) break;
					vrec.push_back(sr);
				}
				doaalcalc_new(sh, vrec);
			}else {
				fprintf(stderr, "FATAL:File handle is a nullptr");
				exit(EXIT_FAILURE);
			}

		}
		lineno++;
	}

	fclose(fin);
	current_summary_id_ = last_summary_id;
	applyweightingstomaps();
	do_calc_end_new();
	
	outputresultscsv_new();
	auto iter = filehandles.begin();
	while (iter != filehandles.end()) {
		fclose(*iter);
		iter++;
	}
	filehandles.clear();	
}

void aalcalc::debug_process_summaryfile(const std::string &filename)
{
	FILE *fin = fopen(filename.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "FATAL:%s: cannot open %s\n", __func__, filename.c_str());
		exit(EXIT_FAILURE);
	}

	int summarycalcstream_type = 0;
	size_t i = fread(&summarycalcstream_type, sizeof(summarycalcstream_type), 1, fin);
	int stream_type = summarycalcstream_type & summarycalc_id;

	if (stream_type != summarycalc_id) {
		fprintf(stderr, "FATAL:%s: Not a summarycalc stream type %d\n", __func__, stream_type);
		exit(-1);
	}
	stream_type = streamno_mask & summarycalcstream_type;

	if (stream_type == 1) {
		int summary_set = 0;
		i = fread(&samplesize_, sizeof(samplesize_), 1, fin);
		if (i != 0) i = fread(&summary_set, sizeof(summary_set), 1, fin);
		printf("event_id,period_no,summary_id,sidx,loss\n");
		summarySampleslevelHeader sh;
		int j = 0;
		while (i != 0) {
			i = fread(&sh, sizeof(sh), 1, fin);
			while (i != 0) {
				sampleslevelRec sr;
				i = fread(&sr, sizeof(sr), 1, fin);
				if (i == 0) break;
				if (sr.sidx == 0) break;
				auto p_iter = event_to_period_[sh.event_id].begin();
				while (p_iter != event_to_period_[sh.event_id].end()) {
					printf("%d,%d,%d,%d,%f\n", sh.event_id, *p_iter, sh.summary_id, sr.sidx, sr.loss);
				}

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
		fprintf(stderr, "FATAL: Unable to open directory %s\n", path.c_str());
		exit(-1);
	}
}
