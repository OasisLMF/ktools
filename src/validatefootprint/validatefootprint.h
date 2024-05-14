#ifndef VALIDATEFOOTPRINT_H_
#define VALIDATEFOOTPRINT_H_

#include <cstring>
#include <vector>

#include "../include/oasis.h"
#include "../include/validate.h"


class ValidateFootprint : public Validate {
public:
  ValidateFootprint() {
    strcpy(fileDescription_, "Footprint");
  };
  ~ValidateFootprint() {};

  void ReadFootprintFile();
  void PrintMaximumIntensityBinIndex();

protected:
  bool convertToBin_ = false;
  bool validationCheck_ = true;
  FootprintRow fr_;
  int prevEventID_;
  long long initialEveID_;
  char eveIDName_[6] = "Event";

  int ScanLine();
  void ReadFirstFootprintLine(OASIS_FLOAT &totalProbability);

private:
  AREAPERIL_INT prevAreaPerilID_;
  std::vector<int> intensityBinIDs_;
  char prevLine_[4096];
  int maxIntensityBin_ = 0;

  inline void StoreLine();
  inline void SetMaximumIntensityBinIndex();
  inline void CheckProbability(const OASIS_FLOAT totalProbability);
  inline void CheckOrder();
  inline void CheckIntensityBins();
  virtual void SetPreviousEventID() {};
  virtual void WriteBinFootprintFile() {};
  virtual void WriteIdxFootprintFile() {};
};

#endif   // VALIDATEFOOTPRINT_H_
