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
Convert fmprofile output to csv
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#include "../include/oasis.h"

using namespace std;

namespace fmprofiletocsv {
	void dostep(bool skipheader) {
		if (skipheader == false) {
            printf("profile_id,calcrule_id,deductible1,deductible2,");
            printf("deductible3,attachment1,limit1,share1,share2,");
            printf("share3,step_id,calcrule_id,trigger_start,trigger_end,");
			printf("payout_start,payout_end,limit2,scale1,");
			printf("scale2\n");
		}

		fm_profile_step q;
		size_t i = fread(&q, sizeof(q), 1, stdin);
		while (i != 0) {
			printf("%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%d,%f,%f,%f,%f,%f,%f,%f\n", q.profile_id,
                q.calcrule_id, q.deductible1, q.deductible2, q.deductible3,
                q.attachment, q.limit1, q.share1, q.share2, q.share3,q.step_id,
				q.trigger_start, q.trigger_end, q.payout_start,
				q.payout_end, q.limit2, q.scale1, q.scale2);

			i = fread(&q, sizeof(q), 1, stdin);
		}
	}
    void doit(bool skipheader, bool step) {
		if (step) {
			dostep(skipheader);
			return;
		}
        if (skipheader == false) {
            printf("profile_id,calcrule_id,deductible1,deductible2,");
            printf("deductible3,attachment1,limit1,share1,share2,");
            printf("share3\n");
        }

        fm_profile q;
        size_t i = fread(&q, sizeof(q), 1, stdin);
        while (i != 0) {
            printf("%d, %d, %f, %f, %f, %f, %f, %f, %f, %f\n", q.profile_id,
                   q.calcrule_id, q.deductible1, q.deductible2, q.deductible3,
                   q.attachment, q.limit, q.share1, q.share2, q.share3);

            i = fread(&q, sizeof(q), 1, stdin);
        }
    }
} // namespace fmprofiletocsv
