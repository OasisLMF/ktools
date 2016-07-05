
#include <iostream>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <getopt.h>
#endif

#include "gulcalc.hpp"

using namespace std;

// options
bool verbose = false;
int samplesize = -1;
double gul_limit = 0.0;
bool userandomnumberfile = false;
bool debug = false;
bool itemLevelOutput = false;
bool coverageLevelOutput = false;
int rand_vector_size = 1000000;
int rand_seed = -1;

FILE *itemout = stdout;
FILE *covout = stdout;

std::string item_output;
std::string coverage_output;

bool getdamagebindictionary(std::vector<damagebindictionary> &damagebindictionary_vec_)
{

	FILE *fin = fopen(DAMAGE_BIN_DICT_FILE, "rb");
	if (fin == NULL) {
		cerr << "getdamagebindictionary: Unable to open " << DAMAGE_BIN_DICT_FILE << "\n";
		exit(-1);
	}

	flseek(fin, 0L, SEEK_END);
	long long sz = fltell(fin);

	flseek(fin, 0L, SEEK_SET);
	unsigned int nrec = sz / sizeof(damagebindictionary);
	damagebindictionary *s1 = new damagebindictionary[nrec];
	if (fread(s1, sizeof(damagebindictionary), nrec, fin) != nrec) {
		cerr << "Error reading file\n";
		exit(-1);
	}
	damagebindictionary_vec_.clear();

	for (unsigned int i = 0; i < nrec; i++) {
		damagebindictionary_vec_.push_back(s1[i]);
	}
	delete[] s1;
	if (verbose) cerr << "damagebindictionary_vec row count " << damagebindictionary_vec_.size() << endl;

	fclose(fin);
	return true;
}

bool getitems(std::map<item_map_key, std::vector<item_map_rec> > &item_map)
{

	FILE *fin = fopen(ITEMS_FILE, "rb");
	if (fin == NULL) {
		cerr << "getitems: Unable to open " << ITEMS_FILE << "\n";
		exit(-1);
	}

	flseek(fin, 0L, SEEK_END);
	long long sz = fltell(fin);
	flseek(fin, 0L, SEEK_SET);

	unsigned int nrec = sz / sizeof(item);

	item_map.clear();

	item itm;
	int i = fread(&itm, sizeof(itm), 1, fin);
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


bool getcoverages(std::vector<float> &coverages)
{
	FILE *fin = fopen(COVERAGES_FILE, "rb");
	if (fin == NULL) {
		cerr << "getcoverages: Unable to open " << COVERAGES_FILE << "\n";
		exit(-1);
	}

	flseek(fin, 0L, SEEK_END);
	long long sz = fltell(fin);
	flseek(fin, 0L, SEEK_SET);

	float tiv;
	unsigned int nrec = sz / sizeof(tiv);

	coverages.resize(nrec + 1);
	int coverage_id = 0;
	int i = fread(&tiv, sizeof(tiv), 1, fin);
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

	std::vector<float> coverages;
	getcoverages(coverages);

	int total_bins = damagebindictionary_vec.size();
	int max_recsize = (int)(total_bins * sizeof(prob_mean)) + sizeof(damagecdfrec) + sizeof(int);
	
	int last_event_id = -1;
	int stream_type = 0;
	bool bSuccess = getrec((char *)&stream_type, stdin, sizeof(stream_type));
	if (bSuccess == false) {
		cerr << "Error: no stream type returned\n";
		return; // exit thread if failed
	}

	void(*itmWriter)(const void *ibuf,int size, int count) ;
	void(*covWriter)(const void *ibuf,int size, int count);
	itmWriter = 0;
	covWriter = 0;

	getRands rnd(userandomnumberfile, rand_vector_size,rand_seed);

	if (itemLevelOutput == true) itmWriter = itemWriter;
	if (coverageLevelOutput == true) covWriter = coverageWriter;

	gulcalc g(damagebindictionary_vec,coverages,item_map,rnd, gul_limit, userandomnumberfile,debug, samplesize, itmWriter, covWriter, iGetrec);
	g.doit();

	return;

}

void help()
{
	cerr << "-S Samplesize (default 0) \n"
		<< "-r use random numer file\n"
		<< "-R [max random numbers] (used to allocate array for random numbers default 1,000,000)"
		<< "-c [outputpipe] - coverage output\n"
		<< "-i [outputpipe] - item output\n"
		<< "-d debug (dump random numbers instead of gul)\n"
		<< "-I [filename] input file (optional)\n"
		<< "-s seed for random number generation (used for debugging)"
		;
}

int main(int argc, char *argv[])
{
	int opt;
	std::string infile;
	std::string outfile;
	// default values
	rand_vector_size = 1000000;
	samplesize = 0;
	rand_seed = -1;
	while ((opt = getopt(argc, argv, "drL:S:I:c:i:R:")) != -1) {
		switch (opt) {
		case 'S':
			samplesize = atoi(optarg);
			break;	
		case 'r':
			userandomnumberfile = true;
			break;
		case 'L':
			gul_limit = atof(optarg);
			break;
		case 'R':
			rand_vector_size = atoi(optarg);
			break;
		case 'I':
			infile = optarg;
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
		fprintf(stderr, "No output option selected\n");
		exit(EXIT_FAILURE);
	}	
	initstreams(infile, outfile);
	doit();

}
