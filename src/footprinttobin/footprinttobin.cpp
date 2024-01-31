#include <cstdio>
#include <vector>

#include <zlib.h>

#include "../include/oasis.h"
#include "footprinttobin.h"


FootprintToBin::FootprintToBin(const int maxIntensityBinIdx,
			       const bool hasIntensityUncertainty,
			       const char* binFileName,
			       const char* idxFileName,
			       const bool uncompressedSize, const bool zip,
			       const bool validationCheck) :
  maxIntensityBinIdx_(maxIntensityBinIdx),
  hasIntensityUncertainty_(hasIntensityUncertainty),
  uncompressedSize_(uncompressedSize), zip_(zip) {

  strcpy(fileDescription_, "Footprint");
  convertToBin_ = true;
  validationCheck_ = validationCheck;
  fileOutBin_ = fopen(binFileName, "wb");
  fileOutIdx_ = fopen(idxFileName, "wb");

}


FootprintToBin::~FootprintToBin() {

    fclose(fileOutBin_);
    fclose(fileOutIdx_);

}


void FootprintToBin::SetPreviousEventID() {

  ++lineno_;
  prevEventID_ = fr_.event_id;

}


void FootprintToBin::ReadFootprintFileNoChecks() {
/* If user is satisfied that the footprint csv file is sound and therefore does
 * not require any validation checks, this method can be executed instead of
 * ValidateFootprint::ReadFootprintFile(). This method assumes that the methods
 * Validate:SkipHeaderRow() and WriteHeader() have been executed beforehand. */

  ReadFirstFootprintLine();
  WriteBinFootprintFile();

  while (fgets(line_, sizeof(line_), stdin) != 0) {

    if (ScanLine() == 4) {

      if (fr_.event_id != prevEventID_) {

        WriteIdxFootprintFile();
	SetPreviousEventID();

      }

      WriteBinFootprintFile();
      continue;

    }

    fprintf(stderr, "ERROR: Invalid data in line %d:\n%s\n", lineno_, line_);
    PrintErrorMessage();

  }

  WriteIdxFootprintFile();

}


void FootprintToBin::WriteHeader() {
/* Binary file header consists of an integer giving the value of the maximum
 * intensity bin index. */

  fwrite(&maxIntensityBinIdx_, sizeof(maxIntensityBinIdx_), 1, fileOutBin_);

  int zipOpts = uncompressedSize_ << 1 | hasIntensityUncertainty_;
  fwrite(&zipOpts, sizeof(zipOpts), 1, fileOutBin_);

  // Set initial offset
  idx_.offset = sizeof(maxIntensityBinIdx_) + sizeof(zipOpts);

}


void FootprintToBin::WriteBinFootprintFile() {

  // Check that intensity bin ID from file does not exceed maximum in header
  if (fr_.intensity_bin_id > maxIntensityBinIdx_) {
    fprintf(stderr, "ERROR: Maximum intensity bin index of %d in header is"
		    " less than that of %d in line %d:\n%s\n",
	    maxIntensityBinIdx_, fr_.intensity_bin_id, lineno_, line_);
    exit(EXIT_FAILURE);
  }
	

  EventRow er = { fr_.areaperil_id, fr_.intensity_bin_id, fr_.probability };
  ++rowCount_;   // Increment row counter

  // Write binary footprint file if no zipped output requested
  if (!zip_) {

    fwrite(&er, sizeof(er), 1, fileOutBin_);
    return;

  }

  // Otherwise, store in vector
  eventRows_.push_back(er);

}


void FootprintToBin::WriteIdxFootprintFile() {

  idx_.event_id = prevEventID_;
  idx_.size = rowCount_ * sizeof(EventRow);

  // Write zipped output to binary file
  if (zip_) {

    std::vector<unsigned char> zer;
    zer.resize(eventRows_.size() * sizeof(EventRow) + 1024);
    unsigned long zippedEventRowsLength = zer.size();
    compress(&zer[0], &zippedEventRowsLength, (unsigned char*)&eventRows_[0],
	     eventRows_.size() * sizeof(EventRow));
    fwrite((unsigned char*)&zer[0], zippedEventRowsLength, 1, fileOutBin_);

    // Overwrite with actual size of compressed buffer
    idx_.size = zippedEventRowsLength;

    eventRows_.clear();   // Reset

  }

  fwrite(&idx_, sizeof(idx_), 1, fileOutIdx_);

  // Write uncompressed size to index file if requested (only relevant for
  // zipped footprint files)
  if (uncompressedSize_) {
    long long originalSize = rowCount_ * sizeof(EventRow);
    fwrite(&originalSize, sizeof(originalSize), 1, fileOutIdx_);
  }

  // Set new offset and reset row counter
  idx_.offset += idx_.size;
  rowCount_ = 0;

}
