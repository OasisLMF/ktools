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
	if (skipheader == false) printf("aggregate_areaperil_id, areaperil_id\n");

	aggregate_areaperil_to_areaperil q;
	int i = fread(&q, sizeof(q), 1, stdin);
	while (i != 0) {
#ifdef AREAPERIL_TYPE_LONG
		printf("%ld, %ld\n", q.aggregate_areaperil_id, q.areaperil_id);
#else
		printf("%d, %d\n", q.aggregate_areaperil_id, q.areaperil_id);
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