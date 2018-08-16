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
*     distribution
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
#include "string.h"
#include <iostream>
#include <list>
#include <set>
#include <map>
#include "getmodel.h"
#include "../include/oasis.h"
#include <zlib.h>

struct Exposure {
  int location_id;
  AREAPERIL_INT areaperil_id;
  int vulnerability_id;
  int group_id;
  OASIS_FLOAT tiv;
};

struct Result {

  Result() : prob(0.0), damage(0.0) {
    //
  }

  Result(OASIS_FLOAT damage, OASIS_FLOAT prob);

  OASIS_FLOAT prob;
  OASIS_FLOAT damage;
};

Result::Result(OASIS_FLOAT prob, OASIS_FLOAT damage) : prob(prob), damage(damage) {
  //
}

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

// Is this the number of damage bins?
const int MAX_RESULTS = 100;

const size_t SIZE_OF_INT = sizeof(int);
const size_t SIZE_OF_OASIS_FLOAT = sizeof(OASIS_FLOAT);
const size_t SIZE_OF_RESULT = sizeof(Result);

const unsigned int OUTPUT_STREAM_TYPE = 1;

bool operator< (const item &a, const item &i) {
	if (a.id < i.id) { return true; }
	else { return false; }
}

getmodel::getmodel() {
  //
}

getmodel::~getmodel() {
  //if (_temp_results != nullptr)
    //delete _temp_results;
}


void getmodel::getItems() {
	// Read the exposures and generate a set of vulnerabilities by area peril

	FILE *finv = fopen(AGGREGATE_VULNERABILITY_TO_VULNERABILITY_FILE, "rb");
	if (finv == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, AGGREGATE_VULNERABILITY_TO_VULNERABILITY_FILE);
		exit(EXIT_FAILURE);
	}
	fread(&_agg_vul_start, sizeof(_agg_vul_start), 1, finv);
	fclose(finv);

	FILE *fina = fopen(AGGREGATE_AREAPERIL_TO_AREAPERIL_FILE, "rb");
	if (fina == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, AGGREGATE_AREAPERIL_TO_AREAPERIL_FILE);
		exit(EXIT_FAILURE);
	}
	fread(&_agg_ap_start, sizeof(_agg_ap_start), 1, fina);
	fclose(fina);

	aggregate_item item_rec;

	FILE *fin = fopen(AGGREGATE_ITEMS_FILE, "rb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, AGGREGATE_ITEMS_FILE);
		exit(EXIT_FAILURE);
	}

	while (fread(&item_rec, sizeof(item_rec), 1, fin) != 0) {
		_aggregate_items.push_back(item_rec);
		if (_vulnerability_ids_by_area_peril.count(item_rec.areaperil_id) == 0)
			_vulnerability_ids_by_area_peril[item_rec.areaperil_id] = std::set<int>();
		_vulnerability_ids_by_area_peril[item_rec.areaperil_id].insert(
			item_rec.vulnerability_id);
		if (item_rec.areaperil_id < _agg_ap_start) {
			_area_perils.insert(item_rec.areaperil_id);
		}
		else {
			_aggregate_areaperil_ids.insert(item_rec.areaperil_id);
		}
		if (item_rec.vulnerability_id < _agg_vul_start) {
			_vulnerability_ids.insert(item_rec.vulnerability_id);
		}
		else {
			_aggregate_vulnerability_ids.insert(item_rec.vulnerability_id);
		}
	}
	fclose(fin);
}

