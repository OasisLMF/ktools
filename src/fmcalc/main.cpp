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
Example implmentation for FM with back allocation

Author: Ben Matharu  email: ben.matharu@oasislmf.org

*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <fcntl.h>
#include <assert.h>
#include "fmcalc.hpp"

#ifdef __unix
    #include <unistd.h>
#endif

#include "../include/oasis.hpp"


void doit(int &maxLevel)
{
    fmcalc fc(maxLevel);
		

	unsigned int fmstream_type = 1 | fmstream_id;

	fwrite(&fmstream_type, sizeof(fmstream_type), 1, stdout);

	int gulstream_type = 0;
	int i = fread(&gulstream_type, sizeof(gulstream_type), 1, stdin);

	int stream_type = gulstream_type & gulstream_id;

	if (stream_type != gulstream_id) {
		std::cerr << "Not a gul stream type\n";
		exit(-1);
	}
	stream_type = streamno_mask &gulstream_type;
	if (stream_type != 1) {
		std::cerr << "Unsupported gul stream type\n";
		exit(-1);
	}

	int last_event_id = -1;
	if (stream_type == 1) {
		int samplesize = 0;
		i = fread(&samplesize, sizeof(samplesize), 1, stdin);
		fwrite(&samplesize, sizeof(samplesize), 1, stdout);
		std::vector<std::vector<float>> event_guls(samplesize + 1);
		std::vector<int> items;
		items.reserve(500000);
		while (i != 0) {
			gulSampleslevelHeader gh;
			i = fread(&gh, sizeof(gh), 1, stdin);
			if (gh.event_id != last_event_id && i == 1) {
				if (last_event_id != -1) {
					fc.dofm(last_event_id, items, event_guls);
				}

				items.clear();
				for (unsigned int i = 0;i < event_guls.size();i++) event_guls[i].clear();
				last_event_id = gh.event_id;
			}
			if (i == 0) {
				if (last_event_id != -1) {
					fc.dofm(last_event_id, items, event_guls);
				}
			}
			while (i != 0) {
				gulSampleslevelRec gr;
				i = fread(&gr, sizeof(gr), 1, stdin);
				if (i == 0) {
					fc.dofm(last_event_id, items, event_guls);
					break;
				}
				if (gr.sidx == 0) break;
				//				if (gr.sidx == mean_idx) gr.sidx = 0;
				gulSampleslevelEventRec gs;
				gs.item_id = gh.item_id;
				gs.sidx = gr.sidx;
				gs.gul = gr.gul;
				if (gs.sidx >= -1) {
					int sidx = gs.sidx;
					if (sidx == -1) sidx = 0;
					event_guls[sidx].push_back(gs.gul);
				}
				if (gr.sidx == -1)  items.push_back(gh.item_id);
			}
		}
	}
}


void help()
{

    std::cerr << "-I inputfilename\n"
        << "-O outputfielname\n"
        << "-M maxlevel\n"
        ;
}


int main(int argc, char* argv[])
{

    std::string inFile;
    std::string outFile;
    int new_max = -1;
#ifdef __unix
	int opt;
    while ((opt = getopt(argc, argv, "hI:O:M:")) != -1) {
        switch (opt) {
        case 'I':
            inFile = optarg;
            break;
         case 'O':
            outFile = optarg;
            break;
         case 'M':
            new_max = atoi(optarg);
            break;
        case 'h':
           help();
           exit(EXIT_FAILURE);
        default:
            help();
            exit(EXIT_FAILURE);
        }
    }
#endif

   initstreams(inFile, outFile);
   
#ifdef __unix
   // posix_fadvise(fileno(stdin), 0, 0, POSIX_FADV_SEQUENTIAL);
   //    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
     

//	int maxlevel = 0;
//	init(maxlevel);
//	if (new_max > -1) maxlevel = new_max;
	doit(new_max);
	return 0;
}
