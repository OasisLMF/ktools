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
Used to multiples pipes functionality of simple cat command with multiple inputs
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/
#include "../include/oasis.h"
#include <map>
#include <vector>
#include <algorithm>


#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#include "../include/dirent.h"
#else
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif
#include <sys/stat.h>

namespace kat {

	struct eltcalcOutput {
		int summary_id;
		int type;
		int event_id;
		OASIS_FLOAT mean;
		OASIS_FLOAT standard_deviation;
		OASIS_FLOAT exposure_value;
	};

	// This function will only sort data by event ID as intended if
	// event IDs have been deterministically assigned to processes.
	// If the event IDs have been randomised, it won't do any harm: they
	// just won't be in order.
	void doitsort(std::vector<FILE*>& infiles) {

		std::map<FILE*, eltcalcOutput> eOut;
		char line[4096];

		// Get first line from each file
		for (std::vector<FILE*>::iterator it = infiles.begin();
		     it != infiles.end(); it++) {

			int inArgs;
			do {
				char * data = fgets(line, sizeof(line), *it);
				if (data == nullptr) {   // Empty file
					eOut[*it].event_id = 0;
					break;
				}
				inArgs = sscanf(line, "%d,%d,%d,%f,%f,%f", &eOut[*it].summary_id, &eOut[*it].type, &eOut[*it].event_id, &eOut[*it].mean, &eOut[*it].standard_deviation, &eOut[*it].exposure_value);
				if (inArgs == 0)   // Print header
					fprintf(stdout, "%s", line);
			} while (inArgs == 0);   // Skip header

		}

		// Determine order of files
		std::vector<std::pair<int, FILE*>> orders;
		for (std::map<FILE*, eltcalcOutput>::iterator it = eOut.begin();
		     it != eOut.end(); it++) {
			if (it->second.event_id != 0) {
				int order = it->second.event_id % eOut.size();
				if (order == 0) order = eOut.size();
				orders.push_back(std::make_pair(order, it->first));
			}
		}
		std::sort(orders.begin(), orders.end());

		// Go through each file in order and output data in order of event ID
		int expectedEventID = 1;
		while (orders.size() > 0) {
			for (std::vector<std::pair<int, FILE*>>::iterator it = orders.begin();
			     it != orders.end(); it++) {
				eltcalcOutput * e = &eOut[it->second];
				if (e->event_id <= expectedEventID) {
					int currentEventID = e->event_id;
					do {
						fprintf(stdout, "%d,%d,%d,%f,%f,%f\n", e->summary_id, e->type, e->event_id, e->mean, e->standard_deviation, e->exposure_value);
						if (fgets(line, sizeof(line), it->second) == 0) {
							orders.erase(it);
							it--;
							break;
						} else {
							sscanf(line, "%d,%d,%d,%f,%f,%f", &e->summary_id, &e->type, &e->event_id, &e->mean, &e->standard_deviation, &e->exposure_value);
						}
						currentEventID = e->event_id;
					} while (currentEventID <= expectedEventID);
				}
				expectedEventID++;
			}
		}

	}

	void doit(std::vector <FILE*>& infiles)
	{
		for (FILE* fin : infiles) {
			unsigned char buf;
			size_t bytes = fread(&buf, 1, sizeof(buf), fin);
			while (bytes) {
				fwrite(&buf, 1, sizeof(buf), stdout);
				bytes = fread(&buf, 1, sizeof(buf), fin);
			}
		}
	}

	void touch(const std::string& filepath)
	{
		FILE* fout = fopen(filepath.c_str(), "wb");
		fclose(fout);
	}
	void setinitdone(int processid)
	{
		if (processid) {
			std::ostringstream s;
			s << SEMA_DIR_PREFIX << "_kat/" << processid << ".id";
			touch(s.str());
		}
	}
}
