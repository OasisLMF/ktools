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

#include "../include/oasis.h"

struct fm_profile_old {
	int policytc_id;
	int calcrule_id;
	int allocrule_id;
	int ccy_id;
	OASIS_FLOAT deductible;
	OASIS_FLOAT limits;
	OASIS_FLOAT share_prop_of_lim;
	OASIS_FLOAT deductible_prop_of_loss;
	OASIS_FLOAT limit_prop_of_loss;
	OASIS_FLOAT deductible_prop_of_tiv;
	OASIS_FLOAT limit_prop_of_tiv;
	OASIS_FLOAT deductible_prop_of_limit;
};

namespace fmprofilebinconv {
    void doit() {

        fm_profile_old q;
        size_t i = fread(&q, sizeof(q), 1, stdin);
        while (i != 0) {
            fm_profile f;
            f.profile_id = q.policytc_id;
            f.calcrule_id = q.calcrule_id;
            switch (f.calcrule_id) {
            case 1:
            case 3: {
                f.deductible1 = q.deductible;
                f.limit = q.limits;
            } break;
            case 2: {
                f.deductible1 = q.deductible;
                f.limit = q.limits;
                f.share1 = q.share_prop_of_lim;
            } break;
            case 5: {
                f.deductible1 = q.deductible_prop_of_loss;
                f.limit = q.limit_prop_of_loss;
            } break;
            case 9: {
                f.deductible1 = q.deductible_prop_of_limit;
                f.limit = q.limits;
            } break;
            case 10: {
                f.deductible3 = q.deductible;
            } break;
            case 11: {
                f.deductible2 = q.deductible;
            } break;
            case 12: {
                f.deductible1 = q.deductible;
            } break;
            case 14: {
                f.limit = q.limits;
            } break;
            case 15: {
                f.limit = q.limit_prop_of_loss;
            } break;
            case 16: {
                f.deductible1 = q.deductible_prop_of_loss;
            } break;
            default:
                fprintf(stderr, "Unknown calcrule_id %d\n", q.calcrule_id);
            }

            f.attachment = 0;

            fwrite(&f, sizeof(f), 1, stdout);
            i = fread(&q, sizeof(q), 1, stdin);
        }
    }
} // namespace fmprofilebinconv
