#include <iostream>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#include "disaggregation.h"


int main() {

	disaggregation disagg;
	initstreams();

	std::vector<item> items;

	disagg.doDisagg(items);

	for (int i = 0; i < items.size(); ++i) {
		item item = items[i];
		std::cerr << item  << std::endl;
	}

	return EXIT_SUCCESS;
}