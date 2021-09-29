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
#ifndef OASIS_H_
#define OASIS_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <stdarg.h>


#include "../../config.h"
#ifdef __unix 
#include <unistd.h>
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <process.h>
#include <fcntl.h>
#include <io.h>
#define flseek _fseeki64
#define fltell _ftelli64
#define GETPID _getpid
#else
#define flseek fseek
#define fltell ftell
#define GETPID getpid
#endif 

// Slowly where applicable we will replace int and OASIS_FLOAT references with
// OASIS_INT and OASIS_FLOAT so its easy to change the size of the streams 
typedef int32_t OASIS_INT ;

// #define OASIS_FLOAT_TYPE_DOUBLE
#ifdef OASIS_FLOAT_TYPE_DOUBLE
typedef double OASIS_FLOAT;
#else
typedef float OASIS_FLOAT;
#endif

// #define AREAPERIL_TYPE_UNSIGNED_LONG_LONG
#ifdef AREAPERIL_TYPE_UNSIGNED_LONG_LONG
typedef unsigned long long AREAPERIL_INT;
#else
typedef unsigned int AREAPERIL_INT;
#endif


const int mean_idx = -1;
const int std_dev_idx = -2;
const int tiv_idx = -3;

// Stream types
const unsigned int cdfstream_id = 0;		// high byte is zero
const unsigned int gulstream_id = 1 << 24;
const unsigned int fmstream_id = 2 << 24;
const unsigned int summarycalc_id = 3 << 24;
//const unsigned int aacalc_id = 4 << 24;
const unsigned int loss_stream_id = 2 << 24;		// This stream (loss stream)  will eventaully be used for both fm and gul. gulstream_id and fmstream_id will be deprecated in the future
													// the streamid is the same as fmstream_id this is because its stucture is identical eventually we will remove the name fmstream_id

const unsigned int gul_item_stream = 1;
const unsigned int gul_coverage_stream = 2;
const unsigned int streamno_mask = 0x00FFFFFF;

#pragma pack(push, 1)
struct damagecdfrec {
        int event_id;
		AREAPERIL_INT areaperil_id;
        int vulnerability_id;
};

struct damagebindictionary {
        int bin_index;
        OASIS_FLOAT bin_from;
        OASIS_FLOAT bin_to;
        OASIS_FLOAT interpolation;
        int interval_type;
};

struct gulitemSampleslevel {
	int event_id;
	int item_id;
	int sidx;		// This has be stored for thresholds cannot be implied
	OASIS_FLOAT loss;		// may want to cut down to singe this causes 4 byte padding for allignment 
};
struct gulcoverageSampleslevel {
	int event_id;
	int coverage_id;
	int sidx;		// This has be stored for thresholds cannot be implied
	OASIS_FLOAT loss;		// may want to cut down to singe this causes 4 byte padding for allignment
};

struct gulSampleslevelEventRec {
	int item_id;
	int sidx;		// This has be stored for thresholds cannot be implied
	OASIS_FLOAT loss;		// may want to cut down to singe this causes 4 byte padding for allignment
};
struct gulSampleslevelHeader {
	int event_id;
	int item_id;
};

struct summarySampleslevelHeader {
	int event_id;
	int summary_id;
	OASIS_FLOAT expval;				// exposure value
};

struct summary_keyz {
	int summary_id;
	int fileindex;
};

struct summary_period {
	int summary_id;
	int fileindex;
	int period_no;
};

struct gulSampleslevelRec {
	int sidx;		// This has be stored for thresholds cannot be implied
	OASIS_FLOAT loss;		// may want to cut down to singe this causes 4 byte padding for allignment
};

struct sampleslevelRec {
	int sidx;		
	OASIS_FLOAT loss;			
};


struct fmlevelhdr {
	int event_id;
	int output_id;
};

struct fmlevelrec {
	int sidx;
	OASIS_FLOAT loss;
};

struct exposure{
	int item_id;
	AREAPERIL_INT areaperil_id;
	int vulnerability_id;
	int group_id;
	OASIS_FLOAT tiv;
};

