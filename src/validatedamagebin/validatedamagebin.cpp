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

  inline bool checkfirstbin(float bin_from, int bin_index, char * line) {

    bool dataValid = true;

    // Issue warning if first bin lower limit != 0
    if(bin_from != 0.0) {

      fprintf(stderr, "Lower limit for first bin is not 0.");
      fprintf(stderr, " Are you sure this is what you want?:\n");
      fprintf(stderr, "%s\n", line);

    }

    // First bin index != 1
    if(bin_index != 1) {

      fprintf(stderr, "First bin index must be 1.\n");
      fprintf(stderr, "%s\n", line);
      dataValid = false;

    }

    return dataValid;

  }

  void doit() {

    damagebindictionary p = {}, q;
    bool dataValid = true;
    char line[4096], prevLine[4096];
    int lineno = 0;
    fgets(line, sizeof(line), stdin);   // Skip header

    // Read first line - this will establish format
    lineno++;
    bool newFormat;
    if (fgets(line, sizeof(line), stdin) != 0) {
      // Deprecated format
#ifdef OASIS_FLOAT_TYPE_DOUBLE
      if (sscanf(line, "%d,%lf,%lf,%lf,%d", &p.bin_index, &p.bin_from, &p.bin_to,
		 &p.interpolation, &p.interval_type) == 5) {
#else
      if (sscanf(line, "%d,%f,%f,%f,%d", &p.bin_index, &p.bin_from, &p.bin_to,
		 &p.interpolation, &p.interval_type) == 5) {
#endif

	fprintf(stderr, "Deprecated format:"
			" interval_type column no longer required.\n");
	newFormat = false;
	dataValid = checkfirstbin(p.bin_from, p.bin_index, line);

      }
      // New format
#ifdef OASIS_FLOAT_TYPE_DOUBLE
      else if (sscanf(line, "%d,%lf,%lf,%lf", &p.bin_index, &p.bin_from, &p.bin_to,
		      &p.interpolation) == 4) {
#else
      else if (sscanf(line, "%d,%f,%f,%f", &p.bin_index, &p.bin_from, &p.bin_to,
		      &p.interpolation) == 4) {
#endif

	p.interval_type = 0;
	newFormat = true;
	dataValid = checkfirstbin(p.bin_from, p.bin_index, line);

      }
      // Does not fit either format
      else {

	fprintf(stderr, "Invalid data in line %d:\n%s\n", lineno, line);
	dataValid = false;

      }
    } else {
      fprintf(stderr, "Empty file\n");
      exit(EXIT_FAILURE);
    }

    sprintf(prevLine, "%d, %f, %f, %f, %d", p.bin_index, p.bin_from, p.bin_to,
	    p.interpolation, p.interval_type);
    lineno++;

    while(fgets(line, sizeof(line), stdin) != 0) {

      bool furtherChecks = true;

#ifdef OASIS_FLOAT_TYPE_DOUBLE
      if (sscanf(line, "%d,%lf,%lf,%lf,%d", &q.bin_index, &q.bin_from, &q.bin_to,
		 &q.interpolation, &q.interval_type) == 5) {
#else
      if (sscanf(line, "%d,%f,%f,%f,%d", &q.bin_index, &q.bin_from, &q.bin_to,
		 &q.interpolation, &q.interval_type) == 5) {
#endif

	if (newFormat) {
	  fprintf(stderr, "Deprecated format used in line %d:\n%s\n",
		  lineno, line);
	  dataValid = false;
	}

      }
#ifdef OASIS_FLOAT_TYPE_DOUBLE
      else if (sscanf(line, "%d,%lf,%lf,%lf", &q.bin_index, &q.bin_from,
			&q.bin_to, &q.interpolation) == 4) {
#else
      else if (sscanf(line, "%d,%f,%f,%f", &q.bin_index, &q.bin_from,
			&q.bin_to, &q.interpolation) == 4) {
#endif

	if (!newFormat) {
	  fprintf(stderr, "Missing interval_type column in line %d:\n%s\n",
		  lineno, line);
	  dataValid = false;
	}
	q.interval_type = 0;

      } else {

	fprintf(stderr, "Invalid data in line %d:\n%s\n", lineno, line);
	dataValid = false;
	furtherChecks = false;

      }

      if (furtherChecks) {

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
