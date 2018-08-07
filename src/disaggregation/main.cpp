#include <iostream>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#include "disaggregation.h"
using namespace std;

// options
bool verbose = false;
int tot_number_items = -1;
bool debug = false;
//bool itemLevelOutput = false;
//bool coverageLevelOutput = false;
int rand_vector_size = 1000000;
int rand_seed = 0;

rd_option rndopt = rd_option::usehashedseed;

void help()
{
	fprintf(stderr,
		"-r use random number file\n"
		"-R [max random numbers] used to allocate array for random numbers default 1,000,000\n"
		//"-c [output pipe] - coverage output\n"
		//"-i [output pipe] - item output\n"
		"-d debug (output random numbers instead of gul)\n"
		"-s seed for random number generation (used for debugging)\n"
		"-a automatically hashed seed driven random number generation (default)"
		"-v version\n"
		"-h help\n"
	);
}

int main(int argc, char *argv[])
{
	int opt;
	while ((opt = getopt(argc, argv, (char *) "avhdrR:s:")) != -1) {
		switch (opt) {
		case 'a':
			rndopt = rd_option::usehashedseed;
			break;
		case 'r':
			rndopt = rd_option::userandomnumberfile;
			break;\
		case 'R':
			rand_vector_size = atoi(optarg);
			break;
		/*
		case 'i':
			item_output = optarg;
			itemLevelOutput = true;
			break;
		case 'c':
			coverage_output = optarg;
			coverageLevelOutput = true;
			break;
		*/
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
		default:
			help();
			exit(EXIT_FAILURE);
		}
	}
	getRands rnd(rndopt, rand_vector_size, rand_seed);
	disaggregation disagg(rndopt, rnd, debug);
	initstreams();

	std::vector<item> items;

	disagg.doDisagg(items);

	for (int i = 0; i < items.size(); ++i) {
		item item = items[i];
		fwrite(&item, sizeof(item), 1, stdout);
		std::cerr << item  << std::endl;
	}

	return EXIT_SUCCESS;
}