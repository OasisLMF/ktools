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

#include <chrono>
#include <thread>

#ifdef __unix
#include <unistd.h>
#endif

using namespace std;
#include "../include/oasis.h"
#include "summarycalc.h"



bool isGulStream(unsigned int stream_type)
{
	unsigned int stype = gulstream_id & stream_type;
	return (stype == gulstream_id);
}

bool isFMStream(unsigned int stream_type)
{
	unsigned int stype = fmstream_id & stream_type;
	return (stype == fmstream_id);
}
summarycalc::summarycalc()
{

#if defined(_MSC_VER) || defined(__MINGW32__)
	_setmode(_fileno(stdout), O_BINARY);
	_setmode(_fileno(stdin), O_BINARY);
#else
	freopen(NULL, "rb", stdin);
	freopen(NULL, "wb", stdout);
#endif

}
void summarycalc::reset_ssl_array(int summary_set, int sample_size, loss_exp **ssl)
{
	int maxsummaryids = max_summary_id_[summary_set] - min_summary_id_[summary_set]+1;
	for (int i = 0; i <= maxsummaryids; i++) {
		for(int j = 0; j < sample_size + 3; j++) {
			ssl[i][j].loss = 0;
		}
	}
}

void summarycalc::reset_sssl_array(int sample_size)
{
	for (int i = 0; i < MAX_SUMMARY_SETS; i++) {		
		if (fout[i] != nullptr) {
			loss_exp **ssl = sssl[i];		// OASIS_FLOAT **ssl two dimensional array of ssl[summary_id][sidx] to loss
			reset_ssl_array(i, sample_size, ssl);
		}
	}
}
loss_exp **summarycalc::alloc_ssl_arrays(int summary_set, int sample_size)
{
	if (min_summary_id_[summary_set] != 1) {
		fprintf(stderr, "summarycalc: Minimum summary ID is not equal to one\n");
		exit(-1);
	}
	int maxsummaryids = max_summary_id_[summary_set] ;
	loss_exp **ssl = new loss_exp*[maxsummaryids + 1];
	for (int i = 0; i <= maxsummaryids; i++){
		ssl[i] = new loss_exp[sample_size + 4];	// allocate for -3,-2,-1,0 as well as samplesize
	}
	return ssl;
}

void summarycalc::alloc_sssl_array(int sample_size)
{
	for (int i = 0; i < MAX_SUMMARY_SETS; i++) {
		if (fout[i] != nullptr) sssl[i] = alloc_ssl_arrays(i,sample_size);
	}
}

void summarycalc::reset_sse_array()
{
	for (int i = 0; i < MAX_SUMMARY_SETS; i++) {
		if (fout[i] != nullptr) {
			OASIS_FLOAT *se = sse[i];		// f array of se[summary_id] to exposure
			int maxsummaryids = max_summary_id_[i] - min_summary_id_[i] + 1;
			for (int j = 0; j <= maxsummaryids; j++) {
				se[j] = 0;
			}
		}
	}
}
OASIS_FLOAT *summarycalc::alloc_sse_arrays(int summary_set)
{
	int maxsummaryids = max_summary_id_[summary_set] - min_summary_id_[summary_set] + 1;
	OASIS_FLOAT *se = new OASIS_FLOAT [maxsummaryids + 1];
	return se;
}

void summarycalc::alloc_sse_array()
{
	for (int i = 0; i < MAX_SUMMARY_SETS; i++) {
		if (fout[i] != nullptr) sse[i] = alloc_sse_arrays(i);
	}
}

