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
/*
Convert occurence to csv

Author: Ben Matharu  email: ben.matharu@oasislmf.org

*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#include "../include/oasis.h"

int date_algorithm_ = 0;
int granular_date_ = 0;

void d(long long g, int &y, int &mm, int &dd)
{
	y = (10000 * g + 14780) / 3652425;
	int ddd = g - (365 * y + y / 4 - y / 100 + y / 400);
	if (ddd < 0) {
		y = y - 1;
		ddd = g - (365 * y + y / 4 - y / 100 + y / 400);
	}
	int mi = (100 * ddd + 52) / 3060;
	mm = (mi + 2) % 12 + 1;
	y = y + (mi + 2) / 12;
	dd = ddd - (mi * 306 + 5) / 10 + 1;
	return;
}

void no_occ_doit(bool skipheader)
{
	if (skipheader == false) printf("event_id,period_no,occ_date_id\n");
	occurrence q;
	int i = fread(&q, sizeof(q), 1, stdin);
	while (i != 0) {
		printf("%d, %d, %d\n", q.event_id, q.period_no, q.occ_date_id);
		i = fread(&q, sizeof(q), 1, stdin);
	}
}

template<typename T>
void occ_doit(T &q)
{

	int occ_year, occ_month, occ_day;
	int occ_hour = 0;
	int occ_minute = 0;
	int i = fread(&q, sizeof(q), 1, stdin);
	while (i != 0) {
		int days = q.occ_date_id / (1440 - 1439 * !granular_date_);
		d(days, occ_year, occ_month, occ_day);
		if (granular_date_) {
			int minutes = q.occ_date_id % 1440;
			occ_hour = minutes / 60;
			occ_minute = minutes % 60;
			printf("%d,%d,%d,%d,%d,%d,%d\n", q.event_id,
			       q.period_no, occ_year, occ_month, occ_day,
			       occ_hour, occ_minute);
		} else {
			printf("%d,%d,%d,%d,%d\n",  q.event_id, q.period_no,
			       occ_year, occ_month, occ_day);
		}
		i = fread(&q, sizeof(q), 1, stdin);
	}

}

void occ_doit()
{

	printf("event_id,period_no,occ_year,occ_month,occ_day");
	if (granular_date_) {
		printf(",occ_hour,occ_minute\n");
		occurrence_granular q;
		occ_doit(q);
	} else {
		printf("\n");
		occurrence q;
		occ_doit(q);
	}

}

void doit(bool skipheader)
{

	int no_of_periods=0;
	int date_opts;
	fread(&date_opts, sizeof(date_opts), 1, stdin);
	date_algorithm_ = date_opts & 1;
	granular_date_ = date_opts >> 1;
	if (granular_date_ && !date_algorithm_) {
		fprintf(stderr, "FATAL: Unknown date algorithm\n");
		exit(EXIT_FAILURE);
	}
	fread(&no_of_periods, sizeof(no_of_periods), 1, stdin);
	if (date_algorithm_) occ_doit();
	else no_occ_doit(skipheader);

}

void help()
{
	fprintf(stderr,
		"-s skip header\n"
		"-v version\n"
		"-h help\n"
	);
}

int main(int argc, char* argv[])
{
	int opt;
	bool skipheader = false;
	while ((opt = getopt(argc, argv, "vh")) != -1) {
		switch (opt) {
		case 's':
			skipheader = true;
			break;
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'h':
		default:
			help();
			exit(EXIT_FAILURE);
		}
	}

	initstreams();

	doit(skipheader);
	return EXIT_SUCCESS;
}
