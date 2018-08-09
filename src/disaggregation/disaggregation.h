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

std::ostream& operator<< (std::ostream &out, const Weight &weight);

class disaggregation {
public:
	disaggregation(rd_option rndopt, getRands &rnd, bool debug);
	~disaggregation();
	void doDisagg(std::vector<item> &i);
	void outputNewCoverages();
	void doWeights(std::map<aggregate_item, std::vector<Weight>> &_item_map_weights);

private:
	getRands *rnd_;

	std::map<AREAPERIL_INT, std::vector<OASIS_FLOAT>> _aggregate_areaperils;
	std::map<int, std::map<AREAPERIL_INT, std::map<int, OASIS_FLOAT>>> _aggregate_vulnerabilities;
	std::map<AREAPERIL_INT, std::map<int, OASIS_FLOAT>> _areaperil_to_vulnerabilities;
	std::map<int, OASIS_FLOAT> _vulnerability_probability;
	std::vector<aggregate_item> _aggregate_items;
	std::vector<aggregate_item> _expanded_aggregate_items;
	std::vector<OASIS_FLOAT> _coverages;
	std::set<int> _group_ids = { 0 };
	std::set<int> _item_ids = { 0 };
	std::vector<OASIS_FLOAT> rands;

	long _num_areaperils = -1;
	int _num_items = -1;
	bool _has_disagg_uncertainty = false;
	bool debug_ = false;
	rd_option rndopt_;
	long long p1_;
	long long p2_;
	long long p3_;

	void getAggregateItems(std::set<int> &v, std::set<AREAPERIL_INT> &a);
	void getAggregateAreaPerils(const std::set<AREAPERIL_INT> &a);
	void getAggregateVulnerabilities(const std::set<int> &v);
	void getCoverages();
	void doAreaPerilCumulativeProbs();

	
	

	void assignNewCoverageID(aggregate_item &a);
	void expandGrouped(aggregate_item &a);
	void expandNotGrouped(aggregate_item &a);
	void getWeights(aggregate_item &i,
		std::map<aggregate_item, std::vector<Weight>> &_item_map_weights);
	void assignDisaggAreaPeril(aggregate_item &a, OASIS_FLOAT rand);
	void assignDisaggVulnerability(aggregate_item &a, OASIS_FLOAT rand);

	void aggregateItemtoItem(aggregate_item a, item &i);
	void getRandomNumbers();
};
#endif
