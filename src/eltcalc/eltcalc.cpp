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
#include <fstream>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __unix
    #include <unistd.h>
#endif

using namespace std;
#include "../include/oasis.hpp"

bool isSummaryCalcStream(unsigned int stream_type)
{
	unsigned int stype = summarycalc_id & stream_type;
	return (stype == summarycalc_id);
}

void doetloutput(int samplesize)
{
	float sumloss = 0.0;
	float sample_mean = 0.0;
	float analytical_mean = 0.0;
	float sd = 0;
	float sumlosssqr = 0.0;
	printf("summary_id,type,event_id,mean,standard_deviation,exposure_value\n");
	summarySampleslevelHeader sh;
	int i = fread(&sh, sizeof(sh), 1, stdin);
	while (i != 0) {
		sampleslevelRec sr;
		i = fread(&sr, sizeof(sr), 1, stdin);
		while (i != 0) {
			if (sr.sidx > 0) {
				sumloss += sr.loss;
				sumlosssqr += (sr.loss * sr.loss);
			}
			if (sr.sidx == -1) analytical_mean = sr.loss;
			i = fread(&sr, sizeof(sr), 1, stdin);
			if (sr.sidx == 0) break;
		}		
		if (samplesize > 1) {
			sample_mean = sumloss / samplesize;
			sd = (sumlosssqr - ((sumloss*sumloss) / samplesize)) / (samplesize - 1);
			float x = sd / sumlosssqr;
			if (x < 0.0000001) sd = 0;   // fix floating point precision problems caused by using large numbers
			sd = sqrt(sd);
		}
		else {
			if (samplesize == 0) {
				sd = 0;
				sample_mean = 0;
			}
			if (samplesize == 1) {
				sample_mean = sumloss / samplesize;
				sd = 0;
			}
		}
		printf("%d,1,%d,%f,0,%f\n", sh.summary_id, sh.event_id, analytical_mean, sh.expval);
		if (samplesize) printf("%d,2,%d,%f,%f,%f\n", sh.summary_id, sh.event_id,sample_mean, sd, sh.expval);

		if (i) i = fread(&sh, sizeof(sh), 1, stdin);
		sumloss = 0.0;
		sumlosssqr = 0.0;
		sd = 0.0;
	}

}
void doit()
{
    unsigned int stream_type = 0;
    int i = fread(&stream_type, sizeof(stream_type), 1, stdin);

	if (isSummaryCalcStream(stream_type) == true) {		
		unsigned int samplesize;
		unsigned int summaryset_id;
		i = fread(&samplesize, sizeof(samplesize), 1, stdin);
		if (i == 1) i = fread(&summaryset_id, sizeof(summaryset_id), 1, stdin);
		if (i == 1) {
			doetloutput( samplesize);
		}
		else {
			std::cerr << "Stream read error\n";
		}
		return;
	}

    std::cerr << "Not a gul stream\n";
    std::cerr << "invalid stream type: " << stream_type << "\n";
    exit(-1);

}

void help()
{

    cerr << "-I inputfilename\n"
        << "-O outputfielname\n"
        ;
}


int main(int argc, char* argv[])
{

    std::string inFile;
    std::string outFile;

   initstreams(inFile, outFile);
   doit();
   return 0;

}

