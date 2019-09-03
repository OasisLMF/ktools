/*
* Copyright (c)2015 - 2019 Oasis LMF Limited
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
Index the summarycalc output
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/


#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "../include/oasis.h"
#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <signal.h>
#include <string.h>
#endif


namespace summaryindex {
	void doit(const std::string& subfolder);
}

char* progname;

#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int signal, siginfo_t* si, void* arg)
{
	fprintf(stderr, "%s: Segment fault at address: %p\n", progname, si->si_addr);
	exit(EXIT_FAILURE);
}
#endif


void help()
{
	fprintf(stderr, "-K workspace sub folder\n");
	fprintf(stderr, "-h help\n-v version\n");
}

int main(int argc, char* argv[])
{
	progname = argv[0];
	std::string subfolder;
	int opt;
	while ((opt = getopt(argc, argv, (char *)"K:vh")) != -1) {
		switch (opt) {
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'K':
			subfolder = optarg;
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

	initstreams();
	try {
		summaryindex::doit(subfolder);
	}
	catch (std::bad_alloc) {
		fprintf(stderr, "%s: Memory allocation failed\n", progname);
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
