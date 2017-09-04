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
#include <stdio.h>
#include <stdlib.h>
#include <random>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif


void genrandomnumbers(bool skipheader, int total, int seed)
{
	std::random_device rd;
	std::uniform_real_distribution<> dis(0, 1);
	std::mt19937 mt;
	if (seed > 0) mt.seed(seed);
	else mt.seed(rd());
	if (skipheader == false) printf("\"random_no\"\n");
	for (int i = 0; i < total; i++) {
		OASIS_FLOAT f = (OASIS_FLOAT)dis(mt);
		printf("%f\n", f);
	}
}
void doit(bool skipheader)
{

    OASIS_FLOAT rand;
	if (skipheader == false) printf("random_no\n");
    while (fread(&rand, sizeof(rand), 1, stdin) == 1){
        printf("%f\n",rand);
    }

}

void help()
{
	fprintf(stderr, "-r Convert binary OASIS_FLOAT input to csv\n");
	fprintf(stderr, "-v version\n");
	fprintf(stderr, "-s skip header\n");
	fprintf(stderr, "-g generate random numbers\n");
	fprintf(stderr, "-S seed value\n");	
	fprintf(stderr, "-h help\n");
}


int main(int argc, char *argv[])
{
	bool skipheader = false;
	bool bintorand = false;
	bool generaterandno = false;
	int totalrandno = 0;
	int seedval = 0;
	int opt;
	while ((opt = getopt(argc, argv, "vhsrg:S:")) != -1) {
		switch (opt) {
		case 's':
			skipheader = true;
			break;
		case 'r':
			bintorand = true;
			break;
		case 'g':
			generaterandno = true;
			totalrandno = atoi(optarg);
			break;
		case 'S':
			seedval = atoi(optarg);
			break;
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			::exit(EXIT_FAILURE);
			break;
		case 'h':
		default:
			help();
			::exit(EXIT_FAILURE);
		}
	}


	initstreams("", "");
	if (bintorand == true) {
		doit(skipheader);
		return 0;
	}

	if (generaterandno == true) {
		genrandomnumbers(skipheader,totalrandno,seedval);
		return EXIT_SUCCESS;
	}
    
}
