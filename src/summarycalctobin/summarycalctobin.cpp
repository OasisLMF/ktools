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

void doit(int maxsampleindex)
{

  summarySampleslevelHeader sh;
  summarySampleslevelHeader last_sh;
  sampleslevelRec sr;
  last_sh.event_id = -1;
  char line[4096];
  int lineno=0;
  fgets(line, sizeof(line), stdin);
  lineno++;
  bool firstrow=true;
  int streamtype = summarycalc_id | 1;
  fwrite(&streamtype, sizeof(int), 1, stdout);
  fwrite(&maxsampleindex, sizeof(int), 1, stdout);
  int summarystreamid = 1;
  fwrite(&summarystreamid, sizeof(int), 1, stdout);
    while (fgets(line, sizeof(line), stdin) != 0)
    {
#ifdef OASIS_FLOAT_TYPE_DOUBLE
      if (sscanf(line, "%d,%lf,%d,%d,%lf", &sh.event_id, &sh.expval, &sh.summary_id, &sr.sidx,&sr.loss) != 5){
#else
      if (sscanf(line, "%d,%f,%d,%d,%f", &sh.event_id, &sh.expval, &sh.summary_id, &sr.sidx,&sr.loss) != 5){
#endif
           fprintf(stderr, "FATAL: Invalid data in line %d:\n%s", lineno, line);
           return;
       }
      else
       {
          if (sh.event_id != last_sh.event_id  || sh.summary_id != last_sh.summary_id 
            || sh.expval != last_sh.expval) {             
            // write header
            last_sh.event_id = sh.event_id;
            last_sh.summary_id = sh.summary_id;
            last_sh.expval = sh.expval;
            if (firstrow == false){
              sampleslevelRec srx;  
              srx.sidx=0;
              srx.loss= 0;
              fwrite(&srx, sizeof(srx), 1, stdout);            
            }else{
              firstrow=false;
            }
			fwrite(&sh, sizeof(sh), 1, stdout);
          }

          fwrite(&sr, sizeof(sr), 1, stdout);
                                        
       }
       lineno++;
    }
    sampleslevelRec srx;  
   srx.sidx=0;
   srx.loss= 0;
   fwrite(&srx, sizeof(srx), 1, stdout);            
}

void help()
{
	fprintf(stderr, 
		"-S Sample size\n"  
		"-h help\n"
		"-v version\n");
}

int main(int argc, char* argv[])
{

	int opt;
	int maxsampleindex = -1;
	while ((opt = getopt(argc, argv, (char *) "vhS:")) != -1) {
		switch (opt) {
		case 'S':
			maxsampleindex = atoi(optarg);
			break;

		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;

		case 'h':
		default:
			help();
			exit(EXIT_FAILURE);
		}
	}
	initstreams();
	if (maxsampleindex == -1) {
		fprintf(stderr, "FATAL: Sample size not supplied - please use -s parameter followed by the sample size\n");
			exit(EXIT_FAILURE);
	}

	doit(maxsampleindex);

	return EXIT_SUCCESS;

}


