#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

namespace quantiletobin {

  void doit(bool header) {

    float quantile;
    char line[4096];
    int lineno = 1;
    if (header) {
      fgets(line, sizeof(line), stdin);   // Skip header line
    }
    lineno++;

    while (fgets(line, sizeof(line), stdin) != 0) {

      if (sscanf(line, "%f", &quantile) != 1) {

	fprintf(stderr, "FATAL: Invalid data in line %d:\n%s\n", lineno, line);
	exit(EXIT_FAILURE);

      } else {
	      
	if (quantile > 1.0 || quantile < 0.0) {
	fprintf(stderr, "FATAL: Invalid quantile %f on line %d", quantile, lineno);
	fprintf(stderr, ": quantiles must lie between 0 and 1 - ignoring...\n");
	continue;
	}

	fwrite(&quantile, sizeof(quantile), 1, stdout);

      }
      lineno++;

    }

  }

}
