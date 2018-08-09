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

ostream& operator<< (std::ostream &out, const Weight &weight) {
	out << "Area Peril: " << weight.areaperil << "\nVulnerability: " << weight.vulnerability << "\nWeight: " << weight.weight;

	return out;
}


void disaggregation::getAggregateItems(set<int> &v, set<AREAPERIL_INT> &a) {
	aggregate_item agg_item;
	FILE *fin = fopen(AGGREGATE_ITEMS_FILE, "rb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, AGGREGATE_ITEMS_FILE);
		exit(EXIT_FAILURE);
	}

	fread(&_num_items, sizeof(_num_items), 1, fin);

	while (fread(&agg_item, sizeof(agg_item), 1, fin) != 0) {
		_aggregate_items.push_back(agg_item);
		v.insert(agg_item.aggregate_vulnerability_id);
		a.insert(agg_item.aggregate_areaperil_id);
	}
	fprintf(stderr, "items file okay\n");
	fclose(fin);
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
	OASIS_FLOAT tiv = _coverages[a.coverage_id];
	tiv /= a.number_items;
	int i = 1;
	while (_coverages[i] != tiv) {
		++i;
		if (i>_coverages.size()) {
			a.coverage_id = _coverages.size();
			_coverages.push_back(tiv);
			return;
		}
	}
	a.coverage_id = i;
}



void disaggregation::getWeights(aggregate_item &i, map<aggregate_item, std::vector<Weight>> &_item_map_weights) {
	vector<Weight> weights;
	if (i.areaperil_id != 0) {
		if (i.vulnerability_id != 0) {
			weights.push_back(Weight(i.areaperil_id, i.vulnerability_id, 1));
		}
		else {
			//only vulnerability aggregated
			auto vul_probs = _aggregate_vulnerabilities[i.aggregate_vulnerability_id][i.areaperil_id];
			for (auto it = vul_probs.begin(); it != vul_probs.end(); ++it) {
				weights.push_back(Weight(i.areaperil_id, it->first, it->second));
			}
		}
	}
	else {
		auto ap_probs = _aggregate_areaperils[i.aggregate_areaperil_id];
		for (AREAPERIL_INT c = 0; c < ap_probs.size(); ++c) {
			if (ap_probs[c] != 0) {
				if (i.vulnerability_id != 0) {
					//only areaperil aggregated
					weights.push_back(Weight((c + 1), i.vulnerability_id, ap_probs[c]));
				}
				else {
					//both aggregated
					auto vul_probs = _aggregate_vulnerabilities[i.aggregate_vulnerability_id][c + 1];
					for (auto it = vul_probs.begin(); it != vul_probs.end(); ++it) {
						OASIS_FLOAT weight = it->second * ap_probs[c];
						weights.push_back(Weight(c + 1, it->first, weight));
					}
				}
			}
		}
	}
	_item_map_weights[i] = weights;
}

//appends new coverages to bin file
void disaggregation::outputNewCoverages() {
	FILE *fin = fopen(COVERAGES_FILE, "a+b");
	if (fin == NULL) {
		fprintf(stderr, "%s: Error reading file %s\n", __func__, COVERAGES_FILE);
		exit(-1);
	}

	flseek(fin, 0L, SEEK_END);
	long long sz = fltell(fin);

	OASIS_FLOAT tiv;
	unsigned int nrec = static_cast<unsigned int>(sz / sizeof(tiv));
	
	++nrec;

	for (nrec; nrec < _coverages.size(); ++nrec) {
		tiv = _coverages[nrec];
		fwrite(&tiv, sizeof(tiv), 1, fin);
	}
	fclose(fin);
}



void disaggregation::doWeights(map<aggregate_item, vector<Weight>> &_item_map_weights) {
	set<AREAPERIL_INT> areas;
	set<int> vuls;

	getAggregateItems(vuls, areas);
	getAggregateAreaPerils(areas);
	getAggregateVulnerabilities(vuls);
	getCoverages();

	for (auto it = _aggregate_items.begin(); it != _aggregate_items.end(); ++it) {
		aggregate_item a = *it;
		if (a.number_items == 0) {
			fprintf(stderr, "Number of items in aggregate item cannot be 0\n");
			exit(EXIT_FAILURE);
		}
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
	for (aggregate_item a : _expanded_aggregate_items) {
		getWeights(a, _item_map_weights);
	}
}
