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
Used to multiples pipes functionality of simple cat command with multiple inputs
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/
#include "../include/oasis.h"
#include <vector>
#include <algorithm>


#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#include "../include/dirent.h"
#else
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif
#include <sys/stat.h>


#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <signal.h>
#include <string.h>
#endif

namespace kat {
	void doitsort(std::vector <FILE*>& infiles);
	void doit(std::vector <FILE*>& infiles);
	void setinitdone(int processid);
} // namespace kat



char* progname = 0;


#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int, siginfo_t *si, void *)
{
	fprintf(stderr, "FATAL:%s: Segment fault at address: %p\n", progname, si->si_addr);
	exit(EXIT_FAILURE);
}
#endif


void help()
{
	fprintf(stderr,
		"-P process_id\n"
		"-d path for concatenation\n"
		"-n no sorting by event ID\n"
		"-h help\n"
		"-v version\n"
	);
}


int main(int argc, char* argv[])
{


	int opt;
	int processid = 0;
	bool sortevents = true;
	std::string path;
	while ((opt = getopt(argc, argv, "d:P:nvh")) != -1) {
		switch (opt) {
		case 'P':
			processid = atoi(optarg);
			break;
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			::exit(EXIT_FAILURE);
			break;
		case 'n':
			sortevents = false;
			break;
		case 'd':
			path = optarg;
			break;
		case 'h':
		default:
			help();
			::exit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;

	std::vector<std::string> filelist;
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
						filelist.push_back(s2);
					}
				}
			}
		}
	}

	std::vector <FILE*> infiles;
	// Sorting the string vector
	sort(filelist.begin(), filelist.end());
	auto iter = filelist.begin();
	while (iter != filelist.end()) {
		FILE* fin = fopen((*iter).c_str(), "rb");
		if (fin == nullptr) {
			fprintf(stderr, "FATAL: [kat] Cannot open %s\n", (*iter).c_str());
			exit(-1);
		}
		infiles.push_back(fin);
		iter++;
	}

	for (int i = 0; i < argc; i++) {
		FILE* fin = fopen(argv[i], "rb");
		if (fin == nullptr) {
			fprintf(stderr, "FATAL: kat: Cannot open %s\n", argv[i]);
			exit(-1);
		}
		infiles.push_back(fin);
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
		kat::setinitdone(processid);
		if (sortevents) {
			// No need to sort if there is only one input file
			if (infiles.size() == 1) kat::doit(infiles);
			else kat::doitsort(infiles);
		}
		else kat::doit(infiles);
	}
	catch (std::bad_alloc&) {
		fprintf(stderr, "FATAL: %s: Memory allocation failed\n", progname);
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
