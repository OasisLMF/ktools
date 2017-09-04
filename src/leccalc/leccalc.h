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

#ifndef LECCALC_H_
#define LECCALC_H_

#include <map>
#include "../include/oasis.h"

enum { AGG_FULL_UNCERTAINTY = 0, AGG_WHEATSHEAF, AGG_SAMPLE_MEAN, AGG_WHEATSHEAF_MEAN, OCC_FULL_UNCERTAINTY, OCC_WHEATSHEAF, OCC_SAMPLE_MEAN, OCC_WHEATSHEAF_MEAN };


struct outkey2 {
	int summary_id;
	int period_no;
	int sidx;
};

struct wheatkey {
	int summary_id;
	int sidx;
};

struct summary_id_period_key {
	int summary_id;
	int period_no;
	int type;
};

struct lossval {
	int period_no;	// for debugging
	double period_weighting;
	OASIS_FLOAT value;
};
typedef  std::vector<OASIS_FLOAT> lossvec;

typedef  std::vector<lossval> lossvec2;

//extern FILE *fout[];

#endif
