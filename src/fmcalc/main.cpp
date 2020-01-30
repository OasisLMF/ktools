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
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <fcntl.h>
#include <assert.h>
#include "fmcalc.h"
#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#include <signal.h>
#include <string.h>
#endif

char *progname = 0;

void help()
{

    fprintf(stderr,
		"-a set allocrule (default none)\n"
		"-o general optimization off\n"
        "-M max level (optional)\n"
		"-p inputpath (relative or full path)\n"
		"-n feed net value (used for reinsurance)\n"
		"-O Alloc rule2 optimization off"
		"-d debug\n"
		"-v version\n"
		"-h help\n"
	);
}

#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int, siginfo_t *si, void *)
{
	fprintf(stderr, "FATAL: %s: Segment fault at address: %p\n", progname, si->si_addr);
	exit(EXIT_FAILURE);
}
#endif


int main(int argc, char* argv[])
{
	progname = argv[0];
    int new_max = -1;
	int allocrule = 0;
	int opt;
	std::string inputpath;
	bool debug = false;
	bool allocruleOptimizationOff = false;
	bool generalOptimization = true;

	bool netvalue = false;
	bool stepped = false;
    while ((opt = getopt(argc, argv, "SdnovhOM:p:a:")) != -1) {
        switch (opt) {
		case 'o':
			generalOptimization = false;
			break;
		case 'O':
			allocruleOptimizationOff = true;
			break;
		case 'd':
			debug = true;
			break;
         case 'M':
            new_max = atoi(optarg);
            break;
		 case 'a':
			 allocrule = atoi(optarg);
			 break;
		 case 'v':
			 fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			 exit(EXIT_FAILURE);
			 break;
		 case 'p':
			 inputpath = optarg;
			 break;
		 case 'n':
			 netvalue = true;
			 break;
		 case 'S':
			 stepped = true;
			 break;
        case 'h':
           help();
           exit(EXIT_FAILURE);
        default:
            help();
            exit(EXIT_FAILURE);
        }
    }

#if !defined(_MSC_VER) && !defined(__MINGW32__)
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = segfault_sigaction;
	sa.sa_flags = SA_SIGINFO;

	sigaction(SIGSEGV, &sa, NULL);
#endif

	if (allocrule < 0 || allocrule > 3) {
		fprintf(stderr, "FATAL:%s: Invalid allocrule %d\n", progname,allocrule);
		exit(EXIT_FAILURE);
	}

	try {
		initstreams("", "");
		fmcalc fc(new_max, allocrule, inputpath, netvalue,debug, allocruleOptimizationOff, generalOptimization,stepped);
		fc.doit();
	}
	catch (const std::bad_alloc &a) {
		fprintf(stderr, "FATAL:%s: bad_alloc: %s\n", progname, a.what());
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
