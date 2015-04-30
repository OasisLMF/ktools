/*
* Copyright (c)2015 Oasis LMF Limited 
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

#pragma once
const int mean_idx = 1 << 24;

// Stream types
const unsigned int cdfstream_id = 0;		// high byte is zero
const unsigned int gulstream_id = 1 << 24;
const unsigned int fmstream_id = 2 << 24;
const unsigned int outputstream_id = 3 << 24;

const unsigned int streamno_mask = 0x00FFFFFF;

#pragma pack(push, 1)
struct damagecdfrec {
        int event_id;
        int areaperil_id;
        int vulnerabilty_id;
};

struct damagebindictionary {
        int bin_index;
        float bin_from;
        float bin_to;
        float interpolation;
        int interval_type;
};

struct gulSampleslevel {
	int event_id;
	int item_id;
	int sidx;		// This has be stored for thresholds cannot be implied
	float gul;		// may want to cut down to singe this causes 4 byte padding for allignment
};

struct gulSampleslevelHeader {
	int event_id;
	int item_id;
};

struct gulSampleslevelRec {
	int sidx;		// This has be stored for thresholds cannot be implied
	float gul;		// may want to cut down to singe this causes 4 byte padding for allignment
};

struct fmlevelhdr {
	int event_id;
	int prog_id;
	int layer_id;
	int output_id;
};

struct fmlevelrec {
	int sidx;
	float loss;
};

struct fmdata {
	int item_id;
	int agg_id;
	int prog_id;
	int level_id;
	int policytc_id;
	int layer_id;
	int calcrule_id;
	int allocrule_id;
	float deductible;
	float limits;
	float share_prop_of_lim;
	float deductible_prop_of_loss;
	float limit_prop_of_loss;
	float deductible_prop_of_tiv;
	float limit_prop_of_tiv;
	float deductible_prop_of_limit;
};

#pragma pack(pop)

struct exposure{
	int item_id;
	int areaperil_id;
	int vulnerability_id;
	int group_id;
	float tiv;
};


// -- UTILITY inline functions 
inline int kfseek ( FILE * stream, long int offset, int origin )
{
	return fseek(stream,offset, origin);
}

