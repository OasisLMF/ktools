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
Convert occurrence csv file to binary
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
int number_of_periods_ = 0;
int granular_date_ = 0;

// return number of years since epoch date
int g(int y, int m, int d)
{
	m = (m + 9) % 12;
	y = y - m / 10;
	return (365 * y + y / 4 - y / 100 + y / 400 + (m * 306 + 5) / 10 + (d - 1));
}

void d(long long g, int &y, int &mm, int &dd)
{

	//long long z = (10000L * g) + 14780L;
	//y = z / 3652425L;
	y = (10000L * g + 14780L) / 3652425L;
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


void no_occ_doit()
{
	occurrence p;
	char line[4096];
	int lineno = 0;
	fgets(line, sizeof(line), stdin);
	lineno++;
	while (fgets(line, sizeof(line), stdin) != 0)
	{
		
		if (sscanf(line, "%d,%d,%d", &p.event_id, &p.period_no, &p.occ_date_id) != 3) {
			fprintf(stderr, "FATAL: Invalid data in line %d:\n%s", lineno, line);
			return;
		}
		else
		{
			if(number_of_periods_ < p.period_no) {
				fprintf(stderr, "FATAL: Period number exceeds maximum supplied\n");
				exit(EXIT_FAILURE);
			}
			fwrite(&p, sizeof(p), 1, stdout);
		}
		lineno++;
	}
}

template<typename T>
void occ_doit(T &p)
{

	char line[4096];
	int lineno = 0;

	fgets(line, sizeof(line), stdin);
	lineno++;
	while (fgets(line, sizeof(line), stdin) != 0)
	{
		int occ_year;
		int occ_month;
		int occ_day;
		int occ_hour;
		int occ_minute;
		int occ_yearx;
		int occ_monthx;
		int occ_dayx;
		if (sscanf(line, "%d,%d,%d,%d,%d,%d,%d", &p.event_id,
			   &p.period_no, &occ_year, &occ_month, &occ_day,
			   &occ_hour, &occ_minute) != 7) {
			if (sscanf(line, "%d,%d,%d,%d,%d", &p.event_id,
				   &p.period_no, &occ_year, &occ_month,
				   &occ_day) != 5) {
				fprintf(stderr,
					"FATAL: Invalid data in line %d:\n%s",
					lineno, line);
					return;
			} else {
				occ_hour = 0;
				occ_minute = 0;
			}
		}

       		if (number_of_periods_ < p.period_no) {
       			fprintf(stderr,
				"FATAL: Period number exceeds maximum supplied\n");
       			exit(EXIT_FAILURE);
       		}

		p.occ_date_id = g(occ_year, occ_month, occ_day);

		d(p.occ_date_id, occ_yearx, occ_monthx, occ_dayx);
		if (occ_yearx != occ_year) {
			std::cerr << "FATAL: We have an error";
		}
		// Convert to minutes if applicable
		// 24 hours per day * 60 minutes per hour = 1440 minutes
		p.occ_date_id *= (1440 / (1440 - 1439 * granular_date_));
		p.occ_date_id += (60 * occ_hour + occ_minute);

		fwrite(&p, sizeof(p), 1, stdout);
	}
	lineno++;
}

void occ_doit()
{

	if (granular_date_) {
		occurrence_granular p;
		occ_doit(p);
	} else {
		occurrence p;
		occ_doit(p);
	}

}

void doit()
{
	int date_opts = granular_date_ << 1 | date_algorithm_;
	fwrite(&date_opts, sizeof(date_opts), 1, stdout);
	fwrite(&number_of_periods_, sizeof(number_of_periods_), 1, stdout);
	if (date_algorithm_)  occ_doit();
	else  no_occ_doit();
}

void help()
{
	fprintf(stderr,
		"option 1: format is event_id, period_no, occ_year, occ_month, occ_day\n"
		"option 2: format is event_id, period_no, occ_date_id\n"
		"option 3: format is event_id, period_no, occ_year, occ_month, occ_day, occ_hour, occ_minute\n"
		"-D use occ_date_id\n"
		"-H use occ_hour and occ_minute fields (set to 0 if not in csv file)\n"
		"-P number of periods\n"
		"-v version\n"
		"-h help\n"
	);
}

int main(int argc, char* argv[])
{
	int opt;
	date_algorithm_ = 1;
	granular_date_ = 0;

	while ((opt = getopt(argc, argv, "vhDHP:")) != -1) {
		switch (opt) {
		case 'D':
		{			
			date_algorithm_ = 0;
		}
		break;
		case 'H':
			granular_date_ = 1;
			break;
		case 'P':
			number_of_periods_ = atoi(optarg);
			break;
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'h':
			help();
			exit(EXIT_FAILURE);
		default:
			help();
			exit(EXIT_FAILURE);
		}
	}	
	initstreams();
	if (number_of_periods_ == 0) {
		fprintf(stderr, "FATAL: Number of periods not supplied\n");
		exit(-1);
	}
	if (!date_algorithm_ && granular_date_) {
		fprintf(stderr, "FATAL: occ_date_id not compatible with occ_hour and occ_minute fields\n");
		exit(-1);
	}
	doit();
    return EXIT_SUCCESS;
}