void getmodel::getAggregateAreaPerils() {
	aggregate_areaperil_to_areaperil agg_ap;
	FILE *fin = fopen(AGGREGATE_AREAPERIL_TO_AREAPERIL_FILE, "rb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, AGGREGATE_AREAPERIL_TO_AREAPERIL_FILE);
		exit(EXIT_FAILURE);
	}
	int current_aggregate_areaperil_id = -1;
	fread(&_agg_ap_start, sizeof(_agg_ap_start), 1, fin);

	while (fread(&agg_ap, sizeof(agg_ap), 1, fin) != 0) {
		if (_aggregate_areaperil_ids.find(agg_ap.aggregate_areaperil_id)
			!= _aggregate_areaperil_ids.end()) { // only process those aggregate area perils that are 
												 // in the item file
			std::map<AREAPERIL_INT, OASIS_FLOAT> _areaperil_prob;
			if (agg_ap.aggregate_areaperil_id != current_aggregate_areaperil_id) {
				_aggregate_areaperils[agg_ap.aggregate_areaperil_id] = _areaperil_prob;
				current_aggregate_areaperil_id = agg_ap.aggregate_areaperil_id;
			}
			_aggregate_areaperils[agg_ap.aggregate_areaperil_id][agg_ap.areaperil_id] =
				agg_ap.probability;
			_disagg_area_perils.insert(agg_ap.areaperil_id);
		}
	}
	fclose(fin);
}

void getmodel::getAggregateVulnerabilities() {
	aggregate_vulnerability_to_vulnerability agg_vul;
	FILE *fin = fopen(AGGREGATE_VULNERABILITY_TO_VULNERABILITY_FILE, "rb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, AGGREGATE_VULNERABILITY_TO_VULNERABILITY_FILE);
		exit(EXIT_FAILURE);
	}
	fread(&_agg_vul_start, sizeof(_agg_vul_start), 1, fin);
	int current_aggregate_vulnerability_id = -1;
	std::map<int, OASIS_FLOAT> _vulnerability_probability;
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
	fclose(fin);
}


