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
Author: Ben Matharu  email : ben.matharu@oasislmf.org
*/

#include <stdio.h>

#include <iostream>
#include <fstream>
#include <sstream>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#if defined(_MSC_VER)
#include <windows.h>
#endif

#include "../include/oasis.h"

void zzSleep(int sleep_millisecs)
{
#if defined(_MSC_VER)
	Sleep(sleep_millisecs);			// windows sleep in milliseconds
#else
	usleep(sleep_millisecs * 1000);   // usleep in millionth of a second
#endif
}

bool file_exists(const char * filename)
{
	if (FILE * file = fopen(filename, "rb"))
	{
		fclose(file);
		return true;
	}
	return false;
}

bool isitreadyready(const std::string &type,int maxpid)
{
	int i = 1;
	while (i < maxpid)
	{
		std::ostringstream s;
		s << SEMA_DIR_PREFIX << "_"<< type << "/" << i << ".id";
		if (file_exists(s.str().c_str()) == false) return false;;
		i++;
	}
	return true;
}

void waitforeltcalc(int maxpid)
{
	while (isitreadyready("elt",maxpid) == false) {
		zzSleep(500);	// sleep for .5 seconds
	}
}

void waitforkatcalc(int maxpid)
{
	while (isitreadyready("elt", maxpid) == false) {
		zzSleep(500);	// sleep for .5 seconds
	}
}

void waitforaalcalc(int maxpid)
{
	while (isitreadyready("aal", maxpid) == false) {
		zzSleep(500);	// sleep for .5 seconds
	}
}


void help()
{
	fprintf(stderr,
		"-v version\n"
		"-h help\n"
		"-k kat wait\n"
		"-e elt wait\n"
		"-P maxprocessid\n"
	);
}

int main(int argc, char* argv[])
{

	int opt;
	int max_processid = 0;
	bool eltcheck = false;
	bool katcheck = false;
	int truecount = 0;
	while ((opt = getopt(argc, argv, "ekvhP:")) != -1) {
		switch (opt) {
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'e':
			truecount++;
			eltcheck = true;
			break;
		case 'k':
			truecount++;
			katcheck = true;
			break;
		case 'P':
			max_processid = atoi(optarg);
			break;
		case 'h':
		default:
			help();
			exit(EXIT_FAILURE);
		}
	}

	if (truecount > 1) {
		fprintf(stderr, "Can only use of the type check switches e or k\n");
		exit(-1);
	}

	if (max_processid && eltcheck) waitforeltcalc(max_processid);
	if (max_processid && katcheck) waitforkatcalc(max_processid);

}