#include <cstring>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

namespace validatedamagebin {

  void doit() {

    damagebindictionary p = {}, q;
    bool dataValid = true;
    char line[4096], prevLine[4096];
    sprintf(prevLine, "%d, %f, %f, %f, %d", p.bin_index, p.bin_from, p.bin_to,
	    p.interpolation, p.interval_type);
    int lineno = 0;
    fgets(line, sizeof(line), stdin);
    lineno++;

    while(fgets(line, sizeof(line), stdin) != 0) {

      // Check each line has five values
      if(sscanf(line, "%d,%f,%f,%f,%d", &q.bin_index, &q.bin_from, &q.bin_to,
		&q.interpolation, &q.interval_type) != 5) {

        fprintf(stderr, "Invalid data in line %d:\n%s\n", lineno, line);
	dataValid = false;

      } else if(lineno == 1) {

	// Issue warning if first bin lower limit != 0
	if(q.bin_from != 0.0) {

          fprintf(stderr, "Lower limit for first bin is not 0.");
	  fprintf(stderr, " Are you sure this is what you want?:\n");
	  fprintf(stderr, "%s\n", line);

	}

	// First bin index != 1
	if(q.bin_index != 1) {

	  fprintf(stderr, "First bin index must be 1.\n");
	  fprintf(stderr, "%s\n", line);
	  dataValid = false;

	}

      } else {

	// Non-contiguous bin indices
	if(p.bin_index != q.bin_index-1) {

	  fprintf(stderr, "Non-contiguous bin indices in lines %d and %d:\n",
		  lineno-1, lineno);
	  fprintf(stderr, "%s\n%s\n", prevLine, line);
	  dataValid = false;

	}

	// Interpolation damage values outside range
	if(q.interpolation < q.bin_from || q.bin_to < q.interpolation) {
	
	  fprintf(stderr, "Interpolation damage value %f for bin %d",
		 q.interpolation, q.bin_index);
	  fprintf(stderr, " lies outside range [%f, %f]:\n",
		 q.bin_from, q.bin_to);
	  fprintf(stderr, "%s\n", line);
	  dataValid = false;

	}

      }

      lineno++;
      p = q;
      memcpy(prevLine, line, strlen(line)+1);
      prevLine[strlen(line)-1] = '\0';

    }

    // Issue warning if last bin upper limit != 1
    if(q.bin_to != 1.0) {

      fprintf(stderr, "Upper limit for last bin is not 1.");
      fprintf(stderr, " Are you sure this is what you want?:\n");
      fprintf(stderr, "%s\n", line);

    }

    if(dataValid == true) {

      fprintf(stderr, "All checks pass. Please take note of any warnings.\n");

    } else {

      fprintf(stderr, "Some checks have failed. Please edit input file.\n");

    }

  }

}