void getmodel::getCoverages() {
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

void getmodel::expandItems(aggregate_item &a) {
	item i;
	int number_of_items = a.number_items;
	OASIS_FLOAT tiv = _coverages[a.coverage_id];
	tiv /= a.number_items;

	if (a.grouped) {
		int group_id = *_group_ids.rbegin() + 1;
		_group_ids.insert(group_id);
		for (int c = 0; c < number_of_items; ++c) {
			i.group_id = group_id;
			i.id = *_item_ids.rbegin() + 1;
			_expanded_items.push_back(i);
			_item_ids.insert(i.id);
		}
	}
	else {
		for (int c = 0; c < number_of_items; ++c) {
			i.group_id = *_group_ids.rbegin() + 1;
			i.id = *_item_ids.rbegin() + 1;
			_expanded_items.push_back(i);
			_group_ids.insert(i.group_id);
			_item_ids.insert(i.id);
		}
	}
	int c = 1;
	while (_coverages[c] != tiv) {
		++c;
		if (c>=_coverages.size()) {
			a.coverage_id = _coverages.size();
			_coverages.push_back(tiv);
			return;
		}
	}
	a.coverage_id = c;

}


//appends new coverages to bin file
void getmodel::outputNewCoverages() {
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



void getmodel::newItems() {
	getItems();
	getAggregateAreaPerils();
	getAggregateVulnerabilities();
	getCoverages();

	for (auto it = _aggregate_items.begin(); it != _aggregate_items.end(); ++it) {
		aggregate_item a = *it;
		if (a.number_items == 0) {
			fprintf(stderr, "Number of items in aggregate item cannot be 0\n");
			exit(EXIT_FAILURE);
		}
		expandItems(a);
	}
	void outputNewCoverages();

	FILE *fin = fopen(ITEMS_FILE, "wb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, ITEMS_FILE);
		exit(EXIT_FAILURE);
	}
	for (item a : _expanded_items) {
		fwrite(&a, sizeof(a), 1, fin);
	}
	fclose(fin);
}

// only get those vulnerabilities that exist in the items file and aggregate vuls - so reduce
// memory footprint
void getmodel::getVulnerabilities() {
	// Read the vulnerabilities
	Vulnerability vulnerability;

	FILE *fin = fopen(VULNERABILITY_FILE, "rb");
	if (fin == nullptr) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, VULNERABILITY_FILE);
		exit(EXIT_FAILURE);
	}
	int current_vulnerability_id = -1;
	fread(&_num_damage_bins, sizeof(_num_damage_bins), 1, fin);

	while (fread(&vulnerability, sizeof(vulnerability), 1, fin) != 0) {
		if (_vulnerability_ids.find(vulnerability.vulnerability_id) !=
			_vulnerability_ids.end()) { // only process those vulnerabilities that are in the item
										// file
			if (_num_intensity_bins >= vulnerability.intensity_bin_id) {
				if (vulnerability.vulnerability_id != current_vulnerability_id) {
					_vulnerabilities[vulnerability.vulnerability_id] =
						std::vector<OASIS_FLOAT>(_num_intensity_bins * _num_damage_bins, 0.0);
					current_vulnerability_id = vulnerability.vulnerability_id;
				}
				int vulnerabilityIndex = getVulnerabilityIndex(
					vulnerability.intensity_bin_id, vulnerability.damage_bin_id);
				_vulnerabilities[vulnerability.vulnerability_id][vulnerabilityIndex] =
					vulnerability.probability;
			}
		}
	}
	fclose(fin);
}
void getmodel::getIntensityInfo() {
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

void getmodel::getFootPrints() {
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

void getmodel::mergeAggregateVulnerability(int aggregate_vulnerability_id, std::map<int, OASIS_FLOAT> &vulnerability_probability) {
	for (auto vul = _vulnerability_ids.begin(); vul != _vulnerability_ids.end(); ++vul) {
		OASIS_FLOAT prob = 0;
		int count = 0;
		for (auto it = _aggregate_vulnerabilities[aggregate_vulnerability_id].begin();
			it != _aggregate_vulnerabilities[aggregate_vulnerability_id].end(); ++it) {
			if (it->second.find(*vul) != it->second.end()) {
				prob += it->second.find(*vul)->second;
				count++;
			}
		}
		prob /= count;
		vulnerability_probability[*vul] = prob;
	}
}


void getmodel::getIntensityProbs(int event_id, std::map<AREAPERIL_INT, std::vector<OASIS_FLOAT>> &areaperil_intensity) {
	if (_zip) {
		auto sizeof_EventKey = sizeof(EventRow);
		auto fin = fopen(ZFOOTPRINT_FILE, "rb");
		if (_event_index_by_event_id.count(event_id) == 0)
			return;
		flseek(fin, _event_index_by_event_id[event_id].offset, 0);

		int size = _event_index_by_event_id[event_id].size;
		_compressed_buf.resize(size + 1);
		_uncompressed_buf.resize(size * 20);
		fread(&_compressed_buf[0], size, 1, fin);
		uLong dest_length = _uncompressed_buf.size();
		int ret = uncompress(&_uncompressed_buf[0], &dest_length, &_compressed_buf[0], size);
		if (ret != Z_OK) {
			fprintf(stderr, "Got bad return code from uncompress %d\n", ret);
			exit(-1);
		}
		// we now have the data length
		int number_of_event_records = static_cast<int>(dest_length / sizeof_EventKey);
		EventRow *event_key = (EventRow *)&_uncompressed_buf[0];
		AREAPERIL_INT current_areaperil_id = -1;
		for (int i = 0; i < number_of_event_records; i++) {
			if (event_key->areaperil_id != current_areaperil_id) {
				areaperil_intensity[event_key->areaperil_id] = std::vector<OASIS_FLOAT>(_num_intensity_bins, 0.0);
				current_areaperil_id = event_key->areaperil_id;
			}
			areaperil_intensity[event_key->areaperil_id][event_key->intensity_bin_id - 1] =
				event_key->probability;
			event_key++;
		}
	}
	else {
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
				areaperil_intensity[event_key.areaperil_id] = std::vector<OASIS_FLOAT>(_num_intensity_bins, 0.0);
				current_areaperil_id = event_key.areaperil_id;
			}
			areaperil_intensity[event_key.areaperil_id][event_key.intensity_bin_id - 1] =
				event_key.probability;
		}
		fclose(fin);
	}
	/*
	for (AREAPERIL_INT areaperil_id : _disagg_area_perils) {
		if (areaperil_intensity.find(areaperil_id) == areaperil_intensity.end()) {
			areaperil_intensity[areaperil_id] = std::vector<OASIS_FLOAT>(_num_intensity_bins, 0.0);
			areaperil_intensity[areaperil_id][0] = 1;
		}
	}
	*/
}

