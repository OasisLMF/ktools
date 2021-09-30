#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#if !defined(_MSC_VER) && !defined(__MIONGW32__)
#include <signal.h>
#include <string.h>
#endif

char *progname;

namespace quantiletocsv {
  void doit(bool skipHeader);
}

#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int, siginfo_t *si, void *) {
  fprintf(stderr, "FATAL: %s: Segment fault at address: %p\n", progname, si->si_addr);
  exit(EXIT_FAILURE);
}
#endif


void help() {

  fprintf(stderr, "-s skip header\n"
		  "-h help\n"
		  "=v version\n");

}

int main(int argc, char* argv[]) {

  progname = argv[0];
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
	break;
      case 'h':
      default:
	help();
	exit(EXIT_FAILURE);

    }

  }

  try {

    initstreams();
    quantiletocsv::doit(skipHeader);
    return EXIT_SUCCESS;

  } catch (std::bad_alloc&) {

    fprintf(stderr, "FATAL: %s: Memory allocation failed\n", progname);
    exit(EXIT_FAILURE);

  }

}
