#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

namespace ensembletobin {

  void doit() {

    Ensemble q;
    char line[4096];
    int lineno = 0;
    fgets(line, sizeof(line), stdin);   // Read column headings
    lineno++;

    // Read data
    while(fgets(line, sizeof(line), stdin) != 0) {
      if (sscanf(line, "%d,%d,%d", &q.sidx, &q.ensemble_id, &q.sample_id) != 3)
      {

	fprintf(stderr, "FATAL: Invalid data in line %d:\n%s", lineno, line);
	return;

      } else {

	fwrite(&q, sizeof(q), 1, stdout);

      }

      lineno++;

    }

  }

}
