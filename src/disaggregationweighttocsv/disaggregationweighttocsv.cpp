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
	if (skipheader == false) printf("areaperil_id, vulnerability_id, count\n");

	Weight q;
	int i = fread(&q, sizeof(q), 1, stdin);
	while (i != 0) {
#ifdef AREAPERIL_TYPE_LONG
		printf("%ld, %d, %f\n", q.areaperil_id, q.vulnerability_id, q.count);
#else
		printf("%d, %d, %f\n", q.areaperil_id, q.vulnerability_id, q.count);
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