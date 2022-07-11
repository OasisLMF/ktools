
#include "aalcalc.h"

#if defined(_MSC_VER)
#include "../include/dirent.h"
#else
#include <dirent.h>
#endif

#include <algorithm>
#include <math.h>
#include <string.h>
#include <set>



namespace summaryindex {
	void doit(const std::string& subfolder, const std::map<int, std::vector<int>> &eventtoperiods);
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


void aalcalc::loadensemblemapping()
{
	FILE *fin = fopen(ENSEMBLE_FILE, "rb");
	if (fin == NULL) return;
	if (ord_output_ == true) {
		fprintf(stderr, "WARNING: ensembles not compatible with ORD format. Ignoring ensemble file.\n");
		return;
	}

	Ensemble e;
	sidxtoensemble_.resize(samplesize_ + 1, 0);
	std::fill(sidxtoensemble_.begin(), sidxtoensemble_.end(), 0);
	size_t i = fread(&e, sizeof(Ensemble), 1, fin);
	while (i != 0) {
		sidxtoensemble_[e.sidx] = e.ensemble_id;
		if (e.ensemble_id > max_ensemble_id_) {
			max_ensemble_id_ = e.ensemble_id;
		}

		i = fread(&e, sizeof(Ensemble), 1, fin);

	}

	// Check all sidx have ensemble IDs
	// Count how many sidx are associated with each ensemble ID
	ensemblecount_.resize(max_ensemble_id_ + 1, 0);
	for (std::vector<int>::const_iterator it = std::next(sidxtoensemble_.begin());
	     it != sidxtoensemble_.end(); it++) {
		if (*it == 0) {
			fprintf(stderr, "FATAL: All sidx must have associated ensemble IDs\n");
			exit(-1);
		}
		ensemblecount_[*it]++;
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

template<typename T>
void aalcalc::loadoccurrence(T &occ, FILE * fin)
{

	size_t i = fread(&no_of_periods_, sizeof(no_of_periods_), 1, fin);
	std::set<int> periods;
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

void aalcalc::loadoccurrence()
{

	int date_opts;
	int granular_date = 0;
	FILE *fin = fopen(OCCURRENCE_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "FATAL: %s: cannot open %s\n", __func__, OCCURRENCE_FILE);
		exit(-1);
	}
	std::set<int> periods;
	fread(&date_opts, sizeof(date_opts), 1, fin);
	granular_date = date_opts >> 1;
	if (granular_date) {
		occurrence_granular occ;
		loadoccurrence(occ, fin);
	} else {
		occurrence occ;
		loadoccurrence(occ, fin);
	}

}

void aalcalc::getsamplesizes()
{
	// Sample size subsets are fixed and non-overlapping, increasing in size
	// by a factor of 2^n, where n = subset number,
	// i.e. for 10 samples, sets are (1), (2, 3), (4, 5, 6, 7)
	// for 20 samples, sets are (1), (2, 3), (4, 5, 6, 7),
	//                          (8, 9, 10, 11, 12, 13, 14, 15)
	if (alct_output_ && samplesize_ > 1) {
		int i = 0;
		while (((1 << i) + ((1 << i) - 1)) <= samplesize_) {
			vec_sample_aal_[1 << i].resize(max_summary_id_ + 1);
			i++;
		}
	}
	vec_sample_aal_[samplesize_].resize(max_summary_id_ + 1);
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
				if (skiprecord == false && last_event_id > 0) {
					event_offset_rec s;
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

inline void aalcalc::fillensemblerec(const int sidx, const double mean,
				     const double weighting)
{
	int current_ensemble_id = sidxtoensemble_[sidx];
	aal_rec_ensemble& ea = vec_ensemble_aal_[max_ensemble_id_ * (current_summary_id_ - 1) + current_ensemble_id];
	ea.summary_id = current_summary_id_;
	ea.type = 2;
	ea.ensemble_id = current_ensemble_id;
	ea.mean += mean * weighting;
	ea.mean_squared += mean * mean * weighting;
}

void aalcalc::do_calc_end(const int period_no) {

	// Get weighting
	double weighting = 1;
	double factor = (double)periodstoweighting_.size();
	if (factor > 0) {
		if (periodstoweighting_.find(period_no) != periodstoweighting_.end()) {
			weighting = periodstoweighting_[period_no] * factor;
		} else {
			weighting = 0;
		}
	}

	// There is always an analytical mean
	double mean = vec_sample_sum_loss_[0];
	aal_rec& aa = vec_analytical_aal_[current_summary_id_];
	aa.type = 1;
	aa.summary_id = current_summary_id_;
	aa.mean += mean * weighting;
	aa.mean_squared += mean * mean * weighting;

	aal_rec_period& a_total = vec_sample_aal_[samplesize_][current_summary_id_];
	a_total.type = 2;
	a_total.summary_id = samplesize_ != 0 ? current_summary_id_ : 0;
	double total_mean_by_period = 0;
	auto iter = vec_sample_aal_.begin();
	for (int sidx = 1; sidx < samplesize_ + 1; sidx++) {
		while (iter->first != samplesize_) {
			aal_rec_period& a = iter->second[current_summary_id_];
			double mean_by_period = 0;
			for (sidx = iter->first; sidx < (iter->first << 1); sidx++)
			{
				mean = vec_sample_sum_loss_[sidx];
				a.type = 2;
				a.summary_id = current_summary_id_;
				mean_by_period += mean * weighting;
				a.mean += mean * weighting;
				total_mean_by_period += mean * weighting;
				a_total.mean += mean * weighting;
				a.mean_squared += mean * mean * weighting;
				a_total.mean_squared += mean * mean * weighting;
				if (sidxtoensemble_.size() > 0)
					fillensemblerec(sidx, mean, weighting);
			}
			a.mean_period += mean_by_period * mean_by_period;
			iter++;
		}
		mean = vec_sample_sum_loss_[sidx];
		total_mean_by_period += mean * weighting;
		a_total.mean += mean * weighting;
		a_total.mean_squared += mean * mean * weighting;
		if (sidxtoensemble_.size() > 0)
			fillensemblerec(sidx, mean, weighting);
	}
	a_total.mean_period += total_mean_by_period * total_mean_by_period;

	std::fill(vec_sample_sum_loss_.begin(), vec_sample_sum_loss_.end(), 0.0);

}


void aalcalc::do_calc_by_period(const summarySampleslevelHeader &sh,
				const std::vector<sampleslevelRec> &vrec) {

	for (auto x : vrec) {
		if (x.loss > 0) {
			int type_idx = (x.sidx != -1);
			int sidx = (type_idx == 0) ? 0 : x.sidx;
			vec_sample_sum_loss_[sidx] += x.loss;
		}
	}

}


template<typename aal_rec_T>
inline void aalcalc::calculatemeansddev(const aal_rec_T &record,
					const int sample_size, const int p1,
					const int p2, const int periods,
					double &mean, double &sd_dev) {

	mean = record.mean / sample_size;
	double mean_squared = record.mean * record.mean;
	double s1 = record.mean_squared - mean_squared / p1;
	double s2 = s1 / p2;
	sd_dev = sqrt(s2);
	mean = mean / periods;

}

inline void aalcalc::calculatemeansddev(const aal_rec_period &record,
					const int sample_size, const int p1,
					const int p2, const int periods,
					double &mean, double &sd_dev,
					double &var_vuln, double &var_haz) {

	calculatemeansddev(record, sample_size, p1, p2, periods, mean, sd_dev);
	double mean_period = record.mean_period / (sample_size * sample_size);

	// Vulnerability contribution
	double v1 = record.mean_squared - sample_size * mean_period;
	double v2 = v1 / (sample_size * periods - sample_size);
	var_vuln = v2 / (sample_size * periods);

	// Hazard contribution
	double h1 = mean_period - periods * mean * mean;
	double h2 = sample_size * h1 / (periods - 1);
	var_haz = h2 / (sample_size * periods);

}

double aalcalc::calculateconfidenceinterval(const double std_err) {

	// Find p-value above 0.5
	// F^-1(p) = -G^-1(1-p)
	double p_value = (1 + confidence_level_) / 2.;
	p_value = sqrt(-2. * log(1 - p_value));

	// Approximation formula for z-value from Abramowitz & Stegun, Handbook
	// of Mathematical Functions: with Formulas, Graphs, and Mathematical
	// Tables, Dover Publications (1965), eq. 26.2.23
	// Also see John D. Cook Consulting, https://www.johndcook.com/blog/cpp_phi_inverse/
	double c[3] = { 2.515517, 0.802853, 0.010328 };
	double d[3] = { 1.432788, 0.189269, 0.001308 };
	double z_value = p_value - ((c[2] * p_value + c[1]) * p_value + c[0]) /
			 (((d[2] * p_value + d[1]) * p_value + d[0]) * p_value + 1.);
	return std_err * z_value;

}

inline void aalcalc::outputrows(const char * buffer, int strLen, FILE * fout) {

	const char * bufPtr = buffer;
	int num;
	int counter = 0;
	do {

	//	num = printf("%s", bufPtr);
		num = fprintf(fout, "%s", bufPtr);
		if (num < 0) {   // Write error
			fprintf(stderr, "FATAL: Error writing %s: %s\n",
				buffer, strerror(errno));
			exit(EXIT_FAILURE);
		} else if (num < strLen) {   // Incomplete write
			bufPtr += num;
			strLen -= num;
		} else return;   // Success

		fprintf(stderr, "INFO: Attempt %d to write %s\n", ++counter,
			buffer);

	} while (counter < 10);

	fprintf(stderr, "FATAL: Maximum attempts to write %s exceeded\n",
		buffer);
	exit(EXIT_FAILURE);

}

#ifdef HAVE_PARQUET
template<typename aal_rec_T>
void aalcalc::outputresultsparquet(const std::vector<aal_rec_T>& vec_aal,
				   int sample_size, parquet::StreamWriter& os)
{
	int p1 = no_of_periods_ * sample_size;
	int p2 = p1 - 1;

	auto v_iter = vec_aal.begin();
	while (v_iter != vec_aal.end()) {
		if (v_iter->summary_id > 0) {
			double mean, sd_dev;
			calculatemeansddev(*v_iter, sample_size, p1, p2,
					   no_of_periods_, mean, sd_dev);
			os << v_iter->summary_id << v_iter->type << (float)mean
			   << (float)sd_dev << parquet::EndRow;
		}
		v_iter++;
	}
}
#endif


void aalcalc::outputresultscsv_new(const std::vector<aal_rec_ensemble> &vec_aal)
{
	auto v_iter = vec_aal.begin();
	while (v_iter != vec_aal.end()) {

		if(v_iter->summary_id > 0) {

			int sample_size = ensemblecount_[v_iter->ensemble_id];
			int p1 = no_of_periods_ * sample_size;
			int p2 = p1 - 1;
			double mean, sd_dev;
			calculatemeansddev(*v_iter, sample_size, p1, p2,
					   no_of_periods_, mean, sd_dev);

			char buffer[4096];
			int strLen;
			strLen = sprintf(buffer, "%d,%d,%f,%f,%d\n",
					v_iter->summary_id, v_iter->type,
					mean, sd_dev, v_iter->ensemble_id);
			outputrows(buffer, strLen);

		}

		v_iter++;

	}
}

template<typename aal_rec_T>
void aalcalc::outputresultscsv_new(std::vector<aal_rec_T>& vec_aal,
				   int sample_size)
{
	int p1 = no_of_periods_ * sample_size;
	int p2 = p1 - 1;

	auto v_iter = vec_aal.begin();
	while (v_iter != vec_aal.end()) {
		if (v_iter->summary_id > 0) {
			double mean, sd_dev;
			calculatemeansddev(*v_iter, sample_size, p1, p2,
					   no_of_periods_, mean, sd_dev);

			const int bufferSize = 4096;
			char buffer[bufferSize];
			int strLen;
			strLen = snprintf(buffer, bufferSize, "%d,%d,%f,%f",
					  v_iter->summary_id, v_iter->type,
					  mean, sd_dev);
			// If relevant use ensemble ID = 0 for calculations
			// across all ensembles
			if (sidxtoensemble_.size() > 0)
				strLen += snprintf(buffer+strLen, bufferSize-strLen, ",0");
			strLen += snprintf(buffer+strLen, bufferSize-strLen, "\n");
			outputrows(buffer, strLen);
		}
		v_iter++;
	}
}

void aalcalc::output_alct(std::map<int, std::vector<aal_rec_period>>& vec_aal)
{

	// Ensure file has csv extension to prevent conflict with potential
	// parquet format output file
	if (alct_outFile_.length() < 4 || alct_outFile_.substr(alct_outFile_.length() - 4, 4) != ".csv") {
		alct_outFile_ += ".csv";
	}
	FILE * fout = fopen(alct_outFile_.c_str(), "wb");
	fprintf(fout, "SummaryId,MeanLoss,SDLoss,SampleSize,LowerCI,UpperCI,"
		      "StandardError,RelativeError,VarElementHaz,"
		      "StandardErrorHaz,RelativeErrorHaz,VarElementVuln,"
		      "StandardErrorVuln,RelativeErrorVuln\n");

	if (!samplesize_) return;

	for (int summary_id = 1; summary_id < max_summary_id_ + 1; summary_id++) {
		for (auto iter = vec_aal.begin(); iter != vec_aal.end(); ++iter)
		{
			double mean, sd_dev, var_vuln, var_haz;
			int p1 = no_of_periods_ * iter->first;
			int p2 = p1 - 1;
			calculatemeansddev(iter->second[summary_id],
					   iter->first, p1, p2, no_of_periods_,
					   mean, sd_dev, var_vuln, var_haz);
			double std_err = sqrt(var_haz + var_vuln);
			double ci = calculateconfidenceinterval(std_err);
			double std_err_haz = sqrt(var_haz);
			double std_err_vuln = sqrt(var_vuln);

			const int bufferSize = 4096;
			char buffer[bufferSize];
			int strLen;
			strLen = snprintf(buffer, bufferSize,
					  "%d,%f,%f,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
					  summary_id, mean, sd_dev, iter->first,
					  mean - ci, mean + ci, std_err,
					  std_err/mean, var_haz, std_err_haz,
					  std_err_haz/mean, var_vuln,
					  std_err_vuln, std_err_vuln/mean);
			outputrows(buffer, strLen, fout);
		}
	}

	fclose(fout);

}

#ifdef HAVE_PARQUET
void aalcalc::output_alct_parquet(std::map<int, std::vector<aal_rec_period>>& vec_aal,
				  parquet::StreamWriter& os) {

	for (int summary_id = 1; summary_id < max_summary_id_ + 1; summary_id++) {
		for (auto iter = vec_aal.begin(); iter != vec_aal.end(); ++iter)
		{
			double mean, sd_dev, var_vuln, var_haz;
			int p1 = no_of_periods_ * iter->first;
			int p2 = p1 - 1;
			calculatemeansddev(iter->second[summary_id],
					   iter->first, p1, p2, no_of_periods_,
					   mean, sd_dev, var_vuln, var_haz);
			double std_err = sqrt(var_haz + var_vuln);
			double ci = calculateconfidenceinterval(std_err);
			double std_err_haz = sqrt(var_haz);
			double std_err_vuln = sqrt(var_vuln);

			os << summary_id << (float)mean << (float)sd_dev
			   << iter->first << (float)(mean - ci)
			   << (float)(mean + ci) << (float)std_err
			   << (float)(std_err/mean) << (float)var_haz
			   << (float)std_err_haz << (float)(std_err_haz/mean)
			   << (float)var_vuln << (float)std_err_vuln
			   << (float)(std_err_vuln/mean) << parquet::EndRow;
		}
	}
}
#endif

void aalcalc::outputresultscsv_new()
{
	/* parquet_output_ | ord_output_ | output files
	 * ---------------------------------------------------
	 *        1        |      1      | ORD parquet and csv
	 *        1        |      0      | ORD parquet
	 *        0        |      1      | ORD csv
	 *        0        |      0      | legacy csv
	 */
	if (skipheader_ == false) {
		if (ord_output_ == true) {
			printf("SummaryID,SampleType,MeanLoss,SDLoss\n");
		} else if (parquet_output_ == false) {
			printf("summary_id,type,mean,standard_deviation");
			if (sidxtoensemble_.size() > 0) printf(",ensemble_id");
			printf("\n");
		}
	}

#ifdef HAVE_PARQUET
	// Write parquet file
	if (parquet_output_) {
		std::vector<OasisParquet::ParquetFields> parquetFields;
		parquetFields.push_back(
			{"SummaryId", parquet::Type::INT32,
			parquet::ConvertedType::INT_32});
		parquetFields.push_back(
			{"SampleType", parquet::Type::INT32,
			parquet::ConvertedType::INT_32});
		parquetFields.push_back(
			{"MeanLoss", parquet::Type::FLOAT,
			parquet::ConvertedType::NONE});
		parquetFields.push_back(
			{"SDLoss", parquet::Type::FLOAT,
			parquet::ConvertedType::NONE});

		parquet::StreamWriter os =
		  OasisParquet::SetupParquetOutputStream(parquet_outFile_,
							 parquetFields);

		outputresultsparquet(vec_analytical_aal_, 1, os);
		outputresultsparquet(vec_sample_aal_[samplesize_], samplesize_,
				     os);

		if (alct_output_) {
			std::vector<OasisParquet::ParquetFields> parquetFields_alct;
			parquetFields_alct.push_back(
				{"SummaryId", parquet::Type::INT32,
				parquet::ConvertedType::INT_32});
			parquetFields_alct.push_back(
				{"MeanLoss", parquet::Type::FLOAT,
				parquet::ConvertedType::NONE});
			parquetFields_alct.push_back(
				{"SDLoss", parquet::Type::FLOAT,
				parquet::ConvertedType::NONE});
			parquetFields_alct.push_back(
				{"SampleSize", parquet::Type::INT32,
				parquet::ConvertedType::INT_32});
			parquetFields_alct.push_back(
				{"LowerCI", parquet::Type::FLOAT,
				parquet::ConvertedType::NONE});
			parquetFields_alct.push_back(
				{"UpperCI", parquet::Type::FLOAT,
				parquet::ConvertedType::NONE});
			parquetFields_alct.push_back(
				{"StandardError", parquet::Type::FLOAT,
				parquet::ConvertedType::NONE});
			parquetFields_alct.push_back(
				{"RelativeError", parquet::Type::FLOAT,
				parquet::ConvertedType::NONE});
			parquetFields_alct.push_back(
				{"VarElementHaz", parquet::Type::FLOAT,
				parquet::ConvertedType::NONE});
			parquetFields_alct.push_back(
				{"StandardErrorHaz", parquet::Type::FLOAT,
				parquet::ConvertedType::NONE});
			parquetFields_alct.push_back(
				{"RelativeErrorHaz", parquet::Type::FLOAT,
				parquet::ConvertedType::NONE});
			parquetFields_alct.push_back(
				{"VarElementVuln", parquet::Type::FLOAT,
				parquet::ConvertedType::NONE});
			parquetFields_alct.push_back(
				{"StandardErrorVuln", parquet::Type::FLOAT,
				parquet::ConvertedType::NONE});
			parquetFields_alct.push_back(
				{"RelativeErrorVuln", parquet::Type::FLOAT,
				parquet::ConvertedType::NONE});

			
			// Ensure file has parquet extension to prevent conflict
			// with potential csv format output file
			std::string parquet_alct_outFile = alct_outFile_;
			if (parquet_alct_outFile.length() < 8 || parquet_alct_outFile.substr(parquet_alct_outFile.length() -8, 8) != ".parquet") {
				parquet_alct_outFile += ".parquet";
			}
			parquet::StreamWriter os_alct =
			  OasisParquet::SetupParquetOutputStream(parquet_alct_outFile,
								 parquetFields_alct);

			output_alct_parquet(vec_sample_aal_, os_alct);

		}
	}
#endif

	// Write csv file
	if (!(parquet_output_ == true && ord_output_ == false)) {
		outputresultscsv_new(vec_analytical_aal_, 1);
		outputresultscsv_new(vec_sample_aal_[samplesize_], samplesize_);

		if (sidxtoensemble_.size() > 0) {
			outputresultscsv_new(vec_ensemble_aal_);
		}

		if (alct_output_) output_alct(vec_sample_aal_);
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
	loadoccurrence();
	summaryindex::doit(subfolder, event_to_period_);
	initsameplsize(path);
	getmaxsummaryid(path);
	getsamplesizes();
	loadperiodtoweigthing();	// move this to after the samplesize_ variable has been set i.e.  after reading the first 8 bytes of the first summary file
	loadensemblemapping();
	char line[4096];
	vec_sample_sum_loss_.resize(samplesize_+1, 0.0);
	vec_ensemble_aal_.resize(max_summary_id_ * max_ensemble_id_ + 1);
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
	int period_no;
	long long file_offset;
	int last_summary_id = -1;
	int last_period_no = -1;
	int last_file_index = -1;
	FILE* summary_fin = nullptr;
	while (fgets(line, sizeof(line), fin) != 0)
	{
		int ret = sscanf(line, "%d, %d, %d, %lld", &summary_id,
				&file_index, &period_no, &file_offset);
		if (ret != 4) {
			fprintf(stderr, "FATAL:Invalid data in line %d:\n%s %d",
				lineno, line, ret);
			exit(-1);
		}
		else
		{
			if (last_summary_id != summary_id) {
				if (last_summary_id != -1) {
					do_calc_end(last_period_no);
				}
				last_period_no = -1;   // Reset
				current_summary_id_ = summary_id;
				last_summary_id = summary_id;
			}
			if (last_period_no != period_no) {
				if (last_period_no != -1) {
					do_calc_end(last_period_no);
				}
				last_period_no = period_no;
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
					if (sr.sidx == chance_of_loss_idx || sr.sidx == max_loss_idx) continue;
					vrec.push_back(sr);
				}
				do_calc_by_period(sh, vrec);
			}else {
				fprintf(stderr, "FATAL:File handle is a nullptr");
				exit(EXIT_FAILURE);
			}

		}
		lineno++;
	}

	fclose(fin);
	current_summary_id_ = last_summary_id;
	if (last_summary_id != -1) do_calc_end(last_period_no);

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
