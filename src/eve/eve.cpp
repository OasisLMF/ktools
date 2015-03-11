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
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <math.h>

#include "../include/oasis.h"

using namespace std;

void emitevents(int chunk_id_,int pno_,int total_)
{

#ifdef _MSC_VER
        _setmode(_fileno(stdout), O_BINARY);
        _setmode(_fileno(stdin), O_BINARY);
#else
        freopen(NULL, "rb", stdin);
        freopen(NULL, "wb", stdout);
#endif

    std::ostringstream oss;
    oss << "e_chunk_" << chunk_id_ << "_data.bin";
    FILE *fin = fopen(oss.str().c_str(), "rb");
    if (fin == NULL){
                cerr << "emitevents: cannot open " << oss.str().c_str() << "\n";
                exit(-1);
    }
    fseek(fin, 0L, SEEK_END);
    long endpos = ftell(fin);

    int total_events =  endpos / 4;
    int chunksize = (int) ceil((float)total_events / total_);
    int end_pos = chunksize * pno_*4;
    pno_--;
    int start_pos = chunksize * pno_*4;
    fseek(fin, start_pos, SEEK_SET);
    while(start_pos < end_pos) {
        int c = fgetc(fin);
        if (c == EOF) break;
        fputc(c,stdout);
        start_pos++;
    }

    fclose(fin);
    return;

}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        cerr << "usage: chunkid processno totalprocesses\n"
        ;
        return -1;
    }

    int chunkid = atoi(argv[1]);
    int pno = atoi(argv[2]);
    int total = atoi(argv[3]);

    emitevents(chunkid,pno,total);

    return 0;
}
