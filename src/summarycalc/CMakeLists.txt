add_executable(summarycalc)
target_sources(summarycalc
	PRIVATE
		summarycalc.cpp summarycalc.h main.cpp
)

if (WIN32)
	target_link_libraries(summarycalc wingetopt)
endif()
