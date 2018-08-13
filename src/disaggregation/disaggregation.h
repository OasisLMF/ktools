#ifndef DISAGG_H_
#define DISAGG_H_

#include <map>
#include <set>
#include <list>
#include <vector>
#include <zlib.h>
#include "../include/oasis.h"


std::ostream& operator<< (std::ostream &out, const aggregate_item &agg_item);
std::ostream& operator<< (std::ostream &out, const item &item);



bool operator< (const aggregate_item &a, const aggregate_item &i);
bool operator< (const item &a, const item &i);

struct Weight {
	AREAPERIL_INT areaperil;
	int vulnerability;
	OASIS_FLOAT weight;

	Weight(AREAPERIL_INT ap, int vul, OASIS_FLOAT w) {
		areaperil = ap;
		vulnerability = vul;
		weight = w;
	}
};
struct AggregateID {
	AREAPERIL_INT aggregate_areaperil_id;
	int aggregate_vulnerability_id;

	AggregateID(AREAPERIL_INT agg_ap, int agg_vul) {
		aggregate_areaperil_id = agg_ap;
		aggregate_vulnerability_id = agg_vul;
	}
};

std::ostream& operator<< (std::ostream &out, const Weight &weight);

class disaggregation {
public:
	disaggregation();
	~disaggregation();
	void outputNewCoverages();
	void newItems();
	void test();

private:

	std::map<AREAPERIL_INT, std::vector<OASIS_FLOAT>> _aggregate_areaperils;
	std::map<int, std::map<AREAPERIL_INT, std::map<int, OASIS_FLOAT>>> _aggregate_vulnerabilities;
	std::map<int, std::map<int, std::vector<OASIS_FLOAT>>> _vulnerabilities;
	std::map<int, EventIndex> _event_index_by_event_id;
	
	
	std::vector<aggregate_item> _aggregate_items;
	std::vector<aggregate_item> _expanded_aggregate_items;
	std::vector<OASIS_FLOAT> _coverages;
	std::set<int> _vulnerability_ids;
	std::set<int> _aggregate_vulnerability_ids;
	std::set<AREAPERIL_INT> _aggregate_areaperil_ids;
	std::set<int> _group_ids = { 0 };
	std::set<int> _item_ids = { 0 };

	long _num_areaperils = -1;
	int _num_items = -1;
	int _num_intensity_bins = -1;
	int _num_damage_bins = -1;

	bool _has_disagg_uncertainty = false;
	bool _has_intensity_uncertainty = false;
	bool _zip = false;

	void getAggregateItems();
	void getAggregateAreaPerils();
	void getAggregateVulnerabilities();
	void getCoverages();
	
	void mergeAggregateVulnerability(int aggregate_vulnerability_id, std::map<int, OASIS_FLOAT> &vulnerability_probability);
	

	void assignNewCoverageID(aggregate_item &a);
	void expandGrouped(aggregate_item &a);
	void expandNotGrouped(aggregate_item &a);
	
	void getIntensityInfo();
	void getVulnerabilities();
	void getFootprints();
	void getIntensityProbs(int event_id, std::map<AREAPERIL_INT, std::vector<OASIS_FLOAT>> &areaperil_intensity);
	void newFootprint(AREAPERIL_INT aggregate_areaperil_id, std::map<AREAPERIL_INT, std::vector<OASIS_FLOAT>> areaperil_intensity,
		std::vector<EventRow> &new_Footprint);
	void newVulnerability(int aggregate_vulnerability_id, std::map<int, std::vector<OASIS_FLOAT>> &new_Vulnerability,
		AREAPERIL_INT areaperil_id);



};
#endif
