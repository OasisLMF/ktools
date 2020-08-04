#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

namespace ensembletocsv {

  void doit() {

    printf("sidx, ensemble_id, sample_id\n");

    Ensemble q;
    size_t i = fread(&q, sizeof(q), 1, stdin);
    while (i != 0) {
      printf("%d, %d, %d\n", q.sidx, q.ensemble_id, q.sample_id);
      i = fread(&q, sizeof(q), 1, stdin);
    }

  }

}