void getmodel::newFootprint(int event_id, AREAPERIL_INT aggregate_areaperil_id, std::vector<EventRow> &new_Footprint) {
	std::map<AREAPERIL_INT, std::vector<OASIS_FLOAT>> areaperil_intensity;
	getIntensityProbs(event_id, areaperil_intensity);
	new_Footprint.resize(_num_intensity_bins);
	for (int intensity_bin_id = 0; intensity_bin_id < _num_intensity_bins; ++intensity_bin_id) {
		OASIS_FLOAT prob = 0.0;
		for (auto it = _aggregate_areaperils[aggregate_areaperil_id].begin(); it != _aggregate_areaperils[aggregate_areaperil_id].end(); ++it) {
			OASIS_FLOAT weight = it->second;
			auto itr = areaperil_intensity.find(it->first);
			if (itr != areaperil_intensity.end()) {
				prob += itr->second[intensity_bin_id] * weight;
			}
			else {
				if (intensity_bin_id == 0) {
					prob += weight;
				}
			}
		}
		new_Footprint[intensity_bin_id] = EventRow(aggregate_areaperil_id, intensity_bin_id + 1, prob);
	}
}

void getmodel::newVulnerability(int aggregate_vulnerability_id,std::vector<OASIS_FLOAT> &new_Vulnerability,
	AREAPERIL_INT areaperil_id = -1) {
	std::map<int, OASIS_FLOAT> vulnerability_probability;
	if (areaperil_id == -1) {
		mergeAggregateVulnerability(aggregate_vulnerability_id, vulnerability_probability);
	}
	else {
		vulnerability_probability = _aggregate_vulnerabilities[aggregate_vulnerability_id][areaperil_id];
	}
	new_Vulnerability.resize(_num_damage_bins*_num_intensity_bins, 0.0);
	for (int i = 0; i < _num_damage_bins; ++i) {
		for (int c = 0; c < _num_intensity_bins; ++c) {
			int index = getVulnerabilityIndex(c + 1, i + 1);
			OASIS_FLOAT prob = 0.0;
			for (auto it = _vulnerabilities.begin(); it != _vulnerabilities.end(); ++it) {
				OASIS_FLOAT weight = vulnerability_probability[it->first];
				prob += weight * it->second[index];
			}
			new_Vulnerability[index] = prob;
		}
	}
}

void getmodel::getDamageBinDictionary() {
  FILE *fin = fopen(DAMAGE_BIN_DICT_FILE, "rb");
  if (fin == nullptr) {
    fprintf(stderr, "%s: cannot open %s\n", __func__, DAMAGE_BIN_DICT_FILE);
    exit(-1);
  }
  flseek(fin, 0L, SEEK_END);
  long long sz = fltell(fin);
  flseek(fin, 0L, SEEK_SET);
  int nrec = static_cast<int>(sz / sizeof(damagebindictionary));

  //damagebindictionary *damage_bins = new damagebindictionary[nrec];
  std::vector<damagebindictionary> damage_bins;
  damage_bins.resize(nrec);
  if (fread(&damage_bins[0], sizeof(damagebindictionary), nrec, fin) != nrec) {
    std::ostringstream poss;
    poss << "Error reading file " << DAMAGE_BIN_DICT_FILE;
    perror(poss.str().c_str());
    exit(-1);
  }

  fclose(fin);

  _mean_damage_bins = std::vector<OASIS_FLOAT>(nrec);
  for (int i = 0; i < nrec; i++) {
    _mean_damage_bins[i] = damage_bins[i].interpolation;
  }

  //delete[] damage_bins;
}


void getmodel::initOutputStream() {
  fwrite(&OUTPUT_STREAM_TYPE, sizeof(OUTPUT_STREAM_TYPE), 1, stdout);
}

