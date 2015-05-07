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

#pragma once
#include <stdio.h>
#include <random>
#include <map>

class getRands {
private:
	bool _fromFile;
	float *_buf;
	int _base_offset;
	unsigned int _buffersize;
	bool getNextBuffer();
	// std::random_device _rd;
	std::mt19937 _gen;
	std::uniform_real_distribution<> _dis;
	int _samplesize;
    std::map<unsigned int,float> _rnd;
public:
	getRands(bool fromFile_, int chunkid_);
	void clearbuff() {
		delete[]_buf;
	}
    void clearmap()
    {
        _rnd.clear();
    }

	inline float rnd(unsigned int ridx_)  {
		if (_fromFile) {
			if (ridx_ >= _buffersize) ridx_ = ridx_ - _buffersize;
			return _buf[ridx_];
		}
		else {
            auto iter=_rnd.find(ridx_);
            if (iter != _rnd.end()) return iter->second;
            else {
                float f = (float) _dis(_gen);
                _rnd[ridx_] = f;
                return f;
            }

		}
	}
	unsigned int getp1(bool _reconcilationmode = false) const;
	unsigned int getp2(unsigned int p1) const;
	int count() { return _buffersize; }
	int rdxmax(bool _reconcilationmode = false) const;
};
