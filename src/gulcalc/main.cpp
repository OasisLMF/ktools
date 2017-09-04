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
Calculate the GUL
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/
#include <iostream>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#include "gulcalc.h"

using namespace std;

// options
bool verbose = false;
int samplesize = -1;
double gul_limit = 0.0;
bool debug = false;
bool itemLevelOutput = false;
bool coverageLevelOutput = false;
int rand_vector_size = 1000000;
int rand_seed = 0;

rd_option rndopt = rd_option::usehashedseed;


FILE *itemout = stdout;
FILE *covout = stdout;

std::string item_output;
std::string coverage_output;

bool getdamagebindictionary(std::vector<damagebindictionary> &damagebindictionary_vec_)
{

	FILE *fin = fopen(DAMAGE_BIN_DICT_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, DAMAGE_BIN_DICT_FILE);
		exit(-1);
	}

	flseek(fin, 0L, SEEK_END);
	long long sz = fltell(fin);

	flseek(fin, 0L, SEEK_SET);
	unsigned int nrec = static_cast<unsigned int>(sz / sizeof(damagebindictionary));
	damagebindictionary *s1 = new damagebindictionary[nrec];
	if (fread(s1, sizeof(damagebindictionary), nrec, fin) != nrec) {
		fprintf(stderr, "%s: Error reading file %s\n", __func__, DAMAGE_BIN_DICT_FILE);
		exit(-1);
	}
	damagebindictionary_vec_.clear();

	for (unsigned int i = 0; i < nrec; i++) {
		damagebindictionary_vec_.push_back(s1[i]);
	}
	delete[] s1;

	fclose(fin);
	return true;
}

bool getitems(std::map<item_map_key, std::vector<item_map_rec> > &item_map)
{

	FILE *fin = fopen(ITEMS_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, ITEMS_FILE);
		exit(-1);
	}

	flseek(fin, 0L, SEEK_END);
	long long sz = fltell(fin);
	flseek(fin, 0L, SEEK_SET);

	unsigned int nrec = static_cast<unsigned int>(sz / sizeof(item));

	item_map.clear();

	item itm;
	size_t i = fread(&itm, sizeof(itm), 1, fin);
	while (i != 0) {
		item_map_key imk;
		imk.areaperil_id = itm.areaperil_id;
		imk.vulnerability_id = itm.vulnerability_id;
		item_map_rec imr;
		imr.item_id = itm.id;
		imr.coverage_id = itm.coverage_id;
		imr.group_id = itm.group_id;
		item_map[imk].push_back(imr);
		i = fread(&itm, sizeof(itm), 1, fin);
	}

	fclose(fin);

	return true;
}


bool getcoverages(std::vector<OASIS_FLOAT> &coverages)
{
	FILE *fin = fopen(COVERAGES_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: Error reading file %s\n", __func__, COVERAGES_FILE);
		exit(-1);
	}

	flseek(fin, 0L, SEEK_END);
	long long sz = fltell(fin);
	flseek(fin, 0L, SEEK_SET);

	OASIS_FLOAT tiv;
	unsigned int nrec = static_cast<unsigned int>(sz / sizeof(tiv));

	coverages.resize(nrec + 1);
	int coverage_id = 0;
	size_t i = fread(&tiv, sizeof(tiv), 1, fin);
	while (i != 0) {
		coverage_id++;
		coverages[coverage_id] = tiv;
		i = fread(&tiv, sizeof(tiv), 1, fin);
	}

	fclose(fin);
	return true;

}

inline bool getrec(char *rec_, FILE *stream, int recsize_)
{
	if (fread(rec_, 1, recsize_, stream) == recsize_) return true;
	return false;

}

// interface callback functions
bool iGetrec(char *rec, int recsize)
{
	return getrec(rec, stdin, recsize);
}

void itemWriter(const void *ibuf, int size,int count) 
{
	fwrite(ibuf, size, count, itemout);
}

