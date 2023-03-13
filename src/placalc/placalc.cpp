#include "../include/oasis.h"
#include <iostream>
#include <unordered_map>
#include <vector>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif


namespace placalc {

  struct item_amplitude {
    int item_id;
    int amplitude_id;
  };

  struct event_amplitude {
    int event_id;
    int amplitude_id;

    bool operator==(const event_amplitude &rhs) const {
      return event_id == rhs.event_id && amplitude_id == rhs.amplitude_id;
    }
  };

  struct hash_event_amplitude {
    long operator()(const event_amplitude &ea) const {
      return ea.event_id + ((long)ea.amplitude_id << 32);
    }
  };

  struct event_count {
    int event_id;
    int count;
  };

  struct amplitude_factor {
    int amplitude_id;
    float factor;
  };

  // Zeroth item ID factor = 0.0
  std::vector<int> item_to_amplitude_(1, 0.0);
  std::unordered_map<event_amplitude, float, hash_event_amplitude> factors_;


  void LoadItemToAmplitude() {

    FILE * fin = fopen(ITEMAMPLITUDE_FILE, "rb");
    if (fin == nullptr) {
      fprintf(stderr, "FATAL: %s: Error opening file %s\n", __func__,
	      ITEMAMPLITUDE_FILE);
      exit(EXIT_FAILURE);
    }

    int opts;
    size_t i = fread(&opts, sizeof(opts), 1, fin);

    item_amplitude ia;
    i = fread(&ia, sizeof(ia), 1, fin);
    while (i != 0) {
      item_to_amplitude_.push_back(ia.amplitude_id);
      i = fread(&ia, sizeof(ia), 1, fin);
    }

    fclose(fin);

  }


  void LoadPostLossAmplificationFactors() {

    FILE *fin = fopen(PLAFACTORS_FILE, "rb");
    if (fin == nullptr) {
      fprintf(stderr, "FATAL: %s: Error opening file %s\n", __func__,
	      PLAFACTORS_FILE);
      exit(EXIT_FAILURE);
    }

    int opts;
    size_t i = fread(&opts, sizeof(opts), 1, fin);

    event_count ec;
    amplitude_factor af;
    while (i != 0) {
      i = fread(&ec, sizeof(ec), 1, fin);
      if (i == 0) break;
      event_amplitude ea;
      ea.event_id = ec.event_id;
      for (int amplitude = 0; amplitude != ec.count; ++amplitude) {
	i = fread(&af, sizeof(af), 1, fin);
	if (i == 0) break;
	ea.amplitude_id = af.amplitude_id;
	factors_[ea] = af.factor;
      }
    }

  }
      

  void doit() {

    LoadItemToAmplitude();
    LoadPostLossAmplificationFactors();

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

    // Read in data from GUL stream, apply Post Loss Amplification (PLA) factors
    // and write out to standard output
    while (i != 0) {
      event_amplitude ea;
      gulSampleslevelHeader gh;
      i = fread(&gh, sizeof(gh), 1, stdin);
      if (i == 0) break;

      fwrite(&gh, sizeof(gh), 1, stdout);
      ea.event_id = gh.event_id;
      ea.amplitude_id = item_to_amplitude_[gh.item_id];
      float factor = 1.0;   // Assume factor = 1.0 if not present
      auto iter = factors_.find(ea);
      if (iter != factors_.end()) factor = iter->second;

      while (i != 0) {
	gulSampleslevelRec gr;
	i = fread(&gr, sizeof(gr), 1, stdin);
	gr.loss *= factor;
	fwrite(&gr, sizeof(gr), 1, stdout);
	if (gr.sidx == 0) break;
      }
    }

  }

}
