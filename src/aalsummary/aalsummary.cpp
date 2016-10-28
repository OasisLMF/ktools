#include <regex>
#include <map>
#include <math.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#include "../include/dirent.h"
#else
#include <dirent.h>
#include <getopt.h>
#endif

#include "../include/oasis.hpp"


int no_of_periods_ = 0;
std::map<int, aal_rec> map_analytical_aal_;
std::map<int, aal_rec> map_sample_aal_;

void outputresultscsv()
{
	printf("summary_id,type,mean,standard_deviation,exposure_value\n");

	for (auto x : map_analytical_aal_) {
		double mean = x.second.mean;
		double sd_dev = sqrt((x.second.mean_squared - (x.second.mean * x.second.mean / no_of_periods_)) / (no_of_periods_ - 1));
		mean = mean / no_of_periods_;
		printf("%d,%d,%f,%f,%f\n", x.first, x.second.type, mean, sd_dev, x.second.max_exposure_value);
	}

	for (auto x : map_sample_aal_) {
		double mean = x.second.mean;
		double sd_dev = sqrt((x.second.mean_squared - (x.second.mean * x.second.mean / no_of_periods_)) / (no_of_periods_ - 1));
		mean = mean / no_of_periods_;
		printf("%d,%d,%f,%f,%f\n", x.first, x.second.type, mean, sd_dev, x.second.max_exposure_value);
	}

}


void getnumberofperiods()
{

	int date_algorithm_ = 0;
	FILE *fin = fopen(OCCURRENCE_FILE, "rb");
	if (fin == NULL) {
		std::cerr << "loadoccurrence: Unable to open " << OCCURRENCE_FILE << "\n";
		exit(-1);
	}

	size_t i = fread(&date_algorithm_, sizeof(date_algorithm_), 1, fin);
	i = fread(&no_of_periods_, sizeof(no_of_periods_), 1, fin);
	
	fclose(fin);
}

void processrec(const aal_rec &aalrec, std::map<int, aal_rec> &map_aal)
{
	auto iter = map_aal.find(aalrec.summary_id);
	if (iter != map_aal.end()) {
		aal_rec &a = iter->second;
		if (a.max_exposure_value < aalrec.max_exposure_value) a.max_exposure_value = aalrec.max_exposure_value;
		a.mean += aalrec.mean;
		a.mean_squared += aalrec.mean_squared;
	}
	else {
		map_aal[aalrec.summary_id] = aalrec;
	}

}
void process_file(const std::string &s)
{

	FILE *fin = fopen(s.c_str(), "rb");
	aal_rec aalrec;
	size_t i = fread(&aalrec, sizeof(aal_rec), 1, fin);
	while (i != 0) {
		if (aalrec.type == 1) processrec(aalrec, map_analytical_aal_);
		if (aalrec.type == 2) processrec(aalrec, map_sample_aal_);
		i = fread(&aalrec, sizeof(aal_rec), 1, fin);
	}
	fclose(fin);
}

void doit(const std::string &subfolder)
{
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
				process_file(s);
			}
		}
	}
	getnumberofperiods();
	outputresultscsv();
}
void help()
{
	fprintf(stderr, "-K workspace sub folder\n");
	exit(-1);
}
int main(int argc, char* argv[])
{

	std::string subfolder;
	int opt;
	while ((opt = getopt(argc, argv, "K:")) != -1) {
		switch (opt) {
		case 'K':
			subfolder = optarg;
			break;		
		default:
			fprintf(stderr, "unknown parameter\n");
			::exit(EXIT_FAILURE);
		}
	}

	if (argc == 1) {
		fprintf(stderr, "Invalid parameters\n");
		help();
	}




	initstreams();
	doit(subfolder);
	return 0;

}
