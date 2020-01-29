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
Convert footprint csv to binary
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <set>
#include <vector>
#include <zlib.h>



#include "../include/oasis.h"

namespace footprinttobin {
	void doitz(int intensity_bins, int hasIntensityUncertainty, const char * binFileName, const char * idxFileName) {
		FILE *foutx = fopen(binFileName, "wb");
		FILE *fouty = fopen(idxFileName, "wb");

		char line[4096];
		int lineno = 0;
		fgets(line, sizeof(line), stdin); // skip header line
		lineno++;
		int last_event_id = 0;
		AREAPERIL_INT last_areaperil_id = 0;
		EventRow r;
		EventIndex idx;
		idx.event_id = 0;
		idx.offset = 0;
		idx.size = 0;
		int event_id = 0;
		std::set<int> events;
		std::set<AREAPERIL_INT> areaperils;
		fwrite(&intensity_bins, sizeof(intensity_bins), 1, foutx);
		idx.offset += sizeof(intensity_bins);
		fwrite(&hasIntensityUncertainty, sizeof(hasIntensityUncertainty), 1, foutx);
		idx.offset += sizeof(hasIntensityUncertainty);
		std::vector<EventRow> rv;
		std::vector<unsigned char> rvz;

		while (fgets(line, sizeof(line), stdin) != 0) {
			lineno++;
#ifdef AREAPERIL_TYPE_UNSIGNED_LONG_LONG
			int ret = sscanf(line, "%d,%llu,%d,%f", &event_id, &r.areaperil_id, &r.intensity_bin_id, &r.probability);
#else
			int ret = sscanf(line, "%d,%d,%d,%f", &event_id, &r.areaperil_id, &r.intensity_bin_id, &r.probability);
#endif
			if (ret != 4) {
				fprintf(stderr, "FATAL: Invalid data in line %d:\n%s", lineno, line);
				return;
			}
			if (r.intensity_bin_id > intensity_bins) {
				fprintf(stderr, "FATAL: Intensity bin id %d is greater than the max supplied (%d) in line %d:\n%s", r.intensity_bin_id, intensity_bins, lineno, line);
				return;
			}
			if (event_id != last_event_id) {
				if (events.find(event_id) == events.end()) {
					events.insert(event_id);
					areaperils.clear();
					last_areaperil_id = r.areaperil_id;
					areaperils.insert(r.areaperil_id);
				}
				else {
					fprintf(stderr, "FATAL: Error (%d):Event_id %d has already been converted - "
						"all event data should be contiguous \n",
						lineno, event_id);
					exit(-1);
				}
				if (last_event_id) {
					rvz.clear();
					rvz.resize(rv.size() * sizeof(r) + 1024);
					unsigned long len = rvz.size();
					compress(&rvz[0], &len, (unsigned char *)&rv[0], rv.size() * sizeof(r));
					fwrite((unsigned char *)&rvz[0], len, 1, foutx);
					rvz.clear();
					//len = sizeof(r) * rv.size();
					//fwrite((unsigned char *)&rv[0], len, 1, foutx);
					rv.clear();
					if (last_event_id) {
						idx.event_id = last_event_id;
						idx.size = len;
						fwrite(&idx, sizeof(idx), 1, fouty);
						idx.offset += idx.size; // offset incremented for the next one
					}
				}
				last_event_id = event_id;
			}
			if (last_areaperil_id != r.areaperil_id) {
				last_areaperil_id = r.areaperil_id;
				if (areaperils.find(r.areaperil_id) == areaperils.end()) {
					areaperils.insert(r.areaperil_id);
				}
				else {
#ifdef AREAPERIL_TYPE_UNSIGNED_LONG_LONG
					fprintf(stderr, "FATAL: Error (%d): areaperil_id %llu data is not contiguous for event_id %d \n", lineno, r.areaperil_id, event_id);
#else
					::fprintf(stderr, "FATAL: Error (%d): areaperil_id %d data is not contiguous for event_id %d \n", lineno, r.areaperil_id, event_id);
#endif
					::exit(-1);
				}
			}
			rv.push_back(r);

		}
		rvz.clear();
		rvz.resize(rv.size() * sizeof(r) + 1024);
		unsigned long len = rvz.size();
		compress(&rvz[0], &len, (unsigned char *)&rv[0], rv.size() * sizeof(r));
		fwrite((unsigned char *)&rvz[0], len, 1, foutx);
		//int len = sizeof(r) * rv.size();
		//fwrite((unsigned char *)&rv[0], sizeof(r) * rv.size(), 1, foutx);
		idx.event_id = last_event_id;
		idx.size = len;
		fwrite(&idx, sizeof(idx), 1, fouty);
		fclose(foutx);
		fclose(fouty);
	}

