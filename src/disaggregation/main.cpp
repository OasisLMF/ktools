#include <iostream>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#include "disaggregation.h"
using namespace std;

// options
bool debug = false;
int rand_seed = 0;

rd_option rndopt = rd_option::usehashedseed;

void help()
{
	fprintf(stderr,
		"-r use random number file\n"
		"-R [max random numbers] used to allocate array for random numbers default 1,000,000\n"
		"-d debug (output random numbers instead of gul)\n"
		"-s seed for random number generation (used for debugging)\n"
		"-a automatically hashed seed driven random number generation (default)"
		"-v version\n"
		"-h help\n"
	);
}

void doit() {

	disaggregation disagg;

	std::map<aggregate_item, std::vector<Weight>> _item_map_weights;
	disagg.doWeights(_item_map_weights);
	for (auto it = _item_map_weights.begin(); it != _item_map_weights.end(); ++it) {
		cerr << it->first << endl;
		for (Weight weight : it->second) {
			cerr << weight << endl;
		}
	}
}

int main(int argc, char *argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, (char *) "vh")) != -1) {
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
	doit();

	fprintf(stderr, "Disaggregation success");
	return EXIT_SUCCESS;
}