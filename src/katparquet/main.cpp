#include "../include/oasis.h"
#include <algorithm>
#include <vector>
#include <sys/stat.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#include "../include/dirent.h"
#else
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <signal.h>
#include <string.h>
#endif


#ifdef HAVE_PARQUET
namespace katparquet {
  void doit(const std::vector<std::string> &inFiles, const std::string outFile,
	    const int tableName);
}
#endif

char *progname = 0;

#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int, siginfo_t *si, void *)
{
  fprintf(stderr, "FATAL: %s: Segment fault at address: %p\n",
	  progname, si->si_addr);
  exit(EXIT_FAILURE);
}
#endif


void help() {

  fprintf(stderr,
	  "-d [directory] path for concatenation\n"
	  "-o [filename] output concatenated file\n"
	  "-M concatenate MPLT files\n"
	  "-m concatenate MELT files\n"
	  "-Q concatenate QPLT files\n"
	  "-q concatenate QELT files\n"
	  "-S concatenate SPLT files\n"
	  "-s concatenate SELT files\n"
	  "-h help\n"
	  "-v version\n"
  );

}


inline int setTableName(int currentTableName, int newTableName) {

  if (currentTableName == 0) return newTableName;
  else {
    fprintf(stderr,
	    "FATAL: %s: Cannot concatenate different input file types.\n",
	    progname);
    exit(EXIT_FAILURE);
  }

}


int main(int argc, char *argv[]) {

// If there are no parquet libraries then this executable has no purpose
#ifndef HAVE_PARQUET
  fprintf(stderr, "FATAL: Apache arrow libraries for parquet output are missing.\n"
		  "Please install libraries and recompile to use this option.\n");
  exit(EXIT_FAILURE);
#else

  enum { NONE = 0, MPLT, QPLT, SPLT, MELT, QELT, SELT };
  progname = argv[0];
  int opt;
  std::string path = "";
  std::string outFile = "";
  int tableName = NONE;
  while ((opt = getopt(argc, argv, "d:o:MmQqSsvh")) != -1) {
    switch (opt) {
      case 'd':
	path = optarg;
	break;
      case 'o':
	outFile = optarg;
	break;
      case 'M':
	tableName = setTableName(tableName, MPLT);
	break;
      case 'm':
	tableName = setTableName(tableName, MELT);
	break;
      case 'Q':
	tableName = setTableName(tableName, QPLT);
	break;
      case 'q':
	tableName = setTableName(tableName, QELT);
	break;
      case 'S':
	tableName = setTableName(tableName, SPLT);
	break;
      case 's':
	tableName = setTableName(tableName, SELT);
	break;
      case 'v':
	fprintf(stderr, "%s : version: %s : Parquet output enabled\n",
		argv[0], VERSION);
	exit(EXIT_FAILURE);
	break;
      case 'h':
      default:
	help();
	exit(EXIT_FAILURE);
    }
  }

  if (outFile == "") {
    fprintf(stderr, "FATAL: %s: Output file name required.\n", progname);
    exit(EXIT_FAILURE);
  }

  // Fill list of files with contents of directory if applicable
  std::vector<std::string> fileList;
  if (path.length() > 0) {
    DIR* dir;
    if ((dir = opendir(path.c_str())) != NULL) {
      struct dirent* ent;
      while ((ent = readdir(dir)) != NULL) {
	std::string s = ent->d_name;
	if (s != "." && s != "..") {
	  std::string s2 = path + ent->d_name;
	  struct stat path_stat;
	  stat(s2.c_str(), &path_stat);
	  if (S_ISREG(path_stat.st_mode)) {
	    fileList.push_back(s2);
	  }
	}
      }
    }
  }

  // Get (additional) files from command line
  argc -= optind;
  argv += optind;
  for (int i = 0; i < argc; i++) {
    fileList.push_back(argv[i]);
  }

  if (fileList.empty()) {
    fprintf(stderr, "FATAL: %s: No input files given.\n", progname);
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
    initstreams();
    katparquet::doit(fileList, outFile, tableName);
  } catch (std::bad_alloc&) {
    fprintf(stderr, "FATAL: %s: Memory allocation failed.\n", progname);
    exit (EXIT_FAILURE);
  }

  return EXIT_SUCCESS;

#endif
}
