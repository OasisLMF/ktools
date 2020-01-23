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
#include "../include/oasis.h"

namespace gultocsv {
	void doit(bool skipheader, bool fullprecision)
	{

		int gulstream_type = 0;
		int i = (int) fread(&gulstream_type, sizeof(gulstream_type), 1, stdin);
		int stream_type = gulstream_type & gulstream_id;
		if (stream_type == 0) {
			stream_type = gulstream_type & loss_stream_id;
		}

		if (stream_type != gulstream_id && stream_type != loss_stream_id) {
			fprintf(stderr, "FATAL: gultocsv: %s: Not a gul stream type\n", __func__);
			exit(-1);
		}
		stream_type = streamno_mask & gulstream_type;

		if (stream_type == 1 || stream_type == 2) {
			if (skipheader == false && stream_type == 1) printf("event_id,item_id,sidx,loss\n");
			if (skipheader == false && stream_type == 2) printf("event_id,coverage_id,sidx,loss\n");
			int samplesize = 0;
			fread(&samplesize, sizeof(samplesize), 1, stdin);
			while (i != 0) {
				gulSampleslevelHeader gh;
				i = (int) fread(&gh, sizeof(gh), 1, stdin);
				while (i != 0) {
					gulSampleslevelRec gr;
					i = (int) fread(&gr, sizeof(gr), 1, stdin);
					if (i == 0) break;
					if (gr.sidx == 0) break;
					if (fullprecision) printf("%d,%d,%d,%f\n", gh.event_id, gh.item_id, gr.sidx, gr.loss);
					else printf("%d,%d,%d,%.2f\n", gh.event_id, gh.item_id, gr.sidx, gr.loss);
				}
			}
			return;
		}
		std::cerr << "FATAL: Unsupported gul stream type\n";

	}

}

