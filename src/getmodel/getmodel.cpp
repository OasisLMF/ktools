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
*     distribution.
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
#ifndef _MSC_VER
#include <zlib.h>
#endif
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

// Is this the number of damage bins?
//const int MAX_RESULTS = 100;
//const size_t SIZE_OF_OASIS_FLOAT = sizeof(OASIS_FLOAT);

const size_t SIZE_OF_INT = sizeof(int);
const size_t SIZE_OF_RESULT = sizeof(Result);

const unsigned int OUTPUT_STREAM_TYPE = 1;

getmodel::getmodel() {
  //
}

getmodel::~getmodel() {
  //if (_temp_results != nullptr)
    //delete _temp_results;
}

inline void getmodel::assignProbabilities(const int vulnerability_id,
  int& current_vulnerability_id, const int intensity_bin_id,
  const int damage_bin_id, OASIS_FLOAT probability) {

  if (_num_intensity_bins >= intensity_bin_id) {

    if (vulnerability_id != current_vulnerability_id) {
      _vulnerabilities[vulnerability_id] =
	std::vector<OASIS_FLOAT>(_num_intensity_bins * _num_damage_bins, 0.0);
      current_vulnerability_id = vulnerability_id;
    }

    int vulnerabilityIndex = getVulnerabilityIndex(intensity_bin_id,
		    				   damage_bin_id);
    _vulnerabilities[vulnerability_id][vulnerabilityIndex] = probability;

  }

}

// only get those vulnerabilities that exist in the items file - so reduce
// memory footprint
void getmodel::getVulnerabilities(const std::set<int> &v) {
  // Read the vulnerabilities

  char vulnerabilityFilename[4096];
  // Determine whether an index file exists
  FILE *fidx = 0;
  bool zip = false;
  fidx = fopen(ZVULNERABILITY_IDX_FILE, "rb");   // zip
  if (fidx != nullptr) {
    strcpy(vulnerabilityFilename, ZVULNERABILITY_FILE);
    zip = true;
  } else {
    strcpy(vulnerabilityFilename, VULNERABILITY_FILE);
    fidx = fopen(VULNERABILITY_IDX_FILE, "rb");   // uncompressed index
  }

  FILE *fin = fopen(vulnerabilityFilename, "rb");
  if (fin == nullptr) {
    fprintf(stderr, "FATAL: %s: cannot open %s\n", __func__, vulnerabilityFilename);
    exit(EXIT_FAILURE);
  }
  int current_vulnerability_id = -1;
  fread(&_num_damage_bins, sizeof(_num_damage_bins), 1, fin);

  if (fidx == nullptr) {   // no index file
    Vulnerability vulnerability;

    while (fread(&vulnerability, sizeof(vulnerability), 1, fin) != 0) {
      if (v.find(vulnerability.vulnerability_id) !=
	  v.end()) { // only process those vulnerabilities that are in the item
		     // file
	assignProbabilities(vulnerability.vulnerability_id,
			    current_vulnerability_id,
			    vulnerability.intensity_bin_id,
			    vulnerability.damage_bin_id,
			    vulnerability.probability);
      }
    }

  } else if (!zip) {   // index file, no compression
    VulnerabilityIndex idx;
    VulnerabilityRow row;

    while (fread(&idx, sizeof(idx), 1, fidx) != 0) {
      if (v.find(idx.vulnerability_id) != v.end()) {
	flseek(fin, idx.offset, SEEK_SET);
	long long i = 0;
	while (i < idx.size) {
	  fread(&row, sizeof(row), 1, fin);
	  assignProbabilities(idx.vulnerability_id, current_vulnerability_id,
			      row.intensity_bin_id, row.damage_bin_id,
			      row.probability);
	  i += sizeof(row);
	}
      }
    }
    fclose(fidx);

  } else {   // compressed vulnerability file
    VulnerabilityIndex idx;

    while (fread(&idx, sizeof(idx), 1, fidx) != 0) {
      if (v.find(idx.vulnerability_id) != v.end()) {
	flseek(fin, idx.offset, SEEK_SET);

	std::vector<unsigned char> compressedBuffer;
	compressedBuffer.resize(idx.size + 1);
	fread(&compressedBuffer[0], idx.size, 1, fin);

	std::vector<unsigned char> uncompressedBuffer;
	uncompressedBuffer.resize(idx.original_size);
	uLongf destLen = (uLongf)uncompressedBuffer.size();

	int ret = uncompress(&uncompressedBuffer[0], &destLen,
			     &compressedBuffer[0], idx.size);
	if (ret != Z_OK) {
	  fprintf(stderr, "FATAL: Got bad return code from uncompress %d\n", ret);
	  exit(EXIT_FAILURE);
	}

	VulnerabilityRow *row = (VulnerabilityRow*)&uncompressedBuffer[0];
	long long i = 0;
	while (i < idx.original_size) {
	  assignProbabilities(idx.vulnerability_id, current_vulnerability_id,
			      row->intensity_bin_id, row->damage_bin_id,
			      row->probability);
	  row++;
	  i += sizeof(VulnerabilityRow);
	}
      }
    }
    fclose(fidx);

  }
  fclose(fin);

}