void summarycalc::init_c_to_s()
{
	for (int i = 0; i < MAX_SUMMARY_SETS; i++) {
		if (fout[i] != nullptr) {
			co_to_s[i] = new coverage_id_or_output_id_to_Summary_id;
			co_to_s[i]->resize(coverages_.size());
		}
	}
}
void summarycalc::init_o_to_s()
{
	for (int i = 0; i < MAX_SUMMARY_SETS; i++) {
		if (fout[i] != nullptr) co_to_s[i] = new coverage_id_or_output_id_to_Summary_id;
	}
}
bool summarycalc::loadcoverages()
{

	FILE *fin = fopen(COVERAGES_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: Error opening file %s\n", __func__, COVERAGES_FILE);
		exit(-1);
	}

	flseek(fin, 0L, SEEK_END);
	long long sz = fltell(fin);
	flseek(fin, 0L, SEEK_SET);

	OASIS_FLOAT tiv;
	unsigned int nrec = sz / sizeof(tiv);

	coverages_.resize(nrec + 1);
	int coverage_id = 0;
	int i = fread(&tiv, sizeof(tiv), 1, fin);
	while (i != 0) {
		coverage_id++;
		coverages_[coverage_id] = tiv;
		i = fread(&tiv, sizeof(tiv), 1, fin);
	}

	fclose(fin);
	return true;

}
void summarycalc::loadgulsummaryxref()
{
	FILE *fin = fopen(GULSUMMARYXREF_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: Error opening file %s\n", __func__, GULSUMMARYXREF_FILE);
		::exit(-1);
	}

	init_c_to_s();
	gulsummaryxref s;
	int i = fread(&s, sizeof(gulsummaryxref), 1, fin);
	while (i != 0) {
		if (s.summaryset_id > 9) {
			fprintf(stderr, "%s: Invalid summaryset id  %d found in %s\n", __func__, s.summaryset_id, GULSUMMARYXREF_FILE);
			::exit(-1);
		}
		if (fout[s.summaryset_id] != nullptr) {
			if (s.summary_id < min_summary_id_[s.summaryset_id]) min_summary_id_[s.summaryset_id] = s.summary_id;
			if (s.summary_id > max_summary_id_[s.summaryset_id]) max_summary_id_[s.summaryset_id] = s.summary_id;
			//co_to_s[s.summaryset_id]->insert({ s.coverage_id,s.summary_id });
			(*co_to_s[s.summaryset_id])[s.coverage_id] = s.summary_id ;
		}
		i = fread(&s, sizeof(gulsummaryxref), 1, fin);
	}

	fclose(fin);
}

void summarycalc::loadfmsummaryxref()
{
	FILE *fin = fopen(FMSUMMARYXREF_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: Error opening file %s\n", __func__, FMSUMMARYXREF_FILE);
		::exit(-1);
	}

	init_o_to_s();

	fmsummaryxref s;
	int i = fread(&s, sizeof(fmsummaryxref), 1, fin);
	while (i != 0) {
		if (s.summaryset_id > 9) {
			fprintf(stderr, "%s: Invalid summaryset id  %d found in %s\n", __func__, s.summaryset_id, FMSUMMARYXREF_FILE);
			::exit(-1);
		}
		if (fout[s.summaryset_id] != nullptr) {
			if (s.summary_id < min_summary_id_[s.summaryset_id]) min_summary_id_[s.summaryset_id] = s.summary_id;
			if (s.summary_id > max_summary_id_[s.summaryset_id]) max_summary_id_[s.summaryset_id] = s.summary_id;
			// co_to_s[s.summaryset_id]->insert({ s.output_id,s.summary_id });
			if ((*co_to_s[s.summaryset_id]).size() < (s.output_id + 1)) (*co_to_s[s.summaryset_id]).resize(s.output_id + 1, 0);
			(*co_to_s[s.summaryset_id])[s.output_id] = s.summary_id ;
		}
		i = fread(&s, sizeof(fmsummaryxref), 1, fin);
	}

	fclose(fin);
}

