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
	Calculate the GUL
	Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/
#ifdef _MSC_VER
#include <windows.h>
#include <io.h>
#endif

#include <fcntl.h>
#include <map>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <memory.h>

#include "getopt.h"
#include "getrands.h"
// #include <unistd.h>

#include "../include/oasis.h"

bool verbose = false;

using namespace std;

struct gulGulSamples {
	int event_id;
	int item_id;
	float tiv;
	int bin_index;
	float prob_from;
	float prob_to;
	float bin_mean;
	int sidx;
	double rval;
};


struct exposure{
	int item_id;
	int areaperil_id;
	int vulnerability_id;
	int group_id;
	float tiv;
};
struct exposure_key{
	int areaperil_id;
	int vulnerability_id;
};

bool operator<(const exposure_key& lhs, const exposure_key& rhs)
{
	if (lhs.vulnerability_id != rhs.vulnerability_id) {
		return lhs.vulnerability_id < rhs.vulnerability_id;
	}
	else {
		return lhs.areaperil_id < rhs.areaperil_id;
	}
}


struct exposure_rec{
	int item_id;
	int group_id;
	float tiv;
};

struct prob_mean {
	float prob_to;
	float bin_mean;
};

struct prob3 {
	float prob_from;
	float prob_to;
	float bin_mean;
};

const int _gularraysize = 1000;
bool _reconcilationmode = false;
int _samplesize = 0;
int _outrec_count = 0;
double _gul_limit = 0.0;
bool _userandomtable = false;
int _chunk_id = -1;
bool _newstream = true;

gulSampleslevel *_bufold = 0;

unsigned char *_buf = 0;
int _bufsize = 0;

int _bufoffset=0;

bool getrecx(char *rec_, FILE *stream, int recsize_)
{
//    fprintf(stderr,"***** IN getrecx recsize %d ***************\n", recsize_);
    int totalread = 0;
    while (totalread != recsize_){
        int ch = getc( stream );
        if (ch == EOF) {
            // fprintf(stderr,"getrecx: GOT END OF FILE %p recsize %d\n", stream, recsize_);
            return false;
        }
        *rec_ = ch;
        totalread++;
        rec_++;
    }

	return true;

}
bool getdamagebindictionary(std::vector<damagebindictionary> &damagebindictionary_vec_)
{
	std::ostringstream oss;
	oss << "damage_bin_dict.bin";

	FILE *fin = fopen(oss.str().c_str(), "rb");
	if (fin == NULL){
		cerr << "getdamagebindictionary: Unable to open " << oss.str() << "\n";
		exit(-1);
	}
	fseek(fin, 0L, SEEK_END);
	long sz = ftell(fin);
	fseek(fin, 0L, SEEK_SET);
	unsigned int nrec = sz / sizeof(damagebindictionary);
	damagebindictionary *s1 = new damagebindictionary[nrec];
	if (fread(s1, sizeof(damagebindictionary), nrec, fin) != nrec) {
		cerr << "Error reading file\n";
		exit(-1);
	}
	damagebindictionary_vec_.clear();

	for (unsigned int i = 0; i < nrec; i++){
		damagebindictionary_vec_.push_back(s1[i]);
	}
	delete[] s1;
	if (verbose) cerr << "damagebindictionary_vec row count " << damagebindictionary_vec_.size() << endl;

	fclose(fin);
	return true;
}

