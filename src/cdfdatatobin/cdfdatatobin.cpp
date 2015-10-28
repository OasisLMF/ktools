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

using namespace std;

#include "../include/oasis.hpp"

struct cdfdata {
	int event_id;
	int areaperil_id;
	int vulnerability_id;
	int bin_index;
	float prob_to;
};

struct damagecdfrec2 {
	int event_id;
	int areaperil_id;
	int vulnerabilty_id;
};


std::string progname;
std::string filename;
int totalbins = 0;

void doit()
{
	int rowcount = 0;
	cdfdata s;
    char line[4096];
    int lineno=0;
	fgets(line, sizeof(line), stdin);
	lineno++;
	std::string fname = filename + ".bin" ;	
	FILE *foutx = fopen(fname.c_str(), "wb");
	fname = filename + ".idx" ;	
	FILE *fouty = fopen(fname.c_str(), "wb");
	float *bins = new float[totalbins+1];
	long long filesize = 0;

	int idx = 0;
	int last_event_id = 0;
	int last_areaperil_id = 0;
	int last_vulnerabilty_id = 0;
	bool firstrow = true;
	bool binpad = false;
    while (fgets(line, sizeof(line), stdin) != 0)
    {
		if (sscanf(line, "%d,%d,%d,%d,%f", &s.event_id, &s.areaperil_id, &s.vulnerability_id, &s.bin_index, &s.prob_to) != 5){
           fprintf(stderr, "Invalid data in line %d:\n%s", lineno, line);
           return;
       }

	    else
       {

			if (idx > totalbins) {
				cerr << "\n*****Error: bin index greater than or equal to total bins****\n"
					 << "Total Bins: " << totalbins << "\n" 
				  << "index: " << idx << "\n" 
				  << "event_id: " << s.event_id << " areaperil_id: " << s.areaperil_id
				  << "\nvulnerability_id: " << s.vulnerability_id << "\n"
				  << "lineno: " << lineno << "\n"
					;
				delete[] bins;
			
				return ;
			}

			if (last_event_id != s.event_id || last_areaperil_id != s.areaperil_id ||
				last_vulnerabilty_id != s.vulnerability_id) {
				if (firstrow == false){
					for (int i = idx; i < totalbins; i++){
						bins[i] = 1;
					}
					// fwrite(bins, sizeof(float), totalbins, foutx);
					// filesize = filesize + (sizeof(float) * totalbins);
					fwrite(&idx, sizeof(int), 1, foutx);
					filesize = filesize + sizeof(int);
					fwrite(bins, sizeof(float), idx, foutx);
					filesize = filesize + (sizeof(float) * idx);
					if (s.bin_index != 1) binpad = true;
				}
				else {
					firstrow = false;
					if (s.bin_index != 1) binpad = true;
				}
				if (last_event_id != s.event_id) {
					int event_id = s.event_id;
					long long offset = filesize;
					fwrite(&event_id, sizeof(event_id), 1, fouty);
					fwrite(&offset, sizeof(offset), 1, fouty);
				}
				last_event_id = s.event_id;
				last_areaperil_id = s.areaperil_id;
				last_vulnerabilty_id = s.vulnerability_id;

				damagecdfrec2 f;
				f.event_id = s.event_id;
				f.areaperil_id = s.areaperil_id;
				f.vulnerabilty_id = s.vulnerability_id;
				fwrite(&f, sizeof(damagecdfrec2), 1, foutx);
				filesize = filesize + sizeof(damagecdfrec2);
				idx = 0;

			}

			if (binpad == true){
				while (idx < (s.bin_index - 1)){
					bins[idx] = 0.0;
					idx++;
				}
				binpad = false;
			}
			bins[idx] = s.prob_to;
			idx++;
			rowcount++;		
           // fwrite(&s, sizeof(s), 1, foutx);
       }
       lineno++;
    }
    for (int i = idx; i < totalbins; i++){
			bins[i] = 1;
	}
	fwrite(&idx, sizeof(int), 1, foutx);
	filesize = filesize + sizeof(int);
	fwrite(bins, sizeof(float), idx, foutx);
	filesize = filesize + (sizeof(float) * idx);

    fclose(foutx);
    fclose(fouty);

}

void usage()
{
    std::cerr << "Usage: " << progname << " damage_cdf_chunk_?  maxbincount \n";
    exit(-1);
}

int main(int argc, char *argv[])
{
	progname = argv[0];

	if (argc < 3) usage();
	filename = argv[1];
	totalbins = atoi(argv[2]);

	initstreams("", "");
    doit();
    return 0;
}
