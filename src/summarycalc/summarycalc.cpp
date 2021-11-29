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
#include <functional>

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



bool summarycalc::isGulStream(unsigned int stream_type)
{
	unsigned int stype = gulstream_id & stream_type;
	return (stype == gulstream_id);
}

bool summarycalc::isFMStream(unsigned int stream_type)
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
void summarycalc::reset_ssl_array(int summary_set, int sample_size, OASIS_FLOAT **ssl)
{
	int maxsummaryids = max_summary_id_[summary_set] - min_summary_id_[summary_set]+1;
	for (int i = 0; i <= maxsummaryids; i++) {
		for (int j = 0; j < sample_size + num_idx_ + 1; j++) {
			ssl[i][j] = 0;
		}
	}
}

void summarycalc::reset_sssl_array(int sample_size)
{
	for (int i = 0; i < MAX_SUMMARY_SETS; i++) {		
		if (fout[i] != nullptr) {
			OASIS_FLOAT **ssl = sssl[i];		// OASIS_FLOAT **ssl two dimensional array of ssl[summary_id][sidx] to loss
			reset_ssl_array(i, sample_size, ssl);
		}
	}
}
OASIS_FLOAT **summarycalc::alloc_ssl_arrays(int summary_set, int sample_size)
{
	if (min_summary_id_[summary_set] != 1) {
		fprintf(stderr, "FATAL: summarycalc: Minimum summary ID is not equal to one\n");
		exit(-1);
	}
	int maxsummaryids = max_summary_id_[summary_set] ;
	OASIS_FLOAT **ssl = new OASIS_FLOAT*[maxsummaryids + 1];
	for (int i = 0; i <= maxsummaryids; i++){
		ssl[i] = new OASIS_FLOAT[sample_size + num_idx_ + 1];   // allocate for -5, -4, -3, -2, -1, 0 as well as samplesize
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
			co_to_s_[i] = new coverage_id_or_output_id_to_Summary_id;
			co_to_s_[i]->resize(coverages_.size());
		}
	}
}
void summarycalc::init_o_to_s()
{
	for (int i = 0; i < MAX_SUMMARY_SETS; i++) {
		if (fout[i] != nullptr) co_to_s_[i] = new coverage_id_or_output_id_to_Summary_id;
	}
}
void summarycalc::loaditemtocoverage()
{
	FILE* fin = NULL;
	std::string file = ITEMS_FILE;
	if (inputpath_.length() > 0) {
		file = inputpath_ + file.substr(5);
	}
	fin = fopen(file.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "FATAL: %s cannot open %s\n", __func__, file.c_str());
		exit(EXIT_FAILURE);
	}

	flseek(fin, 0L, SEEK_END);
	long long sz = fltell(fin);
	flseek(fin, 0L, SEEK_SET);
	int last_item_id = 0;
	unsigned int nrec = (unsigned int)sz / (unsigned int)sizeof(item);
	item_to_coverage_.resize(nrec + 1, 0);

	item itm;
	size_t i = fread(&itm, sizeof(itm), 1, fin);
	while (i != 0) {
		last_item_id++;
		if (itm.id != last_item_id) {
			fprintf(stderr, "FATAL: Item ids are not contiguous or do not start from one");
			exit(-1);
		}
		last_item_id = itm.id;
		item_to_coverage_[itm.id] = itm.coverage_id;
		i = fread(&itm, sizeof(itm), 1, fin);
	}
	fclose(fin);

}
bool summarycalc::loadcoverages()
{
	std::string file = COVERAGES_FILE;
	if (inputpath_.length() > 0) {
		file = inputpath_ + file.substr(5);
	}
	FILE *fin = fopen(file.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "FATAL: %s Error opening file %s\n", __func__, file.c_str());
		exit(-1);
	}

	flseek(fin, 0L, SEEK_END);
	long long sz = fltell(fin);
	flseek(fin, 0L, SEEK_SET);

	OASIS_FLOAT tiv;
	unsigned int nrec = (unsigned int)sz / (unsigned int) sizeof(tiv);

	coverages_.resize(nrec + 1);
	int coverage_id = 0;
	int i = (int) fread(&tiv,sizeof(tiv),1, fin);
	while (i != 0) {
		coverage_id++;
		coverages_[coverage_id] = tiv;
		i = (int)fread(&tiv, sizeof(tiv),
			1, fin);
	}

	fclose(fin);
	return true;

}
void summarycalc::loadgulsummaryxref()
{
	std::string file = GULSUMMARYXREF_FILE;
	if (inputpath_.length() > 0) {
		file = inputpath_ + file.substr(5);
	}
	FILE *fin = fopen(file.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "FATAL: %s: Error opening file %s\n", __func__, file.c_str());
		::exit(-1);
	}

	init_c_to_s();
	gulsummaryxref s;
	int i = (int)fread(&s, sizeof(gulsummaryxref), 1, fin);
	while (i != 0) {
		if (s.summaryset_id > 9) {
			fprintf(stderr, "FATAL: %s: Invalid summaryset id  %d found in %s\n", __func__, s.summaryset_id, file.c_str());
			::exit(-1);
		}
		if (fout[s.summaryset_id] != nullptr) {
			if (s.summary_id < min_summary_id_[s.summaryset_id]) min_summary_id_[s.summaryset_id] = s.summary_id;
			if (s.summary_id > max_summary_id_[s.summaryset_id]) max_summary_id_[s.summaryset_id] = s.summary_id;
			(*co_to_s_[s.summaryset_id])[s.coverage_id] = s.summary_id ;
		}
		i = (int) fread(&s, sizeof(gulsummaryxref), 1, fin);
	}

	fclose(fin);
}