void getmodel::getIntensityInfo() {

	std::string filename = FOOTPRINT_FILE;

	if (_zip) {
		filename = ZFOOTPRINT_FILE;
	}
	FILE *fin = fopen(filename.c_str(), "rb");
	if (fin == nullptr) {
		fprintf(stderr, "FATAL: %s: cannot open %s\n", __func__, filename.c_str());
		exit(EXIT_FAILURE);
	}
	fread(&_num_intensity_bins, sizeof(_num_intensity_bins), 1, fin);
	fread(&_has_intensity_uncertainty, sizeof(_has_intensity_uncertainty), 1,fin);
	fclose(fin);

	if (_zip) {
		// Establish whether uncompressed data size is in index file
		int uncompressedMask = 1 << 1;
		_uncompressed_size =
		    (_has_intensity_uncertainty & uncompressedMask) >> 1;

		// Establish whether data has intensity uncertainty
		int intensityMask = 1;
		_has_intensity_uncertainty =
		    (_has_intensity_uncertainty & intensityMask);
	}

}

void getmodel::getItems(std::set<int> &v) {
  // Read the exposures and generate a set of vulnerabilities by area peril
  item item_rec;

  FILE *fin = fopen(ITEMS_FILE, "rb");
  if (fin == nullptr) {
    fprintf(stderr, "FATAL: %s: cannot open %s\n", __func__, ITEMS_FILE);
    exit(EXIT_FAILURE);
  }

  while (fread(&item_rec, sizeof(item_rec), 1, fin) != 0) {
    if (_vulnerability_ids_by_area_peril.count(item_rec.areaperil_id) == 0)
      _vulnerability_ids_by_area_peril[item_rec.areaperil_id] = std::set<int>();
    _vulnerability_ids_by_area_peril[item_rec.areaperil_id].insert(
        item_rec.vulnerability_id);
    _area_perils.insert(item_rec.areaperil_id);
    v.insert(item_rec.vulnerability_id);
  }
  fclose(fin);
}

void getmodel::getDamageBinDictionary() {
  FILE *fin = fopen(DAMAGE_BIN_DICT_FILE, "rb");
  if (fin == nullptr) {
    fprintf(stderr, "FATAL: %s: cannot open %s\n", __func__, DAMAGE_BIN_DICT_FILE);
    exit(-1);
  }
  flseek(fin, 0L, SEEK_END);
  long long sz = fltell(fin);
  flseek(fin, 0L, SEEK_SET);
  size_t nrec = (sz / sizeof(damagebindictionary));

  //damagebindictionary *damage_bins = new damagebindictionary[nrec];
  std::vector<damagebindictionary> damage_bins;
  damage_bins.resize(nrec);
  if (fread(&damage_bins[0], sizeof(damagebindictionary), nrec, fin) != nrec) {
    std::ostringstream poss;
    poss << "FATAL: Error reading file " << DAMAGE_BIN_DICT_FILE;
    perror(poss.str().c_str());
    exit(-1);
  }

  fclose(fin);

  _mean_damage_bins = std::vector<OASIS_FLOAT>(nrec);
  for (size_t i = 0; i < nrec; i++) {
    _mean_damage_bins[i] = damage_bins[i].interpolation;
  }

  //delete[] damage_bins;
}

void getmodel::getFootPrints(){
	FILE *fin;
	std:: string filename = FOOTPRINT_IDX_FILE;

	if (_zip) {
		filename = ZFOOTPRINT_IDX_FILE;
	}

	fin = fopen(filename.c_str(), "rb");
	if (fin == nullptr) {
		fprintf(stderr, "FATAL: %s: cannot open %s\n", __func__, filename.c_str());
		exit(EXIT_FAILURE);
	}

	EventIndex event_index;
	while (fread(&event_index, sizeof(event_index), 1, fin) != 0) {
		_event_index_by_event_id[event_index.event_id] = event_index;
		if (_uncompressed_size) {
			long long uncompressedSize;
			fread(&uncompressedSize, sizeof(uncompressedSize),
			      1, fin);
			_uncompressed_size_by_event_id[event_index.event_id] =
			    uncompressedSize;
		}
	}

	fclose(fin);
}