void getmodel::doResults(
	int &event_id, AREAPERIL_INT &areaperil_id,
	std::vector<OASIS_FLOAT> intensity) {
	for (int vulnerability_id : _vulnerability_ids_by_area_peril[areaperil_id]) {
		//Result *results = new Result[_num_damage_bins];
		std::vector<Result> results;
		results.resize(_num_damage_bins);
		int result_index = 0;
		std::vector<OASIS_FLOAT> vulnerability;
		if (vulnerability_id < _agg_vul_start) {
			vulnerability = _vulnerabilities[vulnerability_id];
		}
		else {
			if (areaperil_id < _agg_ap_start) {
				newVulnerability(vulnerability_id, vulnerability, areaperil_id);
			}
			else {
				newVulnerability(vulnerability_id, vulnerability);
			}
		}
		int vulnerability_index = 0;
		double cumulative_prob = 0;
		for (int damage_bin_index = 0; damage_bin_index < _num_damage_bins;
			damage_bin_index++) {
			double prob = 0.0f;
			for (int intensity_bin_index = 0;
				intensity_bin_index < _num_intensity_bins; intensity_bin_index++) {
				prob += vulnerability[vulnerability_index++] *
					intensity[intensity_bin_index];
			}

			// if (prob > 0 || damage_bin_index == 0)
			//{
			cumulative_prob += prob;
			results[result_index++] = Result(static_cast<OASIS_FLOAT>(cumulative_prob),
				_mean_damage_bins[damage_bin_index]);
			//}
			if (cumulative_prob > 0.999999940)
				break; // single precision value approx 1
		}
		int num_results = result_index;
		fwrite(&event_id, SIZE_OF_INT, 1, stdout);
		fwrite(&areaperil_id, sizeof(areaperil_id), 1, stdout);
		fwrite(&vulnerability_id, SIZE_OF_INT, 1, stdout);
		fwrite(&num_results, SIZE_OF_INT, 1, stdout);
		fwrite(&results[0], SIZE_OF_RESULT, num_results, stdout);
		//delete[] results;

	}
}

void getmodel::doResultsNoIntensityUncertainty(
	int &event_id, AREAPERIL_INT &areaperil_id,
	int intensity_bin_index) {
	for (int vulnerability_id : _vulnerability_ids_by_area_peril[areaperil_id]) {
		std::vector<OASIS_FLOAT> vulnerability;
		if (vulnerability_id < _agg_vul_start) {
			vulnerability = _vulnerabilities[vulnerability_id];
		}
		else {
			if (areaperil_id < _agg_ap_start) {
				newVulnerability(vulnerability_id, vulnerability, areaperil_id);
			}
			else {
				newVulnerability(vulnerability_id, vulnerability);
			}
		}
		int result_index = 0;
		OASIS_FLOAT cumulative_prob = 0;
		for (int damage_bin_index = 1; damage_bin_index <= _num_damage_bins;
			damage_bin_index++) {
			OASIS_FLOAT prob = vulnerability[getVulnerabilityIndex(
				intensity_bin_index, damage_bin_index)];

			// if (prob > 0 || damage_bin_index == 0)
			//{
			cumulative_prob += prob;
			_temp_results[result_index++].prob = cumulative_prob;
			//}
		}

		int num_results = result_index;
		fwrite(&event_id, SIZE_OF_INT, 1, stdout);
		fwrite(&areaperil_id, sizeof(areaperil_id), 1, stdout);
		fwrite(&vulnerability_id, SIZE_OF_INT, 1, stdout);
		fwrite(&num_results, SIZE_OF_INT, 1, stdout);
		fwrite(&_temp_results[0], SIZE_OF_RESULT, num_results, stdout);
	}
}

int getmodel::getVulnerabilityIndex(int intensity_bin_index,
                                    int damage_bin_index) const {
  return (intensity_bin_index - 1) +
         ((damage_bin_index - 1) * _num_intensity_bins);
}

void getmodel::init(bool zip) {
  _zip = zip;

  getIntensityInfo();
  newItems();
  getVulnerabilities();
  getDamageBinDictionary();
  getFootPrints();
  initOutputStream();

  //_temp_results = new Result[_num_damage_bins];
  _temp_results.resize(_num_damage_bins);
  for (int damage_bin_index = 1; damage_bin_index <= _num_damage_bins; damage_bin_index++) {
    _temp_results[damage_bin_index - 1] =
        Result(0.0, _mean_damage_bins[damage_bin_index - 1]);
  }
}

