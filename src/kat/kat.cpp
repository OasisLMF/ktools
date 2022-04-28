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
#include <cstring>


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

	enum number_of_fields { QELT = 4, SELT = 5, MELT = 11,
				QPLT = MELT, SPLT = 12, MPLT = 17,
       				LegacyELT = 6, LegacyPLT1 = 8, LegacyPLT2 = 10};

	struct period_event {
		int period;
		int eventID;
	};

	bool operator<(const period_event &lhs, const period_event &rhs) {
		if (lhs.period != rhs.period) {
			return lhs.period < rhs.period;
		} else {
			return lhs.eventID < rhs.eventID;
		}
	}

	struct fin_data {
		FILE *fin;
		char data[4096];
	};

	// Check whether input file is QPLT or MELT
	bool CheckQPLT(char *line) {
		int period, eventID, year, month, day, hour, minute, summaryID;
		float periodWeight, quantile, loss;
		if (sscanf(line, "%d,%f,%d,%d,%d,%d,%d,%d,%d,%f,%f", &period,
			   &periodWeight, &eventID, &year, &month, &day, &hour,
			   &minute, &summaryID, &quantile, &loss) == number_of_fields::QPLT)
		{
			return true;
		} else return false;
	}

	void GetRowORD(std::map<period_event, fin_data> &data_plt,
		       const fin_data &row) {
		// Sort by period number and then event number
		period_event periodEventID;
		float periodWeight;   // Not used but needs to be read in
		sscanf(row.data, "%d,%f,%d", &periodEventID.period,
		       &periodWeight, &periodEventID.eventID);
		data_plt[periodEventID] = row;
	}

	void GetRowLegacy(std::map<period_event, fin_data> &data_plt,
			  const fin_data &row) {
		// Sort by period number and then event number
		period_event periodEventID;
		int type, summary_id;
		sscanf(row.data, "%d,%d,%d,%d", &type, &summary_id,
		       &periodEventID.period, &periodEventID.eventID);
		data_plt[periodEventID] = row;
	}

	void GetRowORD(std::map<int, fin_data> &data_elt, const fin_data &row) {
		// Sort by event ID
		int eventID;
		sscanf(row.data, "%d", &eventID);
		data_elt[eventID] = row;
	}

	void GetRowLegacy(std::map<int, fin_data> &data_elt,
			  const fin_data &row) {
		// Sort by event ID
		int summary_id, type, eventID;
		sscanf(row.data, "%d,%d,%d", &summary_id, &type, &eventID);
		data_elt[eventID] = row;
	}

	template<typename rowMapT>
	inline void WriteAndReadRows(rowMapT &data_lt,
				     void (*&GetRow)(rowMapT&, const fin_data&)) {
		while (data_lt.size() != 0) {
			auto iter = data_lt.begin();
			fprintf(stdout, "%s", iter->second.data);

			fin_data row;
			row.fin = iter->second.fin;
			data_lt.erase(iter);
			if (fgets(row.data, sizeof(row.data), row.fin) != nullptr)
			{
				GetRow(data_lt, row);
			}
		}
	}

	void DoKatSort(std::vector<FILE*>& inFiles) {

		char line[4096];
		std::map<int, fin_data> data_elt;
		std::map<period_event, fin_data> data_plt;
		int expectedNArgs = 0;
		bool legacyFiles = false;
		// Use these to check that a combination of QPLT and MELT files
		// have not been entered for concatenation
		const int qpltMELTMask = 1 << 1 | 1;   // = 3
		int qpltMELTCheck = 0;

		// Get first line from each file, check for header and
		// determine table type
		for (std::vector<FILE*>::iterator it = inFiles.begin();
		     it != inFiles.end(); ++it) {

			fin_data row;
			int argCheck;
			int inArgs = 0;
			do {

				char *data = fgets(row.data, sizeof(row.data),
						   *it);
				if (data == nullptr) break;   // Empty file
				row.fin = *it;

				float val;
				int offset = 0;
				argCheck = sscanf(data, "%f%n", &val, &offset);
				if (argCheck == 0) {   // Found header
					fprintf(stdout, "%s", data);
					continue;
				}

				// Use number of fields to determine table type
				while (argCheck == 1) {
					inArgs++;
					data += offset;
					argCheck = sscanf(data, ",%f%n", &val,
							  &offset);
				}

				if (expectedNArgs == 0) expectedNArgs = inArgs;
				else if (inArgs != expectedNArgs) {
					fprintf(stderr,
						"FATAL: File formats do not match.\n");
					exit(EXIT_FAILURE);
				}

				switch (inArgs) {
					case number_of_fields::QELT:
					case number_of_fields::SELT:
						GetRowORD(data_elt, row);
						break;
					case number_of_fields::SPLT:
					case number_of_fields::MPLT:
						GetRowORD(data_plt, row);
						break;
					// As number of fields in MELT and QPLT
					// are identical, a further check must
					// be performed
					case number_of_fields::QPLT:
						if (CheckQPLT(row.data)) {
							GetRowORD(data_plt, row);
							qpltMELTCheck |= (1 << 1);
						} else {
							GetRowORD(data_elt, row);
							qpltMELTCheck |= 1;
						}
						if (qpltMELTCheck == qpltMELTMask) {
							fprintf(stderr,
								"FATAL: Cannot combine QPLT and MELT files\n");
							exit(EXIT_FAILURE);
						}
						break;
					case number_of_fields::LegacyELT:
						GetRowLegacy(data_elt, row);
						legacyFiles = true;
						break;
					case number_of_fields::LegacyPLT1:
					case number_of_fields::LegacyPLT2:
						GetRowLegacy(data_plt, row);
						legacyFiles = true;
						break;
					default:
						fprintf(stderr,
							"FATAL: Unknown file format:\n%s",
							row.data);
						exit(EXIT_FAILURE);
				}

				break;

			} while (true);

		}

		void (*GetELTRow)(std::map<int, fin_data>&, const fin_data&);
		void (*GetPLTRow)(std::map<period_event, fin_data>&, const fin_data&);
		if (legacyFiles) {
			GetELTRow = &GetRowLegacy;
			GetPLTRow = &GetRowLegacy;
		} else {
			GetELTRow = &GetRowORD;
			GetPLTRow = &GetRowORD;
		}
		// First entry should always have lowest event ID
		// Only one of data_elt and data_plt will actually have entries
		// so exploit this with a single check
		if (data_elt.size() > 0) WriteAndReadRows(data_elt, GetELTRow);
		else if (data_plt.size() > 0) WriteAndReadRows(data_plt, GetPLTRow);

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
