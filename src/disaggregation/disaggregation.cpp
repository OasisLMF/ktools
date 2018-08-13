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


void disaggregation::getAggregateItems() {
	aggregate_item agg_item;
	FILE *fin = fopen(AGGREGATE_ITEMS_FILE, "rb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, AGGREGATE_ITEMS_FILE);
		exit(EXIT_FAILURE);
	}

	while (fread(&agg_item, sizeof(agg_item), 1, fin) != 0) {
		_aggregate_items.push_back(agg_item);
		_aggregate_vulnerability_ids.insert(agg_item.aggregate_vulnerability_id);
		_aggregate_areaperil_ids.insert(agg_item.aggregate_areaperil_id);
	}
	fprintf(stderr, "items file okay\n");
	fclose(fin);
}


void disaggregation::getAggregateAreaPerils() {
	aggregate_areaperil_to_areaperil agg_ap;
	FILE *fin = fopen(AGGREGATE_AREAPERIL_TO_AREAPERIL_FILE, "rb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, AGGREGATE_AREAPERIL_TO_AREAPERIL_FILE);
		exit(EXIT_FAILURE);
	}
	int current_aggregate_areaperil_id = -1;
	fread(&_num_areaperils, sizeof(_num_areaperils), 1, fin);

	while (fread(&agg_ap, sizeof(agg_ap), 1, fin) != 0) {
		if (_aggregate_areaperil_ids.find(agg_ap.aggregate_areaperil_id) 
			!= _aggregate_areaperil_ids.end()) { // only process those aggregate area perils that are 
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

void disaggregation::getAggregateVulnerabilities() {
	aggregate_vulnerability_to_vulnerability agg_vul;
	FILE *fin = fopen(AGGREGATE_VULNERABILITY_TO_VULNERABILITY_FILE, "rb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, AGGREGATE_VULNERABILITY_TO_VULNERABILITY_FILE);
		exit(EXIT_FAILURE);
	}

	int current_aggregate_vulnerability_id = -1;
	map<int, OASIS_FLOAT> _vulnerability_probability;
	std::map<AREAPERIL_INT, std::map<int, OASIS_FLOAT>> _areaperil_to_vulnerabilities;

	while (fread(&agg_vul, sizeof(agg_vul), 1, fin) != 0) {
		if (_aggregate_vulnerability_ids.find(agg_vul.aggregate_vulnerability_id)
			!= _aggregate_vulnerability_ids.end()) { // only process those aggregate vuls that are 
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
			_vulnerability_ids.insert(agg_vul.vulnerability_id);
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

void disaggregation::expandGrouped(aggregate_item &a) {\
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


//appends new coverages to bin file
void disaggregation::outputNewCoverages() {
	FILE *fin = fopen(COVERAGES_FILE, "a+b");
	if (fin == NULL) {
		fprintf(stderr, "%s: Error opening file %s\n", __func__, COVERAGES_FILE);
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



void disaggregation::newItems() {
	getAggregateItems();
	getAggregateAreaPerils();
	getAggregateVulnerabilities();
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
	FILE *fin = fopen(AGGREGATE_ITEMS_FILE, "wb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, AGGREGATE_ITEMS_FILE);
		exit(EXIT_FAILURE);
	}
	for (aggregate_item a : _expanded_aggregate_items) {
		fwrite(&a, sizeof(a), 1, fin);
	}
	fclose(fin);
}

void disaggregation::mergeAggregateVulnerability(int aggregate_vulnerability_id, map<int, OASIS_FLOAT> &vulnerability_probability) {
	for (auto vul = _vulnerability_ids.begin(); vul != _vulnerability_ids.end(); ++vul) {
		OASIS_FLOAT prob = 0;
		int count = 0;
		for (auto it = _aggregate_vulnerabilities[aggregate_vulnerability_id].begin();
			it != _aggregate_vulnerabilities[aggregate_vulnerability_id].end(); ++it) {
			prob += it->second.find(*vul)->second;
			count++;
		}
		prob /= count;
		vulnerability_probability[*vul] = prob;
	}
}

void disaggregation::getIntensityInfo() {
	FILE *fin = fopen(FOOTPRINT_FILE, "rb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, FOOTPRINT_FILE);
		exit(EXIT_FAILURE);
	}
	fread(&_num_intensity_bins, sizeof(_num_intensity_bins), 1, fin);
	fread(&_has_intensity_uncertainty, sizeof(_has_intensity_uncertainty), 1,
		fin);
	fclose(fin);
}

void disaggregation::getVulnerabilities() {
	Vulnerability vulnerability;

	FILE *fin = fopen(VULNERABILITY_FILE, "rb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, VULNERABILITY_FILE);
		exit(EXIT_FAILURE);
	}
	int current_vulnerability_id = -1;
	int current_intensity_bin = -1;
	map<int, std::vector<OASIS_FLOAT>> intensity_to_damage;
	fread(&_num_damage_bins, sizeof(_num_damage_bins), 1, fin);

	while (fread(&vulnerability, sizeof(vulnerability), 1, fin) != 0) {
		if (_vulnerability_ids.find(vulnerability.vulnerability_id) !=
			_vulnerability_ids.end()) { // only process those vulnerabilities that are in the item
					   // file
			if (_num_intensity_bins >= vulnerability.intensity_bin_id) {
				if (vulnerability.vulnerability_id != current_vulnerability_id ||
					vulnerability.intensity_bin_id != current_intensity_bin) {
					_vulnerabilities[vulnerability.vulnerability_id][vulnerability.intensity_bin_id]
						= vector<OASIS_FLOAT>(_num_damage_bins, 0.0);
					current_vulnerability_id = vulnerability.vulnerability_id;
					current_intensity_bin = vulnerability.intensity_bin_id;
				}
				_vulnerabilities[vulnerability.vulnerability_id][vulnerability.intensity_bin_id]
					[vulnerability.damage_bin_id-1] = vulnerability.probability;
			}
		}
	}
	fclose(fin);
}


void disaggregation::getFootprints() {
	FILE *fin;
	if (_zip) {
		fin = fopen(ZFOOTPRINT_IDX_FILE, "rb");
	}
	else {
		fin = fopen(FOOTPRINT_IDX_FILE, "rb");
	}
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, FOOTPRINT_IDX_FILE);
		exit(EXIT_FAILURE);
	}

	EventIndex event_index;
	while (fread(&event_index, sizeof(event_index), 1, fin) != 0) {
		_event_index_by_event_id[event_index.event_id] = event_index;
	}

	fclose(fin);

}



void disaggregation::getIntensityProbs(int event_id, map<AREAPERIL_INT, vector<OASIS_FLOAT>> &areaperil_intensity) {
	auto sizeof_EventKey = sizeof(EventRow);
	auto fin = fopen(FOOTPRINT_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "Error opening footprint file\n");
		exit(-1);
	}
	AREAPERIL_INT current_areaperil_id = -1;

	if (_event_index_by_event_id.count(event_id) == 0) {
		fclose(fin);
		return;
	}
	flseek(fin, _event_index_by_event_id[event_id].offset, 0);
	EventRow event_key;
	int number_of_event_records = static_cast<int>(
		_event_index_by_event_id[event_id].size / sizeof_EventKey);
	
	
	for (int i = 0; i < number_of_event_records; ++i) {
		fread(&event_key, sizeof(event_key), 1, fin);
		if (event_key.areaperil_id != current_areaperil_id) {
			areaperil_intensity[event_key.areaperil_id] = vector<OASIS_FLOAT>(_num_intensity_bins, 0.0);
			current_areaperil_id = event_key.areaperil_id;
		}
		areaperil_intensity[event_key.areaperil_id][event_key.intensity_bin_id - 1] =
			event_key.probability;
	}
	fclose(fin);
}

void disaggregation::newFootprint(AREAPERIL_INT aggregate_areaperil_id,
	map<AREAPERIL_INT, vector<OASIS_FLOAT>> areaperil_intensity, vector<EventRow> &new_Footprint) {
	new_Footprint.resize(_num_intensity_bins);
	for (int intensity_bin_id = 0; intensity_bin_id < _num_intensity_bins; ++intensity_bin_id) {
		OASIS_FLOAT prob = 0.0;
		for (auto it = areaperil_intensity.begin(); it != areaperil_intensity.end(); ++it) {
			OASIS_FLOAT weight = _aggregate_areaperils[aggregate_areaperil_id][it->first-1];
			prob += it->second[intensity_bin_id] * weight;
		}
		new_Footprint[intensity_bin_id] = EventRow(aggregate_areaperil_id, intensity_bin_id + 1, prob);
	}
}

void disaggregation::newVulnerability(int aggregate_vulnerability_id, map<int, vector<OASIS_FLOAT>> &new_Vulnerability,
	AREAPERIL_INT areaperil_id = -1) {
	map<int, OASIS_FLOAT> vulnerability_probability;
	if (areaperil_id == -1) {
		mergeAggregateVulnerability(aggregate_vulnerability_id, vulnerability_probability);
	}
	else {
		vulnerability_probability = _aggregate_vulnerabilities[aggregate_vulnerability_id][areaperil_id];
	}
	for (int c = 0; c < _num_intensity_bins; ++c) {
		new_Vulnerability[c + 1] = vector<OASIS_FLOAT>(_num_damage_bins, 0.0);
	}
	for (int i = 0; i < _num_damage_bins; ++i) {
		for (int c = 0; c < _num_intensity_bins; ++c) {
			
			OASIS_FLOAT prob = 0.0;
			for (auto it = _vulnerabilities.begin(); it != _vulnerabilities.end(); ++it) {
				OASIS_FLOAT weight = vulnerability_probability[it->first];
				prob += weight * it->second[c + 1][i];
			}
			new_Vulnerability[c + 1][i] = prob;
		}		
	}
}

void disaggregation::test() {
	getIntensityInfo();
	getVulnerabilities();
	getFootprints();

	map<AREAPERIL_INT, vector<OASIS_FLOAT>> areaperil_intensity;
	getIntensityProbs(26, areaperil_intensity);
	vector<EventRow> new_Footprint;
	newFootprint(1, areaperil_intensity, new_Footprint);
	map<int, vector<OASIS_FLOAT>> new_Vulnerability;
	newVulnerability(1, new_Vulnerability);

}
