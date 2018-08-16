#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

int agg_ap_start_ = -1;


void doit()
{

	aggregate_areaperil_to_areaperil q;
	char line[4096];
	int lineno = 0;
	fwrite(&agg_ap_start_, sizeof(agg_ap_start_), 1, stdout);
	fgets(line, sizeof(line), stdin); // skip header line
	lineno++;
	while (fgets(line, sizeof(line), stdin) != 0)
	{
#ifdef AREAPERIL_TYPE_LONG
		int ret = sscanf(line, "%ld,%ld,%f",  &q.aggregate_areaperil_id, &q.areaperil_id, &q.probability);
#else
		int ret = sscanf(line, "%d,%d,%f", &q.aggregate_areaperil_id, &q.areaperil_id, &q.probability);
#endif 

		if (ret != 3) {
			fprintf(stderr, "Invalid data in line %d:\n%s", lineno, line);
			return;
		}
		else
		{
			fwrite(&q, sizeof(q), 1, stdout);
		}
		lineno++;
	}

}

void help()
{
	fprintf(stderr, "-a minimum aggregated area peril id\n-h help\n-v version\n");
}

int main(int argc, char* argv[])
{

	int opt;

	while ((opt = getopt(argc, argv, (char *) "vha:")) != -1) {
		switch (opt) {
		case 'a':
			agg_ap_start_ = atoi(optarg);
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
	if (agg_ap_start_ == -1) {
		std::cerr << "Min aggregated area peril paramter not supplied\n";
		help();
		exit(EXIT_FAILURE);
	}
	initstreams("", "");
	doit();

	return EXIT_SUCCESS;

}