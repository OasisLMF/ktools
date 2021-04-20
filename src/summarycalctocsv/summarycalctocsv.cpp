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
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <thread>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

using namespace std;
#include "../include/oasis.h"

bool firstOutput = true;

int rowcount = 0;

void doitz(bool skipheader, bool fullprecision, bool show_exposure_value, bool remove_zero_exposure_records, bool ord_output)
{
	int summarycalcstream_type = 0;
	size_t i = fread(&summarycalcstream_type, sizeof(summarycalcstream_type), 1, stdin);
	int stream_type = summarycalcstream_type & summarycalc_id;

	if (stream_type != summarycalc_id) {
		std::cerr << "FATAL:Not a summarycalc stream type\n";
		exit(-1);
	}
	stream_type = streamno_mask & summarycalcstream_type;

	if (stream_type == 1) {
		if (skipheader == false) {
			// ORD output flag takes priority over exposure value flag
			// ImpactedNumLocs not currently supported in ORD output
			if (ord_output == true)
				printf("EventID,SummaryID,SampleID,Loss,ImpactedExposure\n");
			else if (show_exposure_value == true)
				printf("event_id,exposure_value,summary_id,sidx,loss\n");
			else
				printf("event_id,summary_id,sidx,loss\n");
		}
		int samplesize = 0;
		int summary_set = 0;
		i = fread(&samplesize, sizeof(samplesize), 1, stdin);
		if (i != 0) i = fread(&summary_set, sizeof(samplesize), 1, stdin);
		while (i != 0) {
			summarySampleslevelHeader sh;
			i = fread(&sh, sizeof(sh), 1, stdin);
			while (i != 0) {
				sampleslevelRec sr;
				i = fread(&sr, sizeof(sr), 1, stdin);
				if (i == 0) break;
				if (sr.sidx == 0) break;
				bool showRecord = false;
				if (remove_zero_exposure_records) {
					if (sh.expval > 0) {
						showRecord = true;
					}
				}else {
					showRecord = true;
				}
				
				if (showRecord) {
					if (fullprecision == true) {
						// ORD output flag takes priority over exposure value flag
						if (ord_output == true)
							printf("%d,%d,%d,%f,%f\n", sh.event_id, sh.summary_id, sr.sidx, sr.loss, sh.expval);
						else if (show_exposure_value == true)
							printf("%d,%f,%d,%d,%f\n", sh.event_id, sh.expval, sh.summary_id, sr.sidx, sr.loss);
						else
							printf("%d,%d,%d,%f\n", sh.event_id, sh.summary_id, sr.sidx, sr.loss);
					} else {
						// ORD output flag takes priority over exposure value flag
						if (ord_output == true)
							printf("%d,%d,%d,%.2f,%f\n", sh.event_id, sh.summary_id, sr.sidx, sr.loss, sh.expval);
						else if (show_exposure_value == true)
							printf("%d,%f,%d,%d,%.2f\n", sh.event_id, sh.expval, sh.summary_id, sr.sidx, sr.loss);
						else
							printf("%d,%d,%d,%.2f\n", sh.event_id, sh.summary_id, sr.sidx, sr.loss);
					}
					if (firstOutput == true) {
						std::this_thread::sleep_for(std::chrono::milliseconds(PIPE_DELAY));  // used to stop possible race condition with kat
						firstOutput = false;
					}
				}
			}
		}
		return;
	}
	fprintf(stderr, "FATAL: Unsupported summarycalc stream type\n");
}
void doit(bool skipheader, bool fullprecision,bool show_exposure_value, bool remove_zero_exposure_records, bool ord_output)
{
	if (remove_zero_exposure_records == true) {
		doitz(skipheader, fullprecision, show_exposure_value, remove_zero_exposure_records, ord_output);
		return;
	}
	int summarycalcstream_type = 0;
	size_t i = fread(&summarycalcstream_type, sizeof(summarycalcstream_type), 1, stdin);
	int stream_type = summarycalcstream_type & summarycalc_id;

	if (stream_type != summarycalc_id) {
		std::cerr << "FATAL: Not a summarycalc stream type\n";
		exit(-1);
	}
	stream_type = streamno_mask & summarycalcstream_type;

	if (stream_type == 1 ){
		if (skipheader == false) {
			// ORD output flag takes priority over exposure value flag
			// ImpactedNumLocs not currently supported in ORD output
			if (ord_output == true)
				printf("EventID,SummaryID,SampleID,Loss,ImpactedExposure\n");
			else if (show_exposure_value == true)
				printf("event_id,exposure_value,summary_id,sidx,loss\n");
			else
				printf("event_id,summary_id,sidx,loss\n");
		}
		int samplesize=0;
		int summary_set = 0;
		i=fread(&samplesize, sizeof(samplesize), 1, stdin);
		if( i != 0) i=fread(&summary_set, sizeof(samplesize), 1, stdin);
		while (i != 0){
			summarySampleslevelHeader sh;
			i = fread(&sh, sizeof(sh), 1, stdin);
			while (i != 0){
				sampleslevelRec sr;
				i = fread(&sr, sizeof(sr), 1, stdin);
				if (i == 0) break;	
				if (sr.sidx == 0) break;
				rowcount++;
				if (fullprecision == true) {
					// ORD output flag takes priority over exposure value flag
					if (ord_output == true)
						printf("%d,%d,%d,%f,%f\n", sh.event_id, sh.summary_id, sr.sidx, sr.loss, sh.expval);
					else if (show_exposure_value == true)
						printf("%d,%f,%d,%d,%f\n", sh.event_id, sh.expval, sh.summary_id, sr.sidx, sr.loss);
					else
						printf("%d,%d,%d,%f\n", sh.event_id, sh.summary_id, sr.sidx, sr.loss);
				} else {
					// ORD output flag takes priority over exposure value flag
					if (ord_output == true)
						printf("%d,%d,%d,%.2f,%f\n", sh.event_id, sh.summary_id, sr.sidx, sr.loss, sh.expval);
					else if (show_exposure_value == true)
						printf("%d,%f,%d,%d,%.2f\n", sh.event_id, sh.expval, sh.summary_id, sr.sidx, sr.loss);
					else
						printf("%d,%d,%d,%.2f\n", sh.event_id, sh.summary_id, sr.sidx, sr.loss);
				}
				if (firstOutput==true){
					std::this_thread::sleep_for(std::chrono::milliseconds(PIPE_DELAY));  // used to stop possible race condition with kat
					firstOutput=false;
				} 				
			}
		}
		return;
	}
	fprintf(stderr,"FATAL: Unsupported summarycalc stream type\n");

}

void help()
{
	fprintf(stderr,
		"-s skip header\n"
		"-f full precision\n"
		"-e show exposure_value\n"
		"-v version\n"
		"-z remove records with zero exposure values\n"
		"-o Open Results Data (ORD) output\n"
		"-h help\n"
	);
}

int main(int argc, char* argv[])
{

	int opt;
	bool skipheader = false;
	bool fullprecision = false;
	bool show_exposure_value = false;
	bool remove_zero_exposure_records = false;
	bool ord_output = false;
	while ((opt = getopt(argc, argv, "zvhfseo")) != -1) {
		switch (opt) {
		case 's':
			skipheader = true;
			break;
		case 'f':
			fullprecision = true;
			break;
		case 'e':
			show_exposure_value = true;
			break;
		case 'z':
			remove_zero_exposure_records = true;
			break;
		case 'o':
			ord_output = true;
			break;
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'h':
			help();
			exit(EXIT_FAILURE);
		}
	}

	initstreams();
	doit(skipheader, fullprecision, show_exposure_value, remove_zero_exposure_records, ord_output);
	return EXIT_SUCCESS;
}
