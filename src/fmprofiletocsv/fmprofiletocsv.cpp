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
Convert fmprofile output to csv
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <getopt.h>
#endif

#ifdef __unix
    #include <unistd.h>
#endif

#include "../include/oasis.hpp"

using namespace std;


void doit()
{

    printf ("\"policytc_id\", \"calcrule_id\", \"allocrule_id\",\"ccy_id\", ");
	printf("\"deductible\", \"limits\", \"share_prop_of_lim\", \"deductible_prop_of_loss\", \"limit_prop_of_loss\", ");
	printf ("\"deductible_prop_of_tiv\", \"limit_prop_of_tiv\", \"deductible_prop_of_limit\"\n");

    fm_profile q;
    int i = fread(&q, sizeof(q), 1, stdin);
    while (i != 0) {
        printf("%d, %d, %d, %d, %f, %f, %f, %f, %f, %f, %f, %f\n",
               q.policytc_id, q.calcrule_id, q.allocrule_id, 
               q.ccy_id, q.deductible, q.limits,
               q.share_prop_of_lim, q.deductible_prop_of_loss, q.limit_prop_of_loss, q.deductible_prop_of_tiv,
               q.limit_prop_of_tiv, q.deductible_prop_of_limit);

        i = fread(&q, sizeof(q), 1, stdin);
    }
}

void help()
{

    cerr << "-I inputfilename\n"
        << "-O outputfielname\n"
        ;
}

int main(int argc, char* argv[])
{
    int opt;
     std::string inFile;
     std::string outFile;

     while ((opt = getopt(argc, argv, "hI:O:")) != -1) {
         switch (opt) {
         case 'I':
             inFile = optarg;
             break;
          case 'O':
             outFile = optarg;
             break;
         case 'h':
            help();
            exit(EXIT_FAILURE);
         }
     }

    initstreams(inFile, outFile);

	doit();
	return 0;
}
