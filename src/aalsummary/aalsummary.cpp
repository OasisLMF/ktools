/*
* Copyright (c)2015 - 2016 Oasis LMF Limited
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*
*   * Neither the original author of this software nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
* THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*/
/*
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/

#include "aalsummary.h"

#include <regex>
#include <math.h>

#if defined(_MSC_VER)
#include "../include/dirent.h"
#else
#include <dirent.h>
#endif

void aalsummary::outputresultscsv()
{
	if(skipheader_ == false) printf("summary_id,type,mean,standard_deviation,exposure_value\n");
	int p1 = no_of_periods_ * no_of_samples_;
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

	p1 = no_of_periods_ * no_of_samples_;
	p2 = p1 - 1;

	for (auto x : map_sample_aal_) {
		double mean = x.second.mean / no_of_samples_;
		double mean_squared = x.second.mean * x.second.mean;
		double s1 = x.second.mean_squared - mean_squared / p1;
		double s2 = s1 / p2;
		double sd_dev = sqrt(s2);
		//double sd_dev = sqrt((x.second.mean_squared - (x.second.mean * x.second.mean / no_of_periods_)) / (no_of_periods_ - 1));
		mean = mean / no_of_periods_;
		printf("%d,%d,%f,%f,%f\n", x.first, x.second.type, mean, sd_dev, x.second.max_exposure_value);
	}

}

void aalsummary::getnumberofperiods()
{

	int date_algorithm_ = 0;
	FILE *fin = fopen(OCCURRENCE_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, OCCURRENCE_FILE);
		exit(-1);
	}

	size_t i = fread(&date_algorithm_, sizeof(date_algorithm_), 1, fin);
	i = fread(&no_of_periods_, sizeof(no_of_periods_), 1, fin);
	
	fclose(fin);
}

void aalsummary::processrec(const aal_rec &aalrec, std::map<int, aal_rec> &map_aal)
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
void aalsummary::process_file(const std::string &s)
{

	FILE *fin = fopen(s.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "Unable to open %s\n", s.c_str());
		exit(-1);
	}
	size_t i = fread(&no_of_samples_, sizeof(int), 1, fin);
	aal_rec aalrec;
	i = fread(&aalrec, sizeof(aal_rec), 1, fin);
	while (i != 0) {
		if (aalrec.type == 1) processrec(aalrec, map_analytical_aal_);
		if (aalrec.type == 2) processrec(aalrec, map_sample_aal_);
		i = fread(&aalrec, sizeof(aal_rec), 1, fin);
	}
	fclose(fin);
}

void aalsummary::doit(const std::string &subfolder)
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
