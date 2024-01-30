#ifndef VALIDATE_H_
#define VALIDATE_H_

#include "oasis.h"


class Validate {
public:
  Validate() {};
  ~Validate() {};

  void SkipHeaderRow();
  void PrintSuccessMessage();

protected:
  char line_[4096];
  int lineno_ = 1;
  char fileDescription_[4096];
  bool warnings_ = false;

  void PrintErrorMessage();
};

#endif   // VALIDATE_H_
