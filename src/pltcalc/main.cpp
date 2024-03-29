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
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/

#include "../include/oasis.h"
#ifdef HAVE_PARQUET
#include "../include/oasisparquet.h"
#endif

#include <iostream>
#include <vector>
#include <map>
#include <math.h>
#include <chrono>
#include <thread>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#include <signal.h>
#include <string.h>
#endif


namespace pltcalc {
	void doit(bool skipHeader, bool ordOutput, FILE** fout,
		  std::map<int, std::string> &parquetFileNames);
}
char* progname;

#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int, siginfo_t *si, void *) {
	fprintf(stderr, "FATAL: %s: Segment fault at address: %p\n", progname,si->si_addr);
	exit(EXIT_FAILURE);
}
#endif

void openpipe(int output_id, const std::string& pipe, FILE** fout)
{
        if (pipe == "-") {
                fout[output_id] = stdout;
        } else {
                FILE * f = fopen(pipe.c_str(), "wb");
                if (f != nullptr) {
                        fout[output_id] = f;
                } else {
                        fprintf(stderr, "FATAL: Cannot open %s for output\n",
                                pipe.c_str());
                        exit(EXIT_FAILURE);
                }
        }
}

void help()
{
	fprintf(stderr,
	"-M [filename] output Moment Period Loss Table (MPLT)\n"
	"-m [filename] output MPLT in parquet format\n"
	"-Q [filename] output Quantile Period Loss Table (QPLT)\n"
	"-q [filename] output QPLT in parquet format\n"
	"-S [filename] output Sample Period Loss Table (SPLT)\n"
	"-s [filename] output SPLT in parquet format\n"
	"-h help\n"
	"-v version\n"
	"-H skip header\n"
	);
}


int main(int argc, char *argv[])
{

	enum { MPLT = 0, SPLT, QPLT };

	int opt;
	bool skipHeader = false;
	bool ordOutput = false;
	FILE * fout[] = { nullptr, nullptr, nullptr };
	bool parquetOutput = false;
	std::map<int, std::string> parquetOutFiles;
	while ((opt = getopt(argc, argv, "HvhM:m:Q:q:S:s:")) != -1) {
		switch (opt) {
		case 'v':
#ifdef HAVE_PARQUET
			fprintf(stderr, "%s : version: %s : "
					"Parquet output enabled\n",
				argv[0], VERSION);
#else
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
#endif
			::exit(EXIT_FAILURE);
			break;
		case 'M':
			ordOutput = true;
			openpipe(MPLT, optarg, fout);
			break;
		case 'm':
			parquetOutput = true;
#ifdef HAVE_PARQUET
			parquetOutFiles[OasisParquet::MPLT] = optarg;
#endif
			break;
		case 'Q':
			ordOutput = true;
			openpipe(QPLT, optarg, fout);
			break;
		case 'q':
			parquetOutput = true;
#ifdef HAVE_PARQUET
			parquetOutFiles[OasisParquet::QPLT] = optarg;
#endif
			break;
		case 'S':
			ordOutput = true;
			openpipe(SPLT, optarg, fout);
			break;
		case 's':
			parquetOutput = true;
#ifdef HAVE_PARQUET
			parquetOutFiles[OasisParquet::SPLT] = optarg;
#endif
			break;
		case 'H':
			skipHeader = true;
			break;
		case 'h':
		default:
			help();
			::exit(EXIT_FAILURE);
		}
	}

#ifndef HAVE_PARQUET
	if (parquetOutput) {
		fprintf(stderr, "FATAL: Apache arrow libraries for parquet "
				"output are missing.\nPlease install libraries "
				"and recompile to use this option.\n");
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

	try {
		initstreams();
		pltcalc::doit(skipHeader, ordOutput, fout, parquetOutFiles);
	}
	catch (std::bad_alloc&) {
		fprintf(stderr, "FATAL: %s: Memory allocation failed\n", progname);
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}

