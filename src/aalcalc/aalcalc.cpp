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
#include <chrono>
#include <thread>

#include "aalcalc.h"
#include "../../config.h"
#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

bool firstOutput = true;

// Load and normalize weigthting table 
// we must have entry for every return period!!!
// otherwise no way to pad missing ones
// Weightings should be between zero and 1 and should sum to one 
void aalcalc::loadperiodtoweigthing()
{
	FILE *fin = fopen(PERIODS_FILE, "rb");
	if (fin == NULL) return;
	Periods p;
	OASIS_FLOAT total_weighting = 0;
	size_t i = fread(&p, sizeof(Periods), 1, fin);
	while (i != 0) {
		total_weighting += p.weighting;
		periodstoweighting_[p.period_no] = (OASIS_FLOAT) p.weighting;
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
		i = fread(&occ, sizeof(occ), 1, fin);
	}

	fclose(fin);
}


void aalcalc::outputresultscsv()
{
	printf("summary_id,type,mean, mean_squared\n");

	for (auto x : map_analytical_aal_) {
		OASIS_FLOAT mean = static_cast<OASIS_FLOAT>(x.second.mean);
		OASIS_FLOAT sd_dev = static_cast<OASIS_FLOAT>(static_cast<OASIS_FLOAT>(sqrt((x.second.mean_squared - (x.second.mean * x.second.mean / no_of_periods_)) / (no_of_periods_ - 1))));
		printf("%d,%d, %f, %f, %f \n", x.first, x.second.type,mean, sd_dev, x.second.max_exposure_value);
		 if (firstOutput==true){
			std::this_thread::sleep_for(std::chrono::milliseconds(PIPE_DELAY)); // used to stop possible race condition with kat
            firstOutput=false;
         }
	}

	for (auto x : map_sample_aal_) {
		OASIS_FLOAT mean = static_cast<OASIS_FLOAT>(x.second.mean);
		OASIS_FLOAT sd_dev = static_cast<OASIS_FLOAT>(sqrt((x.second.mean_squared - (x.second.mean * x.second.mean / no_of_periods_)) / (no_of_periods_ - 1)));
		printf("%d,%d, %f, %f, %f \n", x.first,x.second.type, mean, sd_dev, x.second.max_exposure_value);
		if (firstOutput==true){
			std::this_thread::sleep_for(std::chrono::milliseconds(PIPE_DELAY)); // used to stop possible race condition with kat
            firstOutput=false;
        }
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

void aalcalc::do_analytical_calc(const summarySampleslevelHeader &sh,  OASIS_FLOAT mean_loss)
{		
	if (periodstoweighting_.size() > 0) {
		auto iter = periodstoweighting_.find(sh.event_id);
		if (iter != periodstoweighting_.end()){
			mean_loss = mean_loss * iter->second;
		}else{
			// no weighting so assume its zero
			mean_loss = 0;
		}
	}
	OASIS_FLOAT mean_squared = mean_loss*mean_loss;
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

void aalcalc::do_sample_calc(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec)
{
	OASIS_FLOAT mean_loss=0;
	for (auto x : vrec) {
		mean_loss += x.loss;
	}
	mean_loss = mean_loss / samplesize_;	
	OASIS_FLOAT mean_squared = mean_loss*mean_loss;
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

void aalcalc::applyweightings(int event_id, const std::map <int, OASIS_FLOAT> &periodstoweighting,std::vector<sampleslevelRec> &vrec)
{
	if (periodstoweighting.size() == 0) return;
	auto iter = periodstoweighting.find(event_id);
	if (iter != periodstoweighting.end()) {
		for (int i = 0;i < vrec.size(); i++) vrec[i].loss = vrec[i].loss * iter->second;
	}else {
	// Event not found in periods.bin so no weighting i.e zero 
		for (int i = 0;i < vrec.size(); i++) vrec[i].loss = 0; 
	//	fprintf(stderr, "Event %d not found in periods.bin\n", event_id);
//		exit(-1);
	}
}

void aalcalc::applyweightingstomap(std::map<int, aal_rec> &m, int i)
{
	auto iter = m.begin();
	while (iter != m.end()) {
		iter->second.mean = iter->second.mean * i;
		iter->second.mean_squared = iter->second.mean_squared * i * i;
		iter++;
	}
}
void aalcalc::applyweightingstomaps()
{
	int i = periodstoweighting_.size();
	if ( i == 0) return;
	applyweightingstomap(map_analytical_aal_, i);
	applyweightingstomap(map_sample_aal_, i);
}
void aalcalc::doit()
{
	loadoccurrence();
	loadperiodtoweigthing();
	int summarycalcstream_type = 0;
	size_t i = fread(&summarycalcstream_type, sizeof(summarycalcstream_type), 1, stdin);
	int stream_type = summarycalcstream_type & summarycalc_id;

	if (stream_type != summarycalc_id) {
		fprintf(stderr, "%s: Not a summarycalc stream type\n", __func__);
		exit(-1);
	}
	stream_type = streamno_mask & summarycalcstream_type;
	bool haveData = false;
	if (stream_type == 1) {		
		int summary_set = 0;		
		i = fread(&samplesize_, sizeof(samplesize_), 1, stdin);
		if (i != 0) i = fread(&summary_set, sizeof(summary_set), 1, stdin);
		std::vector<sampleslevelRec> vrec;
		summarySampleslevelHeader sh;
		int j = 0;
		OASIS_FLOAT mean_loss = 0;
		while (i != 0) {
			i = fread(&sh, sizeof(sh), 1, stdin);
			while (i != 0) {
				haveData = true;
				sampleslevelRec sr;
				i = fread(&sr, sizeof(sr), 1, stdin);
				if (i == 0 || sr.sidx == 0) {
					applyweightings(sh.event_id, periodstoweighting_, vrec);
					doaalcalc(sh, vrec, mean_loss);
					vrec.clear();
					break;
				}
				if (sr.sidx == -1) mean_loss = sr.loss;
				if (sr.sidx >=0) vrec.push_back(sr);
			}

			j++;
		}
		if (haveData == true) {
			applyweightings(sh.event_id, periodstoweighting_, vrec);
			doaalcalc(sh, vrec, mean_loss);
		}
	}

	applyweightingstomaps();
	outputsummarybin();
}

void touch(const std::string &filepath)
{
	FILE *fout = fopen(filepath.c_str(), "wb");
	fclose(fout);
}
void setinitdone(int processid)
{
	if (processid) {
		std::ostringstream s;
		s << SEMA_DIR_PREFIX << "_aal/" << processid << ".id";
		touch(s.str());
	}
}

void help()
{
	fprintf(stderr, "-P processid -h help\n-v version\n");
}

int main(int argc, char* argv[])
{

	int opt;
	int processid = 0;
	while ((opt = getopt(argc, argv, "vhP:")) != -1) {
		switch (opt) {
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'P':
			processid = atoi(optarg);
			break;
		case 'h':
		default:
			help();
			exit(EXIT_FAILURE);
		}
	}
	initstreams();
	setinitdone(processid);
	aalcalc a;
	a.doit();

	return EXIT_SUCCESS;

}
