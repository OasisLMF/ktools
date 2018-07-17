
#include <stdio.h>
#include <stdlib.h>

#include "aalcalc.h"
#include "../../config.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif



void help()
{
	fprintf(stderr, "-h help\n-v version\n");
}

int main(int argc, char* argv[])
{

	int opt;
	int processid = 0;
	while ((opt = getopt(argc, argv, (char *)"vhP:")) != -1) {
		switch (opt) {
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

	aalcalc a;
	a.doit();

	return EXIT_SUCCESS;

}
