#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#include "../include/oasis.h"


void help() {
  fprintf(stderr,
          "-s skip header\n"
          "-v version\n"
          "-h help\n");
}


void doit(const bool skipHeader) {

  if (!skipHeader) printf("areaperil_id,vulnerability_id,weight\n");

  VulnerabilityWeight vw;
  while ((fread(&vw, sizeof(vw), 1, stdin)) != 0) {
    printf("%d,%d,%f\n", vw.areaperil_id, vw.vulnerability_id, vw.weight);
  }

}


int main(int argc, char* argv[]) {

  int opt;
  bool skipHeader = false;

  while ((opt = getopt(argc, argv, "svh")) != -1) {
    switch (opt) {
      case 's':
        skipHeader = true;
	break;
      case 'v':
        fprintf(stderr, "%s: version: %s\n", argv[0], VERSION);
	exit(EXIT_FAILURE);
      case 'h':
      default:
	help();
	exit(EXIT_FAILURE);
    }
  }

  initstreams();
  doit(skipHeader);

  return 0;

}
