#ifndef VALIDATEDAMAGEBIN_H_
#define VALIDATEDAMAGEBIN_H_

#include <cstring>

#include "../include/validate.h"


class ValidateDamageBin : public Validate {
public:
  ValidateDamageBin() {
    strcpy(fileDescription_, "Damage Bin Dictionary");
  };
  ~ValidateDamageBin() {};

  void CheckFormat();
  void CheckFirstBin(const bool convertToBin = false);
  void ReadDamageBinDictFile(const bool convertToBin = false);
  void ReadDamageBinDictFileNoChecks();
  void CheckLastBin();

private:
  damagebindictionary dbd_;
  bool newFormat_;
  int prevBinIndex_ = 0;
  char prevLine_[4096];

  inline int ScanDeprecatedFormat();
  inline int ScanNewFormat();
  inline void StoreLine();
  inline void CheckContiguousBinIndices();
  inline void CheckInterpolationDamageValuesWithinRange();
  inline void ConvertToBin();
};

#endif   // VALIDATEDAMAGEBIN_H_
