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
eve: Event emitter for partioning work between multiple processes
Author: Ben Matharu  email: ben.matharu@oasislmf.org

*/

#include "../include/oasis.h"
#include <math.h>

#include <algorithm> // std::shuffle
#include <iostream>  // std::cout
#include <random>    // std::default_random_engine
#include <vector>    // std::vector

extern char *progname;
namespace eve {

    void readevents(std::vector<int> &events) {
        FILE *fin = fopen(EVENTS_FILE, "rb");
        if (fin == NULL) {
            fprintf(stderr, "FATAL: %s: %s: cannot open %s\n", progname, __func__,
                    EVENTS_FILE);
            exit(EXIT_FAILURE);
        }
        int eventid;
        while (fread(&eventid, sizeof(eventid), 1, fin) == 1) {
            events.push_back(eventid);
        }
        fclose(fin);
    }
    void doshuffle(std::vector<int> &events) {
        unsigned seed = 1234;
        std::default_random_engine e(seed);
        std::shuffle(events.begin(), events.end(), e);
    }

    void emitevents(OASIS_INT pno_, OASIS_INT total_, std::vector<int> &events,
                    bool textmode) {
        size_t total_events = events.size();
        int chunksize = (int)ceil((OASIS_FLOAT)total_events / total_);
        size_t end_pos = chunksize * pno_;
        pno_--;
        int start_pos = chunksize * pno_;
        if (end_pos > (int) events.size())
            end_pos = events.size();

        while (start_pos < end_pos) {
            int eventid = events[start_pos];
            if (textmode == true)
                fprintf(stdout, "%d\n", eventid);
            else
                fwrite(&eventid, sizeof(eventid), 1, stdout);
            start_pos++;
        }
    }
    void emitevents(OASIS_INT pno_, OASIS_INT total_, bool shuffle,
                    bool textmode) {

        std::vector<int> events;
        readevents(events);
        if (shuffle == true) {
            doshuffle(events);
        }
        emitevents(pno_, total_, events, textmode);
        return;
    }
} // namespace eve
