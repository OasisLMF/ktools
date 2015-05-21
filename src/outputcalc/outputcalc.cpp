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
#include <map>
#include <vector>
#include <math.h>


#ifdef __unix
    #include <unistd.h>
#endif

using namespace std;
#include "../include/oasis.hpp"

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
struct vecrec {
    float loss;
    float tiv;
};

struct fmxref
{
    int item_id;
    int output_id;
};

void loadfmxref(std::map<int,std::vector<int> > &fmxref_)
{
    std::ostringstream oss;
    oss << "fm/fmxref.bin";

    FILE *fin = fopen(oss.str().c_str(), "rb");
    if (fin == NULL){
            cerr << "Error opening " << oss.str() << "\n";
            exit(EXIT_FAILURE);
    }
    fmxref rec;

    int i = fread(&rec, sizeof(rec), 1, fin);
    while (i != 0) {

        fmxref_[rec.output_id].push_back(rec.item_id);
        i = fread(&rec, sizeof(rec), 1, fin);
    }

    fclose(fin);

}


void loadfmtiv(std::map<int,float > &fmtiv_,std::map<int,float> &exposure_)
{
    std::map<int,std::vector<int> > fmxref;
    loadfmxref(fmxref);

    auto iter = fmxref.begin();
    while (iter != fmxref.end()){
        float tiv = 0;
        auto iter2 = iter->second.begin();
        while (iter2 != iter->second.end()){
            tiv += exposure_[*iter2];
            iter2++;
        }
        fmtiv_[iter->first] = tiv;
        iter++;
    }


}



void loadexposure(std::map<int,float> &itemtiv_)
{
    std::ostringstream oss;
    oss << "exposures.bin";

    FILE *fin = fopen(oss.str().c_str(), "rb");
    if (fin == NULL){
            cerr << "Error opening " << oss.str() << "\n";
            exit(EXIT_FAILURE);
    }
    exposure rec;

    int i = fread(&rec, sizeof(rec), 1, fin);
    while (i != 0) {
        itemtiv_[rec.item_id] = rec.tiv;
        i = fread(&rec, sizeof(rec), 1, fin);
    }

    fclose(fin);
}


void dofmsummary(std::map<output_key, std::vector<vecrec> > &output_map_, unsigned int sample_size_)
{
    auto iter = output_map_.begin();
    while (iter != output_map_.end()){
        cout << "'" << iter->first.prog_id << "_" << iter->first.layer_id << "',"
             << iter->first.event_id << ","
             ;

       auto iter2 = iter->second.begin();
       float sumloss = 0.0;
       float sumlosssqr = 0.0;
       float maxsumtiv = 0.0;
       while (iter2 != iter->second.end()){
           sumloss+= iter2->loss;
           sumlosssqr += (iter2->loss * iter2->loss);
           if (iter2->tiv > maxsumtiv) maxsumtiv = iter2->tiv;
           iter2++;
       }
// sqrt((sum(z.sumlosssqr) - (sum(z.sumloss)*sum(z.sumloss)/(@sample_size * @sample_size)))/(@sample_size -1)) AS SD

       float mean = sumloss / sample_size_;
       float sd = 0;
       if (sample_size_ > 1) {
           sd = (sumlosssqr - ((sumloss*sumloss)/sample_size_))/(sample_size_ - 1);
           if (sd < 0) sd = 0;
           sd = sqrt(sd);
       }
       cout << mean << "," << sd << "," << maxsumtiv << "\n";

       iter++;
    }
}


