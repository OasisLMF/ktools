#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#include <signal.h>
#include <string.h>
#endif

#include "../include/oasis.h"

char *progname = nullptr;

namespace aalcalcmeanonly {

  void DoIt(const std::string& subFolder, const bool skipHeader,
	    const bool ordOutput);

}

#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int, siginfo_t *si, void *)
{
  fprintf(stderr, "FATAL: %s: Segment fault at address: %p\n", progname,
          si->si_addr);
  exit(EXIT_FAILURE);
}
#endif

void help() {

  fprintf(stderr, "-K [folder] workspace sub folder\n");
  fprintf(stderr, "-o Open Results Data (ORD) output\n");
  fprintf(stderr, "-s skip header\n");
  fprintf(stderr, "-v version\n");
  fprintf(stderr, "-h help\n");

}

int main(int argc, char* argv[]) {

  progname = argv[0];
  std::string subFolder;
  int opt;
  bool skipHeader = false;
  bool ordOutput = false;

  while ((opt = getopt(argc, argv, (char *)"osvhK:")) != -1) {
    switch (opt) {
      case 'v':
        fprintf(stderr, "%s: version: %s\n", argv[0], VERSION);
	exit(EXIT_FAILURE);
      case 's':
        skipHeader = true;
        break;
      case 'o':
        ordOutput = true;
        break;
      case 'K':
        subFolder = optarg;
        break;
      case 'h':
      default:
	help();
	exit(EXIT_FAILURE);
    }
  }

  if (subFolder.length() == 0) {
    fprintf(stderr, "FATAL: No folder supplied for summarycalc files\n");
    exit(EXIT_FAILURE);
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
    logprintf(progname, "INFO", "starting process...\n");
    aalcalcmeanonly::DoIt(subFolder, skipHeader, ordOutput);
    logprintf(progname, "INFO", "ending procecss...\n");
  } catch (std::bad_alloc&) {
    fprintf(stderr, "FATAL: %s: Memory allocation failed\n", progname);
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;

}
