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
Convert footprint to csv
Author: Joh Carter  email: johanna.carter@oasislmf.org
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <getopt.h>
#endif

#ifdef __unix
    #include <unistd.h>
#endif

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <getopt.h>
#endif

#include "../include/oasis.hpp"

bool skipheader = false;
using namespace std;

void printrows(int event_id, FILE *finx, long long size )
{
	long long i=0;
	
	while (i < size) {
		EventRow row;
		fread(&row, sizeof(row), 1,finx);
		printf("%d, %d, %d, %.10e\n", event_id,row.areaperil_id,row.intensity_bin_id, row.probability);

		i += sizeof(row);
	}
}



void doit()
{
	if (skipheader == false)  printf("\"event_id\", \"areaperil_id\", \"intensity_bin_id\", \"probability\"\n");
	FILE *finx = fopen("footprint.bin", "rb");
	FILE *finy = fopen("footprint.idx", "rb");

	EventIndex idx;

	if (finy == nullptr) {
		fprintf(stderr, "Footprint idx open failed\n");
		exit(3);
	}
	int i = fread(&idx, sizeof(idx), 1, finy);
	while (i != 0) {		
		flseek(finx, idx.offset, SEEK_SET);
		//printf("%lld\n", idx.offset);
		printrows(idx.event_id, finx, idx.size);
		i = fread(&idx, sizeof(idx), 1, finy);
	}

	fclose(finx);
	fclose(finy);
}

void help()
{

	cerr << "-s skip header\n"
		<< "-h help"
		;
}

int main(int argc, char* argv[])
{
	int opt;
	while ((opt = getopt(argc, argv, "sh")) != -1) {
		switch (opt) {
		case 's':
			skipheader = true;
			break;
		case 'h':
			help();
			exit(EXIT_FAILURE);
		}
	}

    initstreams();
	doit();
	return 0;
}
