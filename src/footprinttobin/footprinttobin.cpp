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

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <getopt.h>
#endif
using namespace std;

#include "../include/oasis.hpp"
int intensity_bins_ = -1;

void doit()
{
	FILE *foutx = fopen("footprint.bin", "wb");
	FILE *fouty = fopen("footprint.idx", "wb");

	char line[4096];
	int lineno = 0;
	fgets(line, sizeof(line), stdin); // skip header line
	lineno++;
	int last_event_id = 0;
	EventRow r;
	EventIndex idx;
	idx.event_id = 0;
	idx.offset = 0;
	idx.size = 0;
	int event_id = 0;
	int count = 0; // 11616 / 968*12

	fwrite(&intensity_bins_, sizeof(intensity_bins_), 1, foutx);
	idx.offset += sizeof(intensity_bins_);
	while (fgets(line, sizeof(line), stdin) != 0)
	{
		if (sscanf(line, "%d,%d,%d,%f", &event_id, &r.areaperil_id, &r.intensity_bin_id, &r.probability) != 4) {
			fprintf(stderr, "Invalid data in line %d:\n%s", lineno, line);
			return;
		}
		if (event_id != last_event_id) {
			if (last_event_id) {
				idx.event_id = last_event_id;
				idx.size = count * 12;
				fwrite(&idx, sizeof(idx), 1, fouty);
				idx.offset += idx.size;
			}
			last_event_id = event_id;
			count = 0;
		}
		fwrite(&r, sizeof(r), 1, foutx);
		lineno++;
		count++;
	}
	idx.event_id = last_event_id;
	idx.size = count * 12;
	fwrite(&idx, sizeof(idx), 1, fouty);	
	fclose(foutx);
	fclose(fouty);
}

void help()
{

	std::cerr << "-I Intensitybins\n"
		;
}


int main(int argc, char *argv[])
{
	int opt;
	
	while ((opt = getopt(argc, argv, "hI:")) != -1) {
		switch (opt) {
		case 'I':
			intensity_bins_ = atoi(optarg);
			break;		
		case 'h':
			help();
			exit(EXIT_FAILURE);
		default:
			help();
			exit(EXIT_FAILURE);
		}
	}
	if (intensity_bins_ == -1) {
		cerr << "Intensity bin paramter not supplied\n";
		help();
		exit(EXIT_FAILURE);
	}

	initstreams();
	doit();
    return 0;
}
