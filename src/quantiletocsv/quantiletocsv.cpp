#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "../include/oasis.h"


namespace quantiletocsv {

  void doit(bool skipHeader) {

    if (skipHeader == false) printf("quantile\n");

    float quantile;
    while (fread(&quantile, sizeof(quantile), 1, stdin) == 1) {
      printf("%f\n", quantile);
    }

  }

}
