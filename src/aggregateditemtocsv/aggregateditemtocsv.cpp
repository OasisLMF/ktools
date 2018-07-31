#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#include "../include/oasis.h"

using namespace std;

void doit(bool skipheader)
{

	if (skipheader == false) printf("item_id, coverage_id, aggregate_areaperil_id, aggregate_vulnerability_id,areaperil_id,vulnerability_id,number_items, grouped\n");

	aggregated_item q;
	int i = fread(&q, sizeof(q), 1, stdin);
	while (i != 0) {
#ifdef AREAPERIL_TYPE_LONG
		printf("%d, %d, %ld, %d, %ld, %d, %d, %d\n", q.id, q.coverage_id, q.aggregate_areaperil_id, q.aggregate_vulnerability_id, q.areaperil_id, q.vulnerability_id, q.number_items, q.grouped);
#else
		printf("%d, %d, %d, %d, %d, %d, %d, %d\n", q.id, q.coverage_id, q.aggregate_areaperil_id, q.aggregate_vulnerability_id, q.areaperil_id, q.vulnerability_id, q.number_items, q.grouped);
#endif // 		
		i = fread(&q, sizeof(q), 1, stdin);
	}
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
	while ((opt = getopt(argc, argv, (char *) "vhs")) != -1) {
		switch (opt) {
		case 's':
			skipheader = true;
			break;
		case 'v':
			fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
			exit(EXIT_FAILURE);
			break;
		case 'h':
			help();
			exit(EXIT_FAILURE);
		}
	}

	initstreams();
	doit(skipheader);
	return EXIT_SUCCESS;
}