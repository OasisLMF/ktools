/*
 * Copyright (c)2015 - 2018 Oasis LMF Limited
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

#include "../include/oasis.h"
#include <set>
#include <stdio.h>
#include <stdlib.h>

// Read all areaperil and vulnerability IDs into a set and check that the ones
// in items are valid

namespace validateitems {

    bool fileexists(const std::string &name) {
        if (FILE *file = fopen(name.c_str(), "rb")) {
            fclose(file);
            return true;
        } else {
            return false;
        }
    }
    void get_vulnerability_ids(std::set<int> &vulnerability_ids) {
        FILE *fin = fopen(VULNERABILITY_FILE, "rb");
        if (fin == nullptr) {
            fprintf(stderr, "%s: cannot open %s\n", __func__,
                    VULNERABILITY_FILE);
            exit(EXIT_FAILURE);
        }
        int damage_bins;
        fread(&damage_bins, sizeof(damage_bins), 1, fin); // drop the bins

        Vulnerability q;
        int i = fread(&q, sizeof(q), 1, fin);
        OASIS_FLOAT total = 0;
        while (i != 0) {
            vulnerability_ids.insert(q.vulnerability_id);
            i = fread(&q, sizeof(q), 1, fin);
        }

        fclose(fin);
    }

    void check_areaperil() {
        if (fileexists(ZFOOTPRINT_FILE)) {
            printf("TODO: Process zipped file");
        } else {
            printf("TODO: Process unzipped file");
        }
    }

	
    void check_vulnerability() {
        std::set<int> vulnerability_ids;
		get_vulnerability_ids(vulnerability_ids);
        item q;
        FILE *fin = fopen(ITEMS_FILE, "rb");
        int i = fread(&q, sizeof(q), 1, fin);
        while (i != 0) {
            if (vulnerability_ids.find(q.vulnerability_id) ==
                vulnerability_ids.end()) {
                fprintf(stderr,
                        "vulnerability_id %d in items not found in %s\n", q.vulnerability_id,
                        VULNERABILITY_FILE);
            }
            i = fread(&q, sizeof(q), 1, fin);
        }
        fclose(fin);
    }

    void doit() {
        check_areaperil();
        check_vulnerability();
    }
} // namespace validateitems
