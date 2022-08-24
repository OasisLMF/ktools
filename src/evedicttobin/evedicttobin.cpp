#include <cstring>
#include <iostream>
#include <vector>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif


namespace evedicttobin {

  void DoIt(bool header) {

    EventDict row;
    char line[4096];
    int lineno = 1;
    if (header) {   // Skip header line
      fgets(line, sizeof(line), stdin);
      lineno++;
    }
    char description[4096];

    while (fgets(line, sizeof(line), stdin) != 0) {

      if (sscanf(line, "%d,%f,%4095[^\n]", &row.event_id, &row.event_rate,
		 description) == 3) {

	row.description_size = strlen(description);
	fwrite(&row, sizeof(row), 1, stdout);
	fwrite((unsigned char *)&description, row.description_size, 1, stdout);
	lineno++;

      } else {
	fprintf(stderr, "FATAL: Invalid data in line %d:\n%s", lineno, line);
	exit(EXIT_FAILURE);
      }

    }

  }

}
