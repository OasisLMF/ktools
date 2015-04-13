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
#include <map>
#include <vector>
#include <math.h>


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

void dumpfmheader( fmlevelhdr &p)
{
    std::cerr << "fmlevelhdr " <<  p.event_id << " "
                 << p.prog_id << " "
                 << p.layer_id << " "
                 << p.output_id
                 << "\n"
                    ;
}

void dumpfmlevelrec(fmlevelrec &q)
{
    std::cerr << "fmlevelrec: " << q.sidx << " " << q.loss << "\n";
}

struct output_key {
    int event_id;
    int prog_id;
    int layer_id;
};

bool operator<(const output_key& lhs, const output_key& rhs)
{
        if (lhs.event_id != rhs.event_id) {
                return lhs.event_id < rhs.event_id;
        }

        if (lhs.prog_id != rhs.prog_id) {
                return lhs.prog_id < rhs.prog_id;
        }

        return lhs.layer_id < rhs.layer_id;
}
void dofmsummary(std::map<output_key, std::vector<float> > &output_map_, unsigned int sample_size_)
{
    auto iter = output_map_.begin();
    while (iter != output_map_.end()){
        cerr << iter->first.prog_id << iter->first.layer_id << ","
             << iter->first.event_id << ","
             ;

       auto iter2 = iter->second.begin();
       float sumloss = 0.0;
       float sumlosssqr = 0.0;
       while (iter2 != iter->second.end()){
           sumloss+=*iter2;
           sumlosssqr += (*iter2 * *iter2);
           iter2++;
       }
// sqrt((sum(z.sumlosssqr) - (sum(z.sumloss)*sum(z.sumloss)/(@sample_size * @sample_size)))/(@sample_size -1)) AS SD

       float mean = sumloss / sample_size_;
       float sd = sqrt((sumlosssqr - ((sumloss*sumloss)/(sample_size_*sample_size_)))/(sample_size_ - 1));

       cerr << mean << ","
       << sd << "\n";

       iter++;
    }
}

void dofmoutput(unsigned int sample_size_)
{
   std::cerr << "TODO samples: " << sample_size_ << "\n";
    fmlevelhdr p;
    int i = fread(&p, sizeof(fmlevelhdr), 1, stdin);
    int top = 7250;
    int count=0;
    dumpfmheader(p);

    std::map<output_key, std::vector<float> > output_map;

    int last_event_id = 0;

    while (i != 0) {
            fmlevelrec q;
            i = fread(&q, sizeof(fmlevelrec), 1, stdin);
            //dumpfmlevelrec(q);
            while (i != 0) {
                if (q.sidx == 0) break;
                if (q.sidx == mean_idx) q.sidx = 0;
              // if (count < top) fprintf(stderr, "%d, %d, %d, %d, %d, %f\n", p.prog_id, p.layer_id, p.event_id, p.output_id, q.sidx, q.loss);
               // if (count < top && q.sidx > 0 ) fprintf(stderr, "%d%d | %d | %d | %d | %f | %f\n", p.prog_id, p.layer_id, p.event_id, p.output_id, q.sidx, q.loss,sum_loss);
                if (last_event_id != p.event_id ) { // can be made more efficent since event can only change at fmlevelhdr
                    // std::cerr << "TODO: Got all event elements now do outer query event_id : " << last_event_id << "\n";
                    last_event_id = p.event_id;
                    dofmsummary(output_map,sample_size_);
                    output_map.clear();
                }
                if (q.sidx > 0 ) {
                    output_key k;
                    k.event_id = p.event_id;
                    k.layer_id = p.layer_id;
                    k.prog_id = p.prog_id;
                    auto pos = output_map.find(k);
                    if (pos == output_map.end()){
                        std::vector<float> v(sample_size_, 0.0);
                        output_map[k] = v;
                    }
                    output_map[k][q.sidx] += q.loss;
                }

                i = fread(&q, sizeof(fmlevelrec), 1, stdin);
                count++;
            }
            if (i) i = fread(&p, sizeof(fmlevelhdr), 1, stdin);
    }

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
        unsigned int samplesize;
        i = fread(&samplesize, sizeof(samplesize), 1, stdin);
        if (i == 1){
            dofmoutput(samplesize);
        }else {
             std::cerr << "Stream read error\n";
        }
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

