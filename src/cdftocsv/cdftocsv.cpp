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
#include <vector>

#include "../include/oasis.h"

#include <iostream>
#include <fstream>
#include <sstream>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

using namespace std;

bool getdamagebindictionary(std::vector<damagebindictionary> &damagebindictionary_vec_)
{

    FILE *fin = fopen(DAMAGE_BIN_DICT_FILE, "rb");
    if (fin == NULL){
        return false;
    }
    flseek(fin, 0L, SEEK_END);
    long long sz = fltell(fin);
    flseek(fin, 0L, SEEK_SET);
    unsigned int nrec = static_cast<unsigned int> (sz / sizeof(damagebindictionary));
    damagebindictionary *s1 = new damagebindictionary[nrec];
    if (fread(s1, sizeof(damagebindictionary), nrec, fin) != nrec) {
        fprintf(stderr, "Error reading file\n");
        exit(-1);
    }

    for (unsigned int i = 0; i < nrec; i++){
        damagebindictionary_vec_.push_back(s1[i]);
    }
    delete[] s1;

    fclose(fin);
    return true;
}

bool getrec(char *rec_, FILE *stream, int recsize_)
{
    int totalread = 0;
    while (totalread != recsize_){
        int ch = getc( stream );
        if (ch == EOF) {
            return false;
        }
        *rec_ = ch;
        totalread++;
        rec_++;
    }

        return true;

}


struct prob_mean {
        OASIS_FLOAT prob_to;
        OASIS_FLOAT bin_mean;
};

void processrec(char *rec, int recsize,
        const std::vector<damagebindictionary> &damagebindictionary_vec_)
{
    damagecdfrec *d = (damagecdfrec *)rec;
    char *b = rec + sizeof(damagecdfrec);
    int *bin_count = (int *)b;
    b = b + sizeof(int);
    prob_mean *pp = (prob_mean *)b;
    for (int bin_index = 0; bin_index < *bin_count; bin_index++){
        fprintf(stdout,"%d,%d,%d,%d,%f,%f\n",d->event_id, d->areaperil_id, d->vulnerability_id,bin_index+1,pp->prob_to,pp->bin_mean );
        pp++;
    }
}
void doit(bool skipheader)
{

	if (skipheader == false) fprintf(stdout, "event_id,areaperil_id,vulnerability_id,bin_index,prob_to,bin_mean\n");
	std::vector<damagebindictionary> damagebindictionary_vec;
	getdamagebindictionary(damagebindictionary_vec);
	size_t total_bins = damagebindictionary_vec.size();
	if (total_bins == 0 ) total_bins = 10000;
	int max_recsize = (int)(total_bins * sizeof(prob_mean)) + sizeof(damagecdfrec)+sizeof(int);

	char *rec = new char[max_recsize];
	int stream_type = 0;

	bool bSuccess = getrec((char *)&stream_type, stdin, sizeof(stream_type));

	if (stream_type != 1) {
		fprintf(stderr,"Invalid stream type %d expect stream type 1\n", stream_type);
		exit(-1);
	}
	for(;;){
		char *p = rec;
		bSuccess = getrec(p, stdin, sizeof(damagecdfrec));
		if (bSuccess == false) break;
			p = p + sizeof(damagecdfrec);
		bSuccess = getrec(p, stdin, sizeof(int)); // we now have bin count
		int *q = (int *)p;
		p = p + sizeof(int);
		int recsize = (*q) * sizeof(prob_mean);
				// we should now have damagecdfrec in memory
		bSuccess = getrec(p, stdin, recsize);
		recsize += sizeof(damagecdfrec)+sizeof(int);

		processrec(rec, recsize, damagebindictionary_vec);
	}
}

void help()
{
	fprintf(stderr, 
		"-s skip header\n"
		"-h help\n"
		"-v version\n"
	);
}


int main(int argc, char *argv[])
{
	int opt;
	bool skipheader = false;
	while ((opt = getopt(argc, argv, "vhs")) != -1) {
		switch (opt) {
		case 's':
			skipheader = true;
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
    
    initstreams();
    doit(skipheader);
    return EXIT_SUCCESS;
}
