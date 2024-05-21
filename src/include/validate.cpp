#include <climits>
#include <cstdio>
#include <cstdlib>

#include "validate.h"


void Validate::SkipHeaderRow() {

  fgets(line_, sizeof(line_), stdin);
  ++lineno_;

}


void Validate::PrintSuccessMessage() {
/* Only print message regarding warnings if relevant. Some validation checks do
 * not produce any warning messages. */

  fprintf(stderr, "INFO: All %s file validation checks have passed.",
	  fileDescription_);
  if (warnings_) {
    fprintf(stderr, " Please take note of any warnings.");
  }
  fprintf(stderr, "\n");

}


void Validate::PrintErrorMessage() {

  fprintf(stderr, "INFO: Some checks have failed. Please edit %s file.\n",
		  fileDescription_);
  exit(EXIT_FAILURE);

}


int Validate::CheckIDDoesNotExceedMaxLimit(char *idName, long long initialID) {
/* Check ID does not exceed maximum value for integers. */

  if (initialID < MIN_ID_ || initialID > INT_MAX) {

    fprintf(stderr, "ERROR: %s ID %lld on line %d does not lie within"
		    " permitted range [%d, %d]. Please reassign %s IDs to lie"
		    " within this range.\n",
		    idName, initialID, lineno_, MIN_ID_, INT_MAX, idName);
    PrintErrorMessage();

  }

  return (int)initialID;

}


unsigned int Validate::CheckIDDoesNotExceedMaxLimit(char *idName,
	unsigned long long initialID) {
/* Check ID does not exceed maximum value for unsigned integers. */

  // No need to check for negative numbers as ID is of unsigned long long data
  // type and therefore negative numbers should exceed the maximum value for an
  // unsigned integer (i.e. UINT_MAX).
  if (initialID == 0 || initialID > UINT_MAX) {

    fprintf(stderr, "ERROR: %s ID %lld on line %d does not lie within"
		    " permitted range [%d, %u]. Please reassign %s IDs to lie"
		    " within this range.\n",
		    idName, (long long)initialID, lineno_, MIN_ID_, UINT_MAX,
		    idName);
    PrintErrorMessage();

  }

  return (unsigned int)initialID;

}
