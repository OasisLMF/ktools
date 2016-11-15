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

#pragma once
#include <stdio.h>
#include <random>
#include <unordered_map>

bool isPrime(int number);

class getRands {
private:
	bool fromFile_;
	int rand_vec_size_;
	float *buf_;
	int base_offset_;
	unsigned int buffersize_;
	std::mt19937 gen_;
	std::uniform_real_distribution<> dis_;
    //int _randsamplesize;
    std::vector<float> rnd_;
	int rand_seed_;
public:
	getRands(bool fromFile,int rand_vec_size, int rand_seed);
	void clearbuff() {
		delete[]buf_;
	}
    void clearvec()
    {
        rnd_.clear();
        rnd_.resize(rand_vec_size_,-1);
    }

	inline float rnd(unsigned int ridx)  {
		if (fromFile_) {
			if (ridx >= buffersize_) ridx = ridx - buffersize_;
			return buf_[ridx];
		}
		else {
			float f = rnd_ [ridx % rand_vec_size_] ;
			if ( f < 0 ) {
				f = (float) dis_(gen_);
				rnd_ [ridx % rand_vec_size_] = f;
			}
			return f;

		}
	}

	inline unsigned int getp1() const {  return getp2(buffersize_ / 2); }
	inline unsigned int getp2(unsigned int p1) const { // get next prime after p1
		int i = p1 + 1;
		while (true) {
			if (isPrime(i)) return i;
			i++;
		}
	}
	int count() { return buffersize_; }
	inline int rdxmax() const { return buffersize_; }
};
