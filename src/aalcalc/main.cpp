
#include <fstream>
#include "../include/oasis.h"

#include "aalcalc.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#include <signal.h>
#include <string.h>
#endif

char *progname = 0;

#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int, siginfo_t *si, void *)
{
	fprintf(stderr, "FATAL: %s: Segment fault at address: %p\n", progname, si->si_addr);
	exit(EXIT_FAILURE);
}
#endif

void help()
{
	fprintf(stderr, "-K workspace sub folder\n");
	fprintf(stderr, "-o Open Results Data (ORD) output\n");
	fprintf(stderr, "-p [filename] ORD output in parquet format\n");
	fprintf(stderr, "-s skip header\n");
	fprintf(stderr, "-v version\n");
	fprintf(stderr, "-h help\n");
	
}

int main(int argc, char* argv[])
{
	progname = argv[0];
	std::string subfolder;
	int opt;
	bool debug = false;
	bool skipheader = false;
	bool ord_output = false;   // csv output
	std::string parquet_outFile;   // parquet output
	bool parquet_output = false;   // parquet output
	while ((opt = getopt(argc, argv, (char *)"swvdohK:p:")) != -1) {
		switch (opt) {
		case 'v':
#ifdef HAVE_PARQUET
			fprintf(stderr, "%s : version: %s : "
					"Parquet output enabled\n",
				argv[0], VERSION);
#else
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
#endif
			exit(EXIT_FAILURE);
			break;
		case 's':
			skipheader = true;
			break;
		case 'K':
			subfolder = optarg;
			break;
		case 'o':
			ord_output = true;
			break;
		case 'p':
			parquet_output = true;
			parquet_outFile = optarg;
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
		fprintf(stderr, "FATAL: No folder supplied for summarycalc files\n");
		exit(EXIT_FAILURE);
	}

#ifndef HAVE_PARQUET
	if (parquet_output) {
		fprintf(stderr, "FATAL: Apache arrow libraries for parquet output are missing.\n"
				"Please install libraries and recompile to use this option.\n");
		exit(EXIT_FAILURE);
	}
#endif

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
		aalcalc a(skipheader, ord_output, parquet_output,
			  parquet_outFile);
		if (debug == true) {
			a.debug(subfolder);
		}
        	logprintf(progname, "INFO", "starting process..\n");
        	a.doit(subfolder);
        	logprintf(progname, "INFO", "finishing process..\n");
	}
	catch (std::bad_alloc&) {
		fprintf(stderr, "FATAL: %s: Memory allocation failed\n", progname);
		exit(EXIT_FAILURE);
	}


	return EXIT_SUCCESS;

}
