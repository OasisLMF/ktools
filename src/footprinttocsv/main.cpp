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
Convert footprint to csv
Author: Joh Carter  email: johanna.carter@oasislmf.org
*/
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <signal.h>
#include <string.h>
#endif

#include <regex>


namespace footprinttocsv {
	void doit(bool skipheader, int from_event, int to_event, const char * binFileName="footprint.bin", const char * idxFileName="footprint.idx");
	void doitz(bool skipheader,int from_event, int to_event, const char * binFileName="footprint.bin.z", const char * idxFileName="footprint.idx.z");
}

#include "../include/oasis.h"
char *progname;
#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int, siginfo_t *si, void *) {
	fprintf(stderr, "%s: Segment fault at address: %p\n", progname,
		si->si_addr);
	exit(EXIT_FAILURE);
}
#endif


void help()
{
	fprintf(stderr,
		"-s skip header\n"
		"-v version\n"
		"-z zip input\n"
		"-e [event_id from]-[event_id to] extract an inclusive range of events\n"
		"-b [FILE NAME] input bin file name\n"
		"-x [FILE NAME] input idx file name\n"
		"-h help\n"
	);
}

int main(int argc, char* argv[])
{
	int opt;
	bool skipheader = false;
	bool zip = false;
	int from_event = 1;
	int to_event = 999999999;
	char *binFileName = 0;
	bool binFileGiven = false;
	char *idxFileName = 0;
	bool idxFileGiven = false;
	while ((opt = getopt(argc, argv, "e:zvhsb:x:")) != -1) {
		switch (opt) {
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'e':
			{
				std::string s = optarg;
				std::regex ws_re("\\-");
				std::vector<std::string> result{
					std::sregex_token_iterator(s.begin(), s.end(), ws_re, -1), {}
				};
				if (result.size() == 1) {
					from_event = atoi(result[0].c_str());					
					to_event = from_event;					
				}
				if (result.size() == 2) {
					from_event = atoi(result[0].c_str());
					to_event = atoi(result[1].c_str());
				}
				if (from_event == 0) {
					fprintf(stderr, "FATAL: Invalid from event_id\n");
					exit(EXIT_FAILURE);
				}
				if (to_event == 0) {
					fprintf(stderr, "FATAL: Invalid to event_id\n");
					exit(EXIT_FAILURE);
				}
			}
			break;
		case 'z':
			zip = true;
			break;
		case 's':
			skipheader = true;
			break;
		case 'b':
			binFileGiven = true;
			binFileName = optarg;
			break;
		case 'x':
			idxFileGiven = true;
			idxFileName = optarg;
			break;
		case 'h':
			help();
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
		if (zip) {
			if(binFileGiven && idxFileGiven) {
				footprinttocsv::doitz(skipheader, from_event, to_event, binFileName, idxFileName);
			} else if(binFileGiven || idxFileGiven) {
				fprintf(stderr, "FATAL: Must specify both bin and idx file names\n");
				exit(EXIT_FAILURE);
			} else {
				footprinttocsv::doitz(skipheader, from_event, to_event);
			}
		}
		else {
			if(binFileGiven && idxFileGiven) {
				footprinttocsv::doit(skipheader, from_event, to_event, binFileName, idxFileName);
			} else if(binFileGiven || idxFileGiven) {
				fprintf(stderr, "FATAL: Must specify both bin and idx file names\n");
				exit(EXIT_FAILURE);
			} else {
				footprinttocsv::doit(skipheader, from_event, to_event);
			}
		}
	}
	catch (std::bad_alloc&) {
		fprintf(stderr, "FATAL: %s: Memory allocation failed\n", progname);
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
