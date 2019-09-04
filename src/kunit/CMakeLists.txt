add_executable(kunit)
target_sources(kunit
	PRIVATE
		fmcalc_tests.cpp main.cpp ../fmcalc/fmcalcrules.cpp
)

add_test(kunit kunit)
