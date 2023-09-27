#include <iostream>
#include <stdio.h>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../include/wingetopt.h"
#else
#include <unistd.h>
#endif


namespace amplificationstocsv {


  // First 4 bytes reserved for optional flags
  inline size_t ReadOpts() {
    int opts = 0;
    size_t i = fread(&opts, sizeof(opts), 1, stdin);
    return i;
  }


  void DoIt() {

    size_t i = ReadOpts();

    item_amplification ia;

    fprintf(stdout, "item_id,amplification_id\n");   // header

    while ((i = fread(&ia, sizeof(ia), 1, stdin)) != 0) {

      fprintf(stdout, "%d,%d\n", ia.item_id, ia.amplification_id);

    }

  }

}
