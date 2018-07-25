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

  gulitemSampleslevel q;
    char line[4096];
    int lineno=0;
  fgets(line, sizeof(line), stdin);
  lineno++;
  int gulstream_type = gulstream_id | 1;
  fwrite(&gulstream_type, sizeof(int), 1, stdout);
  fwrite(&maxsampleindex, sizeof(int), 1, stdout);
  gulSampleslevelHeader gh;
  gh.event_id = -1;
    while (fgets(line, sizeof(line), stdin) != 0)
    {
    if (sscanf(line, "%d,%d,%d,%f", &q.event_id, &q.item_id, &q.sidx, &q.loss) != 4){
           fprintf(stderr, "Invalid data in line %d:\n%s", lineno, line);
           return;
       }
      else
       {
          if (gh.event_id != q.event_id || gh.item_id != q.item_id){
            if (gh.event_id != -1){
              gulSampleslevelRec gr;
              gr.sidx =  0;
              gr.loss =  0;
              fwrite(&gr, sizeof(gr), 1, stdout);  
            }
            gh.event_id = q.event_id;
            gh.item_id = q.item_id;  
            fwrite(&gh, sizeof(gh), 1, stdout);
            gulSampleslevelRec gr;
            gr.sidx = q.sidx;
            gr.loss = q.loss;
            fwrite(&gr, sizeof(gr), 1, stdout);
          }else {
            gulSampleslevelRec gr;
            gr.sidx = q.sidx;
            gr.loss = q.loss;
            fwrite(&gr, sizeof(gr), 1, stdout);
          }
          
          
       }
       lineno++;
    }

}

void help()
{
	fprintf(stderr, 
		"-S maximum sample index\n"
		"-h help\n"
		"-v version\n");
}

int main(int argc, char* argv[])
{

	int opt;
	int maxsampleindex = -1;
	while ((opt = getopt(argc, argv, "vhS:")) != -1) {
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
		fprintf(stderr, "Sample size not supplied - please use -S parameter followed by the sample size\n");
			exit(EXIT_FAILURE);
	}
	doit(maxsampleindex);

	return EXIT_SUCCESS;

}


