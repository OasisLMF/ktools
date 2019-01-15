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
Convert fm output to csv
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#include "../include/oasis.h"

using namespace std;

void doit(bool skipheader, bool fullprecision)
{

	int fmstream_type;

	size_t i = fread(&fmstream_type, sizeof(fmstream_type), 1, stdin);

	int stream_type = fmstream_type & fmstream_id ;

	if (stream_type != fmstream_id) {
		fprintf(stderr, "%s: Not a fm stream type\n",__func__);
		exit(-1);
	}
	stream_type = streamno_mask & fmstream_type;
	if (stream_type != 1 ) {
		fprintf(stderr, "%s: Unsupported fm stream type: %d\n", __func__, stream_type);
		exit(-1);
	}

	int sample_size = 0;
	i = fread(&sample_size, sizeof(sample_size), 1, stdin);

	if (skipheader == false) 	printf ("event_id,output_id,sidx,loss\n");

	fmlevelhdr p;
	i = fread(&p, sizeof(fmlevelhdr), 1, stdin);
	int count = 0;
	while (i != 0) {
		fmlevelrec q;
		i = fread(&q, sizeof(fmlevelrec), 1, stdin);
		while (i != 0) {
			count++;
			if (q.sidx == 0) break;
			if (fullprecision) printf("%d,%d,%d,%f\n", p.event_id, p.output_id, q.sidx, q.loss);
			else printf("%d,%d,%d,%.2f\n", p.event_id, p.output_id, q.sidx, q.loss);
			i = fread(&q, sizeof(fmlevelrec), 1, stdin);
		}
		if (i) i = fread(&p, sizeof(fmlevelhdr), 1, stdin);
	}

}

