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
#include <random>

#include "../include/oasis.hpp"



void genrandomnumbers(int total, int seed)
{
	std::random_device rd;
	std::uniform_real_distribution<> dis(0, 1);
	std::mt19937 mt;
	if (seed > 0) mt.seed(seed);
	else mt.seed(rd());
	for (int i = 0; i < total; i++) {
		float f = (float)dis(mt);
		printf("%f\n", f);
	}
}
void doit()
{

    float rand;
    while (fread(&rand, sizeof(rand), 1, stdin) == 1){
        printf("%f\n",rand);
    }

}
/*
Normal usage just pipe the random number binary file
with one argument of a integer - generate x number of random numbers
with two argument of a integer - generate x number of random numbers with y as the seed
*/
int main(int argc, char *argv[])
{
	initstreams("", "");
	if (argc == 1) {
		doit();
		return 0;
	}
	if (argc == 2) {
		int total = atoi(argv[1]);
		genrandomnumbers(total,0);
		return 0;
	}
    
	if (argc == 3) {
		int total = atoi(argv[1]);
		int seedval = atoi(argv[2]);
		genrandomnumbers(total,seedval);
		return 0;
	}
}
