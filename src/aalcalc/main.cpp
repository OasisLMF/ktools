
#include <stdio.h>
#include <stdlib.h>

#include "aalcalc.h"
#include "../../config.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

char *progname;

void help()
{
	fprintf(stderr, "-K workspace sub folder\n");
	fprintf(stderr, "-v version\n");
	fprintf(stderr, "-h help\n");
	
}

int main(int argc, char* argv[])
{
	progname = argv[0];
	std::string subfolder;
	int opt;
	int processid = 0;
	bool debug = false;
	bool welford = false;	// Use welford standard deviation
	bool skipheader = false;
	while ((opt = getopt(argc, argv, (char *)"swvdhK:")) != -1) {
		switch (opt) {
		case 'w':
			welford = true;
			break;
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 's':
			skipheader = true;
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

	if (subfolder.length() == 0) {
		fprintf(stderr, "No folder supplied for summarycalc files\n");
		exit(EXIT_FAILURE);
	}

#if !defined(_MSC_VER) && !defined(__MINGW32__)
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = segfault_sigaction;
	sa.sa_flags = SA_SIGINFO;

	sigaction(SIGSEGV, &sa, NULL);
#endif

	initstreams();

	
	try {
		aalcalc a(skipheader);
		if (debug == true) {
			a.debug(subfolder);
		}
		else {
			if (welford == true) {
				a.doitw(subfolder);
			}else {
				a.doit(subfolder);
			}
		}
	}
	catch (std::bad_alloc) {
		fprintf(stderr, "%s: Memory allocation failed\n", progname);
		exit(0);
	}


	return EXIT_SUCCESS;

}
