
#include "getrands.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include "../include/oasis.h"

std::random_device rd;
using namespace std;

void getRands::userandfile()
{
	FILE *fin = fopen(DISAGG_RANDOM_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: Error opening file %s\n", __func__, DISAGG_RANDOM_FILE);
		exit(-1);
	}
	flseek(fin, 0L, SEEK_END);
	long long p = fltell(fin);
	buffersize_ = p / sizeof(OASIS_FLOAT);
	//		_buffersize = _buffersize - 1;		// first 4 bytes is the limit
	fseek(fin, 0L, SEEK_SET);

	buf_ = new OASIS_FLOAT[buffersize_];
	if (fread(buf_, sizeof(OASIS_FLOAT), buffersize_, fin) != buffersize_) {
		fprintf(stderr, "%s: Error reading random number file\n", __func__);
		exit(-1);
	}
	fclose(fin);
}
getRands::getRands(rd_option rndopt, int rand_vec_size, int rand_seed) : gen_(time(0)), dis_(0, 1), rand_vec_size_(rand_vec_size), rand_seed_(rand_seed)
{
	if (rand_seed_ > 0) gen_.seed(rand_seed_);

	rndopt_ = rndopt;

	base_offset_ = 0;
	switch (rndopt_) {
	case rd_option::userandomnumberfile:
		userandfile();
		break;
	case rd_option::usecachedvector:
		rnd_.resize(rand_vec_size, -1);
		break;
	case rd_option::usehashedseed:
		// nothing to do
		break;
	default:
		fprintf(stderr, "%s: Unknow random number option\n", __func__);
		exit(-1);
	}

}