void dofmoutput(std::map<int,float > &fmtiv_,unsigned int sample_size_)
{
    fmlevelhdr p;
    int i = fread(&p, sizeof(fmlevelhdr), 1, stdin);
    int count=0;

    std::map<output_key, std::vector<vecrec> > output_map;

    int last_event_id = 0;

    while (i != 0) {
            fmlevelrec q;
            i = fread(&q, sizeof(fmlevelrec), 1, stdin);
            //dumpfmlevelrec(q);
            while (i != 0) {                
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
                        std::vector<vecrec> v(sample_size_+1, {0.0, 0.0});
                        output_map[k] = v;
                    }
                    output_map[k][q.sidx].loss += q.loss;
                    float tiv= fmtiv_[p.output_id];
                    output_map[k][q.sidx].tiv += tiv;
                }

                i = fread(&q, sizeof(q), 1, stdin);
                if (q.sidx == 0 ) break;
                count++;
            }
            if (i) i = fread(&p, sizeof(p), 1, stdin);
    }

    dofmsummary(output_map,sample_size_);

}
void dogulsummary(int event_id_,std::vector<vecrec> &output_vec_, unsigned int sample_size_)
{

        cout << "'ALL'" << "," << event_id_  << "," ;

       auto iter = output_vec_.begin();
       float sumloss = 0.0;
       float sumlosssqr = 0.0;
       float maxsumtiv = 0.0;
       while (iter != output_vec_.end()){
           sumloss+= iter->loss;
           sumlosssqr += (iter->loss * iter->loss);
           if (iter->tiv > maxsumtiv) maxsumtiv = iter->tiv;
           iter++;
       }
// sqrt((sum(z.sumlosssqr) - (sum(z.sumloss)*sum(z.sumloss)/(@sample_size * @sample_size)))/(@sample_size -1)) AS SD

       float mean = sumloss / sample_size_;
       float sd = 0;
       if (sample_size_ > 1) {
           sd = (sumlosssqr - ((sumloss*sumloss)/sample_size_))/(sample_size_ - 1);
           if (sd < 0) sd = 0;
           sd = sqrt(sd);
       }
       cout << mean << "," << sd << "," << maxsumtiv << "\n";

}
void doguloutput(std::map<int,float> &exposure_,unsigned int sample_size_)
{
    int count=0;
    gulSampleslevelHeader gh;
    int i = fread(&gh, sizeof(gh), 1, stdin);
    int last_event_id = 0;
    std::vector<vecrec> output_vec(sample_size_, {0.0, 0.0}) ;
    while (i != 0) {
        gulSampleslevelRec gr;
        i = fread(&gr, sizeof(gr), 1, stdin);
        while (i != 0){
            if (gr.sidx == mean_idx) gr.sidx = 0;
            if (last_event_id != gh.event_id ) { // can be made more efficent since event can only change at fmlevelhdr
                // std::cerr << "TODO: Got all event elements now do outer query event_id : " << last_event_id << "\n";
                if (last_event_id) dogulsummary(last_event_id,output_vec,sample_size_);
                last_event_id = gh.event_id;
                output_vec = std::vector<vecrec>(sample_size_+1,{0.0,0.0});
            }
            if (gr.sidx > 0 ) {
                output_vec[gr.sidx].loss += gr.gul;
                float tiv = exposure_[gh.item_id];
                output_vec[gr.sidx].tiv += tiv;
            }

            i = fread(&gr, sizeof(gr), 1, stdin);
            if (gr.sidx == 0 ) break;
            count++;
        }
        if (i) i = fread(&gh, sizeof(gh), 1, stdin);
    }
    dogulsummary(gh.event_id,output_vec,sample_size_);
}


void doit(std::map<int,float> &exposure_)
{
    unsigned int gulfmstream_type = 0;
    int i = fread(&gulfmstream_type, sizeof(gulfmstream_type), 1, stdin);

    if (isGulStream(gulfmstream_type) == true){
        int stream_type = gulfmstream_type & streamno_mask ;

        if (stream_type == 1) {
            unsigned int samplesize;
            i = fread(&samplesize, sizeof(samplesize), 1, stdin);
            if (i == 1){
                doguloutput(exposure_,samplesize);
            }else {
                 std::cerr << "Stream read error\n";
            }
            return;
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
        std::map<int,float > fmtiv;
        loadfmtiv(fmtiv,exposure_);
        unsigned int samplesize;
        i = fread(&samplesize, sizeof(samplesize), 1, stdin);
        if (i == 1){
            dofmoutput(fmtiv,samplesize);
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
    }
#endif

   initstreams(inFile, outFile);
   std::map<int,float> exposure;
   loadexposure(exposure);
   doit(exposure);
   return 0;

}

