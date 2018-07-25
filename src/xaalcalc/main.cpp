
#include <stdio.h>
#include <stdlib.h>

#include "aalcalc.h"
#include "../../config.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif



void help()
{
	fprintf(stderr, "-K workspace sub folder\n");
	fprintf(stderr, "-h help\n");
	fprintf(stderr, "-v version\n");
}

int main(int argc, char* argv[])
{
	std::string subfolder;
	int opt;
	int processid = 0;
	bool debug = false;
	while ((opt = getopt(argc, argv, (char *)"vdhK:")) != -1) {
		switch (opt) {
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'K':
			subfolder = optarg;
			break;
		case 'd':
			debug = true;			
			break;
		case 'h':
		default:
			help();
			exit(EXIT_FAILURE);
		}
	}
	initstreams();

	if (subfolder.length() == 0) {
		fprintf(stderr, "No folder supplied for summarycalc files\n");
		exit(EXIT_FAILURE);
	}

	aalcalc a;
	if (debug == true) {
		a.debug(subfolder);
	}
	else {
		a.doit(subfolder);
	}

	return EXIT_SUCCESS;

}
