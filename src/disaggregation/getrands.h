#ifndef GETRANDS_H_
#define GETRANDS_H_

#include <stdio.h>
#include <random>
#include <unordered_map>
#include "../include/oasis.h"

bool isPrime(int number);

enum rd_option {
	userandomnumberfile,		// user supplied random number file
	usehashedseed,				// use hashed seed to get random numbers so no need for random number vector
	usecachedvector				// uses cached vector for random numbers gets simular if not faster performance to file based random numbers
};


class getRands {
private:
	rd_option rndopt_;
	int rand_vec_size_;
	OASIS_FLOAT *buf_;
	int base_offset_;
	unsigned int buffersize_;
	std::mt19937 gen_;
	std::uniform_real_distribution<> dis_;
	//int _randsamplesize;
	std::vector<OASIS_FLOAT> rnd_;
	int rand_seed_;
	void userandfile();
public:
	getRands(rd_option rndopt, int rand_vec_size, int rand_seed);
	void seedRands(int rand_seed) { gen_.seed(rand_seed); }	// used for seeding via group_id and event_id
	void clearbuff() {
		delete[]buf_;
	}
	void clearvec()
	{
		rnd_.clear();
		rnd_.resize(rand_vec_size_, -1);
	}

	inline OASIS_FLOAT nextrnd() { return (OASIS_FLOAT)dis_(gen_); }	// used after seeding via group id and event_id

	inline OASIS_FLOAT rnd(unsigned int ridx) {
		switch (rndopt_) {
		case rd_option::userandomnumberfile:
		{
			if (ridx >= buffersize_) ridx = ridx - buffersize_;
			return buf_[ridx];
		}
		break;
		case rd_option::usecachedvector:
		{
			OASIS_FLOAT f = rnd_[ridx % rand_vec_size_];
			if (f < 0) {
				f = (OASIS_FLOAT)dis_(gen_);
				rnd_[ridx % rand_vec_size_] = f;
			}
			return f;
		}
		break;
		case rd_option::usehashedseed:
		{
			return (OASIS_FLOAT)dis_(gen_);
		}
		break;
		}
	}

	inline unsigned int getp1() const { return getp2(buffersize_ / 2); }
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

#endif // GETRANDS_H_

