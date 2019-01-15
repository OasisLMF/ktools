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
Convert footprint csv to binary
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/

#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

namespace footprinttobin {
	void doit(int intensity_bins, int hasIntensityUncertainty, bool skipheader);
	void doitz(int intensity_bins, int hasIntensityUncertainty);
}

#include "../include/oasis.h"
char *progname;


void help() {
	fprintf(stderr, "-i max intensity bins\n"
		"-n No intensity uncertainty\n"
		"-s skip header\n"
		"-v version\n");
}

int main(int argc, char *argv[]) {
	int opt;
	bool zip = false;
	int intensity_bins = -1;
	int hasIntensityUncertainty = true;
	bool skipheader = false;

	while ((opt = getopt(argc, argv, "zvshni:")) != -1) {
		switch (opt) {
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'i':
			intensity_bins = atoi(optarg);
			break;
		case 'n':
			hasIntensityUncertainty = false;
			break;
		case 's':
			skipheader = true;
			break;
		case 'z':
			zip = true;
			break;
		case 'h':
			help();
			exit(EXIT_FAILURE);
		default:
			help();
			exit(EXIT_FAILURE);
		}
	}
	if (intensity_bins == -1) {
		fprintf(stderr, "%s: Intensity bin parameter not supplied\n", __func__);
		help();
		exit(EXIT_FAILURE);
	}

	initstreams();
	fprintf(stderr, "starting...\n");
	if (zip) {
#ifdef _MSC_VER
		fprintf(stderr, "Zip not supported in Microsoft build\n");
		exit(-1);
#else
		footprinttobin::doitz(intensity_bins, hasIntensityUncertainty);
#endif
	}
	else {
		footprinttobin::doit(intensity_bins,hasIntensityUncertainty,skipheader);
	}

	fprintf(stderr, "done...\n");
	return 0;
}
