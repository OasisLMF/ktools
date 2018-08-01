#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

long areaperils_ = -1;


void doit()
{

	aggregate_areaperil_to_areaperil q;
	char line[4096];
	int lineno = 0;
	fwrite(&areaperils_, sizeof(areaperils_), 1, stdout);
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
			if (areaperils_ < q.areaperil_id) {
				fprintf(stderr, "Max area peril id specifed %d is less than encountered in data %d\n", areaperils_, q.areaperil_id);
			}
			fwrite(&q, sizeof(q), 1, stdout);
		}
		lineno++;
	}

}

void help()
{
	fprintf(stderr, "-a maximum area peril id\n-h help\n-v version\n");
}

int main(int argc, char* argv[])
{

	int opt;

	while ((opt = getopt(argc, argv, (char *) "vha:")) != -1) {
		switch (opt) {
		case 'a':
			areaperils_ = atoi(optarg);
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
	if (areaperils_ == -1) {
		std::cerr << "Area peril paramter not supplied\n";
		help();
		exit(EXIT_FAILURE);
	}
	initstreams("", "");
	doit();

	return EXIT_SUCCESS;

}