#include <iostream>

#include "../include/oasis.h"

namespace evedicttocsv {

  void DoIt(bool header) {

    if (header)
      fprintf(stdout, "event_id,OriginalEventRate,EventDescription\n");

    EventDict row;
    char description[4096];

    // Binary file format is:
    // int event_id
    // float event_rate
    // int description_size (gives size of description character array)
    // char[] description
    int i = 1;
    while (i != 0) {
      i = (int)fread(&row, sizeof(row), 1, stdin);
      i = (int)fread(&description, 1, row.description_size, stdin);
      description[row.description_size] = '\0';   // Null terminate
      fprintf(stdout, "%d,%f,%s\n", row.event_id, row.event_rate, description);
    }

  }

}