void summarycalc::outputsummaryset(int sample_size, int summary_set, int event_id)
{
	loss_exp **ssl = sssl[summary_set]; // OASIS_FLOAT **ssl two dimensional array of ssl[summary_id][sidx] to loss
	OASIS_FLOAT *se = sse[summary_set];
	int maxsummaryids = max_summary_id_[summary_set];
	int minsummaryids = min_summary_id_[summary_set];
	for (int i = minsummaryids ; i <= maxsummaryids; i++) {
		summarySampleslevelHeader sh;
		sh.event_id = event_id;
		sh.summary_id = i;
		sh.expval = se[i];
		fwrite(&sh, sizeof(sh), 1, fout[summary_set]);
		for (int j = 0; j < sample_size + 3; j++) {
			if (j != 2) {
				sampleslevelRec s;
				s.sidx = j - 2;
				if (s.sidx == -2) {
					// skip
				}
				else {
					s.loss = ssl[i][j].loss;
					if (s.sidx > 0) {
						if (s.loss > 0.0) fwrite(&s, sizeof(s), 1, fout[summary_set]);
					}else {
						fwrite(&s, sizeof(s), 1, fout[summary_set]);
					}
					
				}
			}
		}
		sampleslevelRec s;
		s.sidx = 0;
		s.loss = 0.0;
		fwrite(&s, sizeof(s), 1, fout[summary_set]);
	}
}

void summarycalc::openpipe(int summary_id, const std::string &pipe)
{
	if (pipe == "-") fout[summary_id] = stdout;
	else {
		FILE *f = fopen(pipe.c_str(), "wb");
		if (f != nullptr) fout[summary_id] = f;
		else {
			fprintf(stderr, "%s: Cannot open %s for output\n", __func__, pipe.c_str());
			::exit(-1);
		}
	}
}


void summarycalc::outputstreamtype(int summary_set)
{
	int streamtype = summarycalc_id | 1;
	fwrite(&streamtype, sizeof(streamtype), 1, fout[summary_set]);
	std::this_thread::sleep_for(std::chrono::milliseconds(PIPE_DELAY));
}

void summarycalc::outputstreamtype()
{
	for (int i = 0; i < MAX_SUMMARY_SETS; i++) {
		if (fout[i] != nullptr) {
			outputstreamtype(i);
		}
	}
}
void summarycalc::outputsamplesizeandsummaryset(int summary_set, int sample_size)
{
	fwrite(&sample_size, sizeof(sample_size), 1, fout[summary_set]);
	fwrite(&summary_set, sizeof(summary_set), 1, fout[summary_set]);
}
void summarycalc::outputsamplesize(int samplesize)
{
	for (int i = 0; i < MAX_SUMMARY_SETS; i++) {
		if (fout[i] != nullptr) {
			outputsamplesizeandsummaryset(i,samplesize);
		}
	}
}

void summarycalc::outputsummary(int sample_size,int event_id)
{
	for (int i = 0; i < MAX_SUMMARY_SETS; i++) {
		if (fout[i] != nullptr) {
			outputsummaryset(sample_size,i, event_id);
		}
	}
}
void summarycalc::processsummeryset(int summaryset, int event_id, int coverage_id, int sidx, OASIS_FLOAT gul)
{
	loss_exp **ssl = sssl[summaryset];
	coverage_id_or_output_id_to_Summary_id &p = *co_to_s[summaryset];
	int summary_id = p[coverage_id];	
	ssl[summary_id][sidx+2].loss += gul;
}


void summarycalc::dosummary(int sample_size,int event_id,int coverage_or_output_id,int sidx, OASIS_FLOAT gul, OASIS_FLOAT expval)
{
	static int last_event_id = -1;
	static int last_coverage_or_output_id = -1;
		
	if (last_event_id != event_id) {		
		if (last_event_id != -1) {
			outputsummary(sample_size,last_event_id);
			reset_sssl_array(sample_size);
		}
		last_event_id = event_id;
		last_coverage_or_output_id = -1;
		reset_sse_array();
	}
	if (coverage_or_output_id != last_coverage_or_output_id) {
  		for (int i = 0; i < MAX_SUMMARY_SETS; i++) {
			if (fout[i] != nullptr) {
				coverage_id_or_output_id_to_Summary_id &p = *co_to_s[i];
				int summary_id = p[coverage_or_output_id];
				OASIS_FLOAT *se = sse[i];
				se[summary_id] += expval;
			}
		}
		last_coverage_or_output_id = coverage_or_output_id;
	}
	for (int i = 0; i < MAX_SUMMARY_SETS; i++) {
		if (fout[i] != nullptr) processsummeryset(i, event_id, last_coverage_or_output_id, sidx, gul);
	}
}

