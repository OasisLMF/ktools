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
Convert item output to csv
Author: Joh Carter  email: johanna.carter@oasislmf.org
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#include "../include/oasis.h"

using namespace std;

void doit(bool skipheader)
{

	if (skipheader == false) printf("item_id,coverage_id,areaperil_id,vulnerability_id,group_id\n");

	item q;
	size_t i = fread(&q, sizeof(q), 1, stdin);
	while (i != 0) {
#ifdef AREAPERIL_TYPE_UNSIGNED_LONG_LONG
		printf("%d, %d, %llu, %d, %d\n", q.id, q.coverage_id, q.areaperil_id, q.vulnerability_id, q.group_id);
#else
		printf("%d, %d, %u, %d, %d\n", q.id, q.coverage_id, q.areaperil_id, q.vulnerability_id, q.group_id);
#endif // 		
		i = fread(&q, sizeof(q), 1, stdin);
	}
}

void help()
{
	fprintf(stderr,
		"-s skip header\n"
		"-v version\n"
		"-h help\n"
	);
}

int main(int argc, char* argv[])
{
	int opt;
	bool skipheader = false;
	while ((opt = getopt(argc, argv, "vhs")) != -1) {
		switch (opt) {
		case 's':
			skipheader = true;
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

	initstreams();
	doit(skipheader);
	return EXIT_SUCCESS;
}