struct item {
  int id;
  int coverage_id;
  AREAPERIL_INT areaperil_id;
  int vulnerability_id;
  int group_id;
};

struct coverage {
	int id;
	OASIS_FLOAT tiv;
};

struct occurrence {
	int event_id;	// this can occur multiple times - i.e duplicate key
	int period_no;		// for abstract period definitions
	int occ_date_id;		// occurrence year, month and day	
};

struct occurrence_granular {
	int event_id;
	int period_no;
	long long occ_date_id;   // occurrence year, month, day, hour and minute
};

struct gulsummaryxref {
	int coverage_id;
	int summary_id;
	int summaryset_id;
};

struct fmsummaryxref {
	int output_id;
	int summary_id;
	int summaryset_id;
};

struct fm_policyTC {
        int level_id;
        int agg_id;
        int layer_id;
        int profile_id;                 // for some strange reason historically this has been named PolicyTC_id it really is the unique identifier of the profile table so I've renamed it!!
};

struct fm_profile {
	int profile_id = 0;
	int calcrule_id = 0;
	OASIS_FLOAT deductible1 = 0;
	OASIS_FLOAT	deductible2 = 0;
	OASIS_FLOAT deductible3 = 0;
	OASIS_FLOAT attachment = 0;
	OASIS_FLOAT limit = 0;
	OASIS_FLOAT share1 = 0;
	OASIS_FLOAT share2 = 0;
	OASIS_FLOAT	share3 = 0;

};

struct fm_profile_step {
	int profile_id = 0;
	int calcrule_id = 0;
	OASIS_FLOAT deductible1 = 0;
	OASIS_FLOAT	deductible2 = 0;
	OASIS_FLOAT deductible3 = 0;
	OASIS_FLOAT attachment = 0;
	OASIS_FLOAT limit1 = 0;
	OASIS_FLOAT share1 = 0;
	OASIS_FLOAT share2 = 0;
	OASIS_FLOAT	share3 = 0;
	int step_id = 0;
	OASIS_FLOAT trigger_start = 0;
	OASIS_FLOAT	trigger_end = 0;
	OASIS_FLOAT payout_start = 0;
	OASIS_FLOAT payout_end = 0;
	OASIS_FLOAT limit2 = 0;
	OASIS_FLOAT scale1 = 0;
	OASIS_FLOAT scale2 = 0;
};

struct fm_programme {
	int from_agg_id;
	int level_id;
	int to_agg_id;
};

struct fmXref {
	int output_id;
	int agg_id;
	int layer_id;
};

//struct fmxrefxx
//{
//	int item_id;
//	int output_id;
//};


struct summary_id_type_key {
	int summary_id;
	int type;
};

struct aal_rec {
	int summary_id;
	int type;
	double mean;
	double mean_squared;
	double max_exposure_value;
};

struct aal_rec_ensemble : aal_rec {
	int ensemble_id;
};

struct Vulnerability
{
	int vulnerability_id;
	int intensity_bin_id;
	int damage_bin_id;
	OASIS_FLOAT probability;
};

struct VulnerabilityRow
{
	int intensity_bin_id;
	int damage_bin_id;
	OASIS_FLOAT probability;
};

struct VulnerabilityIndex
{
	int vulnerability_id;
	long long offset;
	long long size;
	long long original_size;
};

struct Periods
{
	int period_no;
	double weighting;
};
struct EventRow
{
	AREAPERIL_INT areaperil_id;
	int intensity_bin_id;
	OASIS_FLOAT probability;
};

struct EventIndex
{
	int event_id;
	long long offset;
	long long size;
};

struct Ensemble
{
	int sidx;
	int ensemble_id;
};

#pragma pack(pop)


// -- UTILITY inline functions 


