#include "../include/oasis.h"
#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif


namespace placalc {

  struct hash_event_amplification {
    size_t operator()(const event_amplification &ea) const {
      return ea.event_id * std::numeric_limits<int>::max() + ea.amplification_id;
    }
  };

  // Zeroth item ID factor = 0.0
  std::vector<int> item_to_amplification_(1, 0.0);
  std::unordered_map<event_amplification, float, hash_event_amplification> factors_;


  void LoadItemToAmplification() {

    FILE * fin = fopen(AMPLIFICATION_FILE, "rb");
    if (fin == nullptr) {
      fprintf(stderr, "FATAL: %s: Error opening file %s\n", __func__,
	      AMPLIFICATION_FILE);
      exit(EXIT_FAILURE);
    }

    int opts;
    size_t i = fread(&opts, sizeof(opts), 1, fin);

    item_amplification ia;
    int expectedItemID = 1;
    i = fread(&ia, sizeof(ia), 1, fin);
    while (i != 0) {
      item_to_amplification_.push_back(ia.amplification_id);
      if (ia.item_id != expectedItemID) {
	fprintf(stderr, "FATAL: %s: Item IDs are not contiguous. "
			"Expected item ID %d but read item ID %d\n",
		__func__, expectedItemID, ia.item_id);
	exit(EXIT_FAILURE);
      }
      ++expectedItemID;
      i = fread(&ia, sizeof(ia), 1, fin);
    }

    fclose(fin);

  }


  void LoadPostLossAmplificationFactors(const float secondaryFactor) {

    FILE *fin = fopen(PLAFACTORS_FILE, "rb");
    if (fin == nullptr) {
      fprintf(stderr, "FATAL: %s: Error opening file %s\n", __func__,
	      PLAFACTORS_FILE);
      exit(EXIT_FAILURE);
    }

    int opts;
    size_t i = fread(&opts, sizeof(opts), 1, fin);

    event_count ec;
    amplification_factor af;
    while (i != 0) {
      i = fread(&ec, sizeof(ec), 1, fin);
      if (i == 0) break;
      event_amplification ea;
      ea.event_id = ec.event_id;
      for (int amplification = 0; amplification != ec.count; ++amplification) {
	i = fread(&af, sizeof(af), 1, fin);
	if (i == 0) break;
	ea.amplification_id = af.amplification_id;
	factors_[ea] = 1. + (af.factor - 1.) * secondaryFactor;
      }
    }

  }
      

  void doit(const float relativeSecondaryFactor,
	    const float absoluteFactor) {

    LoadItemToAmplification();
    // Ignore loss factors file if absolute factor is specified
    if (absoluteFactor == 0.0) {
      LoadPostLossAmplificationFactors(relativeSecondaryFactor);
    }

    // Check input stream type is GUL item stream or loss stream and write type
    // to output
    int gulstreamType = 0;
    size_t i = fread(&gulstreamType, sizeof(gulstreamType), 1, stdin);
    int streamType = gulstreamType & gulstream_id;
    if (streamType == 0) {
      streamType = gulstreamType & loss_stream_id;
    }
    if (streamType != gulstream_id && streamType != loss_stream_id) {
      fprintf(stderr, "FATAL: placalc: %s: Not a valid gul stream type %d\n",
	      __func__, gulstreamType);
      exit(EXIT_FAILURE);
    }
    fwrite(&gulstreamType, sizeof(gulstreamType), 1, stdout);

    // Read in number of samples and write to output
    int sampleSize = 0;
    i = fread(&sampleSize, sizeof(sampleSize), 1, stdin);
    if (i == 0) {
      fprintf(stderr, "FATAL: placalc: %s: Invalid sample size %d\n", __func__,
	      sampleSize);
    }
    fwrite(&sampleSize, sizeof(sampleSize), 1, stdout);

    // Default factor is 1.0 or absolute factor given by user
    const float defaultFactor = (absoluteFactor == 0.0) ? 1.0 : absoluteFactor;

    // Read in data from GUL stream, apply Post Loss Amplification (PLA) factors
    // and write out to standard output
    while (i != 0) {
      event_amplification ea;
      gulSampleslevelHeader gh;
      i = fread(&gh, sizeof(gh), 1, stdin);
      if (i == 0) break;

      fwrite(&gh, sizeof(gh), 1, stdout);
      ea.event_id = gh.event_id;
      ea.amplification_id = item_to_amplification_[gh.item_id];
      // Default factor is used if post loss amplification factor is not present
      float factor = defaultFactor;
      auto iter = factors_.find(ea);
      if (iter != factors_.end()) factor = iter->second;

      gulSampleslevelRec gr;
      while ((i = fread(&gr, sizeof(gr), 1, stdin)) != 0) {
	gr.loss *= factor;
	fwrite(&gr, sizeof(gr), 1, stdout);
	if (gr.sidx == 0) break;
      }
    }

  }

}