bool getexposures(std::map<exposure_key, std::vector<exposure_rec> > &exposure_map_)
{
	std::ostringstream oss;
	oss << "exposures.bin";

	FILE *fin = fopen(oss.str().c_str(), "rb");
	if (fin == NULL){
		cerr << "getexposures: Unable to open " << oss.str() << "\n";
		exit(-1);
	}
	fseek(fin, 0L, SEEK_END);
	long sz = ftell(fin);
	fseek(fin, 0L, SEEK_SET);
	unsigned int nrec = sz / sizeof(exposure);
	exposure *s1 = new exposure[nrec];
	if (fread(s1, sizeof(exposure), nrec, fin) != nrec) {
		cerr << "Error reading file\n";
		exit(-1);
	}

    exposure_map_.clear();
	for (unsigned int i = 0; i < nrec; i++){
		exposure_key k;
		k.areaperil_id = s1[i].areaperil_id;
		k.vulnerability_id = s1[i].vulnerability_id;
		exposure_rec r;
		r.group_id = s1[i].group_id;
		r.item_id = s1[i].item_id;
		r.tiv = s1[i].tiv;
		exposure_map_[k].push_back(r);
		// exposure_vec_.push_back(s1[i]);
	}
	delete[] s1;
	if (verbose) cerr << "exposure_vec_ row count " << exposure_map_.size() << endl;
	fclose(fin);
	return true;
}
float getgul(damagebindictionary &b, gulGulSamples &g)
{
	float gul = 0;
	if (b.bin_from == b.bin_to) {
		gul = b.bin_to * g.tiv;
		return gul;
	}

	double x = (g.bin_mean - b.bin_from) / (b.bin_to - b.bin_from);
	int z = (int) (round(x * 100000));
	if (z == 50000) {
		gul = (b.bin_from + ((g.rval - g.prob_from) / (g.prob_to - g.prob_from) * (b.bin_to - b.bin_from))) * g.tiv;
		return gul;
	}

	double bin_width = b.bin_to - b.bin_from;
	double bin_height = g.prob_to - g.prob_from;

	double aa = (3 * bin_height / (bin_width * bin_width)) * ((2 * x) - 1);

	double bb = (2 * bin_height / bin_width) * (2 - (3* x)) ;

	double cc = g.prob_from - g.rval;
	gul = (b.bin_from + (sqrt(bb*bb - (4 * aa*cc)) - bb) / (2 * aa)) * g.tiv;
	//double gul2 = (b.bin_from - (sqrt(bb*bb - (4 * aa*cc)) - bb) / (2 * aa)) * g.tiv;
	//if (gul1 > gul2) gul = gul1;
	//else gul = gul2;

	return gul;
}

void outputgulold(gulSampleslevel &gg)
{
    // int pid = getpid();
    // fprintf(stderr,"%d: Gul::outputgul ********* BUF POINTER SET to %p!!! *******\n",pid, _buf);
	if (_bufoffset < _gularraysize) {
		_bufold[_bufoffset] = gg;
		_bufoffset++;
	}
	else {
        fwrite(_bufold, sizeof(gulSampleslevel), _gularraysize, stdout);
		_bufoffset = 0;
		_bufold[_bufoffset] = gg;
		_bufoffset++;
	}
	_outrec_count++;
}

void outputgulnewold(gulSampleslevel &gg)
{
	// int pid = getpid();
	// fprintf(stderr,"%d: Gul::outputgul ********* BUF POINTER SET to %p!!! *******\n",pid, _buf);
	// _bufoffset now refers to bytes
	if (_bufoffset >= _bufsize) {
		fwrite(_buf, sizeof(unsigned char), _bufoffset, stdout);
		_bufoffset = 0;
	}
	memcpy(_buf + _bufoffset, &gg, sizeof(gg));
	_bufoffset += sizeof(gg);
	_outrec_count++;
}

gulSampleslevelHeader _lastheader;
bool _isFirstEvent = true;

void outputgulnew(gulSampleslevel &gg)
{
	if (_bufoffset >= _bufsize) {
		fwrite(_buf, sizeof(unsigned char), _bufoffset, stdout);
		_bufoffset = 0;
	}

	if (gg.event_id != _lastheader.event_id || gg.item_id != _lastheader.item_id) {
		if (_isFirstEvent == false){
			gulSampleslevelRec r;
			r.sidx = 0;
			r.gul = 0;
			memcpy(_buf + _bufoffset, &r, sizeof(gulSampleslevelRec));
			_bufoffset += sizeof(gulSampleslevelRec);	// null terminate list
		}
		else {
			_isFirstEvent = false;
		}
		_lastheader.event_id = gg.event_id;
		_lastheader.item_id = gg.item_id;
		memcpy(_buf + _bufoffset, &_lastheader, sizeof(_lastheader));
		_bufoffset += sizeof(_lastheader);
	}

	gulSampleslevelRec r;
	r.sidx = gg.sidx;
	r.gul = gg.gul;
	memcpy(_buf + _bufoffset, &r, sizeof(gulSampleslevelRec));
	_bufoffset += sizeof(gulSampleslevelRec);

}

