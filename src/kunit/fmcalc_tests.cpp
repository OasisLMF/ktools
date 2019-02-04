#include "../include/catch.hpp"

#include "../fmcalc/fmcalc.h"


TEST_CASE("calc rules", "[applycalcrule]")
{
	profile_rec_new profile;
	LossRec l;
	int calcrule_id = 1;
	int layer = 1;
	profile.calcrule_id = 1;
	add_tc(deductible_1,1000, profile.tc_vec);
	add_tc(limit_1, 35000, profile.tc_vec);
	l.loss = 40000;
	applycalcrule(profile, l, layer);
	
    CHECK(l.loss == 35000.0);
    CHECK(l.effective_deductible == 1000);

}
