
#include <iostream>
#include "gulcalc.h"


using namespace std;

FILE *itemout = stdout;
FILE *covout = stdout;

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


void getcoverages(std::vector<OASIS_FLOAT> &coverages)
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

void itemWriter(const void *ibuf, int size, int count)
{
	fwrite(ibuf, size, count, itemout);
}

void coverageWriter(const void *ibuf, int size, int count)
{
	fwrite(ibuf, size, count, covout);
}

void doit(const gulcalcopts &opt)
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

	void(*itmWriter)(const void *ibuf, int size, int count);
	void(*covWriter)(const void *ibuf, int size, int count);
	itmWriter = 0;
	covWriter = 0;

	getRands rnd(opt.rndopt, opt.rand_vector_size, opt.rand_seed);
	itemout = opt.itemout;
	covout = opt.covout;
	if (opt.itemLevelOutput == true) itmWriter = itemWriter;
	if (opt.coverageLevelOutput == true) covWriter = coverageWriter;

	gulcalc g(damagebindictionary_vec, coverages, item_map, rnd, opt.loss_threshold, opt.rndopt, opt.debug, opt.samplesize, itmWriter, covWriter, iGetrec,opt.rand_seed);
	if (opt.mode == 0) g.mode0();		// classic gulcalc
	if (opt.mode == 1) g.mode1();		// first type of back allocation

	return;

}