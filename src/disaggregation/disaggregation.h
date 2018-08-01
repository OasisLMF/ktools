#ifndef DISAGG_H_
#define DISAGG_H_

#include <map>
#include <set>
#include <list>
#include <vector>
#include "../include/oasis.h"

class disaggregation {
public:
	disaggregation();
	~disaggregation();
	void init();
	void doDisagg(aggregate_item &a);

private:
	std::map<AREAPERIL_INT, std::vector<OASIS_FLOAT>> _aggregate_areaperils;
	std::map<int, std::vector<OASIS_FLOAT>> _aggregate_vulnerabilities;
	std::map<AREAPERIL_INT, std::vector<int>> _vulnerabilities_by_areaperil;
	std::vector<aggregate_item> _aggregate_items;
	
	long _num_areaperils = -1;
	long _num_vulnerabilities = -1;
	int _num_items = -1;
	bool _has_disagg_uncertainty = false;

	void getAggregateAreaPerils(const std::set<int> &a);
	void getAggregateVulnerabilities(const std::set<int> &v);
	void getAggregateItems();
	
	int getVulIndex(int vulnerability_index, int areaperil_index) const;


	void assignDisaggAreaPeril(aggregate_item &a, OASIS_FLOAT r);
	void assignDisaggVulnerability(aggregate_item &a, OASIS_FLOAT r);
	void expandGrouped(std::vector<aggregate_item> &a);
	void expandNotGrouped(std::vector<aggregate_item> &a);

	void aggregateItemtoItem(std::vector<aggregate_item> &a, std::vector<item> &i);

};
#endif
