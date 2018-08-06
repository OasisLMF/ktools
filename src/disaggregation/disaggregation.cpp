#include <iostream>
#include "disaggregation.h"
#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif




using namespace std;

const size_t SIZE_OF_INT = sizeof(int);
const size_t SIZE_OF_OASIS_FLOAT = sizeof(OASIS_FLOAT);

//const unsigned int OUTPUT_STREAM_TYPE = 1;

disaggregation::disaggregation() {
}

disaggregation::~disaggregation() {
}

/*
bool operator< (const aggregate_item &a, const aggregate_item &i) {
	if (a.id < i.id) { return true; }
	else { return false; }
}
*/
bool operator< (const item &a, const item &i) {
	if (a.id < i.id) { return true; }
	else { return false; }
}


ostream& operator<< (std::ostream &out, const aggregate_item &agg_item) {
	out << "Item :\nID: " << agg_item.id << "\nCoverage ID: " << agg_item.coverage_id << "\nAgg Areaperil ID: " << agg_item.aggregate_areaperil_id
		<< "\nAgg Vulnerability ID: " << agg_item.aggregate_vulnerability_id << "\nAreaperil ID: " << agg_item.areaperil_id << "\nVulnerability ID: "
		<< agg_item.vulnerability_id << "\nGrouped: " << agg_item.grouped << "\nNumber of Items: " << agg_item.number_items;

	return out;
}

ostream& operator<< (std::ostream &out, const item &item) {
	out << "Item :\nID: " << item.id << "\nCoverage ID: " << item.coverage_id << "\nAreaperil ID: " << item.areaperil_id << "\nVulnerability ID: "
		<< item.vulnerability_id << "\nGroup ID: " << item.group_id;

	return out;
}


void disaggregation::getAggregateAreaPerils(const set<AREAPERIL_INT> &a) {
	aggregate_areaperil_to_areaperil agg_ap;
	FILE *fin = fopen(AGGREGATE_AREAPERIL_TO_AREAPERIL_FILE, "rb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, AGGREGATE_AREAPERIL_TO_AREAPERIL_FILE);
		exit(EXIT_FAILURE);
	}
	int current_aggregate_areaperil_id = -1;
	fread(&_num_areaperils, sizeof(_num_areaperils), 1, fin);

	while (fread(&agg_ap, sizeof(agg_ap), 1, fin) != 0) {
		if (a.find(agg_ap.aggregate_areaperil_id) != a.end()) { // only process those aggregate area perils that are 
																// in the item file
			if (agg_ap.aggregate_areaperil_id != current_aggregate_areaperil_id) {
					_aggregate_areaperils[agg_ap.aggregate_areaperil_id] =
					vector<OASIS_FLOAT>(_num_areaperils , 0.0);
					current_aggregate_areaperil_id = agg_ap.aggregate_areaperil_id;
			}
				_aggregate_areaperils[agg_ap.aggregate_areaperil_id][agg_ap.areaperil_id-1] =
					agg_ap.probability;
		}
	}
	fprintf(stderr, "areaperil file okay\n");
	fclose(fin);
}

void disaggregation::getAggregateVulnerabilities(const set<int> &v) {
	aggregate_vulnerability_to_vulnerability agg_vul;
	FILE *fin = fopen(AGGREGATE_VULNERABILITY_TO_VULNERABILITY_FILE, "rb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, AGGREGATE_VULNERABILITY_TO_VULNERABILITY_FILE);
		exit(EXIT_FAILURE);
	}

	int current_aggregate_vulnerability_id = -1;


	while (fread(&agg_vul, sizeof(agg_vul), 1, fin) != 0) {
		if (v.find(agg_vul.aggregate_vulnerability_id) != v.end()) { // only process those aggregate vuls that are 
																// in the item file
			if (agg_vul.aggregate_vulnerability_id != current_aggregate_vulnerability_id) {
				_aggregate_vulnerabilities[agg_vul.aggregate_vulnerability_id] =
					_areaperil_to_vulnerabilities;
				current_aggregate_vulnerability_id = agg_vul.aggregate_vulnerability_id;

				int current_areaperil_id = -1;
				if (agg_vul.areaperil_id != current_areaperil_id) {
					_areaperil_to_vulnerabilities[agg_vul.areaperil_id] = _vulnerability_probability;
					current_areaperil_id = agg_vul.areaperil_id;
				}
			}
			_aggregate_vulnerabilities[agg_vul.aggregate_vulnerability_id][agg_vul.areaperil_id][agg_vul.vulnerability_id] =
				agg_vul.probability;
		}
	}
	fprintf(stderr, "vulnerability file okay\n");
	fclose(fin);
}



void disaggregation::getAggregateItems(set<int> &v, set<AREAPERIL_INT> &a) {
	aggregate_item agg_item;
	FILE *fin = fopen(AGGREGATE_ITEMS_FILE, "rb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, AGGREGATE_ITEMS_FILE);
		exit(EXIT_FAILURE);
	}

	while (fread(&agg_item, sizeof(agg_item), 1, fin) != 0) {
		_aggregate_items.push_back(agg_item);
		v.insert(agg_item.aggregate_vulnerability_id);
		a.insert(agg_item.aggregate_areaperil_id);
	}
	fprintf(stderr, "items file okay\n");
	fclose(fin);
}

