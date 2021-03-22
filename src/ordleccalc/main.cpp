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
Loss exceedance curve
Author: Ben Matharu  email : ben.matharu@oasislmf.org
*/



#include <vector>
#include "leccalc.h"


#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#include "../include/dirent.h"
#else
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <string.h>
#endif

char* progname;
#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int, siginfo_t *si, void *) {
	fprintf(stderr, "FATAL: %s Segment fault at address: %p\n", progname,
		si->si_addr);
	exit(EXIT_FAILURE);
}
#endif

namespace leccalc {
	void doit(const std::string &subfolder, FILE **fout, const bool useReturnPeriodFile, const bool skipheader, bool *outputFlags, bool ordFlag);
}


void openpipe(int output_id, const std::string& pipe, FILE** fout)
{
	if (pipe == "-") fout[output_id] = stdout;
	else {
		FILE* f = fopen(pipe.c_str(), "wb");
		if (f != nullptr) fout[output_id] = f;
		else {
			fprintf(stderr, "FATAL: Cannot open %s for output\n", pipe.c_str());
			::exit(-1);
		}
	}
}


void help()
{
	fprintf(stderr, "-O [filename] Exceedance Probability Table\n");
	fprintf(stderr, "-o [filename] Per Sample Exceedance Probability Table\n");
	fprintf(stderr, "-F Aggregate Full Uncertainty\n");
	fprintf(stderr, "-W Aggregate Wheatsheaf\n");
	fprintf(stderr, "-S Aggregate Sample Mean\n");
	fprintf(stderr, "-M Aggregate Wheatsheaf mean\n");
	fprintf(stderr, "-f Occurrence Full Uncertainty\n");
	fprintf(stderr, "-w Occurrence Wheatsheaf\n");
	fprintf(stderr, "-s Occurrence Sample Mean\n");
	fprintf(stderr, "-m Occurrence Wheatsheaf mean\n");
	fprintf(stderr, "-K [directory] Workspace sub folder\n");
	fprintf(stderr, "-r Use return period file\n");
	fprintf(stderr, "-H Skip header\n");
	fprintf(stderr, "-h help\n");
	fprintf(stderr, "-v version\n");
}


int main(int argc, char* argv[])
{
	bool useReturnPeriodFile = false;
	FILE* fout[] = { nullptr, nullptr };
	bool outputFlags[8] = { false, false, false, false,
				false, false, false, false };
	const int maxStreams = 2;
	bool outputStreams[maxStreams] = { false, false };
	const bool ordFlag = true;   // Turn on ORD output

	std::string subfolder;
	int opt;
	bool skipheader = false;
	while ((opt = getopt(argc, argv, "vhrHFWMSK:fwsmO:o:")) != -1) {
		switch (opt) {
		case 'K':
			subfolder = optarg;
			break;
		case 'H':
			skipheader = true;
			break;
		case 'F':
			outputFlags[AGG_FULL_UNCERTAINTY] = true;
			outputStreams[EPT] = true;
			break;
		case 'f':
			outputFlags[OCC_FULL_UNCERTAINTY] = true;
			outputStreams[EPT] = true;
			break;
		case 'W':
			outputFlags[AGG_WHEATSHEAF] = true;
			outputStreams[PSEPT] = true;
			break;
		case 'w':
			outputFlags[OCC_WHEATSHEAF] = true;
			outputStreams[PSEPT] = true;
			break;
		case 'S':
			outputFlags[AGG_SAMPLE_MEAN] = true;
			outputStreams[EPT] = true;
			break;
		case 's':
			outputFlags[OCC_SAMPLE_MEAN] = true;
			outputStreams[EPT] = true;
			break;
		case 'M':
			outputFlags[AGG_WHEATSHEAF_MEAN] = true;
			outputStreams[EPT] = true;
			break;
		case 'm':
			outputFlags[OCC_WHEATSHEAF_MEAN] = true;
			outputStreams[EPT] = true;
			break;
		case 'r':
			useReturnPeriodFile = true;
			break;
		case 'O':
			openpipe(EPT, optarg, fout);
			break;
		case 'o':
			openpipe(PSEPT, optarg, fout);
			break;
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'h':
			help();
			::exit(EXIT_FAILURE);
			break;
		default:
			help();
			::exit(EXIT_FAILURE);
		}
	}

	if (argc == 1) {
		fprintf(stderr, "FATAL: Invalid parameters\n");
		help();
	}

	// Check for valid combinations of output streams and ORD files
	for (int i = 0; i < maxStreams; i++) {
		std::string streamName;
		if (i == EPT) streamName = "EPT";
		else if (i == PSEPT) streamName = "PSEPT";
		else streamName = "Error";   // Should never get here

		if (outputStreams[i] == false && fout[i] != nullptr) {
			fprintf(stderr, "WARNING: no valid output stream to fill %s file.\n",
				streamName.c_str());
		} else if (outputStreams[i] == true && fout[i] == nullptr) {
			fprintf(stderr, "FATAL: no destination for %s output.\n",
				streamName.c_str());
			exit(EXIT_FAILURE);
		}
	}

	progname = argv[0];
#if !defined(_MSC_VER) && !defined(__MINGW32__)
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = segfault_sigaction;
	sa.sa_flags = SA_SIGINFO;

	sigaction(SIGSEGV, &sa, NULL);
#endif

	try {
		initstreams();
        	logprintf(progname, "INFO", "starting process..\n");
		leccalc::doit(subfolder, fout, useReturnPeriodFile, skipheader, outputFlags, ordFlag);
        	logprintf(progname, "INFO", "finishing process..\n");
		return EXIT_SUCCESS;
	} catch (std::bad_alloc&) {
		fprintf(stderr, "FATAL: %s: Memory allocation failed\n", progname);
		exit(EXIT_FAILURE);
	}
	
}
