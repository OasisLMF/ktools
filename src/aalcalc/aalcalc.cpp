/*
* Copyright (c)2016 Oasis LMF Limited
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
#include "aalcalc.hpp"
#include "../../config.h"
#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <getopt.h>
#endif

#ifdef __unix
#include <unistd.h>
#endif


void aalcalc::loadoccurrence()
{
	
	int date_algorithm_ = 0;
	FILE *fin = fopen(OCCURRENCE_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, OCCURRENCE_FILE);
		exit(-1);
	}

	size_t i = fread(&date_algorithm_, sizeof(date_algorithm_), 1, fin);
	i = fread(&no_of_periods_, sizeof(no_of_periods_), 1, fin);
	occurrence occ;
	i = fread(&occ, sizeof(occ), 1, fin);
	while (i != 0) {
		event_count_[occ.event_id] = event_count_[occ.event_id] + 1;
		i = fread(&occ, sizeof(occ), 1, fin);
	}

	fclose(fin);
}


void aalcalc::outputresultscsv()
{
	printf("summary_id,type,mean, mean_squared\n");

	for (auto x : map_analytical_aal_) {
		float mean = static_cast<float>(x.second.mean);
		float sd_dev = static_cast<float>(static_cast<float>(sqrt((x.second.mean_squared - (x.second.mean * x.second.mean / no_of_periods_)) / (no_of_periods_ - 1))));
		printf("%d,%d, %f, %f, %f \n", x.first, x.second.type,mean, sd_dev, x.second.max_exposure_value);
	}

	for (auto x : map_sample_aal_) {
		float mean = static_cast<float>(x.second.mean);
		float sd_dev = static_cast<float>(sqrt((x.second.mean_squared - (x.second.mean * x.second.mean / no_of_periods_)) / (no_of_periods_ - 1)));
		printf("%d,%d, %f, %f, %f \n", x.first,x.second.type, mean, sd_dev, x.second.max_exposure_value);
	}

}

void aalcalc::outputsummarybin()
{

	for (auto x : map_analytical_aal_) {
		fwrite(&x.second, sizeof(aal_rec), 1, stdout);
	}

	for (auto x : map_sample_aal_) {
		fwrite(&x.second, sizeof(aal_rec), 1, stdout);
	}
}

void aalcalc::do_analytical_calc(const summarySampleslevelHeader &sh,  float mean_loss)
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

void aalcalc::do_sample_calc(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec, int samplesize)
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

void aalcalc::doaalcalc(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec, float mean_loss,int samplesize)
{
	do_analytical_calc(sh, mean_loss);
	if (samplesize) do_sample_calc(sh, vrec, samplesize);
}

void aalcalc::doit()
{
	loadoccurrence();
	int summarycalcstream_type = 0;
	size_t i = fread(&summarycalcstream_type, sizeof(summarycalcstream_type), 1, stdin);
	int stream_type = summarycalcstream_type & summarycalc_id;

	if (stream_type != summarycalc_id) {
		fprintf(stderr, "%s: Not a summarycalc stream type\n", __func__);
		exit(-1);
	}
	stream_type = streamno_mask & summarycalcstream_type;
	
	if (stream_type == 1) {		
		int summary_set = 0;
		int samplesize = 0;
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

	outputsummarybin();
}

void help()
{
	fprintf(stderr, "-h help\n-v version\n");
}

int main(int argc, char* argv[])
{

	int opt;

	while ((opt = getopt(argc, argv, "vh")) != -1) {
		switch (opt) {
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'h':
		default:
			help();
			exit(EXIT_FAILURE);
		}
	}
	initstreams();
	aalcalc a;
	a.doit();

	return 0;

}
