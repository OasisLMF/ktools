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
struct Attributes;

bool operator< (const item &a, const item &i);

bool operator< (const Attributes &id1, const Attributes &id2);
bool operator> (const Attributes &id1, const Attributes &id2);
bool operator== (const Attributes &id1, const Attributes &id2);

class getmodel {

public:

    getmodel();
    ~getmodel();
    void init(bool zip, bool disaggregation);
	void doCdf(int event_id);


private:
	
	std::map<int, std::vector<OASIS_FLOAT> > _vulnerabilities;	
    std::map<AREAPERIL_INT, std::set<int> > _vulnerability_ids_by_area_peril;
    std::map<int, EventIndex> _event_index_by_event_id;
	std::map<AREAPERIL_INT, std::set<AREAPERIL_INT>> _aggregate_areaperils;
	std::map<int, std::set<int>> _aggregate_vulnerabilities;
	std::map <AREAPERIL_INT, std::map<int, std::vector<OASIS_FLOAT>>> _aggregate_vulnerability_probs;

	std::set<AREAPERIL_INT> _area_perils;
	std::set<int> _vulnerability_ids;
	std::set<int> _aggregate_vulnerability_ids;
	std::set<AREAPERIL_INT> _aggregate_areaperil_ids;
	
	std::set<int> _group_ids = { 0 };
	std::set<int> _item_ids = { 0 };

	std::vector<OASIS_FLOAT> _mean_damage_bins;
    std::vector<unsigned char > _compressed_buf;
    std::vector<unsigned char > _uncompressed_buf;
	std::vector<Weight> _weights;

	std::vector<Result> _temp_results;
	//Result* _temp_results;
    
	int _num_intensity_bins = -1;
    int _num_damage_bins = -1;

	//aggregate ids larger than non aggregate ids, this is the first id that is aggregate
	int _agg_vul_start = -1;
	int _agg_ap_start = -1;

	bool _disaggregation = false;
	bool _has_intensity_uncertainty = false;
    bool _zip = false;

	void getAggregateItems(std::vector<aggregate_item> &aggregate_items);
	void getAggregateAreaPerils(std::set<AREAPERIL_INT> &disagg_area_perils);
	void getAggregateVulnerabilities(std::set<AREAPERIL_INT> &disagg_vulnerabilities);
	void getDisaggregationWeights(const std::set<AREAPERIL_INT> &disagg_area_perils, const std::set<AREAPERIL_INT> &disagg_vulnerabilities);
	void getCoverages(std::vector<OASIS_FLOAT> &coverages);
	void expandItems(aggregate_item a, std::vector<OASIS_FLOAT> &coverages, std::vector<item> &expanded_items);
	void outputNewCoverages(std::vector<OASIS_FLOAT> &coverages);

	void newItems();
	void getItems();

    void getVulnerabilities();
    void getDamageBinDictionary();
    void getFootPrints();
	void getIntensityInfo();

	void calcProbAggVul(AREAPERIL_INT areaperil_id, int vulnerability_id, std::map<int, OASIS_FLOAT> &vulnerability_probability);
	void calcProbAggAp(int vulnerability_id, AREAPERIL_INT areaperil_id, std::map<AREAPERIL_INT, OASIS_FLOAT> &area_peril_probability);
	void calcProbAgg(AREAPERIL_INT areaperil_id, int vulnerability_id,  std::map<Attributes, OASIS_FLOAT> &probabilities);

	void getIntensityProbs(int event_id,
		std::map<AREAPERIL_INT, std::vector<OASIS_FLOAT>> &areaperil_intensity,
		std::set<AREAPERIL_INT> &areaperils);
	void newIntensity(int event_id, AREAPERIL_INT aggregate_areaperil_id, std::vector<OASIS_FLOAT> &new_Intensity, int vulnerability_id);
	void newVulnerabilities();

    void doCdfInner(int event_id);
	void doCdfAggregate(int event_id);

    void doCdfInnerNoIntensityUncertainty(int event_id);
    void doCdfInnerNoIntensityUncertaintyz(int event_id);

    void  doResults(
		int &event_id, AREAPERIL_INT areaperil_id,
		std::map<AREAPERIL_INT, std::set<int>> &vulnerability_ids_by_area_peril);
	void doResultsAggregate(
		int &event_id,
		AREAPERIL_INT aggregate_areaperil_id,
		const std::set<int> &aggregate_vulnerability_ids,
		std::map<Attributes, std::vector<double>> &results_map);
	void  doResultsNoIntensityUncertainty(
		int &event_id,
		AREAPERIL_INT &areaperil_id,
		int intensity_bin_index) ;

	void getDisaggregateCdfs(int event_id,
		std::map<Attributes, std::vector<double>> &results_map,
		const std::map<AREAPERIL_INT, std::set<int>> &disaggregated_vul_by_ap);
	
	static void initOutputStream();
    int getVulnerabilityIndex(int intensity_bin_index, int damage_bin_index) const;
};

#endif // GETMODEL_H_
