/*
 * Copyright (c)2015 - 2016 Oasis LMF Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *
 *   * Neither the original author of this software nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/*
eve: Event emitter for partioning work between multiple processes
Author: Ben Matharu  email: ben.matharu@oasislmf.org

*/

#include "../include/oasis.h"
#include <fstream>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"

#else
#include <unistd.h>
#endif

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <signal.h>
#include <string.h>
#endif

namespace eve {
    void emitevents(OASIS_INT pno_, OASIS_INT total_, bool shuffle,
                    bool textmode);
}
char *progname;

#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int signal, siginfo_t *si, void *arg) {
    fprintf(stderr, "%s: Segment fault at address: %p\n", progname,
            si->si_addr);
    exit(EXIT_FAILURE);
}
#endif

void help() {
    fprintf(stderr, "usage: processno totalprocesses\n"
                    "-h help\n"
                    "-n No shuffled events\n"
                    "-v version\n"
                    "-t text mode\n");
}
int main(int argc, char *argv[]) {
    progname = argv[0];

    int opt;
    bool shuffle = true;
    bool textmode = false;

    while ((opt = getopt(argc, argv, "nvht")) != -1) {
        switch (opt) {
        case 'v': {
            fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);

#if defined(__GNUC__) && !defined(__clang_version__)
			fprintf(stderr, "%s : compiler: gcc %d.%d.%d \n", argv[0], __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif
#if defined(__clang_version__)
			fprintf(stderr, "%s : compiler: %s \n", argv[0], __clang_version__);
#endif
#if defined(_MSC_VER)
			fprintf(stderr, "%s : compiler: Microsoft version: %d \n", argv[0], _MSC_VER);
#endif
            ::exit(EXIT_FAILURE);
        } break;
        case 'n': {
            shuffle = false;
        } break;
        case 't': {
            textmode = true;
        } break;
        case 'h':
        default:
            help();
            ::exit(EXIT_FAILURE);
        }
    }

    bool parameter_error = false;

    if (argv[optind] == NULL) {
        fprintf(stderr,"FATAL:%s:arguments for processno not supplied\n", progname);
        parameter_error = true;
    }
    if (argv[optind + 1] == NULL) {
        fprintf(stderr,"FATAL:%s: totalprocesses not supplied\n", progname);
        parameter_error = true;
    }

    if (parameter_error == true) {
        exit(EXIT_FAILURE);
    }

    OASIS_INT pno = atoi(argv[optind]);
    OASIS_INT total = atoi(argv[optind + 1]);

#if !defined(_MSC_VER) && !defined(__MINGW32__)
    struct sigaction sa;

    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_sigaction;
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);
#endif

    try {
        initstreams("", "");
        logprintf(progname, "INFO","starting part no: %d total: %d shuffle: %d\n",  pno, total, shuffle);
        eve::emitevents(pno, total, shuffle, textmode);
        logprintf(progname, "INFO","finishing part no: %d\n",pno);
    } catch (std::bad_alloc) {
        fprintf(stderr, "FATAL:%s: Memory allocation failed\n", progname);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
