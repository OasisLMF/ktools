#include <cmath>
#include <cstring>
#include <iostream>
#include <set>
#include <stdio.h>
#include <stdlib.h>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

struct FootprintRow : EventRow {

  int event_id;

};

namespace validatefootprint {

  inline bool ProbabilityCheck(float prob) {

    return roundf(prob * 100000) / 100000 != 1.0;

  }

  inline bool ProbabilityError(FootprintRow r, float prob) {
  
    fprintf(stderr, "Probabilities for event ID %d", r.event_id);
    fprintf(stderr, " and areaperil ID %d", r.areaperil_id);
    fprintf(stderr, " do not sum to 1.\n");
    fprintf(stderr, "Probability = %f\n", prob);
    
    return false;

  }

  inline bool OutOfOrderError(char const * name, int lineno, char * prevLine,
		  	      char * line) {

    fprintf(stderr, "%s IDs in lines %d and %d", name, lineno-1, lineno);
    fprintf(stderr, " not in ascending order:\n");
    fprintf(stderr, "%s\n%s\n", prevLine, line);

    return false;

  }

  void doit() {

    FootprintRow p = {}, q;
    bool dataValid = true;
    std::set<int> intensityBins;
    int maxIntensityBin = 0;
    char const * sEvent = "Event";
    char const * sAreaperil = "Areaperil";
    float prob = 0.0;
    char prevLine[4096], line[4096];
    sprintf(prevLine, "%d, %d, %d, %f", p.event_id, p.areaperil_id,
	    p.intensity_bin_id, p.probability);
    int lineno = 0;
    fgets(line, sizeof(line), stdin);   // Skip header line
    lineno++;

    while(fgets(line, sizeof(line), stdin) != 0) {

      // Check for invalid data
      if(sscanf(line, "%d,%d,%d,%f", &q.event_id, &q.areaperil_id,
		&q.intensity_bin_id, &q.probability) != 4) {

	fprintf(stderr, "Invalid data in line %d:\n%s\n", lineno, line);
	dataValid = false;

      }

      // New event ID
      if(q.event_id != p.event_id) {

	// Check total probability for event-areaperil combination is 1.0
	if(ProbabilityCheck(prob) && p.event_id != 0) {
	
	  dataValid = ProbabilityError(p, prob);

	}

	intensityBins.clear();
	prob = 0.0;

        // Check event IDs listed in ascending order
        if(q.event_id < p.event_id) {

	  dataValid = OutOfOrderError(sEvent, lineno, prevLine, line);

        }

      } else if(q.areaperil_id != p.areaperil_id) {

	// Check total probability for event-areaperil combination is 1.0
	if(ProbabilityCheck(prob)) {

	  dataValid = ProbabilityError(p, prob);

	}

	intensityBins.clear();
	prob = 0.0;

	// Check areaperil IDs listed in ascending order
	if(q.areaperil_id < p.areaperil_id) {

	  dataValid = OutOfOrderError(sAreaperil, lineno, prevLine, line);

	}

      }

      // Check no duplicate intensity bins for each event-areaperil combination
      if(intensityBins.find(q.intensity_bin_id) == intensityBins.end()) {

	intensityBins.insert(q.intensity_bin_id);
	prob += q.probability;

	// Get maximum value of intensity_bin_index
	if(q.intensity_bin_id > maxIntensityBin) {

	  maxIntensityBin = q.intensity_bin_id;

	}

      } else {

	fprintf(stderr, "Duplicate intensity bin for event-areaperil");
	fprintf(stderr, " combination:\n");
	fprintf(stderr, "%s\n", line);
	dataValid = false;

      }

      lineno++;
      p = q;
      memcpy(prevLine, line, strlen(line)+1);
      prevLine[strlen(line)-1] = '\0';

    }

    // Check total probability for last event-areaperil combination is 1.0
    if(ProbabilityCheck(prob)) {

      dataValid = ProbabilityError(q, prob);

    }

    if(dataValid == true) {

      fprintf(stderr, "All checks pass.\n");
      fprintf(stderr, "Maximum value of intensity_bin_index = %d\n", maxIntensityBin);

    } else {

      fprintf(stderr, "Some checks have failed. Please edit input file.\n");

    }

  }

}
