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

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

namespace fmprofiletobin {
	void dostep() {
		fm_profile_step q;
		char line[4096];
		int lineno = 0;
		fgets(line, sizeof(line), stdin);	// skip header
		lineno++;
		while (fgets(line, sizeof(line), stdin) != 0) {
#ifdef OASIS_FLOAT_TYPE_DOUBLE
			if (sscanf(line, "%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
				   &q.profile_id, &q.calcrule_id, &q.deductible1, &q.deductible2,
				   &q.deductible3, &q.attachment, &q.limit1, &q.share1, &q.share2,
				   &q.share3, &q.step_id, &q.trigger_start, &q.trigger_end,
				   &q.payout_start, &q.payout_end, &q.limit2, &q.scale1,
				   &q.scale2) != 18)
#else
			if (sscanf(line, "%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%d,%f,%f,%f,%f,%f,%f,%f",
				   &q.profile_id, &q.calcrule_id, &q.deductible1, &q.deductible2,
				   &q.deductible3, &q.attachment, &q.limit1, &q.share1, &q.share2,
				   &q.share3, &q.step_id, &q.trigger_start, &q.trigger_end,
				   &q.payout_start, &q.payout_end, &q.limit2, &q.scale1,
				   &q.scale2) != 18)
#endif
			{
				fprintf(stderr, "FATAL: Invalid data in line %d:\n%s", lineno, line);
				return;
			}
			else {
				fwrite(&q, sizeof(q), 1, stdout);
			}
			lineno++;
		}
	}
    void doit(bool step) {
		if (step) {
			dostep();
			return;
		}
        fm_profile q;
        char line[4096];
        int lineno = 0;
        fgets(line, sizeof(line), stdin);  // skip header
        lineno++;
        while (fgets(line, sizeof(line), stdin) != 0) {
#ifdef OASIS_FLOAT_TYPE_DOUBLE
            if (sscanf(line, "%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", &q.profile_id,
                       &q.calcrule_id, &q.deductible1, &q.deductible2,
                       &q.deductible3, &q.attachment, &q.limit, &q.share1,
                       &q.share2, &q.share3) != 10) {
#else
            if (sscanf(line, "%d,%d,%f,%f,%f,%f,%f,%f,%f,%f", &q.profile_id,
                       &q.calcrule_id, &q.deductible1, &q.deductible2,
                       &q.deductible3, &q.attachment, &q.limit, &q.share1,
                       &q.share2, &q.share3) != 10) {
#endif
                fprintf(stderr, "FATAL: Invalid data in line %d:\n%s", lineno, line);
                return;
            } else {
                fwrite(&q, sizeof(q), 1, stdout);
            }
            lineno++;
        }
    }

} // namespace fmprofiletobin