inline void initstreams(std::string inFile="", std::string outFile="")
{

   if (outFile.length() > 0){
       if (freopen(outFile.c_str(), "wb", stdout) == NULL) {
		   fprintf(stderr, "%s: Error opening file %s\n", __func__, outFile.c_str());
           exit(-1);
        }
   }

   if (inFile.length() > 0){
        if (freopen(inFile.c_str(), "rb", stdin) == NULL) {
			fprintf(stderr, "%s: Error opening file %s\n", __func__, inFile.c_str());
            exit(-1);
         }
   }

#if defined(_MSC_VER) || defined(__MINGW32__)
	_setmode(_fileno(stdout), O_BINARY);
	_setmode(_fileno(stdin), O_BINARY);
#elif __APPLE__
	freopen(NULL, "wb", stdout);
	freopen(NULL, "rb", stdin);
#elif __linux
	freopen(NULL, "a+b", stdout);
	freopen(NULL, "rb", stdin);
#endif

}

template<class T>
T base_name(T const& path, T const& delims = "/\\")
{
	return path.substr(path.find_last_of(delims) + 1);
}
template<class T>
T remove_extension(T const& filename)
{
	typename T::size_type const p(filename.find_last_of('.'));
	return p > 0 && p != T::npos ? filename.substr(0, p) : filename;
}

inline  void logprintf(const std::string &program_name,const std::string &msgtype,const char* format, ...)
{
	
	std::string base = base_name(program_name);
	base = remove_extension(base_name(base));
	std::string b2 = base + ":" + std::to_string(GETPID());
	base = "log/" + base + "_" + std::to_string(GETPID()) + ".log";

	FILE* fout = fopen(base.c_str(), "a");
	if (fout != nullptr) {
		fprintf(fout, "%s:%s:",msgtype.c_str(),b2.c_str());
		va_list args;
		va_start(args, format);
		vfprintf(fout, format, args);
		va_end(args);
		fclose(fout);
	}
	
}

#define PIPE_DELAY	1000

// Relative paths for file names
#define COVERAGES_FILE "input/coverages.bin"
#define GULSUMMARYXREF_FILE "input/gulsummaryxref.bin"
#define FMSUMMARYXREF_FILE "input/fmsummaryxref.bin"
#define FMPOLICYTC_FILE "input/fm_policytc.bin"
#define FMPROGRAMME_FILE "input/fm_programme.bin"
#define FMPROFILE_FILE_STEP "input/fm_profile_step.bin"
#define FMPROFILE_FILE "input/fm_profile.bin"
#define FMXREF_FILE "input/fm_xref.bin"
#define FMXREF_FILE_A0 "input/fm_xref_a0.bin"
#define ITEMS_FILE "input/items.bin"
#define EVENTS_FILE "input/events.bin"
#define RETURNPERIODS_FILE "input/returnperiods.bin"
#define OCCURRENCE_FILE "input/occurrence.bin"
#define PERIODS_FILE "input/periods.bin"		// period to weighting mapping
#define ENSEMBLE_FILE "input/ensemble.bin"

#define DAMAGE_BIN_DICT_FILE "static/damage_bin_dict.bin"
#define DAMAGE_CDF_BIN_FILE "static/damage_cdf.bin"
#define DAMAGE_CDF_IDX_FILE "static/damage_cdf.idx"
#define RANDOM_FILE "static/random.bin"
#define VULNERABILITY_FILE  "static/vulnerability.bin"
#define VULNERABILITY_IDX_FILE "static/vulnerability.idx"
// compressed variant of vulnerabilities
#define ZVULNERABILITY_FILE "static/vulnerability.bin.z"
#define ZVULNERABILITY_IDX_FILE "static/vulnerability.idx.z"
#define FOOTPRINT_FILE  "static/footprint.bin"
#define FOOTPRINT_IDX_FILE  "static/footprint.idx"
// compressed variant of footprint
#define ZFOOTPRINT_FILE  "static/footprint.bin.z"	
#define ZFOOTPRINT_IDX_FILE  "static/footprint.idx.z"

#define SEMA_DIR_PREFIX "work/sema"
#endif  // OASIS_H_
