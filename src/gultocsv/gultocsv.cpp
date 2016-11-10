/*
* Copyright (c)2015 Oasis LMF Limited 
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
Author: Ben Matharu  email : ben.matharu@oasislmf.org
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

using namespace std;
#include "../include/oasis.hpp"

bool skipheader = false;
bool fullprecision = false;

void doit()
{

	int gulstream_type = 0;
	int i = fread(&gulstream_type, sizeof(gulstream_type), 1, stdin);
	int stream_type = gulstream_type & gulstream_id ;

	if (stream_type != gulstream_id) {
		fprintf(stderr, "%s: Not a gul stream type\n", __func__);
		exit(-1);
	}
	stream_type = streamno_mask &gulstream_type;

	if (stream_type == 1 || stream_type == 2){
		if (skipheader == false && stream_type==1) printf ("event_id,item_id,sidx,loss\n");
		if (skipheader == false && stream_type==2) printf ("event_id,coverage_id,sidx,loss\n");
		int samplesize=0;
		fread(&samplesize, sizeof(samplesize), 1, stdin);
		while (i != 0){
			gulSampleslevelHeader gh;
			i = fread(&gh, sizeof(gh), 1, stdin);
			while (i != 0){
				gulSampleslevelRec gr;
				i = fread(&gr, sizeof(gr), 1, stdin);
				if (i == 0) break;
				if (gr.sidx == 0) break;
				if(fullprecision) printf("%d,%d,%d,%f\n", gh.event_id, gh.item_id, gr.sidx, gr.loss);
				else printf("%d,%d,%d,%.2f\n", gh.event_id, gh.item_id, gr.sidx, gr.loss);
			}
		}
		return;
	}
	std::cerr << "Unsupported gul stream type\n";

}

void help()
{
	fprintf(stderr,
		"-I inputfilename\n"
	    "-O outputfielname\n"
	    "-s skip header\n"
		"-f full precision\n"
		"-v version\n"
		"-h help\n"
	) ;
}

int main(int argc, char* argv[])
{

	int opt;
	std::string inFile;
	std::string outFile;

	while ((opt = getopt(argc, argv, "vhfsI:O:")) != -1) {
		switch (opt) {
		case 'I':
			inFile = optarg;
			break;
		case 'O':
			outFile = optarg;
			break;
		case 's':
			skipheader = true;
			break;
		case 'f':
			fullprecision = true;
			break;
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'h':
		default:
			help();
			exit(EXIT_FAILURE);
		}
	}

	initstreams(inFile, outFile);
	doit();
	return 0;
}
