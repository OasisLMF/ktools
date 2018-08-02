#include <iostream>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#include "disaggregation.h"

int main() {
	aggregate_item item;
	item.aggregate_areaperil_id = 1;
	item.aggregate_vulnerability_id = 1;
	item.areaperil_id = 0;
	item.coverage_id = 15;
	item.grouped = 0;
	item.id = 1;
	item.number_items = 1;
	item.vulnerability_id = 0;

	disaggregation disagg;
	initstreams();
	disagg.doDisagg();

	std::cerr << item;

	return EXIT_SUCCESS;
}