void disaggregation::getCoverages() {
	FILE *fin = fopen(COVERAGES_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: Error reading file %s\n", __func__, COVERAGES_FILE);
		exit(-1);
	}

	flseek(fin, 0L, SEEK_END);
	long long sz = fltell(fin);
	flseek(fin, 0L, SEEK_SET);

	OASIS_FLOAT tiv;
	unsigned int nrec = static_cast<unsigned int>(sz / sizeof(tiv));

	_coverages.resize(nrec + 1);
	int coverage_id = 0;
	size_t i = fread(&tiv, sizeof(tiv), 1, fin);
	while (i != 0) {
		coverage_id++;
		_coverages[coverage_id] = tiv;
		i = fread(&tiv, sizeof(tiv), 1, fin);
	}

	fclose(fin);
}

void disaggregation::doCumulativeProbs() {
	for (map<AREAPERIL_INT, vector<OASIS_FLOAT>>::iterator it = _aggregate_areaperils.begin(); it != _aggregate_areaperils.end(); ++it) {
		for (int i = 1; i < _num_areaperils; ++i) {
			it->second[i] += it->second[i - 1];
		}
	}
	for (map<int, map<AREAPERIL_INT, map<int, OASIS_FLOAT>>>::iterator it = _aggregate_vulnerabilities.begin(); it != _aggregate_vulnerabilities.end(); ++it) {
		for (map<AREAPERIL_INT, map<int, OASIS_FLOAT>>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
			OASIS_FLOAT prob = 0.0;
			for (map<int, OASIS_FLOAT>::iterator it3 = it2->second.begin(); it3 != it2->second.end(); ++it3) {
				it3->second += prob;
				prob = it3->second;
			}
		}
	}
}

void disaggregation::expandNotGrouped(aggregate_item &a) {
	//agg_index = _aggregate_items.erase(agg_index);
	int number_of_items = a.number_items;
	for (int c = 0; c < number_of_items; ++c) {
		a.group_id = *_group_ids.rbegin() + 1;
		a.id = *_item_ids.rbegin() + 1;
		a.number_items = 1;
		_expanded_aggregate_items.push_back(a);
		_group_ids.insert(a.group_id);
		_item_ids.insert(a.id);
	}
	
}

void disaggregation::expandGrouped(aggregate_item &a) {
	//agg_index = _aggregate_items.erase(agg_index);
	int number_of_items = a.number_items;
	int group_id = *_group_ids.rbegin() + 1;
	_group_ids.insert(group_id);
	for (int c = 0; c < number_of_items; ++c) {
		a.group_id = group_id;
		a.id = *_item_ids.rbegin() + 1;
		a.number_items = 1;
		_expanded_aggregate_items.push_back(a);
		_item_ids.insert(a.id);
	}
}

void disaggregation::assignNewCoverageID(aggregate_item &a) {
	//call only if number_items > 1
	int old_coverage_id = a.coverage_id;
	OASIS_FLOAT tiv = _coverages[old_coverage_id];
	tiv /= a.number_items;
	a.coverage_id = _coverages.size();
	_coverages.push_back(tiv);
}

void disaggregation::assignDisaggAreaPeril(aggregate_item &a, OASIS_FLOAT rand) {
	vector<OASIS_FLOAT> dist = _aggregate_areaperils[a.aggregate_areaperil_id];
	AREAPERIL_INT areaperil_id = 1;
	while (rand > dist[areaperil_id-1]) {
		++areaperil_id;
	}
	a.areaperil_id = areaperil_id;
}

void disaggregation::assignDisaggVulnerability(aggregate_item &a, OASIS_FLOAT rand) {
	map<AREAPERIL_INT, map<int, OASIS_FLOAT>> dist_all_areaperils = _aggregate_vulnerabilities[a.aggregate_vulnerability_id];
	map<int, OASIS_FLOAT> dist = dist_all_areaperils[a.areaperil_id];

	map<int, OASIS_FLOAT>::iterator it = dist.begin();

	while (rand > it->second) {
		++it;
	}
	a.vulnerability_id = it->first;
}


void disaggregation::aggregateItemtoItem(aggregate_item a, item &i) {
	i.id = a.id;
	i.coverage_id = a.coverage_id;
	i.areaperil_id = a.areaperil_id;
	i.vulnerability_id = a.vulnerability_id;
	i.group_id = a.group_id;
}

void disaggregation::doDisagg(vector<item> &i) {


	set<AREAPERIL_INT> areas;
	set<int> vuls;
	vector<OASIS_FLOAT> randoms(60, 0.5);	//need to figure out how to assign randoms numbers to vector from file/seed

	getAggregateItems(vuls, areas);
	getAggregateAreaPerils(areas);
	getAggregateVulnerabilities(vuls);
	getCoverages();
	doCumulativeProbs();


	//getRands

	int randc = 0;

	for (vector<aggregate_item>::iterator it = _aggregate_items.begin(); it != _aggregate_items.end(); ++it) {
		aggregate_item a = *it;
		if (a.number_items != 1) {
			assignNewCoverageID(a);
		}
		if (a.grouped) {
			expandGrouped(a);
		}
		else {
			expandNotGrouped(a);
		}
	}

	i.reserve(_expanded_aggregate_items.size());

	for (int c = 0; c < _expanded_aggregate_items.size(); ++c) {
		item item;
		aggregate_item a = _expanded_aggregate_items[c]; 
		
		OASIS_FLOAT r = randoms[randc];
		if (a.areaperil_id == 0) { assignDisaggAreaPeril(a, r); ++randc; }
		if (a.vulnerability_id == 0) { assignDisaggVulnerability(a, r); ++randc; }

		aggregateItemtoItem(a, item);

		i.push_back(item);
	}
	
}



