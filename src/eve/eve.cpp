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

    void randomiseevents(OASIS_INT pno_, OASIS_INT total_,
			 std::vector<int> &events, bool textmode) {

	unsigned seed = 1234;
	std::default_random_engine e(seed);

	// As we only consider swapping each element once as we go through the
	// vector sequentially, we can output events IDs as we proceed. We are
	// only interested in a particular process ID, so only output events IDs
	// assigned to that process
	auto eventsSize = events.size();
	int counter = 1;
	for (std::vector<int>::iterator it = events.begin(); it != events.end();
	     it++) {

	    // Implement Fisher-Yates shuffle
	    std::uniform_int_distribution<> d(0, --eventsSize);
	    std::iter_swap(it, it + d(e));

	    if ((counter - pno_) % total_ == 0) {

		if (textmode) fprintf(stdout, "%d\n", *it);
		else fwrite(&(*it), sizeof(*it), 1, stdout);

	    }
	    counter++;

	}

    }

    void randomiseeventslegacy(std::vector<int> &events) {

	unsigned seed = 1234;
	std::default_random_engine e(seed);
	std::shuffle(events.begin(), events.end(), e);

    }

    void doshuffle(OASIS_INT pno_, OASIS_INT total_, std::vector<int> &events,
		   bool textmode) {

	// As we go through event IDs in vector, assign them to processes in the
	// same fashion as dealing a deck of cards. We are only interested in a
	// particular process ID, so only output event IDs dealt to that process
	int counter = 1;
	for (std::vector<int>::iterator it = events.begin(); it != events.end();
	     it++) {

	    if ((counter - pno_) % total_ == 0) {

		if (textmode) fprintf(stdout, "%d\n", *it);
		else fwrite(&(*it), sizeof(*it), 1, stdout);

	    }
	    counter++;

	}

    }

    void emitevents(OASIS_INT pno_, OASIS_INT total_, std::vector<int> &events,
                    bool textmode) {

	int total_events = (int)events.size();
	int min_chunksize = total_events / total_;
	int first_min_pno = (total_events % total_) + 1;
	int start_pos = (min_chunksize + 1) * (std::min(first_min_pno, pno_) - 1);
	start_pos += min_chunksize * std::max(pno_ - first_min_pno, 0);
	int end_pos = (min_chunksize + 1) * (std::min(first_min_pno, pno_+1) - 1);
	end_pos += min_chunksize * std::max(pno_+1 - first_min_pno, 0);

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
		    bool randomise, bool randomiselegacy, bool textmode) {

	std::vector<int> events;
	readevents(events);
	if (!shuffle) {   // No shuffling
	    emitevents(pno_, total_, events, textmode);

	} else {

	    if (randomise) {   // Implement Fisher-Yates shuffle
		randomiseevents(pno_, total_, events, textmode);

	    } else if (randomiselegacy) {   // Implement std::shuffle
		randomiseeventslegacy(events);
		emitevents(pno_, total_, events, textmode);

	    } else {   // Implement deterministic approach
		doshuffle(pno_, total_, events, textmode);

	    }
	}

    }

} // namespace eve
