/*
* Copyright (c)2015 Oasis LMF Limited
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
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/

#include <iostream>
#include <fstream>
#include <sstream>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#include "../include/oasis.hpp"
#include "summarycalc.hpp"
#include <string>

void help()
{

	fprintf(stderr,
		"Usage: \n"
		"-v version\n"
		"-h help\n"
		"-f input stream = fm\n"
		"-g input stream = gul\n"
		"-[summarysetid] outfilepipe\n"
		"where summarysetid range is 0 to 9\n"
	);
		;
}

/*

-f -1 fmsummarycalc_set1b.bin -2 -  < fmcalc_out.bin  > fmsummarycalc_set2b.bin
-g -1 summarycalc_set1a.bin -2 -  < gulcalc_coverage_out1.bin > summarycalc_set2a.bin

*/
int main(int argc, char* argv[])
{
	summarycalc f;
	int opt;
	bool noneTrue = true;
	bool inputtypegul = false;
	bool inputtypefm = false;
	while ((opt = getopt(argc, argv, "vhfg0:1:2:3:4:5:6:7:8:9:")) != -1) {
		switch (opt) {
		case 'g':
			inputtypegul = true;
			f.setgulmode();
			break;
		case 'f':
			inputtypefm = true;
			f.setfmmode();
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			f.openpipe(opt - '0', optarg);
			noneTrue = false;
			break;
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'h':
		default:
			help();
			::exit(EXIT_FAILURE);
		}
	}

	if (inputtypegul == false && inputtypefm == false) {
		std::cerr << "no input type selected\n Please use -g or -f\n";
		return -1;
	}
	if (inputtypegul == true && inputtypefm == true) {
		std::cerr << "Can't have -f and -g switches together\n";
		return -1;
	}

	if (noneTrue) return -1; // no output stream so nothing to do...
	
	f.doit();
	return 0;

}