void outputgul(gulSampleslevel &gg)
{
	if (_newstream == false) outputgulnewold(gg);
	if (_newstream == true) outputgulnew(gg);
}

void output_mean(const exposure_rec &er, prob_mean *pp, int bin_count, float &gul_mean,  float &std_dev)
{
	float last_prob_to = 0;
	gul_mean = 0;
	std_dev = 0;
	float ctr_var = 0;

	for (int bin_index = 0; bin_index < bin_count; bin_index++){
		prob3 p;
		if (bin_index == 0) p.prob_from = 0;
		else p.prob_from = last_prob_to;
		p.prob_to = pp->prob_to;
		p.bin_mean = pp->bin_mean;
		last_prob_to = pp->prob_to;
		gul_mean = gul_mean + ((p.prob_to - p.prob_from) *p.bin_mean * er.tiv);
		ctr_var = ctr_var + ((p.prob_to - p.prob_from) *p.bin_mean*p.bin_mean * er.tiv * er.tiv);
		pp++;
	}
	float g2 = gul_mean * gul_mean;
	std_dev = ctr_var - g2;
	std_dev = sqrt(std_dev);
}

void processrec(char *rec, int recsize,
	const std::vector<damagebindictionary> &damagebindictionary_vec_,
	const std::map<exposure_key, std::vector<exposure_rec>> &exposures_map_,
	std::vector<gulSampleslevel> &event_guls_,
	getRands &rnd_)
{
damagecdfrec *d = (damagecdfrec *)rec;
	char *endofRec = rec + recsize;
	long long p1 = rnd_.getp1(_reconcilationmode);	// prime p1	make these long to force below expression to not have sign problem
	long long p2 = rnd_.getp2((unsigned int)p1);  // prime no p2
	int rnd_count = rnd_.rdxmax(_reconcilationmode);

	exposure_key k;
	k.areaperil_id = d->areaperil_id;
	k.vulnerability_id = d->vulnerabilty_id;

	auto pos = exposures_map_.find(k);
	if (pos != exposures_map_.end()){
		auto iter = pos->second.begin();
		while (iter != pos->second.end()){
            gulSampleslevel gx;
			gx.event_id = d->event_id;
			gx.item_id = iter->item_id;
			char *b = rec + sizeof(damagecdfrec);
			int *bin_count = (int *)b;
			b = b + sizeof(int);
			prob_mean *pp = (prob_mean *)b;
			float std_dev;
			float gul_mean;
			output_mean(*iter, pp, *bin_count, gul_mean, std_dev);
			gx.event_id = d->event_id;
			gx.item_id = iter->item_id;
			gx.gul = gul_mean;
			gx.sidx = mean_idx;
			outputgul(gx);
			gx.event_id = d->event_id;
			gx.item_id = iter->item_id;
			gx.gul = std_dev;
			gx.sidx = -1;
			outputgul(gx);
			int ridx = 0; // dummy value
			if (_userandomtable) ridx = ((iter->group_id * p1) + (d->event_id * p2)) % rnd_count;
			if (_reconcilationmode) ridx = ridx * _samplesize;
			for (int i = 0; i < _samplesize; i++){
				float  rval;
				rval = rnd_.rnd(ridx + i);
				float last_prob_to = 0;
				pp = (prob_mean *)b;
				for (int bin_index = 0; bin_index < *bin_count; bin_index++){
					if ((char *)pp > endofRec) {
						cerr << "Reached end of record"
							; // this is an error condition
						pp--;
					}

					prob3 p;
					if (bin_index == 0) p.prob_from = 0;
					else p.prob_from = last_prob_to;
					p.prob_to = pp->prob_to;
					p.bin_mean = pp->bin_mean;
					last_prob_to = pp->prob_to;
					if (rval < p.prob_to){
						gulGulSamples g;
						g.event_id = d->event_id;
						g.item_id = iter->item_id;
						g.tiv = iter->tiv;
						g.bin_index = bin_index;
						g.prob_from = p.prob_from;
						g.prob_to = p.prob_to;
						g.bin_mean = p.bin_mean;
						g.rval = rval;
						g.sidx = i + 1;
						gulSampleslevel gg;
						damagebindictionary b = damagebindictionary_vec_[g.bin_index];
						// gg.gul = (b.bin_from + ((g.rval - g.prob_from) / (g.prob_to - g.prob_from) * (b.bin_to - b.bin_from))) * g.tiv;
						gg.gul = getgul(b, g);
						gg.sidx = g.sidx;
						gg.event_id = g.event_id;
						gg.item_id = g.item_id;
						// if (_fm_output) event_guls_.push_back(gg);
						if (gg.gul >= _gul_limit) {
							outputgul(gg);
						}
						break; // break the for loop
					}

					pp++;
				}
			}
			iter++;
		}
	}

	// int ridx = ((d->group_id * p1) + (d->event_id * p2)) % rnd_count;
     // fprintf(stderr,"AT THE END\n");
    fflush(stderr);

}