void summarycalc::dogulsummary()
{
	loadcoverages();
	loadgulsummaryxref();	
	outputstreamtype();
	unsigned int streamtype = 0;
	int i = fread(&streamtype, sizeof(streamtype), 1, stdin);

	if (isGulStream(streamtype) == true) {
		int stream_type = streamtype & streamno_mask;
		unsigned int samplesize = 0;
		if (stream_type == 2) {
			i = fread(&samplesize, sizeof(samplesize), 1, stdin);
			alloc_sssl_array(samplesize);
			reset_sssl_array(samplesize);
			alloc_sse_array();
			reset_sse_array();
			outputsamplesize(samplesize);
			gulSampleslevelHeader gh;
			bool havedata = false;
			while (i == 1) {
				i = fread(&gh, sizeof(gh), 1, stdin);
				if (i > 0 && havedata == false) havedata = true;
				while (i != 0) {
					gulSampleslevelRec gr;
					i = fread(&gr, sizeof(gr), 1, stdin);
					if (i == 0) break;
					if (gr.sidx == 0) break;
					dosummary(samplesize, gh.event_id, gh.item_id, gr.sidx, gr.loss, coverages_[gh.item_id]);
				}
			}
			if (havedata) outputsummary(samplesize, gh.event_id);
			return;
		}

		std::cerr << "summarycalc: Gul stream type " << stream_type << " not supported\n";
		::exit(-1);
	}
	else {
		std::cerr << "summarycalc: Not a gul stream\n";
		std::cerr << "summarycalc: invalid stream type: " << streamtype << "\n";
	}
	
	::exit(-1);
}

void summarycalc::dofmsummary()
{
	loadfmsummaryxref();
	outputstreamtype();
	unsigned int streamtype = 0;
	int i = fread(&streamtype, sizeof(streamtype), 1, stdin);
	if (isFMStream(streamtype) == true) {
		int stream_type = streamtype & streamno_mask;
		unsigned int samplesize = 0;
		i = fread(&samplesize, sizeof(samplesize), 1, stdin);
		alloc_sssl_array(samplesize);
		reset_sssl_array(samplesize);
		alloc_sse_array();
		reset_sse_array();
		outputsamplesize(samplesize);
		fmlevelhdr fh;
		OASIS_FLOAT expure_val = 0;
		bool havedata = false;
		while (i == 1) {
			i = fread(&fh, sizeof(fh), 1, stdin);
			if (i > 0 && havedata == false) havedata = true;
			while (i != 0) {
				sampleslevelRec sr;
				i = fread(&sr, sizeof(sr), 1, stdin);
				if (i == 0) break;
				if (sr.sidx == 0) break;
				if (sr.sidx == -3) {
					expure_val = sr.loss;
				}else {
					dosummary(samplesize, fh.event_id, fh.output_id, sr.sidx, sr.loss, expure_val);
				}				
			}			
		}
		if (havedata) outputsummary(samplesize, fh.event_id);
		return;
	}else {
		std::cerr << "summarycalc: Not a fm stream\n";
		std::cerr << "summarycalc: invalid stream type: " << streamtype << "\n";
	}	
	::exit(-1);

}
void summarycalc::doit()
{
	if (inputtype_ == UNKNOWN) {
		fprintf(stderr,"summarycalc: tream type unknown\n");
		return;
	}
	
	if (inputtype_ == FM_STREAM) {
		dofmsummary();
	}
	
	if (inputtype_ == GUL_STREAM) {
		dogulsummary();
	}
	
}
