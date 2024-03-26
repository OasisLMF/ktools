#include <cstdio>
#include <cstring>

#include "validatedamagebin.h"


inline int ValidateDamageBin::ScanDeprecatedFormat() {
/* The deprecated format includes the interval type field. The interval type
 * column was dropped from damage bin dictionary files in ktools v3.4.1. For
 * backward compatability, the interval type still exists in the binary
 * file.
 * The bin_from, bin_to and interpolation fields are defined according to the
 * data type assigned to OASIS_FLOAT in include/oasis.h */

#ifdef OASIS_FLOAT_TYPE_DOUBLE
  return sscanf(line_, "%d,%lf,%lf,%lf,%d", &dbd_.bin_index, &dbd_.bin_from,
                                            &dbd_.bin_to, &dbd_.interpolation,
                                            &dbd_.interval_type);
#else
  return sscanf(line_, "%d,%f,%f,%f,%d", &dbd_.bin_index, &dbd_.bin_from,
                                         &dbd_.bin_to, &dbd_.interpolation,
                                         &dbd_.interval_type);
#endif

}


inline int ValidateDamageBin::ScanNewFormat() {
/* The new format excludes the interval type field. The interval type column
 * was dropped from damage bin dictionary files in ktools v3.4.1. For backwards
 * compatability, the interval type still exists in the binary file.
 * The bin_from, bin_to and interpolation fields are defined according to the
 * data type assigned to OASIS_FLOAT in include/oasis.h */

#ifdef OASIS_FLOAT_TYPE_DOUBLE
  return sscanf(line_, "%d,%lf,%lf,%lf", &dbd_.bin_index, &dbd_.bin_from,
                                      &dbd_.bin_to, &dbd_.interpolation);
#else
  return sscanf(line_, "%d,%f,%f,%f", &dbd_.bin_index, &dbd_.bin_from,
                                      &dbd_.bin_to, &dbd_.interpolation);
#endif

}


void ValidateDamageBin::CheckFormat() {

  if (fgets(line_, sizeof(line_), stdin) != 0) {

    if (ScanDeprecatedFormat() == 5) {

      fprintf(stderr, "WARNING: Deprecated format:"
                      " interval_type column no longer required.\n");
      newFormat_ = false;
      warnings_ = true;
      return;

    } else if (ScanNewFormat() == 4) {

      // Although interval type has been dropped in the new format, it exists
      // in the binary file for backwards compatibility. Therefore, the
      // "missing" interval type from the csv file is set to 0.
      dbd_.interval_type = 0;
      newFormat_ = true;
      return;

    }

    // Could not find valid format
    fprintf(stderr, "ERROR: Invalid data in line %d:\n%s\n", lineno_, line_);
    PrintErrorMessage();

  }

  // No line read
  fprintf(stderr, "ERROR: Empty file\n");
  PrintErrorMessage();

}


void ValidateDamageBin::CheckFirstBin(const bool convertToBin) {
/* First bin should have a bin index of 1. Additionally, in most cases the
 * lower limit for the first bin should be 0. */

  if (dbd_.bin_from != 0.0) {

    fprintf(stderr, "WARNING: Lower limit for first bin is not 0."
                    " Are you sure this is what you want?:\n%s\n",
                    line_);
    warnings_ = true;

  }

  if (dbd_.bin_index != 1) {

    fprintf(stderr, "ERROR: First bin index must be 1:\n%s\n", line_);
    PrintErrorMessage();

  }

  CheckInterpolationDamageValuesWithinRange();
  StoreLine();
  if (convertToBin) ConvertToBin();

}


inline void ValidateDamageBin::StoreLine() {
/* Store current line before going to the next one. Used in some error message
 * output. */

    ++lineno_;
    prevBinIndex_ = dbd_.bin_index;
    memcpy(prevLine_, line_, strlen(line_) + 1);
    prevLine_[strlen(line_) - 1] = '\0';

}