void coverageWriter(const void *ibuf,int size, int count)
{
	fwrite(ibuf, size, count, covout);
}

void doit()
{
	std::vector<damagebindictionary> damagebindictionary_vec;
	getdamagebindictionary(damagebindictionary_vec);

	std::map<item_map_key, std::vector<item_map_rec> > item_map;
	getitems(item_map);

	std::vector<OASIS_FLOAT> coverages;
	getcoverages(coverages);

	size_t total_bins = damagebindictionary_vec.size();
	int max_recsize = (int)(total_bins * sizeof(prob_mean)) + sizeof(damagecdfrec) + sizeof(int);
	
	int last_event_id = -1;
	int stream_type = 0;
	bool bSuccess = getrec((char *)&stream_type, stdin, sizeof(stream_type));
	if (bSuccess == false) {
		cerr << "gulcalc: Error no stream type returned\n";
		return; // exit thread if failed
	}

	void(*itmWriter)(const void *ibuf,int size, int count) ;
	void(*covWriter)(const void *ibuf,int size, int count);
	itmWriter = 0;
	covWriter = 0;

	getRands rnd(rndopt, rand_vector_size,rand_seed);

	if (itemLevelOutput == true) itmWriter = itemWriter;
	if (coverageLevelOutput == true) covWriter = coverageWriter;

	gulcalc g(damagebindictionary_vec,coverages,item_map,rnd, gul_limit, rndopt,debug, samplesize, itmWriter, covWriter, iGetrec);
	g.doit();

	return;

}

void help()
{
	fprintf(stderr,
		"-S Sample size (default 0) \n"
		"-r use random number file\n"
		"-R [max random numbers] used to allocate array for random numbers default 1,000,000\n"
		"-c [output pipe] - coverage output\n"
		"-i [output pipe] - item output\n"
		"-d debug (output random numbers instead of gul)\n"
		"-s seed for random number generation (used for debugging)\n"
		"-a automatically hashed seed driven random number generation (default)"
		"-l legacy mechanism driven by random numbers generated dynamically per group - will be removed in future"
		"-L gul limit (default 0)"
		"-v version\n"
		"-h help\n"
		);
}

int main(int argc, char *argv[])
{
	int opt;
	// default values
	//rand_vector_size = 1000000;
	//samplesize = 0;
	//rand_seed = -1;
	while ((opt = getopt(argc, argv, "alvhdrL:S:c:i:R:s:")) != -1) {
		switch (opt) {
		case 'S':
			samplesize = atoi(optarg);
			break;
		case 'l':
			rndopt = rd_option::usecachedvector;
			break;
		case 'a':
			rndopt = rd_option::usehashedseed;
			break;
		case 'r':
			rndopt = rd_option::userandomnumberfile;
			break;
		case 'L':
			gul_limit = atof(optarg);
			break;
		case 'R':
			rand_vector_size = atoi(optarg);
			break;
		case 'i':
			item_output = optarg;
			itemLevelOutput = true;
			break;
		case 'c':
			coverage_output = optarg;
			coverageLevelOutput = true;
			break;
		case 'd':
			debug = true;
			break;		
		case 's':
			rand_seed = atoi(optarg);
			break;
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'h':
		default: /* '?' */
			help();
			exit(EXIT_FAILURE);
		}
	}
	
	if (itemLevelOutput == true) {
		if (item_output == "-") itemout = stdout;
		else itemout = fopen(item_output.c_str(), "wb");
	}
	if (coverageLevelOutput == true) {
		if (coverage_output == "-") covout = stdout;
		else covout = fopen(coverage_output.c_str(), "wb");
	}

	if (itemLevelOutput == false && coverageLevelOutput == false) {
		fprintf(stderr, "%s: No output option selected\n", argv[0]);
		exit(EXIT_FAILURE);
	}	
	initstreams();
	doit();

}
