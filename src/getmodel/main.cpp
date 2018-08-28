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

Author: Mark Pinkerton  email: mark.pinkerton@oasislmf.org

*/

#include <iostream>
#include <stdio.h>
#include <ctime>
#include "getmodel.h"
#include "../include/oasis.h"

#if defined(_WIN32) || defined(_WIN64)
#include "../wingetopt/wingetopt.h"
#endif
#ifdef __unix
#include <unistd.h>
#include <getopt.h>
#endif

void help()
{
	fprintf(stderr,
		"-d disaggregation\n"
		"-v version\n"
		"-h help\n"
	);
}

void doIt(bool zip, bool disaggregation)
{

	getmodel cdf_generator;

	cdf_generator.init(zip, disaggregation);

	int event_id = -1;
	while (fread(&event_id, sizeof(event_id), 1, stdin) != 0)
	{
		cdf_generator.doCdf(event_id);
	}
}

int main(int argc, char** argv)
{
	std::clock_t start;
	double duration;

	start = std::clock();

	bool disaggregation = false;

	int opt;	
	while ((opt = getopt(argc, argv, "dvh")) != -1) {
		switch (opt) {
		case 'd':
			disaggregation = true;
			break;
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'h':
			help();
			exit(EXIT_FAILURE);
		default:
			help();
			exit(EXIT_FAILURE);
		}
	}

	bool zip = false;
	FILE *fin = fopen(ZFOOTPRINT_FILE, "rb");
	if (fin != nullptr) zip=true;

	initstreams();
	doIt(zip, disaggregation);

	duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;

	fprintf(stderr, "%lf\n", duration);

	return EXIT_SUCCESS;
}