void getmodel::initOutputStream() {
  fwrite(&OUTPUT_STREAM_TYPE, sizeof(OUTPUT_STREAM_TYPE), 1, stdout);
}

void getmodel::doResults(
  int &event_id, AREAPERIL_INT &areaperil_id,
  std::map<AREAPERIL_INT, std::set<int>> &vulnerabilities_by_area_peril,
  std::map<int, std::vector<OASIS_FLOAT>> &vulnerabilities,
  std::map<int, OASIS_FLOAT> intensity) const {

  for (int vulnerability_id : vulnerabilities_by_area_peril[areaperil_id]) {
    std::vector<Result> results;
    results.resize(_num_damage_bins);
    int result_index = 0;
    std::vector<OASIS_FLOAT> vulnerability;
    try {
      vulnerability = vulnerabilities.at(vulnerability_id);
    } catch (const std::out_of_range) {
      fprintf(stderr, "FATAL: %s: vulnerability ID %d is out of range. "
		      "Check items and vulnerability files.\n",
	      __func__, vulnerability_id);
      exit(-1);
    }
    double cumulative_prob = 0;
    for (int damage_bin_index = 1; damage_bin_index <= _num_damage_bins;
         damage_bin_index++) {
      double prob = 0.0f;
      for (auto i : intensity) {
	prob += vulnerability[getVulnerabilityIndex(i.first, damage_bin_index)] *
		i.second;
      }

      cumulative_prob += prob;
      results[result_index++] = Result(static_cast<OASIS_FLOAT>(cumulative_prob),
                                       _mean_damage_bins[damage_bin_index-1]);

      if (cumulative_prob > 0.999999940)
        break; // single precision value approx 1
    }

    int num_results = result_index;
    fwrite(&event_id, SIZE_OF_INT, 1, stdout);
    fwrite(&areaperil_id, sizeof(areaperil_id), 1, stdout);
    fwrite(&vulnerability_id, SIZE_OF_INT, 1, stdout);
    fwrite(&num_results, SIZE_OF_INT, 1, stdout);
    fwrite(&results[0], SIZE_OF_RESULT, num_results, stdout);
  }

}