void summarycalc::loadsummaryxref(const std::string& filename)
{
	std::string file = filename;
	if (inputpath_.length() > 0) {
		file = inputpath_ + file.substr(5);
	}
	FILE *fin = fopen(file.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "FATAL: %s: Error opening file %s\n", __func__, file.c_str());
		::exit(-1);
	}

	init_o_to_s();

	fmsummaryxref s;
	int i = (int)fread(&s, sizeof(fmsummaryxref), 1, fin);
	while (i != 0) {
		if (s.summaryset_id > 9) {
			fprintf(stderr, "FATAL: %s: Invalid summaryset id  %d found in %s\n", __func__, s.summaryset_id, file.c_str());
			::exit(-1);
		}
		if (fout[s.summaryset_id] != nullptr) {
			if (s.summary_id < min_summary_id_[s.summaryset_id]) min_summary_id_[s.summaryset_id] = s.summary_id;
			if (s.summary_id > max_summary_id_[s.summaryset_id]) max_summary_id_[s.summaryset_id] = s.summary_id;
			if ( (*co_to_s_[s.summaryset_id]).size() < (size_t)(s.output_id + 1)) (*co_to_s_[s.summaryset_id]).resize(s.output_id + 1, 0);
			(*co_to_s_[s.summaryset_id])[s.output_id] = s.summary_id ;
		}
		i = (int) fread(&s, sizeof(fmsummaryxref), 1, fin);
	}

		fclose(fin);
}

void summarycalc::outputsummaryset(int sample_size, int summary_set, int event_id)
{
	OASIS_FLOAT **ssl = sssl[summary_set]; // OASIS_FLOAT **ssl two dimensional array of ssl[summary_id][sidx] to loss
	OASIS_FLOAT *se = sse[summary_set];
	int maxsummaryids = max_summary_id_[summary_set];
	int minsummaryids = min_summary_id_[summary_set];
	for (int i = minsummaryids ; i <= maxsummaryids; i++) {
		summarySampleslevelHeader sh;
		sh.event_id = event_id;
		sh.summary_id = i;
		sh.expval = se[i];
		if (sh.expval > 0 || zerooutput_ == true) {
			if (idxout[summary_set] != nullptr) {   // Fill index file
				fprintf(idxout[summary_set], "%d,%lld\n", i, offset_[summary_set]);
			}
			fwrite(&sh, sizeof(sh), 1, fout[summary_set]);
			offset_[summary_set] += sizeof(sh);
			for (int j = first_idx_; j < sample_size + num_idx_ + 1; j++) {
				if (j != num_idx_) {   // sidx = 0
					sampleslevelRec s;
					s.sidx = j - num_idx_;
					if (s.sidx == tiv_idx || s.sidx == std_dev_idx) {
						// skip
					}
					else {
						s.loss = ssl[i][j];
						if (zerooutput_ == false) {
							if (s.loss > 0.0) {
								fwrite(&s, sizeof(s), 1, fout[summary_set]);
								offset_[summary_set] += sizeof(s);
							}
						}
						else {
							fwrite(&s, sizeof(s), 1, fout[summary_set]);
							offset_[summary_set] += sizeof(s);
						}

					}
				}
			}
			sampleslevelRec s;
			s.sidx = 0;
			s.loss = 0.0;
			fwrite(&s, sizeof(s), 1, fout[summary_set]);
			offset_[summary_set] += sizeof(s);
		}
	}
}

