
#include "aalsummary.hpp"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#include "../include/dirent.h"
#else
#include <dirent.h>
#include <getopt.h>
#endif

#include "../../config.h"

void help()
{
	fprintf(stderr, "-K workspace sub folder-h help\n-v version\n");
}

int main(int argc, char* argv[])
{

	std::string subfolder;
	int opt;

	if (argc == 1) {
		fprintf(stderr, "Invalid parameters\n");
		help();
		::exit(EXIT_FAILURE);
	}

	while ((opt = getopt(argc, argv, "hvK:")) != -1) {
		switch (opt) {
		case 'K':
			subfolder = optarg;
			break;
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			::exit(EXIT_FAILURE);
			break;
		case 'h':
		default:
			help();
			::exit(EXIT_FAILURE);
		}
	}

	initstreams();

	aalsummary a;
	a.doit(subfolder);
	return EXIT_SUCCESS;

}
