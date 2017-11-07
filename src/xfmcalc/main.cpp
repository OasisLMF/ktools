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
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <fcntl.h>
#include <assert.h>
#include "fmcalc.h"
#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

int getmaxnoofitems()
{
	FILE *fin = fopen(ITEMS_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr,"Error cannot open items.bin");
		exit(-1);
	}

	flseek(fin, 0L, SEEK_END);
	long long p = fltell(fin);
	p = p / sizeof(item);
	fclose(fin);
	return int(p);

}

void doit(int &maxLevel)
{
    fmcalc fc(maxLevel);
		

	unsigned int fmstream_type = 1 | fmstream_id;

	fwrite(&fmstream_type, sizeof(fmstream_type), 1, stdout);

	int gulstream_type = 0;
	size_t i = fread(&gulstream_type, sizeof(gulstream_type), 1, stdin);

	int stream_type = gulstream_type & gulstream_id;

	if (stream_type != gulstream_id) {
		fprintf(stderr, "%s: Not a gul stream type\n",__func__);
		exit(-1);
	}

	stream_type = streamno_mask &gulstream_type;
	if (stream_type != 1) {
		fprintf(stderr, "%s: Unsupported gul stream type %d\n", __func__, stream_type);
		exit(-1);
	}

	int last_event_id = -1;
	if (stream_type == 1) {
		int samplesize = 0;
		i = fread(&samplesize, sizeof(samplesize), 1, stdin);
		fwrite(&samplesize, sizeof(samplesize), 1, stdout);
		std::vector<std::vector<OASIS_FLOAT>> event_guls(samplesize + 2);	// one additional for mean and tiv
		std::vector<int> items;
		int max_no_of_items = getmaxnoofitems();
		int current_item_index = 0;
		
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
				gulSampleslevelEventRec gs;
				gs.item_id = gh.item_id;
				gs.sidx = gr.sidx;
				gs.loss = gr.loss;
				if (gr.sidx >= mean_idx) {
					if (gr.sidx == mean_idx) {
						items.push_back(gh.item_id);
						for (unsigned int i = 0; i < event_guls.size(); i++) event_guls[i].resize(items.size());
						current_item_index = static_cast<int> (items.size() - 1);
					}
					int sidx = gs.sidx + 1;
					if (gs.sidx == mean_idx) sidx = 1;
					event_guls[sidx][current_item_index] = gs.loss;
					if (gs.sidx == mean_idx) { // add additional row for tiv
						sidx = 0;
						gs.loss = fc.gettiv(gs.item_id);
						event_guls[sidx][current_item_index] = gs.loss;
					}					
				}
				
			}
		}
	}
}

void help()
{

    fprintf(stderr,
        "-M max level (optional)\n"
		"-v version\n"
		"-h help\n"
	);
}


int main(int argc, char* argv[])
{
    int new_max = -1;

	int opt;
    while ((opt = getopt(argc, argv, "vhoM:")) != -1) {
        switch (opt) {
         case 'M':
            new_max = atoi(optarg);
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

   initstreams("", "");   
	doit(new_max);
   
	return 0;
}
