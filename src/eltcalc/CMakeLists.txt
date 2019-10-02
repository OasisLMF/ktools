add_executable(eltcalc)
target_sources(eltcalc
	PRIVATE
		eltcalc.cpp main.cpp
)
if (WIN32)
	target_link_libraries(eltcalc wingetopt)
endif()
