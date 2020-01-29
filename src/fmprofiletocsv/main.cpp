
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#include "../include/oasis.h"

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <signal.h>
#include <string.h>
#endif

char *progname;

#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int signal, siginfo_t *si, void *arg) {
    fprintf(stderr, "FATAL: %s: Segment fault at address: %p\n", progname,
            si->si_addr);
    exit(EXIT_FAILURE);
}
#endif

namespace fmprofiletocsv {
    void doit(bool skipheader,bool step);
} // namespace fmprofiletocsv

void help() {
    fprintf(stderr, 
		"-S step\n"
		"-s Skip Header\n"
        "-v version\n"
        "-h help"
	);
}

int main(int argc, char *argv[]) {
    int opt;
    bool skipheader = false;
    bool oldFMProfile = false;
	bool step = false;
    while ((opt = getopt(argc, argv, "vhsoS")) != -1) {
        switch (opt) {
        case 's':
            skipheader = true;
            break;
		case 'S':
			step = true;
			break;
        case 'v':
            fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
            exit(EXIT_FAILURE);
            break;
        case 'h':
            help();
            exit(EXIT_FAILURE);
        }
    }

    progname = argv[0];
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
        fmprofiletocsv::doit(skipheader,step);        
    } catch (std::bad_alloc) {
        fprintf(stderr, "FATAL: %s: Memory allocation failed\n", progname);
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