void summarycalc::openpipe(int summary_id, const std::string &pipe)
{
	if (pipe == "-") fout[summary_id] = stdout;
	else {
		FILE *f = fopen(pipe.c_str(), "wb");
		if (f != nullptr) {
			fout[summary_id] = f;
			std::string indexFileName = pipe;
			if (pipe.length() > 4 && pipe.substr(pipe.length() - 4, 4) == ".bin") {
				indexFileName.erase(indexFileName.length() - 4);
			}
			indexFileName += ".idx";
			indexFiles[summary_id] = indexFileName;
		} else {
			fprintf(stderr, "FATAL: %s: Cannot open %s for output\n", __func__, pipe.c_str());
			::exit(-1);
		}
	}
}


void summarycalc::openindexfiles()
{
	for (auto x : indexFiles) {
		FILE *f = fopen(x.second.c_str(), "w");
		idxout[x.first] = f;
	}
}


void summarycalc::outputstreamtype(int summary_set)
{
	int streamtype = summarycalc_id | 1;
	fwrite(&streamtype, sizeof(streamtype), 1, fout[summary_set]);
	offset_[summary_set] += sizeof(streamtype);
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
	offset_[summary_set] += sizeof(sample_size) + sizeof(summary_set);
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


OASIS_FLOAT summarycalc::add_losses(const OASIS_FLOAT loss,
				    const OASIS_FLOAT gul)
{
	return loss + gul;
}


OASIS_FLOAT summarycalc::calculate_chanceofloss(const OASIS_FLOAT loss,
						const OASIS_FLOAT gul)
{
	if (loss == 0.0) return gul;
	else return loss * gul;
}


void summarycalc::processsummaryset(int summaryset, int coverage_id, int sidx,
				    OASIS_FLOAT gul,
				    OASIS_FLOAT (summarycalc::*PropagateLosses)(const OASIS_FLOAT, const OASIS_FLOAT))
{
	OASIS_FLOAT **ssl = sssl[summaryset];
	coverage_id_or_output_id_to_Summary_id &p = *co_to_s_[summaryset];
	int summary_id = p[coverage_id];
	ssl[summary_id][sidx+num_idx_] = (this->*PropagateLosses)(ssl[summary_id][sidx+num_idx_], gul);
}


inline void summarycalc::reset_for_new_event(const int sample_size,
					     const int event_id)
{
	if (last_event_id_ != -1) {
		outputsummary(sample_size, last_event_id_);
		reset_sssl_array(sample_size);
	}
	last_event_id_ = event_id;
	last_coverage_or_output_id_ = -1;
	reset_sse_array();
}


inline void summarycalc::processsummarysets(const int coverage_or_output_id,
					    const int sidx,
					    const OASIS_FLOAT gul,
					    OASIS_FLOAT (summarycalc::*PropagateLosses)(const OASIS_FLOAT, const OASIS_FLOAT))
{
	for (int i = 0; i < MAX_SUMMARY_SETS; i++) {
		int cov_id = coverage_or_output_id;
		if (item_to_coverage_.size() > 0) {
			cov_id = item_to_coverage_[coverage_or_output_id];
		}
		if (fout[i] != nullptr) processsummaryset(i, cov_id, sidx, gul,
							  PropagateLosses);
	}
}


void summarycalc::dosummary_chanceofloss_maxloss(int sample_size, int event_id,
						 int coverage_or_output_id,
						 int sidx, OASIS_FLOAT gul)
{
	first_idx_ = 0;   // ChanceOfLoss & MaxLoss fields exist
	if (last_event_id_ != event_id)
		reset_for_new_event(sample_size, event_id);
	if (sidx == chance_of_loss_idx) {
		PropagateLosses = &summarycalc::calculate_chanceofloss;
	} else {
		PropagateLosses = &summarycalc::add_losses;
	}
	processsummarysets(coverage_or_output_id, sidx, gul, PropagateLosses);
}


void summarycalc::dosummary(int sample_size,int event_id,int coverage_or_output_id,int sidx, OASIS_FLOAT gul, OASIS_FLOAT expval)
{
	if (last_event_id_ != event_id)
		reset_for_new_event(sample_size, event_id);

	if (coverage_or_output_id != last_coverage_or_output_id_) {
  		for (int i = 0; i < MAX_SUMMARY_SETS; i++) {
			if (fout[i] != nullptr) {
				coverage_id_or_output_id_to_Summary_id &p = *co_to_s_[i];
				int summary_id = 0;
				if (item_to_coverage_.size() > 0) {
					int cov_id = item_to_coverage_[coverage_or_output_id];
					summary_id = p[cov_id];
				} else {
					summary_id = p[coverage_or_output_id];
				}
				OASIS_FLOAT *se = sse[i];
				se[summary_id] += expval;
			}
		}
		last_coverage_or_output_id_ = coverage_or_output_id;
	}

	PropagateLosses = &summarycalc::add_losses;
	processsummarysets(last_coverage_or_output_id_, sidx, gul,
			   PropagateLosses);
}
// item like stream
void summarycalc::dogulitemxsummary()
{
	loadcoverages();
	loaditemtocoverage();
	loadgulsummaryxref();
	outputstreamtype();
	unsigned int streamtype = 0;
	int i = (int) fread(&streamtype, sizeof(streamtype), 1, stdin);
	if (isGulStream(streamtype) == true) {
		int stream_type = streamtype & streamno_mask;
		unsigned int samplesize = 0;
		if (stream_type == 1) {
			i = (int) fread(&samplesize, sizeof(samplesize), 1, stdin);
			alloc_sssl_array(samplesize);
			reset_sssl_array(samplesize);
			alloc_sse_array();
			reset_sse_array();
			outputsamplesize(samplesize);
			gulSampleslevelHeader gh;
			bool havedata = false;
			while (i == 1) {
				i = (int)fread(&gh, sizeof(gh), 1, stdin);
				if (i > 0 && havedata == false) havedata = true;
				while (i != 0) {
					gulSampleslevelRec gr;
					i = (int) fread(&gr, sizeof(gr), 1, stdin);
					if (i == 0) break;
					if (gr.sidx == 0) break;
					int coverage_id = item_to_coverage_[gh.item_id];
					dosummary(samplesize, gh.event_id, coverage_id, gr.sidx, gr.loss, coverages_[coverage_id]);
				}
			}
			return;
		}
		std::cerr << "summarycalc: Unexpected Gul stream type " << stream_type << " expecting gulitem stream\n";
		::exit(-1);
	}
}
void summarycalc::dogulcoveragesummary()
{
	loadcoverages();
	loadgulsummaryxref();	
	outputstreamtype();
	unsigned int streamtype = 0;
	int i = (int) fread(&streamtype, sizeof(streamtype), 1, stdin);

	if (isGulStream(streamtype) == true) {
		int stream_type = streamtype & streamno_mask;
		unsigned int samplesize = 0;
		if (stream_type == 2) {
			i = (int) fread(&samplesize, sizeof(samplesize), 1, stdin);
			alloc_sssl_array(samplesize);
			reset_sssl_array(samplesize);
			alloc_sse_array();
			reset_sse_array();
			outputsamplesize(samplesize);
			gulSampleslevelHeader gh;
			bool havedata = false;
			while (i == 1) {
				i = (int) fread(&gh, sizeof(gh), 1, stdin);
				if (i > 0 && havedata == false) havedata = true;
				while (i != 0) {
					gulSampleslevelRec gr;
					i = (int)fread(&gr, sizeof(gr), 1, stdin);
					if (i == 0) break;
					if (gr.sidx == 0) break;
					dosummary(samplesize, gh.event_id, gh.item_id, gr.sidx, gr.loss, coverages_[gh.item_id]);
				}
			}
			if (havedata) outputsummary(samplesize, gh.event_id);
			return;
		}

		std::cerr << "FATAL: summarycalc: Gul stream type " << stream_type << " not supported\n";
		::exit(-1);
	}
	else {
		std::cerr << "FATAL: summarycalc: Not a gul stream invalid stream type: " << streamtype << "\n";
	}
	
	::exit(-1);
}
void summarycalc::dosummaryprocessing(int samplesize)
{
	alloc_sssl_array(samplesize);
	reset_sssl_array(samplesize);
	alloc_sse_array();
	reset_sse_array();
	outputsamplesize(samplesize);
	fmlevelhdr fh;	
	bool havedata = false;
	int i = 1;
	while (i == 1) {
		i = (int) fread(&fh, sizeof(fh), 1, stdin);
		OASIS_FLOAT expure_val = 0;
		if (i > 0 && havedata == false) havedata = true;
		while (i != 0) {
			sampleslevelRec sr;
			i = (int)fread(&sr, sizeof(sr), 1, stdin);
			if (i == 0) break;
			if (sr.sidx == 0) break;
			if (sr.sidx == tiv_idx) {
				expure_val = sr.loss;
			} else if (sr.sidx == chance_of_loss_idx || sr.sidx == max_loss_idx) {
				dosummary_chanceofloss_maxloss(samplesize, fh.event_id, fh.output_id, sr.sidx, sr.loss);
			} else {
				dosummary(samplesize, fh.event_id, fh.output_id, sr.sidx, sr.loss, expure_val);
			}
		}
	}
	if (havedata) outputsummary(samplesize, fh.event_id);
}
void summarycalc::dofmsummary()
{
	loadsummaryxref(FMSUMMARYXREF_FILE);
	outputstreamtype();
	unsigned int streamtype = 0;
	int i = (int) fread(&streamtype, sizeof(streamtype), 1, stdin);
	if (i) {
		if (isFMStream(streamtype) == true) {
			unsigned int samplesize = 0;
			i = (int) fread(&samplesize, sizeof(samplesize), 1, stdin);
			if (i)	dosummaryprocessing(samplesize);
			else std::cerr << "FATAL: summarycalc: Read error on stream\n";
			return;
		}
		else {
			std::cerr << "FATAL: summarycalc: Not a fm stream\n";
			std::cerr << "FATAL: summarycalc: invalid stream type: " << streamtype << "\n";
		}
	}
	else {
		std::cerr << "FATAL: summarycalc: Read error on stream\n";
	}
	::exit(-1);

}

// the item stream is going behave more like the fm stream execept the input will be of type item not fm 
void summarycalc::dogulitemsummary()
{
	loadsummaryxref(GULSUMMARYXREF_FILE);
	outputstreamtype();
	unsigned int streamtype = 0;
	int i = (int) fread(&streamtype, sizeof(streamtype), 1, stdin);
	if (i) {
		if (isFMStream(streamtype) == true) {
			unsigned int samplesize = 0;
			i =(int) fread(&samplesize, sizeof(samplesize), 1, stdin);
			if (i)	dosummaryprocessing(samplesize);
			else std::cerr << "FATAL: summarycalc: Read error on stream\n";
			return;
		}
		else {
			std::cerr << "FATAL: summarycalc: Not a fm stream\n";
			std::cerr << "FATAL: summarycalc: invalid stream type: " << streamtype << "\n";
		}
	}
	else {
		std::cerr << "FATAL: summarycalc: Read error on stream\n";
	}
	::exit(-1);

}
void summarycalc::doit()
{
	if (inputtype_ == UNKNOWN) {
		fprintf(stderr,"FATAL: summarycalc: stream type unknown\n");
		return;
	}
	
	if (inputtype_ == FM_STREAM) {
		dofmsummary();
	}
	
	if (inputtype_ == GUL_COVERAGE_STREAM) {
		dogulcoveragesummary();
	}
	if (inputtype_ == GUL_ITEM_STREAM) {
		dogulitemsummary();
	}
	if (inputtype_ == GUL_ITEMX_STREAM) {
		dogulitemxsummary();
	}
}
