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

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>

#include "../include/oasis.hpp"

std::vector<int> return_periods;
int lastreturnperiod = -1;
void writereturnperiods()
{
	std::sort(return_periods.rbegin(), return_periods.rend());

	for (int return_period : return_periods) {
		if (lastreturnperiod != return_period) {
			fwrite(&return_period, sizeof(return_period), 1, stdout);
			lastreturnperiod = return_period;
		}
	}
}

void doit()
{
    int return_period;
    char line[4096];
    int lineno=0;
	fgets(line, sizeof(line), stdin);	// skip first line
    while (fgets(line, sizeof(line), stdin) != 0)
    {
       if (sscanf(line, "%d", &return_period) != 1){
           fprintf(stderr, "Invalid data in line %d:\n%s", lineno, line);
           return;
       }
       else
       {           
		   return_periods.push_back(return_period);
       }
       lineno++;
    }
	writereturnperiods();
}


int main()
{
	initstreams("", "");
    doit();
    return 0;
}
