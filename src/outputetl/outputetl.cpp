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

#ifdef __unix
    #include <unistd.h>
#endif

#ifdef _MSC_VER
#include <fcntl.h>
#include <io.h>
#endif

using namespace std;
#include "../include/oasis.h"


bool isGulStream(unsigned int stream_type)
{
    unsigned int stype = gulstream_id & stream_type;
    return (stype == gulstream_id);
}

bool isFMStream(unsigned int stream_type)
{
    unsigned int stype = fmstream_id & stream_type;
    return (stype == fmstream_id);
}

void doit()
{
    unsigned int gulfmstream_type = 0;
    int i = fread(&gulfmstream_type, sizeof(gulfmstream_type), 1, stdin);

    if (isGulStream(gulfmstream_type) == true){
        int stream_type = gulfmstream_type & streamno_mask ;

        if (stream_type == 1) {
            while (i != 0){
                gulSampleslevelHeader gh;
                i = fread(&gh, sizeof(gh), 1, stdin);
                while (i != 0){
                    gulSampleslevelRec gr;
                    i = fread(&gr, sizeof(gr), 1, stdin);
                    if (i == 0) break;
                    if (gr.sidx == 0) break;
                    if (gr.sidx == mean_idx) gr.sidx = 0;
                    printf("%d, %d, %d, %f\n", gh.event_id, gh.item_id, gr.sidx, gr.gul);
                }
            }
        }

        if (stream_type == 2) {
            gulSampleslevel p;
            i = fread(&p, sizeof(gulSampleslevel), 1, stdin);
            while (i != 0) {
                if (p.sidx == mean_idx) p.sidx = 0;
                printf("%d, %d, %d, %f\n", p.event_id, p.item_id, p.sidx, p.gul);
                i = fread(&p, sizeof(gulSampleslevel), 1, stdin);
            }
        }

        return;
    }

    if (isFMStream(gulfmstream_type) == true){
        std::cerr << "Found FM stream\n";
        return;
    }

    std::cerr << "Not a gul stream\n";
    std::cerr << "invalid stream type: " << gulfmstream_type << "\n";
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
    int opt;
    std::string inFile;
    std::string outFile;

#ifdef __unix
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
#endif
    }

   if (inFile.length() > 0){
        if (freopen(inFile.c_str(), "rb", stdin) == NULL) {
            cerr << "Error opening " << inFile << "\n";
         }
   }else {
       freopen(NULL, "rb", stdin);
   }

   if (outFile.length() > 0){
       if (freopen(outFile.c_str(), "wb", stdout) == NULL) {
           cerr << "Error opening " << outFile << "\n";
        }
   }else{
       freopen(NULL, "wb", stdout);
   }

    doit();
    return 0;

}