void getmodel::doCdf(int event_id) {
	doCdfAggregate(event_id);

	if (_has_intensity_uncertainty) {
		if (_zip)
			doCdfInnerz(event_id);
		else
			doCdfInner(event_id);
	}
	else {
		if (_zip)
			doCdfInnerNoIntensityUncertaintyz(event_id);
		else
			doCdfInnerNoIntensityUncertainty(event_id);
	}
}

void getmodel::doCdfAggregate(int event_id) {
	auto intensity = std::vector<OASIS_FLOAT>(_num_intensity_bins, 0.0f);
	bool do_cdf_for_area_peril = false;
	AREAPERIL_INT current_areaperil_id = -1;

	for (AREAPERIL_INT aggregate_areaperil_id : _aggregate_areaperil_ids) {
		std::vector<EventRow> new_Footprint;
		newFootprint(event_id, aggregate_areaperil_id, new_Footprint);
		for (int i = 0; i < new_Footprint.size(); i++) {
			auto event_key = new_Footprint[i];
			if (event_key.areaperil_id != current_areaperil_id) {
				if (do_cdf_for_area_peril) {
					// Generate and write the results
					doResults(event_id, current_areaperil_id, intensity);
					intensity = std::vector<OASIS_FLOAT>(_num_intensity_bins, 0.0f);
				}
				current_areaperil_id = event_key.areaperil_id;
				do_cdf_for_area_peril = (_aggregate_areaperil_ids.count(current_areaperil_id) == 1);
			}
			if (do_cdf_for_area_peril) {
				intensity[event_key.intensity_bin_id - 1] = event_key.probability;
			}
		}
	}
	if (do_cdf_for_area_peril) {
		// Generate and write the results
		doResults(event_id, current_areaperil_id, intensity);
	}

}

void getmodel::doCdfInnerz(int event_id) {
	auto sizeof_EventKey = sizeof(EventRow);
	auto fin = fopen(ZFOOTPRINT_FILE, "rb");
	auto intensity = std::vector<OASIS_FLOAT>(_num_intensity_bins, 0.0f);
	bool do_cdf_for_area_peril = false;
	intensity = std::vector<OASIS_FLOAT>(_num_intensity_bins, 0.0f);
	if (_event_index_by_event_id.count(event_id) == 0)
		return;
	flseek(fin, _event_index_by_event_id[event_id].offset, 0);

	int size = _event_index_by_event_id[event_id].size;
	_compressed_buf.resize(size + 1);
	_uncompressed_buf.resize(size * 20);
	fread(&_compressed_buf[0], size, 1, fin);
	uLong dest_length = _uncompressed_buf.size();
	int ret = uncompress(&_uncompressed_buf[0], &dest_length, &_compressed_buf[0], size);
	if (ret != Z_OK) {
		fprintf(stderr, "Got bad return code from uncompress %d\n", ret);
		exit(-1);
	}
	// we now have the data length
	int number_of_event_records = static_cast<int>(dest_length / sizeof_EventKey);
	EventRow *event_key = (EventRow *)&_uncompressed_buf[0];
	AREAPERIL_INT current_areaperil_id = -1;
	for (int i = 0; i < number_of_event_records; i++) {
		if (event_key->areaperil_id != current_areaperil_id) {
			if (do_cdf_for_area_peril) {
				// Generate and write the results
				doResults(event_id, current_areaperil_id,
					intensity);
				intensity = std::vector<OASIS_FLOAT>(_num_intensity_bins, 0.0f);
			}
			current_areaperil_id = event_key->areaperil_id;
			do_cdf_for_area_peril = (_area_perils.count(current_areaperil_id) == 1);
		}
		if (do_cdf_for_area_peril) {
			intensity[event_key->intensity_bin_id - 1] = event_key->probability;
		}
		event_key++;
	}
  // Write out results for last event record
    if (do_cdf_for_area_peril) {
    // Generate and write the results
      doResults(event_id, current_areaperil_id, intensity);
    }
  _compressed_buf.clear();
  _uncompressed_buf.clear();
  fclose(fin);
}