void doitold()
{
    std::vector<damagebindictionary> damagebindictionary_vec;
    getdamagebindictionary(damagebindictionary_vec);
    std::map<exposure_key, std::vector<exposure_rec> > exposure_map;
	getexposures(exposure_map);

    int total_bins = damagebindictionary_vec.size();
	int max_recsize = (int)(total_bins * 8) + sizeof(damagecdfrec)+sizeof(int);

#ifdef _MSC_VER 
	_setmode(_fileno(stdout), O_BINARY);
	_setmode(_fileno(stdin), O_BINARY);
#endif

#ifdef __unix 
	freopen(NULL, "rb", stdin);
	freopen(NULL, "wb", stdout);
#endif
	int gulstream_type = 2;
	fwrite(&gulstream_type, sizeof(gulstream_type), 1, stdout);
    _bufold = new gulSampleslevel[_gularraysize];
    char *rec = new char[max_recsize];
    damagecdfrec *d = (damagecdfrec *)rec;
    std::vector<gulSampleslevel> event_guls;
    int last_event_id = -1;
    int stream_type = 0;
	bool bSuccess = getrecx((char *)&stream_type, stdin, sizeof(stream_type));
    if (bSuccess == false) {
		cerr << "Error: no stream type returned\n";
		return; // exit thread if failed
	}
	getRands rnd(_userandomtable, _chunk_id);

    for (;;)
	{
		//damagecdfrec c;
		char *p = rec;
		bSuccess = getrecx(p, stdin, sizeof(damagecdfrec));
		if (bSuccess == false) break;
		p = p + sizeof(damagecdfrec);
		bSuccess = getrecx(p, stdin, sizeof(int)); // we now have bin count
		int *q = (int *)p;
		p = p + sizeof(int);
		int recsize = (*q) * 8;
		// we should now have damagecdfrec in memory
		bSuccess = getrecx(p, stdin, recsize);
		recsize += sizeof(damagecdfrec)+sizeof(int);
		if (d->event_id != last_event_id) {
			//if (last_event_id != -1) dofm(event_guls);
			last_event_id = d->event_id;
			event_guls.clear();
		}


		processrec(rec, recsize, damagebindictionary_vec, exposure_map, event_guls,rnd);
	}

	fwrite(_buf, sizeof(gulSampleslevel), _bufoffset, stdout);
}


