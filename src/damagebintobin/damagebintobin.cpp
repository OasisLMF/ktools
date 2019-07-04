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
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

namespace damagebintobin {
	void doit()
	{

		damagebindictionary q;
		damagebindictionary p = {};
		bool dataValid = true;
		char line[4096];
		int lineno=0;
		fgets(line, sizeof(line), stdin);
		lineno++;
		while (fgets(line, sizeof(line), stdin) != 0)
		{
			if (sscanf(line, "%d,%f,%f,%f,%d", &q.bin_index, &q.bin_from, &q.bin_to, &q.interpolation, &q.interval_type) != 5){
			fprintf(stderr, "Invalid data in line %d:\n%s", lineno, line);
			return;
		}

			else
		{
			if (lineno == 1) {
				if (q.bin_from != 0.0) {
					fprintf(stderr, "Lower limit for first bin is not 0. Are you sure this is what you want?\n");
				}
				if (q.bin_index != 1) {
					fprintf(stderr, "First bin index must be 1.\n");
					return;
				}
			}
			if (p.bin_index != q.bin_index-1) {
				fprintf(stderr, "Non-contiguous bin indices %d line %d and %d line %d.\n", p.bin_index, lineno-1, q.bin_index, lineno);
				dataValid = false;
			}
			if (q.interpolation < q.bin_from || q.bin_to < q.interpolation) {
				fprintf(stderr, "Interpolation damage value %f for bin %d lies outside range [%f, %f].\n", q.interpolation, q.bin_index, q.bin_from, q.bin_to);
				dataValid = false;
			}
			
			fwrite(&q, sizeof(q), 1, stdout);
		}
		lineno++;
		p = q;
		}

		if (q.bin_to != 1.0) {
			fprintf(stderr, "Upper limit for last bin is not 1. Are you sure this is what you want?\n");
		}

		if (dataValid == false) return;

	}
}	
