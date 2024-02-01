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