void doit()
{
	std::vector<damagebindictionary> damagebindictionary_vec;
	getdamagebindictionary(damagebindictionary_vec);
	std::map<exposure_key, std::vector<exposure_rec> > exposure_map;
	getexposures(exposure_map);

	int total_bins = damagebindictionary_vec.size();
	int max_recsize = (int)(total_bins * 8) + sizeof(damagecdfrec)+sizeof(int);


	freopen(NULL, "rb", stdin);
	freopen(NULL, "wb", stdout);

	int gulstream_type = 2 | gulstream_id;
	if (_newstream == true) {
		gulstream_type = 1 | gulstream_id;
	}
	fwrite(&gulstream_type, sizeof(gulstream_type), 1, stdout);

	// Now output the sample size
	fwrite(&_samplesize, sizeof(_samplesize), 1, stdout);

	_bufsize = sizeof(gulSampleslevel)* _gularraysize;
	_buf = new unsigned char[_bufsize + sizeof(gulSampleslevel)]; // make the allocation bigger by 1 record to avoid overrunning
	
	char *rec = new char[max_recsize];
	damagecdfrec *d = (damagecdfrec *)rec;
	std::vector<gulSampleslevel> event_guls;
	int last_event_id = -1;
	int stream_type = 0;
	bool bSuccess = getrecx((char *)&stream_type, stdin, sizeof(stream_type));
	if (bSuccess == false) {
		cerr << "Error: no stream type returned\n";
		return; // exit thread if failed
	}
	getRands rnd(_userandomtable, _chunk_id);

	for (;;)
	{
		//damagecdfrec c;
		char *p = rec;
		bSuccess = getrecx(p, stdin, sizeof(damagecdfrec));
		if (bSuccess == false) break;
		p = p + sizeof(damagecdfrec);
		bSuccess = getrecx(p, stdin, sizeof(int)); // we now have bin count
		int *q = (int *)p;
		p = p + sizeof(int);
		int recsize = (*q) * 8;
		// we should now have damagecdfrec in memory
		bSuccess = getrecx(p, stdin, recsize);
		recsize += sizeof(damagecdfrec)+sizeof(int);
		if (d->event_id != last_event_id) {
			//if (last_event_id != -1) dofm(event_guls);
			last_event_id = d->event_id;
			event_guls.clear();
		}

		processrec(rec, recsize, damagebindictionary_vec, exposure_map, event_guls, rnd);
	}

	fwrite(_buf, sizeof(unsigned char), _bufoffset, stdout);

}
	// fwrite(_buf, sizeof(unsigned char), _bufoffset, stdout);

void help()
{

    cerr << "-S Samplesize\n"
        << "-r use randomtables\n"
        << "-R reconcilation mode"
        << "-C Chunk id"
        ;
}

int main(int argc, char *argv[])
{
    int opt;
     while ((opt = getopt(argc, argv, "ORrL:S:C:")) != -1) {
        switch (opt) {
        case 'S':
			_samplesize = atoi(optarg);
			break;
         case 'R':
			_reconcilationmode = true;
			_userandomtable = true;
			break;
        case 'r':
			_userandomtable = true;
			break;
        case 'L':
			_gul_limit = atof(optarg);
			break;
	case 'C':
			_chunk_id = atoi(optarg);
			break;
	case 'O':
			_newstream=false;
			break;
        default: /* '?' */
           help();
            exit(EXIT_FAILURE);
        }
    }

    if (_samplesize == 0 ){
        fprintf(stderr,"-S sample size parameter not supplied\n");
        exit(EXIT_FAILURE);
    }

    if (_chunk_id == -1 ){
        fprintf(stderr,"-C chunk id parameter not supplied\n");
        exit(EXIT_FAILURE);
    }

     if (_chunk_id == -1 ){
        fprintf(stderr,"-C chunk id parameter not supplied\n");
        exit(EXIT_FAILURE);
    }

    //if (_newstream == false) doitold();
	doit();

}
