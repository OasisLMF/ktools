#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "../include/oasis.h"
#include "footprinttobin.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <csignal>
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

  fprintf(stderr, "-i [value] maximum intensity bin index\n"
		  "-n no intensity uncertainty\n"
		  "-b [file name] output bin file name (default: footprint.bin)\n"
		  "-x [file name] output idx file name (default: footprint.idx)\n"
		  "-z zip footprint data\n"
		  "-u index file should include uncompressed data size\n"
		  "-N no validation checks\n"
		  "-h help\n"
		  "-v version\n");

}


void DoIt(const int maxIntensityBinIdx, const bool hasIntensityUncertainty,
	  const char* binFileName, const char* idxFileName,
	  const bool uncompressedSize, const bool zip,
	  const bool validationCheck) {

  FootprintToBin fTB(maxIntensityBinIdx, hasIntensityUncertainty, binFileName,
		     idxFileName, uncompressedSize, zip, validationCheck);
  fTB.SkipHeaderRow();
  fTB.WriteHeader();

  if (validationCheck) {

    fTB.ReadFootprintFile();
    fTB.PrintSuccessMessage();

    return;

  }

  fTB.ReadFootprintFileNoChecks();

}


int main(int argc, char *argv[]) {

  progname = argv[0];
  int maxIntensityBinIdx = -1;
  bool hasIntensityUncertainty = true;
  char binFileName[4096] = "footprint.bin";
  char idxFileName[4096] = "footprint.idx";
  bool zip = false;
  bool uncompressedSize = false;
  bool validationCheck = true;
  int opt;
  while ((opt = getopt(argc, argv, "i:nb:x:zuNhv")) != -1) {
    switch (opt) {
      case 'i':
        maxIntensityBinIdx = atoi(optarg);
	break;
      case 'n':
	hasIntensityUncertainty = false;
	break;
      case 'b':
	strcpy(binFileName, optarg);
	break;
      case 'x':
	strcpy(idxFileName, optarg);
	break;
      case 'z':
	zip = true;
	break;
      case 'u':
	uncompressedSize = true;
	break;
      case 'N':
	validationCheck = false;
	break;
      case 'v':
#ifdef AREAPERIL_TYPE_UNSIGNED_LONG_LONG
        fprintf(stderr, "%s : version : %s :"
			" Areaperil ID data type unsigned long long\n",
		argv[0], VERSION);
#else
	fprintf(stderr, "%s : version : %s\n", argv[0], VERSION);
#endif
        exit(EXIT_FAILURE);
      case 'h':
      default:
	Help();
	exit(EXIT_FAILURE);
    }
  }

  if (maxIntensityBinIdx == -1) {
    fprintf(stderr, "ERROR: Intensity bin parameter not supplied.\n");
    exit(EXIT_FAILURE);
  }

  if (uncompressedSize && !zip) {
    fprintf(stderr, "WARNING: Zip footprint data argument not supplied."
		    " Ignoring request to include uncompressed data size in"
		    " index file.\n");
    uncompressedSize = false;
  }

  // Ensure zip file names have .z suffix
  if (zip) {
    char endChars[3];

    if (strlen(binFileName) < 2) {
      strcat(binFileName, ".z");
    } else {
      strcpy(endChars, binFileName + (strlen(binFileName) - 2));
      if (strcmp(endChars, ".z")) strcat(binFileName, ".z");
    }

    if (strlen(idxFileName) < 2) {
      strcat(idxFileName, ".z");
    } else {
      strcpy(endChars, idxFileName + (strlen(idxFileName) - 2));
      if (strcmp(endChars, ".z")) strcat(idxFileName, ".z");
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
    DoIt(maxIntensityBinIdx, hasIntensityUncertainty, binFileName, idxFileName,
	 uncompressedSize, zip, validationCheck);
    return 0;

  } catch (std::bad_alloc&) {

    fprintf(stderr, "ERROR: %s: Memory allocation failed.\n", progname);
    exit(EXIT_FAILURE);

  }

}
