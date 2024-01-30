#include <algorithm>
#include <cstdio>
#include <cstring>

#include "validatefootprint.h"


int ValidateFootprint::ScanLine() {
/* The areaperil_id field is defined according to the data type assigned to
 * AREAPERIL_INT in include/oasis.h
 * The probability field is defined according to the data type assigned to
 * OASIS_FLOAT in include/oasis.h */

#ifdef AREAPERIL_TYPE_UNSIGNED_LONG_LONG
  #ifdef OASIS_FLOAT_TYPE_DOUBLE
  return sscanf(line_, "%d,%llu,%d,%lf", &fr_.event_id, &fr_.areaperil_id,
				      &fr_.intensity_bin_id, &fr_.probability);
  #else
  return sscanf(line_, "%d,%llu,%d,%f", &fr_.event_id, &fr_.areaperil_id,
				      &fr_.intensity_bin_id, &fr_.probability);
  #endif
#else
  #ifdef OASIS_FLOAT_TYPE_DOUBLE
  return sscanf(line_, "%d,%u,%d,%lf", &fr_.event_id, &fr_.areaperil_id,
				      &fr_.intensity_bin_id, &fr_.probability);
  #else
  return sscanf(line_, "%d,%u,%d,%f", &fr_.event_id, &fr_.areaperil_id,
				      &fr_.intensity_bin_id, &fr_.probability);
  #endif
#endif

}


inline void ValidateFootprint::StoreLine() {
/* Store current line before going to the next one. Used in some error message
 * output. */

    ++lineno_;
    prevEventID_ = fr_.event_id;
    prevAreaPerilID_ = fr_.areaperil_id;

    memcpy(prevLine_, line_, strlen(line_) + 1);
    prevLine_[strlen(line_) - 1] = '\0';

}


inline void ValidateFootprint::SetMaximumIntensityBinIndex() {
/* The maximum intensity bin index is a required input argument to convert
 * footprint csv files to binary format with footprinttobin. */

  if (maxIntensityBin_ < fr_.intensity_bin_id) {

    maxIntensityBin_ = fr_.intensity_bin_id;

  }

}


void ValidateFootprint::ReadFirstFootprintLine(OASIS_FLOAT &totalProbability) {
/* After skipping header, first line of footprint csv file is read primarily to
 * establish initial event ID. */

  if (fgets(line_, sizeof(line_), stdin) != 0) {

    if (ScanLine() == 4) {

      // In the case when there are no validation checks to perform, the
      // initial previous event ID must still be set and line number
      // incremented. In this case, total probability is not used.
      if (!validationCheck_) {

        SetPreviousEventID();

	return;

      }

      StoreLine();
      intensityBinIDs_.push_back(fr_.intensity_bin_id);
      totalProbability = fr_.probability;

      SetMaximumIntensityBinIndex();

      return;

    }

    fprintf(stderr, "ERROR: Invalid data in line %d:\n%s\n", lineno_, line_);
    PrintErrorMessage();

  }

  fprintf(stderr, "ERROR: Empty file\n");
  PrintErrorMessage();

}


inline void ValidateFootprint::CheckProbability(const OASIS_FLOAT totalProbability) {
/* Check probabilities for each event ID-areaperil ID pair sum to 1.0. */

  // Define precision to avoid floating point errors
  int probCheck = (int)(totalProbability * 10000 + 0.5);
  if (probCheck != 10000) {

    fprintf(stderr, "ERROR: Probabilities for event ID %d and areaperil ID %u"
		    " do not sum to 1.0 (total probability = %f).\n",
	    prevEventID_, prevAreaPerilID_, totalProbability);
    PrintErrorMessage();

  }

}


inline void ValidateFootprint::CheckOrder() {
/* Check event IDs and areaperil IDs are in ascending order. */

  char idName[10];

  if (fr_.event_id < prevEventID_) {

    strcpy(idName, "Event");

  } else if ((fr_.areaperil_id < prevAreaPerilID_) &&
	     (fr_.event_id == prevEventID_)) {

    strcpy(idName, "Areaperil");

  } else return;

  fprintf(stderr, "%s IDs in lines %d and %d are not in ascending"
		  " order:\n%s\n%s\n",
	  idName, lineno_ - 1, lineno_, prevLine_, line_);
  PrintErrorMessage();

}


inline void ValidateFootprint::CheckIntensityBins() {

  if (find(intensityBinIDs_.begin(), intensityBinIDs_.end(),
           fr_.intensity_bin_id) == intensityBinIDs_.end()) {

    intensityBinIDs_.push_back(fr_.intensity_bin_id);

    return;

  }

  fprintf(stderr, "ERROR: Duplicate intensity bin %d on line %d"
		  " for event ID %d areaperil ID %u:\n%s\n",
	  fr_.intensity_bin_id, lineno_, fr_.event_id, fr_.areaperil_id, line_);
  PrintErrorMessage();

}


void ValidateFootprint::ReadFootprintFile() {
/* After reading in a line from the csv file and checking for data integrity,
 * this method calls other methods to check:
 * 
 * - Total probability for each event ID-areaperil ID pair sums to 1.0.
 * - Event and areaperil IDs are in ascending order.
 * - Unique intensity bin IDs for each event ID-areaperil ID pair. */

  OASIS_FLOAT totalProbability = 0.0;
  ReadFirstFootprintLine(totalProbability);
  if (convertToBin_) WriteBinFootprintFile();   // Write first footprint line

  while (fgets(line_, sizeof(line_), stdin) != 0) {

    if (ScanLine() == 4) {

      // New event and/or areaperil IDs.
      // Check probabilities sum to 1.0 for each event ID-areaperil ID
      // combination and event IDs and areaperil IDs are in ascending order.
      if ((fr_.event_id != prevEventID_) || 
	  (fr_.areaperil_id != prevAreaPerilID_)) {

        CheckProbability(totalProbability);
	CheckOrder();

	// Reset for next event
	intensityBinIDs_.clear();
	intensityBinIDs_.push_back(fr_.intensity_bin_id);
	totalProbability = fr_.probability;

      } else {

        CheckIntensityBins();
	totalProbability += fr_.probability;

      }

      if (convertToBin_) {

	if (fr_.event_id != prevEventID_) WriteIdxFootprintFile();
        WriteBinFootprintFile();

      } else {

        // Maximum intensity bin index value is required for binary conversion
	// in the future.
        SetMaximumIntensityBinIndex();

      }

      StoreLine();
      continue;

    }

    fprintf(stderr, "ERROR: Invalid data in line %d:\n%s\n", lineno_, line_);
    PrintErrorMessage();

  }

  // Check probabilities sum to 1.0 for last event ID-areaperil ID combination
  // and event IDs and areaperil IDs are in ascending order for last row
  CheckProbability(totalProbability);
  CheckOrder();
  // Write index row for last event ID-areaperil ID combination
  if (convertToBin_) WriteIdxFootprintFile();

}


void ValidateFootprint::PrintMaximumIntensityBinIndex() {

  fprintf(stderr, "INFO: Maximum value of intensity bin index = %d\n",
	  maxIntensityBin_);

}
