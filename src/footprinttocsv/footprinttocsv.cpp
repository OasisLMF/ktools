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
Convert cdfdata to csv
Author: Joh Carter  email: johanna.carter@oasislmf.org
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#ifdef __unix
    #include <unistd.h>
#endif

#include "../include/oasis.hpp"

using namespace std;

struct cdfdata {
	int event_id;
	int areaperil_id;
	int vulnerability_id;
	int bin_index;
	float prob_to;
};

#define MAX_NO_OF_BINS 1000

void doit()
{
	printf("\"event_id\", \"areaperil_id\", \"vulnerability_id\", \"bin_index\", \"prob_to\"\n");
	/*
	This is a complex data structure because of the variable length cdfs. WIP.
	*/

    damagecdfrec df;
    int no_of_bins = 0;
    float binp[MAX_NO_OF_BINS];
    int i = fread(&df, sizeof(df), 1, stdin);
	while (i != 0) {
        fread(&no_of_bins, sizeof(no_of_bins), 1, stdin);
        fread(binp, sizeof(float), no_of_bins, stdin);
        for(int j=0; j < no_of_bins; j++) {
            cdfdata q;
            q.event_id = df.event_id;
            q.areaperil_id = df.areaperil_id;
            q.vulnerability_id = df.vulnerabilty_id;
            q.bin_index = j + 1;
            q.prob_to = binp[j];
            printf("%d, %d, %d, %d, %f\n",
			q.event_id, q.areaperil_id, q.vulnerability_id, q.bin_index, q.prob_to);
        }

        i = fread(&df, sizeof(df), 1, stdin);
	}
}

void help()
{

    cerr << "-I inputfilename\n"
        << "-O outputfielname\n"
        ;
}

int main(int argc, char* argv[])
{
    initstreams();
	printf("TODO!!\n");
	// doit();
	return 0;
}
