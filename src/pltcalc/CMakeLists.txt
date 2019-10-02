add_executable(pltcalc)
target_sources(pltcalc
	PRIVATE
		pltcalc.cpp main
)

if (WIN32)
	target_link_libraries(pltcalc wingetopt)
endif()
