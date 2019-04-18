/*
* Copyright (c)2015 - 2019 Oasis LMF Limited
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
Index the summarycalc output
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/
#include <stdio.h>
#include <string>
#if defined(_MSC_VER)
#include "../include/dirent.h"
#else
#include <dirent.h>
#endif

#include "../include/oasis.h"

namespace summaryindex {

void indexevents(const std::string& fullfilename, std::string &filename) {
	fprintf(stderr, "Indexing: %s\n", fullfilename.c_str());
	FILE* fin = fopen(fullfilename.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, fullfilename.c_str());
		exit(EXIT_FAILURE);
	}
	long long offset = 0;
	int summarycalcstream_type = 0;
	size_t i = fread(&summarycalcstream_type, sizeof(summarycalcstream_type), 1, fin);
	if (i != 0) offset += sizeof(summarycalcstream_type);
	int stream_type = summarycalcstream_type & summarycalc_id;

	if (stream_type != summarycalc_id) {
		fprintf(stderr, "%s: Not a summarycalc stream type %d\n", __func__, stream_type);
		exit(-1);
	}

	stream_type = streamno_mask & summarycalcstream_type;
	int samplesize;
	i = fread(&samplesize, sizeof(samplesize), 1, fin);
	if (i != 0) offset += sizeof(samplesize);
	int summary_set = 0;
	if (i != 0) i = fread(&summary_set, sizeof(summary_set), 1, fin);
	if (i != 0) offset += sizeof(summary_set);
	summarySampleslevelHeader sh;
	int last_event_id = -1;
	while (i != 0) {
		i = fread(&sh, sizeof(sh), 1, fin);
		if (i != 0) {
			if (last_event_id != sh.event_id) {
				last_event_id = sh.event_id;
				fprintf(stdout, "%d, %s, %lld\n", sh.event_id, filename.c_str(),offset);
			}
			offset += sizeof(sh);
		}

		while (i != 0) {
			sampleslevelRec sr;
			i = fread(&sr, sizeof(sr), 1, fin);
			if (i != 0) offset += sizeof(sr);
			if (i == 0) break;
			if (sr.sidx == 0) break;
		}
	}

	fclose(fin);
}
void doit(const std::string& subfolder)
{
	std::string path = "work/" + subfolder;
	if (path.substr(path.length() - 1, 1) != "/") {
		path = path + "/";
	}

	DIR* dir;
	struct dirent* ent;
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			std::string s = ent->d_name;
			if (s.length() > 4 && s.substr(s.length() - 4, 4) == ".bin") {
				std::string s2 = path + ent->d_name;
				indexevents(s2, s);
			}
		}
	}
	else {
		fprintf(stderr, "Unable to open directory %s\n", path.c_str());
		exit(-1);
	}
	// todo
}


};