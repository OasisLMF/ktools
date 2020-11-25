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
Convert footprint to csv
Author: Joh Carter  email: johanna.carter@oasislmf.org
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "../include/oasis.h"
#if defined(_MSC_VER)
// zlib.h relies on this flag - sometimes our headers lie because we're 
// developing on multiple platforms
#undef HAVE_UNISTD_H
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif


#include <vector>
#include <zlib.h>

using namespace std;
namespace footprinttocsv {
	void printrows(int event_id, FILE *finx, long long size)
	{
		long long i = 0;

		while (i < size) {
			EventRow row;
			fread(&row, sizeof(row), 1, finx);
#ifdef AREAPERIL_TYPE_UNSIGNED_LONG_LONG
			printf("%d, %llu, %d, %.10e\n", event_id, row.areaperil_id, row.intensity_bin_id, row.probability);
#else
			printf("%d, %d, %d, %.10e\n", event_id, row.areaperil_id, row.intensity_bin_id, row.probability);
#endif
			i += sizeof(row);
		}
	}

	void printrowsz(int event_id, FILE *finx, long long size,
			long long uncompressed_length)
	{
		uLongf i = 0;
		std::vector<unsigned char > compressed_buf;
		compressed_buf.resize(size + 1);

		std::vector<unsigned char > uncompressed_buf;
		if (uncompressed_length > 0) {
			uncompressed_buf.resize(uncompressed_length);
		} else {
			uncompressed_buf.resize(size * 20);
		}
		fread(&compressed_buf[0], size, 1, finx);
		uLongf dest_length = (uLongf)uncompressed_buf.size();
		int ret = uncompress(&uncompressed_buf[0], &dest_length, &compressed_buf[0], size);
		if (ret != Z_OK) {
			fprintf(stderr, "FATAL: Got bad return code from uncompress %d\n", ret);
			exit(-1);
		}

		EventRow *row = (EventRow *)&uncompressed_buf[0];
		while (i < dest_length) {
#ifdef AREAPERIL_TYPE_UNSIGNED_LONG_LONG
			printf("%d, %llu, %d, %.10e\n", event_id, row->areaperil_id, row->intensity_bin_id, row->probability);
#else
			printf("%d, %d, %d, %.10e\n", event_id, row->areaperil_id, row->intensity_bin_id, row->probability);
#endif
			row++;
			i += sizeof(EventRow);
		}
	}



	void doitz(bool skipheader, int from_event, int to_event, const char * binFileName="footprint.bin.z", const char * idxFileName="footprint.idx.z")
	{
		if (skipheader == false)  printf("event_id, areaperil_id, intensity_bin_id, probability\n");
		FILE *finx = fopen(binFileName, "rb");
		FILE *finy = fopen(idxFileName, "rb");
		EventIndex current_idx;
		EventIndex next_idx;

		// Establish whether uncompressed data size is in index file
		int zipOpts;
		int uncompressedMask = 1 << 1;
		flseek(finx, sizeof(int), SEEK_SET);
		fread(&zipOpts, sizeof(zipOpts), 1, finx);
		bool uncompressedSize = (zipOpts & uncompressedMask) >> 1;

		if (finy == nullptr) {
			fprintf(stderr, "FATAL: Footprint idx open failed\n");
			exit(3);
		}

		size_t i = fread(&current_idx, sizeof(current_idx), 1, finy);
		long long uncompressed_length = 0;
		if (uncompressedSize) {
			fread(&uncompressed_length, sizeof(uncompressed_length),
			      1, finy);
		}
		size_t compressed_length = 0;
		while (i != 0) {
			i = fread(&next_idx, sizeof(next_idx), 1, finy);
			if (i != 0) {
				compressed_length = next_idx.offset - current_idx.offset;
			}
			else {
				flseek(finx, 0L, SEEK_END);
				long long pos = fltell(finx);
				compressed_length = pos - current_idx.offset;
			}
			if (current_idx.event_id >= from_event && current_idx.event_id <= to_event) {
				flseek(finx, current_idx.offset, SEEK_SET);
				printrowsz(current_idx.event_id, finx, compressed_length, uncompressed_length);
			}
			if (uncompressedSize) {
				fread(&uncompressed_length,
				      sizeof(uncompressed_length), 1, finy);
			}
			if (i != 0) current_idx = next_idx;
		}

		fclose(finx);
		fclose(finy);
	}

	void doit(bool skipheader, int from_event, int to_event, const char * binFileName="footprint.bin", const char * idxFileName="footprint.idx")
	{
		if (skipheader == false)  printf("event_id, areaperil_id, intensity_bin_id, probability\n");
		FILE *finx = fopen(binFileName, "rb");
		FILE *finy = fopen(idxFileName, "rb");

		EventIndex idx;

		if (finy == nullptr) {
			fprintf(stderr, "FATAL: Footprint idx open failed\n");
			exit(3);
		}
		size_t i = fread(&idx, sizeof(idx), 1, finy);
		while (i != 0) {
			if (idx.event_id >= from_event && idx.event_id <= to_event) {
				flseek(finx, idx.offset, SEEK_SET);
				printrows(idx.event_id, finx, idx.size);
			}
			i = fread(&idx, sizeof(idx), 1, finy);
		}

		fclose(finx);
		fclose(finy);
	}

}