void getmodel::doResultsNoIntensityUncertainty(
    int &event_id, AREAPERIL_INT &areaperil_id,
    std::map<AREAPERIL_INT, std::set<int>> &vulnerabilities_by_area_peril,
    std::map<int, std::vector<OASIS_FLOAT>> &vulnerabilities,
    int intensity_bin_index)  {
  for (int vulnerability_id : vulnerabilities_by_area_peril[areaperil_id]) {
    int result_index = 0;
    OASIS_FLOAT cumulative_prob = 0;
    for (int damage_bin_index = 1; damage_bin_index <= _num_damage_bins;
         damage_bin_index++) {
      OASIS_FLOAT prob = vulnerabilities[vulnerability_id][getVulnerabilityIndex(
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

  std::set<int> v; // set of vulnerabilities;
  getItems(v);
  getVulnerabilities(v);
  v.clear(); // set no longer required release memory
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

void getmodel::doCdf(FILE* fin, int event_id) {
  if (_has_intensity_uncertainty) {
	  if (_zip) {
#ifndef _MSC_VER
		  doCdfInnerz(fin, event_id);
#else
		  fprintf(stderr, "FATAL: zip not supported with microsoft build\n");
		  exit(-1);
#endif
	  }
	  else {
		  doCdfInner(fin, event_id);
	  }
  } else {
	  if (_zip) {
#ifndef _MSC_VER
		  doCdfInnerNoIntensityUncertaintyz(fin, event_id);
#else
		  fprintf(stderr, "FATAL: zip not supported with microsft build\n");
		  exit(-1);
#endif
	  }else {
		  doCdfInnerNoIntensityUncertainty(fin, event_id);
	  }
  }
}
#ifndef _MSC_VER
void getmodel::doCdfInnerz(FILE* fin, int event_id) {
  auto sizeof_EventKey = sizeof(EventRow);
  std::map<int, OASIS_FLOAT> intensity;
  bool do_cdf_for_area_peril = false;
  if (_event_index_by_event_id.count(event_id) == 0)
    return;
  flseek(fin, _event_index_by_event_id[event_id].offset, 0);

  int size = _event_index_by_event_id[event_id].size;
  _compressed_buf.resize(size + 1);
  // If uncompressed size available, use that for length of destination buffer
  if (_uncompressed_size) {
    _uncompressed_buf.resize(_uncompressed_size_by_event_id[event_id]);
  } else {   // Otherwise assume compression ratio no greater than 20:1
    _uncompressed_buf.resize(size * 20);
  }
  fread(&_compressed_buf[0], size, 1, fin);
  uLong dest_length = _uncompressed_buf.size();
  int ret = uncompress(&_uncompressed_buf[0], &dest_length, &_compressed_buf[0], size);
  if (ret != Z_OK) {
    fprintf(stderr, "FATAL: Got bad return code from uncompress %d\n", ret);
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
                  _vulnerability_ids_by_area_peril, _vulnerabilities,
                  intensity);
	intensity.clear();
      }
      current_areaperil_id = event_key->areaperil_id;
      do_cdf_for_area_peril = (_area_perils.count(current_areaperil_id) == 1);
    }
    if (do_cdf_for_area_peril) {
      if (event_key->probability > 0) {
	intensity[event_key->intensity_bin_id] = event_key->probability;
      }
    }      
    event_key++;
  }
  // Write out results for last event record
    if (do_cdf_for_area_peril) {
    // Generate and write the results
      doResults(event_id, current_areaperil_id,_vulnerability_ids_by_area_peril, _vulnerabilities, intensity);
    }
  _compressed_buf.clear();
  _uncompressed_buf.clear();
}
#endif
void getmodel::doCdfInner(FILE* fin, int event_id) {

  auto sizeof_EventKey = sizeof(EventRow);

  std::map<int, OASIS_FLOAT> intensity;

  AREAPERIL_INT current_areaperil_id = -1;
  bool do_cdf_for_area_peril = false;

  if (_event_index_by_event_id.count(event_id) == 0){
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
        doResults(event_id, current_areaperil_id, _vulnerability_ids_by_area_peril, _vulnerabilities, intensity);
	intensity.clear();
      }
      current_areaperil_id = event_key.areaperil_id;
      do_cdf_for_area_peril = (_area_perils.count(current_areaperil_id) == 1);
    }
    if (do_cdf_for_area_peril) {
      if (event_key.probability > 0) {
	intensity[event_key.intensity_bin_id] = event_key.probability;
      }
    }
  }
  // Write out results for last event record
  if (do_cdf_for_area_peril) {
    // Generate and write the results
    doResults(event_id, current_areaperil_id, _vulnerability_ids_by_area_peril, _vulnerabilities, intensity);
  }
}

#ifndef _MSC_VER
void getmodel::doCdfInnerNoIntensityUncertaintyz(FILE* fin, int event_id) {
  auto sizeof_EventKey = sizeof(EventRow);
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
    fprintf(stderr, "FATAL: Got bad return code from uncompress %d\n", ret);
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
          event_id, event_key->areaperil_id, _vulnerability_ids_by_area_peril,
          _vulnerabilities, event_key->intensity_bin_id);
    }
    event_key++;
  }
}
#endif

void getmodel::doCdfInnerNoIntensityUncertainty(FILE* fin, int event_id) {
  auto sizeof_EventKey = sizeof(EventRow);

  if (_event_index_by_event_id.count(event_id) == 0)
    return;

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
          event_id, event_key.areaperil_id, _vulnerability_ids_by_area_peril,
          _vulnerabilities, event_key.intensity_bin_id);
    }
  }
}

void doIt(bool zip)
{

	getmodel cdf_generator;

	cdf_generator.init(zip);

  std::string footprint_filename = FOOTPRINT_FILE;

	if (zip) {
		footprint_filename = ZFOOTPRINT_FILE;
	}

  FILE* fin = fopen(footprint_filename.c_str(), "rb");
  if (fin == NULL) {
      fprintf(stderr, "FATAL: Error opening footprint file: %s\n", footprint_filename.c_str());
      exit(-1);
  }

	int event_id = -1;
	while (fread(&event_id, sizeof(event_id), 1, stdin) != 0)
	{
		cdf_generator.doCdf(fin, event_id);
	}

  fclose(fin);
}