void getmodel::doCdfInner(int event_id) {
	auto sizeof_EventKey = sizeof(EventRow);
	auto fin = fopen(FOOTPRINT_FILE, "rb");
	if (fin == NULL) {
		fprintf(stderr, "Error opening footprint file\n");
		exit(-1);
	}
	auto intensity = std::vector<OASIS_FLOAT>(_num_intensity_bins, 0.0f);

	AREAPERIL_INT current_areaperil_id = -1;
	bool do_cdf_for_area_peril = false;
	intensity = std::vector<OASIS_FLOAT>(_num_intensity_bins, 0.0f);

	if (_event_index_by_event_id.count(event_id) == 0) {
		fclose(fin);
		return;
	}
	flseek(fin, _event_index_by_event_id[event_id].offset, 0);
	EventRow event_key;
	int number_of_event_records = static_cast<int>(
		_event_index_by_event_id[event_id].size / sizeof_EventKey);
	for (int i = 0; i < number_of_event_records; i++) {
		fread(&event_key, sizeof(event_key), 1, fin);
		if (event_key.areaperil_id != current_areaperil_id) {
			if (do_cdf_for_area_peril) {
				// Generate and write the results
				doResults(event_id, current_areaperil_id, intensity);
				intensity = std::vector<OASIS_FLOAT>(_num_intensity_bins, 0.0f);
			}
			current_areaperil_id = event_key.areaperil_id;
			do_cdf_for_area_peril = (_area_perils.count(current_areaperil_id) == 1);
		}
		if (do_cdf_for_area_peril) {
			intensity[event_key.intensity_bin_id - 1] = event_key.probability;
		}
	}
	// Write out results for last event record
	if (do_cdf_for_area_peril) {
		// Generate and write the results
		doResults(event_id, current_areaperil_id, intensity);
	}
  fclose(fin);
}

void getmodel::doCdfInnerNoIntensityUncertaintyz(int event_id) {
	auto sizeof_EventKey = sizeof(EventRow);
	FILE *fin = fopen(ZFOOTPRINT_FILE, "rb");
	if (_event_index_by_event_id.count(event_id) == 0)
		return;
	flseek(fin, _event_index_by_event_id[event_id].offset, 0);
	int size = _event_index_by_event_id[event_id].size;
	_compressed_buf.resize(size + 1);
	_uncompressed_buf.resize(size * 20);
	fread(&_compressed_buf[0], size, 1, fin);
	uLong dest_length = _uncompressed_buf.size();
	int ret = uncompress(&_uncompressed_buf[0], &dest_length,
		&_compressed_buf[0], size);
	if (ret != Z_OK) {
		fprintf(stderr, "Got bad return code from uncompress %d\n", ret);
		exit(-1);
	}
	// we now have the data length
	int number_of_event_records =
		static_cast<int>(dest_length / sizeof_EventKey);
	EventRow *event_key = (EventRow *)&_uncompressed_buf[0];
	for (int i = 0; i < number_of_event_records; i++) {
		bool do_cdf_for_area_peril =
			(_area_perils.count(event_key->areaperil_id) == 1);
		if (do_cdf_for_area_peril) {
			// Generate and write the results
			doResultsNoIntensityUncertainty(
				event_id, event_key->areaperil_id, event_key->intensity_bin_id);
		}
		event_key++;
	}

  fclose(fin);
}

void getmodel::doCdfInnerNoIntensityUncertainty(int event_id) {
	auto sizeof_EventKey = sizeof(EventRow);

	if (_event_index_by_event_id.count(event_id) == 0)
		return;

	auto fin = fopen(FOOTPRINT_FILE, "rb");

	flseek(fin, _event_index_by_event_id[event_id].offset, SEEK_SET);

	int number_of_event_records = static_cast<int>(
		_event_index_by_event_id[event_id].size / sizeof_EventKey);
	for (int i = 0; i < number_of_event_records; i++) {
		EventRow event_key;
		fread(&event_key, sizeof(event_key), 1, fin);

		bool do_cdf_for_area_peril =
			(_area_perils.count(event_key.areaperil_id) == 1);
		if (do_cdf_for_area_peril) {
			// Generate and write the results
			doResultsNoIntensityUncertainty(
				event_id, event_key.areaperil_id, event_key.intensity_bin_id);
		}
	}

  fclose(fin);
}

