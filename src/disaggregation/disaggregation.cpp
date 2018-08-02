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

bool operator< (const aggregate_item &a, const aggregate_item &i) {
	if (a.id < i.id) { return true; }
	else { return false; }
}

ostream& operator<< (std::ostream &out, const aggregate_item &agg_item)
{
	out << "Item :\nID: " << agg_item.id << "\nCoverage ID: " << agg_item.coverage_id << "\nAgg Areaperil ID: " << agg_item.aggregate_areaperil_id
		<< "\nAgg Vulnerability ID: " << agg_item.aggregate_vulnerability_id << "\nAreaperil ID: " << agg_item.areaperil_id << "\nVulnerability ID: "
		<< agg_item.vulnerability_id << "\nGrouped: " << agg_item.grouped << "\nNumber of Items: " << agg_item.number_items;

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
	fprintf(stderr, "areaperil file read\n");
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
	fread(&_num_vulnerabilities, sizeof(_num_vulnerabilities), 1, fin);

	while (fread(&agg_vul, sizeof(agg_vul), 1, fin) != 0) {
		if (v.find(agg_vul.aggregate_vulnerability_id) != v.end()) { // only process those aggregate vuls that are 
																// in the item file
			if (agg_vul.aggregate_vulnerability_id != current_aggregate_vulnerability_id) {
				_aggregate_vulnerabilities[agg_vul.aggregate_vulnerability_id] =
					vector<OASIS_FLOAT>(_num_areaperils * _num_vulnerabilities, 0.0);
				current_aggregate_vulnerability_id = agg_vul.aggregate_vulnerability_id;
			}
			int VulnerabilityIndex = getVulIndex(agg_vul.vulnerability_id, agg_vul.areaperil_id);
			_aggregate_vulnerabilities[agg_vul.aggregate_vulnerability_id][VulnerabilityIndex] =
				agg_vul.probability;
		}
	}
	fprintf(stderr, "vulnerability file read\n");
	fclose(fin);
}

int disaggregation::getVulIndex(int vulnerability_index, int areaperil_index) const {
	return (vulnerability_index - 1) + ((areaperil_index - 1) * _num_vulnerabilities);
}

void disaggregation::getAggregateItems(set<int> &v, set<AREAPERIL_INT> &a) {
	aggregate_item agg_item;
	FILE *fin = fopen(AGGREGATE_ITEMS_FILE, "rb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, AGGREGATE_ITEMS_FILE);
		exit(EXIT_FAILURE);
	}

	while (fread(&agg_item, sizeof(agg_item), 1, fin) != 0) {
		_aggregate_items.insert(agg_item);
		v.insert(agg_item.aggregate_vulnerability_id);
		a.insert(agg_item.aggregate_areaperil_id);
	}
	fprintf(stderr, "items file read\n");
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

void disaggregation::expandNotGrouped(aggregate_item &a) {
	_aggregate_items.erase(a);
	int number_of_items = a.number_items;
	for (int c = 0; c < number_of_items; ++c) {
		a.group_id = *_group_ids.end() + 1;
		a.id = *_item_ids.end() + 1;
		a.number_items = 1;
		_aggregate_items.insert(a);
		_group_ids.insert(a.group_id);
		_item_ids.insert(a.id);
	}
	
}

void disaggregation::expandGrouped(aggregate_item &a) {
	_aggregate_items.erase(a);
	int number_of_items = a.number_items;
	int group_id = *_group_ids.end() + 1;
	_group_ids.insert(group_id);
	for (int c = 0; c < number_of_items; ++c) {
		a.group_id = group_id;
		a.id = *_item_ids.end() + 1;
		a.number_items = 1;
		_aggregate_items.insert(a);
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
	if (a.grouped) {
		expandGrouped(a);
	}
	else {
		expandNotGrouped(a);
	}
}

void disaggregation::assignDisaggAreaPeril(aggregate_item &a, OASIS_FLOAT rand) {
	vector<OASIS_FLOAT> dist = _aggregate_areaperils[a.aggregate_areaperil_id];
	for (int i = 1; i < _num_areaperils; ++i) {
		dist[i] += dist[i - 1];
	}
	AREAPERIL_INT areaperil_id = 1;
	while (rand > dist[areaperil_id-1]) {
		++areaperil_id;
	}
	a.areaperil_id = areaperil_id;
}

void disaggregation::assignDisaggVulnerability(aggregate_item &a, OASIS_FLOAT rand) {
	vector<OASIS_FLOAT> dist_all_areaperils = _aggregate_vulnerabilities[a.aggregate_vulnerability_id];
	int first = getVulIndex(1, a.areaperil_id);
	vector<OASIS_FLOAT> dist;
	vector<OASIS_FLOAT>::iterator it = dist_all_areaperils.begin() + first;
	dist.assign(it, it + _num_vulnerabilities);
	for (int i = 1; i < _num_vulnerabilities; ++i) {
		dist[i] += dist[i - 1];
	}
	int vulnerability_id = 1;

	while (rand > dist[vulnerability_id - 1]) {
		++vulnerability_id;
	}
	a.vulnerability_id = vulnerability_id;
}




void disaggregation::doDisagg() {


	set<AREAPERIL_INT> areas;
	set<int> vuls;
	vector<OASIS_FLOAT> randoms;	//need to figure out how to assign randoms numbers to vector from file/seed
	
	getAggregateItems(vuls, areas);
	getAggregateAreaPerils(areas);
	getAggregateVulnerabilities(vuls);

	int randc = 0;

	for (set<aggregate_item>::iterator it = _aggregate_items.begin(); it != _aggregate_items.end(); ++it) {
		aggregate_item a = *it;
		if (a.number_items != 1) {
			assignNewCoverageID(a);
		}
	}
	for (set<aggregate_item>::iterator it = _aggregate_items.begin(); it != _aggregate_items.end(); ++it) {
		aggregate_item a = *it;
		OASIS_FLOAT r = randoms[randc];
		if (!a.areaperil_id) { assignDisaggAreaPeril(a, r); ++randc; }
		if (!a.vulnerability_id) { assignDisaggVulnerability(a, r); ++randc; }
	}
	
}

