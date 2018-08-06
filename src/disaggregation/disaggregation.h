#ifndef DISAGG_H_
#define DISAGG_H_

#include <map>
#include <set>
#include <list>
#include <vector>
#include "getrands.h"
#include "../include/oasis.h"


std::ostream& operator<< (std::ostream &out, const aggregate_item &agg_item);
std::ostream& operator<< (std::ostream &out, const item &item);

//bool operator< (const aggregate_item &a, const aggregate_item &i);
bool operator< (const item &a, const item &i);

class disaggregation {
public:
	disaggregation();
	~disaggregation();
	void init();
	void doDisagg(std::vector<item> &i);

private:
	std::map<AREAPERIL_INT, std::vector<OASIS_FLOAT>> _aggregate_areaperils;
	std::map<int, std::map<AREAPERIL_INT, std::map<int, OASIS_FLOAT>>> _aggregate_vulnerabilities;
	std::map<AREAPERIL_INT, std::map<int, OASIS_FLOAT>> _areaperil_to_vulnerabilities;
	std::map<int, OASIS_FLOAT> _vulnerability_probability;
	std::vector<aggregate_item> _aggregate_items;
	std::vector<aggregate_item> _expanded_aggregate_items;
	std::vector<OASIS_FLOAT> _coverages;
	std::set<int> _group_ids = { 0 };
	std::set<int> _item_ids = { 0 };

	long _num_areaperils = -1;
	bool _has_disagg_uncertainty = false;

	void getAggregateAreaPerils(const std::set<AREAPERIL_INT> &a);
	void getAggregateVulnerabilities(const std::set<int> &v);
	void getAggregateItems(std::set<int> &v, std::set<AREAPERIL_INT> &a);
	void getCoverages();
	
	int getVulIndex(int vulnerability_index, int areaperil_index) const;


	void assignNewCoverageID(aggregate_item &a);
	void expandGrouped(aggregate_item &a);
	void expandNotGrouped(aggregate_item &a);
	void assignDisaggAreaPeril(aggregate_item &a, OASIS_FLOAT rand);
	void assignDisaggVulnerability(aggregate_item &a, OASIS_FLOAT rand);

	void aggregateItemtoItem(aggregate_item a, item &i);

};
#endif
