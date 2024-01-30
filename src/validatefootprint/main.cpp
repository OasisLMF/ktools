#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "../include/oasis.h"
#include "validatefootprint.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <csignal>
#include <cstring>
#endif

char *progname;

#if !defined(_MSC_VER) && !defined(__MINGW32__)
void SegFaultSigAction(int, siginfo_t *si, void *) {
  fprintf(stderr, "ERROR: %s: Segment fault at address: %p\n",
	  progname, si->si_addr);
  exit(EXIT_FAILURE);
}
#endif


void Help() {
  fprintf(stderr, "-h help\n"
		  "-v version\n");
}


void DoIt() {

  ValidateFootprint vFP;
  vFP.SkipHeaderRow();
  vFP.ReadFootprintFile();
  vFP.PrintMaximumIntensityBinIndex();
  vFP.PrintSuccessMessage();

}


int main(int argc, char *argv[]) {

  progname = argv[0];
  int opt;
  while ((opt = getopt(argc, argv, "vh")) != -1) {
    switch(opt) {
      case 'v':
        fprintf(stderr, "%s : version : %s\n", argv[0], VERSION);
	exit(EXIT_FAILURE);
	break;
      case 'h':
      default:
	Help();
	exit(EXIT_FAILURE);
    }
  }

#if !defined(_MSC_VER) && !defined(__MINGW32__)
  struct sigaction sa;

  memset(&sa, 0, sizeof(struct sigaction));
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = SegFaultSigAction;
  sa.sa_flags = SA_SIGINFO;

  sigaction(SIGSEGV, &sa, NULL);
#endif

  try {

    initstreams();
    DoIt();
    return 0;

  } catch (std::bad_alloc&) {

    fprintf(stderr, "ERROR: %s: Memory allocation failed.\n", progname);
    exit(EXIT_FAILURE);

  }

}
