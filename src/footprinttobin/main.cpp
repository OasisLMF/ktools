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
	void doit(int intensity_bins, int hasIntensityUncertainty, const char * binFileName="footprint.bin", const char * idxFileName="footprint.idx");
	void doitz(int intensity_bins, int hasIntensityUncertainty, int uncompressedSize, const char * binFileName="footprint.bin.z", const char * idxFileName="footprint.idx.z");
}

#include "../include/oasis.h"

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <signal.h>
#include <string.h>
#endif

char *progname;
#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int, siginfo_t *si, void *) {
	fprintf(stderr, "FATAL: %s: Segment fault at address: %p\n", progname,
		si->si_addr);
	exit(EXIT_FAILURE);
}
#endif



void help() {
	fprintf(stderr, "-i max intensity bins\n"
		"-n No intensity uncertainty\n"
		"-s skip header\n"
		"-v version\n"
		"-b [FILE NAME] output bin file name\n"
		"-x [FILE NAME] output idx file name\n"
		"-z zip footprint data\n"
		"-u index file includes uncompressed data size\n"
	);
}

int main(int argc, char *argv[]) {
	int opt;
	bool zip = false;
	int uncompressedSize = false;
	int intensity_bins = -1;
	int hasIntensityUncertainty = true;
	char *binFileName = 0;
	bool binFileGiven = false;
	char *idxFileName = 0;
	bool idxFileGiven = false;
	progname = argv[0];
	while ((opt = getopt(argc, argv, "zuvshni:b:x:")) != -1) {
		switch (opt) {
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'b':
			binFileGiven = true;
			binFileName = optarg;
			break;
		case 'i':
			intensity_bins = atoi(optarg);
			break;
		case 'n':
			hasIntensityUncertainty = false;
			break;
		case 'x':
			idxFileGiven = true;
			idxFileName = optarg;
			break;
		case 'z':
			zip = true;
			break;
		case 'u':
			uncompressedSize = true;
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
		fprintf(stderr, "FATAL: Intensity bin parameter not supplied\n");
		help();
		exit(EXIT_FAILURE);
	}
	if (uncompressedSize && !zip) {
		fprintf(stderr, "WARNING: No request to zip footprint data\n"
			"         Ignoring request to include uncompressed data size in index file\n");
		uncompressedSize = false;
	}

	initstreams();

	if (zip) {
#ifdef _MSC_VER
		fprintf(stderr, "FATAL: Zip not supported in Microsoft build\n");
		exit(-1);
#else
		if(binFileGiven && idxFileGiven) {
			footprinttobin::doitz(intensity_bins, hasIntensityUncertainty, uncompressedSize, binFileName, idxFileName);
		} else if(binFileGiven || idxFileGiven) {
			fprintf(stderr, "FATAL: Must specify both bin and idx file names - aborted\n");
			exit(EXIT_FAILURE);
		} else {
			footprinttobin::doitz(intensity_bins, hasIntensityUncertainty, uncompressedSize);
		}
#endif
	}
	else {
		if(binFileGiven && idxFileGiven) {
			footprinttobin::doit(intensity_bins, hasIntensityUncertainty, binFileName, idxFileName);
		} else if(binFileGiven || idxFileGiven) {
			fprintf(stderr, "FATAL: Must specify both bin and idx file names - aborted\n");
			exit(EXIT_FAILURE);
		} else {
			footprinttobin::doit(intensity_bins, hasIntensityUncertainty);
		}
	}

	return EXIT_SUCCESS;
}
