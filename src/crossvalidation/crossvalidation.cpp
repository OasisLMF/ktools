#include <cstring>
#include <fstream>
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

namespace crossvalidation {

  void doit(char const * damagebinFileName, char const * footprintFileName,
	    char const * vulnerabilityFileName) {

    // Go through damage bin dict file to create set of damage bin indexes
    char line[4096];
    int lineno = 0;
    bool dataValid = true;
    damagebindictionary d;
    std::set<int> damageBins;

    FILE * damagebinFile;
    damagebinFile = fopen(damagebinFileName, "r");
    if(damagebinFile == NULL) {
      fprintf(stderr, "Error opening %s\n", damagebinFileName);
      return;
    }

    fgets(line, sizeof(line), damagebinFile);   // Skip header
    lineno++;
    damageBins.clear();
    while(fgets(line, sizeof(line), damagebinFile) != 0) {

      // No need to check for deprecated format here as this is done by
      // validatedamagebin; the bin indexes are the only values required
#ifdef OASIS_FLOAT_TYPE_DOUBLE
      if(sscanf(line, "%d,%lf,%lf,%lf", &d.bin_index, &d.bin_from, &d.bin_to,
		&d.interpolation) != 4) {
#else
      if(sscanf(line, "%d,%f,%f,%f", &d.bin_index, &d.bin_from, &d.bin_to,
		&d.interpolation) != 4) {
#endif

	fprintf(stderr, "File %s\n", damagebinFileName);
	fprintf(stderr, "Invalid data in line %d:\n%s\n", lineno, line);
	return;

      } else {

	damageBins.insert(d.bin_index);

      }

      lineno++;

    }

    fclose(damagebinFile);

    // Go through vulnerability file to create set of intensity bin IDs while
    // checking damage bin IDs are subset of those in damage bin dict file
    lineno = 0;
    Vulnerability v;
    std::set<int> intensityBins;

    FILE * vulnerabilityFile;
    vulnerabilityFile = fopen(vulnerabilityFileName, "r");
    if(vulnerabilityFile == NULL) {
      fprintf(stderr, "Error opening %s\n", vulnerabilityFileName);
      return;
    } else {
      fprintf(stderr, "Checking %s ...\n", vulnerabilityFileName);
    }

    fgets(line, sizeof(line), vulnerabilityFile);   // Skip header
    lineno++;
    intensityBins.clear();
    while(fgets(line, sizeof(line), vulnerabilityFile) != 0) {

#ifdef OASIS_FLOAT_TYPE_DOUBLE
      if(sscanf(line, "%d,%d,%d,%lf", &v.vulnerability_id, &v.intensity_bin_id,
		&v.damage_bin_id, &v.probability) != 4) {
#else
      if(sscanf(line, "%d,%d,%d,%f", &v.vulnerability_id, &v.intensity_bin_id,
		&v.damage_bin_id, &v.probability) != 4) {
#endif

	fprintf(stderr, "File %s\n", vulnerabilityFileName);
	fprintf(stderr, "Invalid data in line %d:\n%s\n", lineno, line);
	return;

      } else {

	intensityBins.insert(v.intensity_bin_id);

	if(damageBins.find(v.damage_bin_id) == damageBins.end()) {
	
	  fprintf(stderr, "Unknown damage bin ID %d", v.damage_bin_id);
	  fprintf(stderr, " in line %d:\n%s\n", lineno, line);
	  dataValid = false;

	}

      }

      lineno++;

    }

    fprintf(stderr, "... Done.\n");
    fclose(vulnerabilityFile);

    // Go through footprint file to check intensity bin IDs are subset of those
    // in vulnerability file
    lineno = 0;
    FootprintRow f;

    FILE * footprintFile;
    footprintFile = fopen(footprintFileName, "r");
    if(footprintFile == NULL) {
      fprintf(stderr, "Error opening %s\n", footprintFileName);
      return;
    } else {
      fprintf(stderr, "Checking %s ...\n", footprintFileName);
    }

    fgets(line, sizeof(line), footprintFile);   // Skip header
    lineno++;
    while(fgets(line, sizeof(line), footprintFile) != 0) {

#ifdef AREAPERIL_TYPE_UNSIGNED_LONG_LONG
  #ifdef OASIS_FLOAT_TYPE_DOUBLE
      if(sscanf(line, "%d,%llu,%d,%lf", &f.event_id, &f.areaperil_id,
		&f.intensity_bin_id, &f.probability) != 4) {
  #else
      if(sscanf(line, "%d,%llu,%d,%f", &f.event_id, &f.areaperil_id,
		&f.intensity_bin_id, &f.probability) != 4) {
  #endif
#else
  #ifdef OASIS_FLOAT_TYPE_DOUBLE
      if(sscanf(line, "%d,%u,%d,%lf", &f.event_id, &f.areaperil_id,
		&f.intensity_bin_id, &f.probability) != 4) {
  #else
      if(sscanf(line, "%d,%u,%d,%f", &f.event_id, &f.areaperil_id,
		&f.intensity_bin_id, &f.probability) != 4) {
  #endif
#endif

	fprintf(stderr, "File %s\n", footprintFileName);
	fprintf(stderr, "Invalid data in line %d:\n%s\n", lineno, line);
	return;

      } else {

	if(intensityBins.find(f.intensity_bin_id) == intensityBins.end()) {

	  fprintf(stderr, "Unknown intensity bin ID %d", f.intensity_bin_id);
	  fprintf(stderr, " in line %d:\n%s\n", lineno, line);
	  dataValid = false;

	}

      }

      lineno++;

    }

    fprintf(stderr, "... Done.\n");
    fclose(footprintFile);

    if(dataValid == true) {

      fprintf(stderr, "Cross checks pass.\n");

    } else {

      fprintf(stderr, "Some checks have failed. Please edit input files.\n");

    }

  }

}
