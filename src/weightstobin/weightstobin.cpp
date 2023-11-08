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
          "-v version\n"
	  "-h help\n");
}


void doit() {

  char line[4096];
  fgets(line, sizeof(line), stdin);   // skip header
  int lineno = 2;
  VulnerabilityWeight vw;

  while (fgets(line, sizeof(line), stdin) != 0) {

    if (sscanf(line, "%d,%d,%f", &vw.areaperil_id, &vw.vulnerability_id,
				 &vw.weight) != 3) {
      fprintf(stderr, "FATAL: Invalid data in line %d:\n%s", lineno, line);
      exit(EXIT_FAILURE);
    }

    fwrite(&vw, sizeof(vw), 1, stdout);
    lineno++;

  }

}


int main(int argc, char* argv[]) {

  int opt;
  while ((opt = getopt(argc, argv, "vh")) != -1) {
    switch (opt) {
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
  doit();

  return 0;

}