inline void ValidateDamageBin::CheckContiguousBinIndices() {

  if (prevBinIndex_ != (dbd_.bin_index - 1)) {

    fprintf(stderr, "ERROR: Non-contiguous bin indices %d on line %d"
                    " and %d on line %d:\n%s\n%s\n",
            prevBinIndex_, lineno_ - 1, dbd_.bin_index, lineno_,
            prevLine_, line_);
    PrintErrorMessage();

  }

}


inline void ValidateDamageBin::CheckInterpolationDamageValuesWithinRange() {

  if ((dbd_.interpolation < dbd_.bin_from) ||
      (dbd_.bin_to < dbd_.interpolation)) {

    fprintf(stderr, "ERROR: Interpolation damage value %f for bin %d"
                    " on line %d lies outside range [%f, %f]:\n%s\n",
            dbd_.interpolation, dbd_.bin_index, lineno_,
            dbd_.bin_from, dbd_.bin_to, line_);
    PrintErrorMessage();

  }

  OASIS_FLOAT midPoint = (dbd_.bin_to - dbd_.bin_from) / 2.0 + dbd_.bin_from;
  int binCentre = (int)(midPoint * 10000 + 0.5);
  if ((int)(dbd_.interpolation * 10000 + 0.5) != binCentre) {

    fprintf(stderr, "WARNING: Interpolation damage value %f does not lie at"
		    " centre %f of bin %d on line %d. Are you sure this is"
		    " what you want?\n%s\n",
	    dbd_.interpolation, midPoint, dbd_.bin_index, lineno_, line_);
    warnings_ = true;

  }

}


inline void ValidateDamageBin::ConvertToBin() {

  fwrite(&dbd_, sizeof(dbd_), 1, stdout);

}


void ValidateDamageBin::ReadDamageBinDictFileNoChecks() {
/* If user is satisfied that the damage bin dictionary csv file is sound and
 * therefore does not require any validation checks, this method can be
 * executed instead of
 * ValidateDamageBinDict::ReadDamageBinDictFile(convertToBin = true). This
 * method assumes that the method Validate::SkipHeaderRow() has been executed
 * beforehand. */

  while (fgets(line_, sizeof(line_), stdin) != 0) {

    if (ScanDeprecatedFormat() != 5) {

      if (ScanNewFormat() == 4) {

        dbd_.interval_type = 0;

      } else {

        fprintf(stderr, "ERROR: Invalid data in line %d:\n%s\n", lineno_, line_);
        PrintErrorMessage();

      }

    }

    ConvertToBin();

    ++lineno_;

  }

  // Only header row exists
  if (lineno_ == 2) {

    fprintf(stderr, "Empty file\n");
    PrintErrorMessage();

  }

}


void ValidateDamageBin::ReadDamageBinDictFile(const bool convertToBin) {
/* After reading in a line from the csv file and checking for data integrity,
 * this method calls other methods to check for contiguous bin indices and
 * whether the interpolation value lies within the defined range. */

  while (fgets(line_, sizeof(line_), stdin) != 0) {

    if (ScanDeprecatedFormat() == 5) {

      if (newFormat_) {

        fprintf(stderr, "ERROR: Deprecated format used in line %d:\n%s\n",
                lineno_, line_);
        PrintErrorMessage();

      }

    } else if (ScanNewFormat() == 4) {

      if (!newFormat_) {

        fprintf(stderr, "ERROR: Missing interval type column in line %d:\n%s\n",
                lineno_, line_);
        PrintErrorMessage();

      }

      dbd_.interval_type = 0;

    } else {

      fprintf(stderr, "ERROR: Invalid data in line %d:\n%s\n", lineno_, line_);
      PrintErrorMessage();

    }

    CheckContiguousBinIndices();
    CheckInterpolationDamageValuesWithinRange();
    StoreLine();
    if (convertToBin) ConvertToBin();

  }

}


void ValidateDamageBin::CheckLastBin() {

  if (dbd_.bin_to != 1.0) {

    fprintf(stderr, "WARNING: Upper limit for last bin is not 1."
                    " Are you sure this is what you want?:\n%s\n", line_);
    warnings_ = true;

  }

}
