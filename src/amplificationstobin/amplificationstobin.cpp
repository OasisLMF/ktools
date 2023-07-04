#include <iostream>
#include <stdio.h>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../include/wingetopt.h"
#else
#include <unistd.h>
#endif


namespace amplificationstobin {


  // First 4 bytes reserved for optional flags
  inline void WriteOpts() {
    int opts = 0;
    fwrite(&opts, sizeof(opts), 1, stdout);
  }


  void DoIt() {

    WriteOpts();

    int expected_item_id = 1;
    item_amplification ia;
    char line[4096];

    fgets(line, sizeof(line), stdin);   // Skip header row
    int lineno = 2;

    while (fgets(line, sizeof(line), stdin) != 0) {

      if (sscanf(line, "%d,%d", &ia.item_id, &ia.amplification_id) != 2) {
        fprintf(stderr, "FATAL: Invalid data in line %d:\n%s\n", lineno, line);
	exit(EXIT_FAILURE);
      }

      if (expected_item_id != ia.item_id) {
        fprintf(stderr, "FATAL: Item IDs not contiguous:\n"
			"Expected item ID %d but read item ID %d on line %d\n",
			expected_item_id, ia.item_id, lineno);
	exit(EXIT_FAILURE);
      }

      ++lineno;
      ++expected_item_id;

      fwrite(&ia, sizeof(ia), 1, stdout);

    }

  }

}
