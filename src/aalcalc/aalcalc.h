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


#ifndef AALCALC_H_
#define AALCALC_H_

#include "../include/oasis.h"

#include <map>
#include <vector>
#include <math.h>

class aalcalc {
private:
	std::map<int, int> event_count_;	// count of events in occurrence table used to create cartesian effect on event_id
	std::map<int, int> event_to_period_;	// Mapping of event to period no
	int no_of_periods_ = 0;
	int samplesize_ = 0;
	std::map<int, aal_rec> map_analytical_aal_;
	std::map<int, aal_rec> map_sample_aal_;
	std::map <int, OASIS_FLOAT> periodstoweighting_;	
	// functions
	void loadperiodtoweigthing();
	void loadoccurrence();
	void outputresultscsv();
	void outputsummarybin();
	void do_analytical_calc(const summarySampleslevelHeader &sh, OASIS_FLOAT mean_loss);
	void do_sample_calc(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec);
	void doaalcalc(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec, OASIS_FLOAT mean_loss);
	void applyweightingstomap(std::map<int, aal_rec> &m, int i);
	void applyweightingstomaps();
	void applyweightings(int event_id, const std::map <int, OASIS_FLOAT> &periodstoweighting, std::vector<sampleslevelRec> &vrec);
public:
	void doit();
};

#endif  // AALCALC_H_
