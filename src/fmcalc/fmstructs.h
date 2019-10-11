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
#ifndef FMSTUCTS_H_
#define FMSTUCTS_H_


#include <vector>
#include <memory>

struct LossRec {
	OASIS_FLOAT loss = 0;
	OASIS_FLOAT retained_loss = 0;	// accumulated_deductible
	OASIS_FLOAT proportion = 0;
	OASIS_FLOAT gul_total = 0;	
	OASIS_FLOAT previous_layer_retained_loss = 0;
	OASIS_FLOAT effective_deductible = 0;
	OASIS_FLOAT accumulated_tiv = 0;	// accumulated tiv 
	OASIS_FLOAT over_limit = 0;
	OASIS_FLOAT under_limit = 0;
	int agg_id = 0;
	int policytc_id = 0;
	int next_vec_idx = -1;
	const std::vector<int> *item_idx = 0;
	std::shared_ptr<std::vector<OASIS_FLOAT>> item_prop;
	std::shared_ptr<std::vector<OASIS_FLOAT>> item_net;
};

struct policytcvidx {
	int policytc_id = 0;
	int agg_id = 0;
	int next_vidx = -1;
	std::vector<int> item_idx;
};

struct tc_rec {
	OASIS_FLOAT tc_val;
	unsigned char tc_id;
};

struct profile_rec_new {
	int calcrule_id;	
	std::vector<tc_rec> tc_vec;

};

#endif  // FMSTUCTS_H_
