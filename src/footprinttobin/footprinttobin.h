#ifndef FOOTPRINTTOBIN_H_
#define FOOTPRINTTOBIN_H_

#include <cstdio>
#include <vector>

#include "../include/oasis.h"
#include "../include/validate.h"
#include "../validatefootprint/validatefootprint.h"


class FootprintToBin : public ValidateFootprint {
public:
  FootprintToBin(const int maxIntensityBinIdx,
		 const bool hasIntensityUncertainty, const char* binFileName,
		 const char* idxFileName, const bool uncompressedSize,
		 const bool zip, const bool validationCheck);
  ~FootprintToBin();

  void WriteHeader();
  void ReadFootprintFileNoChecks();

private:
  const int maxIntensityBinIdx_;
  const bool hasIntensityUncertainty_;
  const bool uncompressedSize_;
  const bool zip_;
  FILE *fileOutBin_ = nullptr;
  FILE *fileOutIdx_ = nullptr;
  int rowCount_ = 0;
  EventIndex idx_;
  std::vector<EventRow> eventRows_;

  inline void ReadFirstFootprintLine() {
    OASIS_FLOAT totalProbability = 0.0;
    ValidateFootprint::ReadFirstFootprintLine(totalProbability);
  }
  void SetPreviousEventID() override;
  void WriteBinFootprintFile() override;
  void WriteIdxFootprintFile() override;

};

#endif   // FOOTPRINTTOBIN_H_
