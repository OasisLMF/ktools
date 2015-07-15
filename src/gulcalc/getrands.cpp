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

#include "getrands.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "../include/oasis.hpp"


bool isPrime(int number);
std::random_device rd;
using namespace std;

getRands::getRands(bool fromFile_, int chunkid_) : _gen(rd()), _dis(0, 1)
{
	FILE *fin;
	std::ostringstream oss;
	oss << "random_" << chunkid_ << ".bin";
	_fromFile = fromFile_;
	_base_offset = 0;
	if (_fromFile) {
		fin = fopen(oss.str().c_str(), "rb");
		if (fin == NULL){
			cerr << "getRands: cannot open " << oss.str().c_str() << "\n";
			exit(-1);
		}
		flseek(fin, 0L, SEEK_END);
		long long p = fltell(fin);
		_buffersize = p / 4;
		_buffersize = _buffersize - 1;		// first 4 bytes is the limit
		fseek(fin, 0L, SEEK_SET);

        if (fread(&_randsamplesize, sizeof(int), 1, fin) != 1) {
			cerr << "Error reading random number file\n";
			exit(-1);
		}

		_buf = new float[_buffersize];
		if (fread(_buf, sizeof(float), _buffersize, fin) != _buffersize) {
			cerr << "Error reading random number file\n";
			exit(-1);
		}
		fclose(fin);
	}else {
		_rnd.resize(RND_VECTOR_SIZE,-1);
	}
}

// first prime greater that ;
unsigned int  getRands::getp1(bool _reconcilationmode) const
{
	if (_reconcilationmode == false) {
		unsigned int i = _buffersize / 2;
		return getp2(i);
	}
	else {
        unsigned int i = _buffersize / (2 * _randsamplesize);
		return getp2(i);
	}
}

// get next prime after p1
unsigned int  getRands::getp2(unsigned int p1) const
{
	int i = p1 + 1;
	while (true){
		if (isPrime(i)) return i;
		i++;
	}
}

int getRands::rdxmax(bool _reconcilationmode) const
{
	if (_reconcilationmode == false) {
		return _buffersize;
	}
	else{
        return _buffersize / _randsamplesize;
	}
}
