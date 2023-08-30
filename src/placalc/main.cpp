#include <iostream>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#include <signal.h>
#include <string.h>
#endif


namespace placalc {
  void doit(const float relativeSecondaryFactor,
	    const float absoluteFactor);
}
char *progname;


#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int, siginfo_t *si, void *) {
  fprintf(stderr, "FATAL: %s: Segment fault at address %p\n", progname, si->si_addr);
  exit(EXIT_FAILURE);
}
#endif


void help() {
  fprintf(stderr,
	  "-f optional relative secondary factor within range [0, 1]\n"
	  "-F optional absolute post loss amplification factor\n"
	  "-v version\n"
	  "-h help\n");
}


int main(int argc, char *argv[]) {

  int opt;
  float relativeSecondaryFactor = 1.;
  float absoluteFactor = 0.0;
  progname = argv[0];
  while ((opt = getopt(argc, argv, "f:F:vh")) != -1) {
    switch (opt) {
      case 'f':
	relativeSecondaryFactor = atof(optarg);
	break;
      case 'F':
	absoluteFactor = atof(optarg);
	break;
      case 'v':
	fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
	exit(EXIT_FAILURE);
	break;
      case 'h':
      default:
	help();
	exit(EXIT_FAILURE);
    }
  }

  if (relativeSecondaryFactor < 0.0 || relativeSecondaryFactor > 1.0) {
    fprintf(stderr,
	    "FATAL: Relative secondary factor %f must lie within range [0, 1]\n",
	    relativeSecondaryFactor);
    exit(EXIT_FAILURE);
  }
  if (absoluteFactor < 0.0) {
    fprintf(stderr, "FATAL: Absolute factor %f must be positive value\n",
	    absoluteFactor);
    exit(EXIT_FAILURE);
  }
  if (relativeSecondaryFactor < 1.0 && absoluteFactor > 0.0) {
    fprintf(stderr,
	    "WARNING: Relative secondary and absolute factors are incompatible\n"
	    "INFO: Ignoring relative secondary factor\n");
    relativeSecondaryFactor = 1.;
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
    logprintf(progname, "INFO", "starting process...\n");
    placalc::doit(relativeSecondaryFactor, absoluteFactor);
    logprintf(progname, "INFO", "finishing process...\n");
  } catch (std::bad_alloc&) {
    fprintf(stderr, "FATAL: %s: Memory allocation failed.\n", progname);
    exit(EXIT_FAILURE);
  }

}
