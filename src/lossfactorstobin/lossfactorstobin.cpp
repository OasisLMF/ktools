#include <iostream>
#include <stdio.h>
#include <vector>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../include/wingetopt.h"
#else
#include <unistd.h>
#endif


namespace lossfactorstobin {


  // First 4 bytes reserved for optional flags
  inline void WriteOpts() {
    int opts = 0;
    fwrite(&opts, sizeof(opts), 1, stdout);
  }


  inline void WriteCounterAmplitudesAndFactors(const int counter,
    const std::vector<amplification_factor> &factors) {

    fwrite(&counter, sizeof(counter), 1, stdout);
    for (auto af : factors) fwrite(&af, sizeof(af), 1, stdout);

  }


  void DoIt() {

    WriteOpts();

    int current_event_id = 0;
    int counter = 0;
    event_amplification ea;
    amplification_factor af;
    std::vector<amplification_factor> factors;
    float factor;
    char line[4096];

    fgets(line, sizeof(line), stdin);   // Skip header row
    int lineno = 2;

    while (fgets(line, sizeof(line), stdin) != 0) {

      if (sscanf(line, "%d,%d,%f", &ea.event_id, &ea.amplification_id,
		 &factor) != 3) {
	fprintf(stderr, "FATAL: Invalid data in line %d:\n%s\n", lineno, line);
	exit(EXIT_FAILURE);
      }

      if (ea.event_id != current_event_id) {
	if (current_event_id != 0) {
	  WriteCounterAmplitudesAndFactors(counter, factors);
	}
	fwrite(&ea.event_id, sizeof(ea.event_id), 1, stdout);
	current_event_id = ea.event_id;
	counter = 0;
	factors.clear();
      }

      af.amplification_id = ea.amplification_id;
      af.factor = factor;
      factors.push_back(af);
      ++counter;
      ++lineno;

    }

    WriteCounterAmplitudesAndFactors(counter, factors);

  }

}
