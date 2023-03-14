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
  inline size_t ReadOpts() {
    int opts = 0;
    size_t i = fread(&opts, sizeof(opts), 1, stdin);
    return i;
  }


  inline void WriteCounterAmplitudesAndFactors(const int counter,
    const std::vector<amplification_factor> &factors) {

    fwrite(&counter, sizeof(counter), 1, stdout);
    for (auto af : factors) fwrite(&af, sizeof(af), 1, stdout);

  }


  void DoIt() {

    size_t i = ReadOpts();

    event_count ec;
    amplification_factor af;

    fprintf(stdout, "event_id,amplification_id,factor\n");   // Header

    while (i = fread(&ec, sizeof(ec), 1, stdin) != 0) {

      for (int counter = 0; counter != ec.count; ++counter) {

        i = fread(&af, sizeof(af), 1, stdin);
	if (i == 0) {
	  fprintf(stderr, "FATAL: Incomplete data for Event ID = %d\n",
		  ec.event_id);
	  exit(EXIT_FAILURE);
	}

	fprintf(stdout, "%d,%d,%f\n", ec.event_id, af.amplification_id,
		af.factor);

      }

    }

  }

}
