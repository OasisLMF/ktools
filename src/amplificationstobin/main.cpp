#include <iostream>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#include <signal.h>
#include <string.h>
#endif


namespace amplificationstobin {
  void DoIt();
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
          "-v version\n"
          "-h help\n");
}


int main(int argc, char *argv[]) {

  int opt;
  progname = argv[0];
  while ((opt = getopt(argc, argv, "vh")) != -1) {
    switch (opt) {
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
    amplificationstobin::DoIt();
  } catch (std::bad_alloc&) {
    fprintf(stderr, "FATAL: %s: Memory allocation failed.\n", progname);
    exit(EXIT_FAILURE);
  }

}
