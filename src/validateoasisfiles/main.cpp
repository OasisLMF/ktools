#include <cstring>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

namespace validateoasisfiles {
  void doit(char const *, bool);
}

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <signal.h>
#include <string.h>
#endif

char *progname;

#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int signal, siginfo_t *st, void *arg) {
  fprintf(stderr, "%s: Segment fault at address: %p\n", progname, st->si_addr);
  exit(0);
}
#endif

void help() {
  fprintf(stderr, "-h help\n-d [oasis files directory]\n"
		  "-g GUL files only\n-v version\n");
}

int main(int argc, char *argv[]) {

  progname = argv[0];
  char *oasisFilesDir;
  bool gulOnly = false;
  int opt;
  while((opt = getopt(argc, argv, ":d:gvh")) != -1) {
    switch(opt) {
      case 'd':
	oasisFilesDir = optarg;
	break;
      case ':':
	fprintf(stderr, "option %c needs a value\n", optopt);
	exit(EXIT_FAILURE);
      case '?':
	fprintf(stderr, "unknown option %c\n", optopt);
	exit(EXIT_FAILURE);
      case 'g':
	gulOnly = true;
	break;
      case 'v':
	fprintf(stderr, "%s: version: %s\n", argv[0], VERSION);
	exit(EXIT_FAILURE);
	break;
      case 'h':
      deafult:
	help();
	exit(EXIT_FAILURE);
    }
  }

#if !defined(_MSC_VER) && !defined(__MINGW32__)
  struct sigaction sa;

  memset(&sa, 0, sizeof(struct sigaction));
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = segfault_sigaction;
  sa.sa_flags = SA_SIGINFO;

  sigaction(SIGSEGV, &sa, NULL);
#endif

  try {
    initstreams();
    validateoasisfiles::doit(oasisFilesDir, gulOnly);
    return 0;
  }
  catch(std::bad_alloc) {
    fprintf(stderr, "%s: Memory allocation failed.\n", progname);
    exit(0);
  }

}
