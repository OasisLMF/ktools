#include <iostream>
#include <list>
#include <set>
#include <map>
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


void disaggregation::getAggregateAreaPerils(const set<int> &a) {
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

void disaggregation::getAggregateItems() {
	aggregate_item agg_item;
	FILE *fin = fopen(AGGREGATE_ITEMS_FILE, "rb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, AGGREGATE_ITEMS_FILE);
		exit(EXIT_FAILURE);
	}
	int index = 0;
	int group = 1;
	while (fread(&agg_item, sizeof(agg_item), 1, fin) != 0) {
		_aggregate_items.push_back(agg_item);
	}
	fprintf(stderr, "items file read\n");
	fclose(fin);
}

void disaggregation::assignDisaggAreaPeril(aggregate_item &a, OASIS_FLOAT r) {
	vector<OASIS_FLOAT> dist = _aggregate_areaperils[a.aggregate_areaperil_id];
	for (int i = 1; i <= _num_areaperils; ++i) {
		dist[i] += dist[i - 1];
	}
	AREAPERIL_INT areaperil_id = 1;
	while (r > dist[areaperil_id-1]) {
		++areaperil_id;
	}
	a.areaperil_id = areaperil_id;
}

void disaggregation::assignDisaggVulnerability(aggregate_item &a, OASIS_FLOAT r) {
	vector<OASIS_FLOAT> dist_all_areaperils = _aggregate_vulnerabilities[a.aggregate_vulnerability_id];
	int first = getVulIndex(1, a.areaperil_id);
	vector<OASIS_FLOAT> dist;
	vector<OASIS_FLOAT>::iterator it = dist_all_areaperils.begin() + first;
	dist.assign(it, it + _num_vulnerabilities);
	for (int i = 1; i <= _num_vulnerabilities; ++i) {
		dist[i] += dist[i - 1];
		fprintf(stderr, "%f\n", dist[i]);
	}
	int vulnerability_id = 1;

	while (r > dist[vulnerability_id - 1]) {
		++vulnerability_id;
	}
	a.vulnerability_id = vulnerability_id;
}




void disaggregation::doDisagg(aggregate_item &a) {
	set<int> areas = { 1 };
	set<int> vuls = { 1 };
	OASIS_FLOAT r = 0.75;
	OASIS_FLOAT b = 0.4;
	getAggregateAreaPerils(areas);
	
	//getAggregateVulnerabilities(vuls);

	assignDisaggAreaPeril(a, r);
	fprintf(stderr, "%d, %d, %d, %d\n", a.aggregate_areaperil_id, a.areaperil_id, a.aggregate_vulnerability_id, a.vulnerability_id);
	
	//assignDisaggVulnerability(a, b);
	fprintf(stderr, "%d, %d, %d, %d\n", a.aggregate_areaperil_id, a.areaperil_id, a.aggregate_vulnerability_id, a.vulnerability_id);
}

int main() {
	aggregate_item item;
	item.aggregate_areaperil_id = 1;
	item.aggregate_vulnerability_id = 1;
	item.areaperil_id = 0;
	item.coverage_id = 15;
	item.grouped = 0;
	item.id = 1;
	item.number_items = 1;
	item.vulnerability_id = 0;

	disaggregation disagg;
	initstreams();
	disagg.doDisagg(item);

	fprintf(stderr, "%d, %d, %d, %d", item.aggregate_areaperil_id, item.areaperil_id, item.aggregate_vulnerability_id, item.vulnerability_id);

	return EXIT_SUCCESS;
}