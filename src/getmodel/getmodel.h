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

Author: Mark Pinkerton  email: mark.pinkerton@oasislmf.org

*/

#ifndef GETMODEL_H_
#define GETMODEL_H_

#include <map>
#include <set>
#include <list>
#include <vector>
#include "../include/oasis.h"

struct Result;
struct EventIndex;

class getmodel {

public:

    explicit getmodel(bool zip = false);
    ~getmodel();
    void init();
	void doCdf(std::list<int> event_ids);

private:

	std::map<int, std::vector<OASIS_FLOAT> > _vulnerabilities;
	std::map<int, std::set<int> > _vulnerability_ids_by_area_peril;
	std::set<int> _area_perils;
	std::vector<OASIS_FLOAT> _mean_damage_bins;
    std::vector<unsigned char > _compressed_buf;
    std::vector<unsigned char > _uncompressed_buf;

    int _num_intensity_bins = -1;
    int _num_damage_bins = -1;
    int _has_intensity_uncertainty = false;
    Result* _temp_results;
    bool _zip = false;
    void getVulnerabilities(const std::set<int> &v);
    void getVulnerabilities();
    void getDamageBinDictionary();
    auto getItems() -> std::unique_ptr<std::set<int>>;
	void getIntensityInfo();
    void doCdfInner(std::list<int> &event_ids, std::map<int, EventIndex> &event_index_by_event_id);
    void doCdfInnerNoIntensityUncertainty(std::list<int> &event_ids, std::map<int, EventIndex> &event_index_by_event_id);
    void doCdfInnerz(std::list<int> &event_ids, std::map<int, EventIndex> &event_index_by_event_id);
    void doCdfInnerNoIntensityUncertaintyz(std::list<int> &event_ids,
    std::map<int, EventIndex> &event_index_by_event_id);
    void  doResults(
        int &event_id,
        int &areaperil_id,
        std::map<int, std::set<int> > &vulnerabilities_by_area_peril,
        std::map<int, std::vector<OASIS_FLOAT> > &vulnerabilities,
        std::vector<OASIS_FLOAT> intensity) const;

	void  doResultsNoIntensityUncertainty(
		int &event_id,
		int &areaperil_id,
		std::map<int, std::set<int> > &vulnerabilities_by_area_peril,
		std::map<int, std::vector<OASIS_FLOAT> > &vulnerabilities,
		int intensity_bin_index) const;

	static void initOutputStream();
    int getVulnerabilityIndex(int intensity_bin_index, int damage_bin_index) const;
};

#endif // GETMODEL_H_
