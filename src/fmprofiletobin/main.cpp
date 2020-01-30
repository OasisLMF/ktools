
#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <signal.h>
#include <string.h>
#endif

char *progname;

#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int, siginfo_t *si, void *)
{
	fprintf(stderr, "FATAL: %s: Segment fault at address: %p\n", progname, si->si_addr);
	exit(EXIT_FAILURE);
}
#endif


namespace fmprofiletobin {
    void doit(bool step);
} // namespace fmprofiletobin

void help() { 
	fprintf(stderr, 
		"-h help\n" 
		"-v version\n" 
		"-S step\n"
	);
}

int main(int argc, char *argv[]) {

    int opt;
	bool step = false;
    while ((opt = getopt(argc, argv, "vhoS")) != -1) {
        switch (opt) {
        case 'v':
            fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
            exit(EXIT_FAILURE);
            break; 
		case 'S':
			step = true;
			break;
        case 'h':
        default:
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
        fmprofiletobin::doit(step);
    } catch (std::bad_alloc&) {
        fprintf(stderr, "FATAL: %s: Memory allocation failed\n", progname);
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