	void doit(int intensity_bins, int hasIntensityUncertainty, bool skipheader, const char * binFileName, const char * idxFileName){
		FILE *foutx = fopen(binFileName, "wb");
		FILE *fouty = fopen(idxFileName, "wb");

		char line[4096];
		int lineno = 0;
		fgets(line, sizeof(line), stdin); // skip header line
		lineno++;
		int last_event_id = 0;
		AREAPERIL_INT last_areaperil_id = 0;
		EventRow r;
		EventIndex idx;
		idx.event_id = 0;
		idx.offset = 0;
		idx.size = 0;
		int event_id = 0;
		int count = 0; // 11616 / 968*12
		std::set<int> events;
		std::set<AREAPERIL_INT> areaperils;
		fwrite(&intensity_bins, sizeof(intensity_bins), 1, foutx);
		idx.offset += sizeof(intensity_bins);
		fwrite(&hasIntensityUncertainty, sizeof(hasIntensityUncertainty), 1, foutx);
		idx.offset += sizeof(hasIntensityUncertainty);
		while (fgets(line, sizeof(line), stdin) != 0) {
			lineno++;
#ifdef AREAPERIL_TYPE_UNSIGNED_LONG_LONG
			int ret = sscanf(line, "%d,%llu,%d,%f", &event_id, &r.areaperil_id, &r.intensity_bin_id, &r.probability);
#else
			int ret = sscanf(line, "%d,%d,%d,%f", &event_id, &r.areaperil_id, &r.intensity_bin_id, &r.probability);
#endif
			if (ret != 4) {
				fprintf(stderr, "FATAL: Invalid data in line %d:\n%s", lineno, line);
				return;
			}
			if (r.intensity_bin_id > intensity_bins) {
				fprintf(stderr, "FATAL: Intensity bin id %d is greater than the max supplied (%d) in line %d:\n%s", r.intensity_bin_id, intensity_bins, lineno, line);
				return;
			}
			if (event_id != last_event_id) {
				if (events.find(event_id) == events.end()) {
					events.insert(event_id);
					areaperils.clear();
					last_areaperil_id = r.areaperil_id;
					areaperils.insert(r.areaperil_id);
				}
				else {
					fprintf(stderr, "FATAL: Error (%d):Event_id %d has already been converted - "
						"all event data should be contiguous \n",
						lineno, event_id);
					exit(-1);
				}
				if (last_event_id) {
					idx.event_id = last_event_id;
					idx.size = count * sizeof(EventRow);
					fwrite(&idx, sizeof(idx), 1, fouty);
					idx.offset += idx.size;
				}
				last_event_id = event_id;
				count = 0;
			}
			if (last_areaperil_id != r.areaperil_id) {
				last_areaperil_id = r.areaperil_id;
				if (areaperils.find(r.areaperil_id) == areaperils.end()) {
					areaperils.insert(r.areaperil_id);
				}
				else {
#ifdef AREAPERIL_TYPE_UNSIGNED_LONG_LONG
					fprintf(stderr, "FATAL: Error (%d): areaperil_id %llu data is not contiguous for event_id %d \n", lineno, r.areaperil_id, event_id);
#else
					fprintf(stderr, "FATAL: Error (%d): areaperil_id %d data is not contiguous for event_id %d \n", lineno, r.areaperil_id, event_id);
#endif
					exit(-1);
				}
			}
			fwrite(&r, sizeof(r), 1, foutx);
			count++;
		}
		idx.event_id = last_event_id;
		idx.size = count * sizeof(EventRow);
		fwrite(&idx, sizeof(idx), 1, fouty);
		fclose(foutx);
		fclose(fouty);
}
}
