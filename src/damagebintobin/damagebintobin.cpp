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

	int doscan(char* line, int& bin_index, float& bin_from, float& bin_to, float& interpolation, int &interval_type) {
		return sscanf(line, "%d,%f,%f,%f,%d", &bin_index, &bin_from, &bin_to, &interpolation, &interval_type);
	}

	int doscan(char* line, int& bin_index, double& bin_from, double& bin_to, double& interpolation, int& interval_type) {
		return sscanf(line, "%d,%lf,%lf,%lf,%d", &bin_index, &bin_from, &bin_to, &interpolation, &interval_type);
	}
	
	void doit()
	{

		damagebindictionary q;
		char line[4096];
		int lineno=0;
		fgets(line, sizeof(line), stdin);
		lineno++;
		while (fgets(line, sizeof(line), stdin) != 0)
		{
			int ret = doscan(line, q.bin_index, q.bin_from, q.bin_to, q.interpolation, q.interval_type);
			if (ret != 5){
			fprintf(stderr, "FATAL: Invalid data in line %d:\n%s", lineno, line);
			return;
		}

			else
		{
			fwrite(&q, sizeof(q), 1, stdout);
		}
		lineno++;
		}

	}
}	
