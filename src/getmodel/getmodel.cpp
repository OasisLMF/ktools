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

#include <stdio.h>
#include <stdlib.h> 
#include <iostream>
#include <fstream>
#include <sstream>
#include <fcntl.h>

#include <map>
#include <vector>

#ifdef __unix 
#include <unistd.h>
#endif

#include "../include/oasis.hpp"


#define BUFSIZE 4096

unsigned int outputstreamtype = 1;	// OASIS STREAM TYPE

#pragma pack(push, 1)
struct idx {
	int event_id;
	long long offset;
};
#pragma pack(pop)
struct idxrec {
	long long offset;
	int length;
};

struct idx32 {
	int event_id;
	int offset;
};
struct idxrec32 {
	int offset;
	int length;
};

std::string progname;

bool getdamagebindictionary(std::vector<damagebindictionary> &damagebindictionary_vec_)
{
	std::ostringstream oss;
	oss << "damage_bin_dict.bin";

	FILE *fin = fopen(oss.str().c_str(), "rb");
	if (fin == NULL){
		std::ostringstream poss;    	
		poss <<  progname << " : Error opening input file "  << oss.str();
        perror(poss.str().c_str());
		exit(-1);
	}
	fseek(fin, 0L, SEEK_END);
	long long sz = fltell(fin);
	fseek(fin, 0L, SEEK_SET);
	unsigned int nrec = sz / sizeof(damagebindictionary);
	damagebindictionary *s1 = new damagebindictionary[nrec];
	if (fread(s1, sizeof(damagebindictionary), nrec, fin) != nrec) {
		std::ostringstream poss;    	
			poss <<  progname << " : Error reading file "  << oss.str();
        perror(poss.str().c_str());
		exit(-1);
	}

	for (unsigned int i = 0; i < nrec; i++){
		damagebindictionary_vec_.push_back(s1[i]);
	}
	delete[] s1;

	fclose(fin);
	return true;
}


void getindex(std::map<int, idxrec> &imap_, int chunkid_)
{
	std::ostringstream oss;

	oss << "cdf/damage_cdf_chunk_" << chunkid_ << ".idx";
	FILE *fin = fopen(oss.str().c_str(), "rb");
    if (fin == NULL){
		std::ostringstream poss;    	
			poss <<  progname << " : Error opening input file "  << oss.str();
        perror(poss.str().c_str());
        exit(-1);
    }
	idx x;
	idx y;
	int i = fread(&x, sizeof(idx), 1, fin);
	i = fread(&y, sizeof(idx), 1, fin);
	while (i == 1){
		idxrec z;
		z.offset = x.offset;
		z.length = (int)(y.offset - x.offset);
		imap_[x.event_id] = z;
		x = y;
		i = fread(&y, sizeof(idx), 1, fin);
	}
	idxrec z;
	z.offset = x.offset;
	z.length = -1;
	imap_[x.event_id] = z;
}

FILE *fin;

void sendevent(int event_id_, std::map<int, idxrec> &imap_, int max_no_of_bins_, float *interpolation_)
{
	
    long long offset = imap_[event_id_].offset;
	int length = imap_[event_id_].length;
	if (length == -1) {    

		flseek(fin, 0, SEEK_END);
		long long fs = fltell(fin);
		length = (int)(fs - offset);
	}    
    
	flseek(fin, offset, SEEK_SET);
	long long pos = fltell(fin);
	float *binp = new float[max_no_of_bins_];
	int no_of_bins = 0;
	while (length > 0) {
		damagecdfrec df;
		fread(&df, sizeof(damagecdfrec), 1, fin);
		if (df.event_id != event_id_){
			std::cerr << progname << ": Event ID: " << event_id_ << "not found\n";
			exit(-1);
		}
		fread(&no_of_bins, sizeof(no_of_bins), 1, fin);
		fread(binp, sizeof(float), no_of_bins, fin);
		int len = sizeof(damagecdfrec);
		char *p = (char *)&df;
		int i = 0;
		while (i < len) {
			putc(*p, stdout);
			p++;
			i++;
		}
		fwrite(&no_of_bins, sizeof(int), 1, stdout); // output count of bins being processed
		float *q = binp;
		for (i = 0; i < no_of_bins; i++){
			fwrite(q, sizeof(float), 1, stdout);			
			float *bin_mean = interpolation_ + i;
			fwrite(bin_mean, sizeof(float), 1, stdout);			
			q++;
		}
		int rec_length = sizeof(damagecdfrec) + sizeof(no_of_bins) + (sizeof(float)*no_of_bins);

		length = length - rec_length;
		if (length < 0) {
			std::cerr << progname << ": Error Event record format error\n";
			exit(-1);
		}
	}

	delete[] binp;
}

void doit(int chunkid_)
{
 
	int event_id = 0;

	std::map<int, idxrec> imap;

	getindex(imap, chunkid_);

	std::vector<damagebindictionary> damagebindictionary_vec;
	getdamagebindictionary(damagebindictionary_vec);

    float *interpolation = new float[damagebindictionary_vec.size()];
	for (unsigned int i = 0; i < damagebindictionary_vec.size(); i++){
		interpolation[i] = damagebindictionary_vec[i].interpolation;
	}

	std::ostringstream oss;

	oss.str("");
	oss << "cdf/damage_cdf_chunk_" << chunkid_ << ".bin";
	fin = fopen(oss.str().c_str(), "rb");
	if (fin == NULL) {
		std::ostringstream poss;    	
		poss <<  progname << " : Error opening input file "  << oss.str();
        perror(poss.str().c_str());
		exit(-1);
	}

	fwrite(&outputstreamtype, sizeof(outputstreamtype), 1, stdout);

	while (1){
		if (fread(&event_id, sizeof(int), 1, stdin) != 1) break;        
        sendevent(event_id, imap, damagebindictionary_vec.size(),interpolation);
	}
	delete[] interpolation;
}

void usage()
{
    std::cerr << "Usage: " << progname << " chunkid \n";
    exit(-1);
}
int main(int argc, char *argv[])
{
	progname = argv[0];
	if (argc != 2) usage();

	int chunkid = atoi(argv[1]);

    initstreams("", "");
    doit(chunkid);

}
