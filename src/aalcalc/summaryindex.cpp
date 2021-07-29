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

#include <vector> 
#include <string>
#include <map>


bool operator<(const summary_period &lhs, const summary_period &rhs) {

	if (lhs.summary_id != rhs.summary_id) {
		return lhs.summary_id < rhs.summary_id;
	} else if (lhs.period_no != rhs.period_no) {
		return lhs.period_no < rhs.period_no;
	} else {
		return lhs.fileindex < rhs.fileindex;
	}

}

namespace summaryindex {

	void indexevents(const std::string& fullfilename, std::map<summary_period, std::vector<long long>> &summaryfileperiod_to_offset, int file_id, const std::map<int, std::vector<int>> &eventtoperiods) {
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
				summary_period k;
				k.summary_id = sh.summary_id;
				k.fileindex = file_id;
				for (auto period : eventtoperiods.at(sh.event_id)) {
					k.period_no = period;
					summaryfileperiod_to_offset[k].push_back(offset);
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

	void indexevents(const std::string &binfilename, std::string &idxfilename, std::map<summary_period, std::vector<long long>> &summaryfileperiod_to_offset, int file_id, const std::map<int, std::vector<int>> &eventtoperiods) {

		FILE * fidx = fopen(idxfilename.c_str(), "r");
		if (fidx == nullptr) {
			fprintf(stderr, "FATAL: Cannot open %s\n", idxfilename.c_str());
			exit(EXIT_FAILURE);
		}
		FILE * fbin = fopen(binfilename.c_str(), "r");
		if (fbin == nullptr) {
			fprintf(stderr, "FATAL: Cannot open %s\n", binfilename.c_str());
			exit(EXIT_FAILURE);
		}

		char line[4096];
		summary_period k;
		k.fileindex = file_id;
		while (fgets(line, sizeof(line), fidx) != 0) {
			long long offset;
			int ret = sscanf(line, "%d,%lld", &k.summary_id, &offset);
			if (ret != 2) {
				fprintf(stderr, "FATAL: Invalid data in file %s:\n%s", idxfilename.c_str(), line);
				exit(EXIT_FAILURE);
			}
			// Get period numbers from event IDs
			flseek(fbin, offset, SEEK_SET);
			summarySampleslevelHeader sh;
			size_t i = fread(&sh, sizeof(sh), 1, fbin);
			std::map<int, std::vector<int>>::const_iterator iter = eventtoperiods.find(sh.event_id);
			if (iter == eventtoperiods.end()) continue;   // Event not found so don't process it
			for (auto period : iter->second) {
				k.period_no = period;
				summaryfileperiod_to_offset[k].push_back(offset);
			}
		}

		fclose(fidx);
		fclose(fbin);

	}

void doit(const std::string& subfolder, const std::map<int, std::vector<int>> &eventtoperiods)
{	
	std::string path = "work/" + subfolder;
	if (path.substr(path.length() - 1, 1) != "/") {
		path = path + "/";
	}

	std::vector<std::string> files;
	std::map<summary_period, std::vector<long long>> summaryfileperiod_to_offset;
	DIR* dir;
	struct dirent* ent;
	int file_index = 0;
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			std::string s = ent->d_name;
			if (s.length() > 4 && s.substr(s.length() - 4, 4) == ".bin") {
				std::string s2 = path + ent->d_name;
				files.push_back(s);
				std::string s3 = s2;
				s3.erase(s2.length() - 4);
				s3 += ".idx";
				FILE * fidx = fopen(s3.c_str(), "r");
				if (fidx == nullptr) {
					indexevents(s2, summaryfileperiod_to_offset, file_index, eventtoperiods);
				} else {
					fclose(fidx);
					indexevents(s2, s3, summaryfileperiod_to_offset, file_index, eventtoperiods);
				}
				file_index++;
			}
		}
	}
	else {
		fprintf(stderr, "Unable to open directory %s\n", path.c_str());
		exit(-1);
	}

	std::string filename =  path + "filelist.idx";
	FILE* fout = fopen(filename.c_str(), "wb");
	if (fout == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, filename.c_str());
		exit(EXIT_FAILURE);
	}
	auto iter = files.begin();
	while (iter != files.end()) {
		fprintf(fout, "%s\n", iter->c_str());
		iter++;
	}

	fclose(fout);

	int max_summary_id = 0;
	filename = path + "summaries.idx";
	fout = fopen(filename.c_str(), "wb");
	if (fout == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, filename.c_str());
		exit(EXIT_FAILURE);
	}

	auto s_iter = summaryfileperiod_to_offset.begin();
	while (s_iter != summaryfileperiod_to_offset.end()) {
		if (s_iter->first.summary_id > max_summary_id) max_summary_id = s_iter->first.summary_id;
		auto v_iter = s_iter->second.begin();
		while (v_iter != s_iter->second.end()) {
			fprintf(fout, "%d, %d, %d, %lld\n", s_iter->first.summary_id, s_iter->first.fileindex, s_iter->first.period_no, *v_iter);
			v_iter++;
		}
		s_iter++;
	}
	fclose(fout);

	filename = path + "max_summary_id.idx";
	fout = fopen(filename.c_str(), "wb");
	if (fout == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, filename.c_str());
		exit(EXIT_FAILURE);
	}
	fprintf(fout, "%d", max_summary_id);
	fclose(fout);
	
}


};
